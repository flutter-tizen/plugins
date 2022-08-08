// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:convert' as convert;
import 'dart:io';

import 'package:http/http.dart' as http;

import 'authorization_exception.dart';

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
  static AuthorizationResponse fromJson(Map<String, Object?> json) {
    return AuthorizationResponse._(
      deviceCode: json['device_code']! as String,
      userCode: json['user_code']! as String,
      verificationUrl: Uri.parse(json['verification_url']! as String),
      expiresIn: Duration(seconds: json['expires_in']! as int),
      interval: json['interval'] is int
          ? Duration(seconds: json['interval']! as int)
          : null,
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
  static TokenResponse fromJson(Map<String, Object?> json) {
    return TokenResponse._(
      accessToken: json['access_token']! as String,
      tokenType: json['token_type']! as String,
      expiresIn: Duration(seconds: json['expires_in']! as int),
      refreshToken: json['refresh_token'] as String?,
      scope: json['scope'] is String
          ? (json['scope']! as String).split(' ').toList()
          : null,
      idToken: json['id_token']! as String,
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

/// OAuth 2.0 client that handles Device Authorization Grant.
/// See:
///  - [Google SignIn guide for limited input devices](https://developers.google.com/identity/gsi/web/guides/devices)
///  - [Google OAuth 2.0 guide for limited input devices](https://developers.google.com/identity/protocols/oauth2/limited-input-device)
///  - [OAuth 2.0 Device Authorization Grant spec](https://datatracker.ietf.org/doc/html/rfc8628)
///  - [OpenID Connect spec](https://openid.net/specs/openid-connect-core-1_0.html)
class DeviceAuthClient {
  /// Creates an instance of [DeviceAuthClient].
  DeviceAuthClient({
    required this.authorizationEndPoint,
    required this.tokenEndPoint,
    required this.revokeEndPoint,
    http.Client? httpClient,
  }) : _httpClient = httpClient ?? http.Client();

  /// The server endpoint that returns authroization grant when user grants access.
  final Uri authorizationEndPoint;

  /// The server endpoint that returns access token when given authorization grant.
  final Uri tokenEndPoint;

  /// The server endpoint that revokes future access to issued tokens.
  final Uri revokeEndPoint;

  final http.Client _httpClient;

  bool _isPolling = false;

  /// Checks whether polling started by by [pollToken] is in progress.
  bool get isPolling => _isPolling;

  /// Stops poll request started by [pollToken].
  void cancelPollToken() => _isPolling = false;

  /// Requests authroization grant from [authorizationEndPoint].
  Future<AuthorizationResponse> requestAuthorization(
      String clientId, List<String> scope) async {
    final Map<String, String> body = <String, String>{
      'client_id': clientId,
      'scope': scope.join(' '),
    };

    final http.Response response =
        await _httpClient.post(authorizationEndPoint, body: body);

    if (response.statusCode != 200) {
      _handleErrorResponse(response);
    }

    return AuthorizationResponse.fromJson(
        convert.jsonDecode(response.body) as Map<String, Object?>);
  }

  /// Requests tokens from [tokenEndPoint].
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

    if (response.statusCode != 200) {
      _handleErrorResponse(response);
    }

    return TokenResponse.fromJson(
        convert.jsonDecode(response.body) as Map<String, Object?>);
  }

  /// Repeat sending token request to [tokenEndPoint] until user grants access.
  Future<TokenResponse?> pollToken({
    required String clientId,
    required String clientSecret,
    required String deviceCode,
    required Duration interval,
  }) async {
    if (isPolling) {
      throw StateError(
        'Client is already polling token from server, cancel the '
        'previous poll request before starting a new one.',
      );
    }
    _isPolling = true;
    while (isPolling) {
      try {
        final TokenResponse tokenResponse = await Future<TokenResponse>.delayed(
          interval,
          () => requestToken(clientId, clientSecret, deviceCode),
        );
        _isPolling = false;
        return tokenResponse;
      } on AuthorizationException catch (e) {
        // Subsequent requests MUST be increased by 5 seconds.
        // See: https://datatracker.ietf.org/doc/html/rfc8628#section-3.5.
        if (e.error == 'slow_down') {
          interval = interval + const Duration(seconds: 5);
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

  /// Requests a revoke token request to [revokeEndPoint].
  Future<void> revokeToken(String token) async {
    final Map<String, String> body = <String, String>{
      'token': token,
    };

    final http.Response response =
        await _httpClient.post(revokeEndPoint, body: body);
    if (response.statusCode != 200) {
      _handleErrorResponse(response);
    }
  }

  /// Requests a refresh token request to [tokenEndPoint].
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

    if (response.statusCode != 200) {
      _handleErrorResponse(response);
    }
    return TokenResponse.fromJson(
        convert.jsonDecode(response.body) as Map<String, Object?>);
  }

  void _handleErrorResponse(http.Response response) {
    // Google Token endpoint returns status code 428 for 'authroization_pending'
    // response which is not specified in the spec: https://datatracker.ietf.org/doc/html/rfc8628#section-3.5.
    if (response.statusCode == 400 ||
        response.statusCode == 401 ||
        response.statusCode == 428) {
      final Map<String, Object?> json =
          convert.jsonDecode(response.body) as Map<String, Object?>;
      throw AuthorizationException(
        json['error']! as String,
        json['error_description']! as String?,
        json['error_uri'] is String
            ? Uri.parse(json['error_uri']! as String)
            : null,
      );
    } else {
      throw HttpException(
          'Status code: ${response.statusCode}, ${response.reasonPhrase}.');
    }
  }
}
