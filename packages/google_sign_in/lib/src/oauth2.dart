import 'dart:convert' as convert;

import 'package:flutter/services.dart';
import 'package:http/http.dart' as http;

import 'authorization_exception.dart';
import 'utils.dart' as utils;

/// The class that represents successful device authorization response.
/// See: https://datatracker.ietf.org/doc/html/rfc8628#section-3.2
class AuthorizationResponse {
  AuthorizationResponse._({
    required this.deviceCode,
    required this.userCode,
    required this.verificationUrl,
    required this.expiresIn,
    Duration? interval,
  }) : interval = interval ?? const Duration(seconds: 5);

  /// Creates a [AuthorizationResponse] from a json object.
  static AuthorizationResponse fromJson(Map<String, dynamic> json) {
    utils.checkFormat<String>(
      <String>['device_code', 'user_code', 'verification_url'],
      json,
    );
    utils.checkFormat<int>(
      <String>['expires_in', 'interval'],
      json,
    );

    final Duration? interval = json['interval'] != null
        ? Duration(seconds: json['interval'] as int)
        : null;

    return AuthorizationResponse._(
      deviceCode: json['device_code'] as String,
      userCode: json['user_code'] as String,
      verificationUrl: Uri.parse(json['verification_url'] as String),
      expiresIn: Duration(seconds: json['expires_in'] as int),
      interval: interval,
    );
  }

  /// The device verification code.
  final String deviceCode;

  /// The end-user verification code.
  final String userCode;

  /// The end-user verification URI on the authorization server.
  final Uri verificationUrl;

  /// The lifetime of the [deviceCode] and [userCode].
  final Duration expiresIn;

  /// The minimum amount of time that the client should wait between polling
  /// requests to the token endpoint.
  final Duration interval;
}

/// The class that represents successful token response.
/// See:
///  - [OAuth2 spec](https://datatracker.ietf.org/doc/html/rfc6749#section-5.1)
///  - [OpenID Connect spec](https://openid.net/specs/openid-connect-core-1_0.html#TokenResponse)
class TokenResponse {
  TokenResponse._({
    required this.accessToken,
    required this.tokenType,
    required this.expiresIn,
    this.refreshToken,
    List<String>? scope,
    required this.idToken,
  }) : scope = scope ?? <String>[];

  /// Creates a [TokenResponse] from a json object.
  static TokenResponse fromJson(Map<String, dynamic> json) {
    utils.checkFormat<String>(
      <String>[
        'access_token',
        'refresh_token',
        'scope',
        'token_type',
        'id_token',
      ],
      json,
    );
    utils.checkFormat<int>(
      <String>['expires_in'],
      json,
    );

    final String? refreshToken =
        json['refresh_token'] != null ? json['refresh_token'] as String : null;
    final List<String>? scope = json['scope'] != null
        ? (json['scope'] as String).split(' ').toList()
        : null;

    return TokenResponse._(
      accessToken: json['access_token'] as String,
      tokenType: json['token_type'] as String,
      expiresIn: Duration(seconds: json['expires_in'] as int),
      refreshToken: refreshToken,
      scope: scope,
      idToken: json['id_token'] as String,
    );
  }

  /// The access token issued by the authorization server.
  final String accessToken;

  /// The type of the token issued.
  ///
  /// In the context of Google sign-in, this type is always 'Bearer'.
  final String tokenType;

  /// The lifetime of the [accessToken].
  final Duration expiresIn;

  /// The token that can be used to obtain new access tokens when they are expired.
  final String? refreshToken;

  /// The API scope of the access token.
  final List<String> scope;

  /// The token issued by the Google authorization server that holds Google
  /// account information.
  final String idToken;
}

class DeviceAuthClient {
  /// Creates an instance of [DeviceAuthClient].
  DeviceAuthClient({
    required this.authorizationEndPoint,
    required this.tokenEndPoint,
    required this.revokeEndPoint,
    http.Client? httpClient,
  }) : _httpClient = httpClient ?? http.Client();

  final Uri authorizationEndPoint;

  final Uri tokenEndPoint;

  final Uri revokeEndPoint;

  final http.Client _httpClient;

  bool _isPolling = false;

  bool get isPolling => _isPolling;

  void cancelPollToken() => _isPolling = false;

  Future<AuthorizationResponse> requestAuthorization(
      String clientId, List<String> scope) async {
    final Map<String, String> body = <String, String>{
      'client_id': clientId,
    };
    body['scope'] = scope.join(' ');

    final http.Response response =
        await _httpClient.post(authorizationEndPoint, body: body);

    final Map<String, dynamic> jsonResponse =
        convert.jsonDecode(response.body) as Map<String, dynamic>;
    if (response.statusCode != 200) {
      _handleErrorResponse(jsonResponse);
    }

    return AuthorizationResponse.fromJson(jsonResponse);
  }

  Future<TokenResponse> requestToken(
    String clientId,
    String clientSecret,
    String deviceCode,
  ) async {
    final Map<String, String> body = <String, String>{
      'grant_type': 'http://oauth.net/grant_type/device/1.0',
      'client_id': clientId,
      'client_secret': clientSecret,
      'code': deviceCode,
    };

    final http.Response response =
        await _httpClient.post(tokenEndPoint, body: body);
    final Map<String, dynamic> jsonResponse =
        convert.jsonDecode(response.body) as Map<String, dynamic>;

    if (response.statusCode != 200) {
      _handleErrorResponse(jsonResponse);
    }

    return TokenResponse.fromJson(jsonResponse);
  }

  Future<TokenResponse?> pollToken({
    required String clientId,
    required String clientSecret,
    required String deviceCode,
    required Duration interval,
  }) async {
    if (isPolling) {
      throw PlatformException(
        code: 'already-polling',
        message: 'Client is already polling token from server, cancel the '
            'previous poll request before starting a new one.',
      );
    }
    _isPolling = true;
    Duration currentInterval = Duration(seconds: interval.inSeconds);
    while (isPolling) {
      try {
        final TokenResponse tokenResponse = await Future<TokenResponse>.delayed(
          currentInterval,
          () => requestToken(
            clientId,
            clientSecret,
            deviceCode,
          ),
        );
        _isPolling = false;
        return tokenResponse;
      } on AuthorizationException catch (e) {
        // Subsequent requests MUST be increased by 5 seconds.
        // See: https://datatracker.ietf.org/doc/html/rfc8628#section-3.5.
        if (e.error == 'slow_down') {
          currentInterval = currentInterval + const Duration(seconds: 5);
        }
        // The authorization request is still pending as the end user hasn't
        // yet completed the user-interaction steps.
        else if (e.error != 'authorization_pending') {
          _isPolling = false;
          rethrow;
        }
      }
    }
    return null;
  }

  Future<void> revokeToken(String token) async {
    final Map<String, String> body = <String, String>{
      'token': token,
    };

    final http.Response response =
        await _httpClient.post(revokeEndPoint, body: body);
    final Map<String, dynamic> jsonResponse =
        convert.jsonDecode(response.body) as Map<String, dynamic>;
    if (response.statusCode != 200) {
      _handleErrorResponse(jsonResponse);
    }
  }

  Future<TokenResponse> refreshToken({
    required String clientId,
    required String clientSecret,
    required String refreshToken,
  }) async {
    final Map<String, String> body = <String, String>{
      'client_id': clientId,
      'client_secret': clientSecret,
      'refresh_token': refreshToken,
      'grant_type': 'refresh_token',
    };

    final http.Response response =
        await _httpClient.post(tokenEndPoint, body: body);
    final Map<String, dynamic> jsonResponse =
        convert.jsonDecode(response.body) as Map<String, dynamic>;

    if (response.statusCode != 200) {
      _handleErrorResponse(jsonResponse);
    }

    return TokenResponse.fromJson(jsonResponse);
  }

  void _handleErrorResponse(Map<String, dynamic> jsonResponse) {
    utils.checkFormat<String>(
      <String>['error', 'error_description', 'error_uri'],
      jsonResponse,
    );
    final String error = jsonResponse['error'] as String;
    final String? description = jsonResponse['error_description'] != null
        ? jsonResponse['error_description'] as String
        : null;
    final String? uriString = jsonResponse['error_uri'] != null
        ? jsonResponse['error_uri'] as String
        : null;
    final Uri? uri = uriString == null ? null : Uri.parse(uriString);
    throw AuthorizationException(error, description, uri);
  }
}
