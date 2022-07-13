// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:convert' as convert;

import 'package:flutter/widgets.dart';
import 'package:http/http.dart' as http;

import 'authorization_exception.dart';
import 'usercode_display_widget.dart';

final Uri _authorizationEndPoint =
    Uri.parse('https://oauth2.googleapis.com/device/code');
final Uri _tokenEndPoint = Uri.parse('https://oauth2.googleapis.com/token');
final Uri _revokeEndPoint = Uri.parse('htpps://oauth2.googleapis.com/revoke');

/// The class that represents a Google user account.
class GoogleUser {
  /// Creates an instance of [GoogleUser].
  GoogleUser({
    this.userId,
    this.profile,
    required this.authentication,
    List<String>? scope,
  }) : grantedScope = scope ?? <String>[];

  /// The Google user ID.
  final String? userId;

  /// The basic profile data for the user.
  final ProfileData? profile;

  /// The authentication info for the user.
  final Authentication authentication;

  /// The API scopes granted to the app.
  final List<String> grantedScope;
}

/// The class that represents OAuth 2.0 entities needed for sign-in.
class Authentication {
  /// Creates an instance of [Authentication].
  const Authentication({
    required this.clientId,
    required this.accessToken,
    required this.accessTokenExpirationDate,
    this.refreshToken,
    this.idToken,
    this.idTokenExpirationDate,
  });

  /// The client ID associated with the authentication.
  final String clientId;

  /// The OAuth2 access token to access Google services.
  final String accessToken;

  /// The estimated expiration date of [accessToken].
  final DateTime accessTokenExpirationDate;

  /// The OAuth2 refresh token to exchange for new access tokens.
  final String? refreshToken;

  /// The OpenID Connect ID token that identifies the user.
  final String? idToken;

  /// The estimated expiration date of [idToken].
  final DateTime? idTokenExpirationDate;
}

/// The class that represents basic profile information.
class ProfileData {
  /// Creates an instance of [ProfileData].
  const ProfileData({
    required this.email,
    required this.name,
    this.givenName,
    this.familyName,
  });

  /// The Google user's email.
  final String email;

  /// The Google user's full name.
  final String name;

  /// The Google user's given name.
  final String? givenName;

  /// The Google user's family name.
  final String? familyName;

  /// Creates a [ProfileData] from json object.
  static ProfileData fromJson(Map<String, dynamic> json) {
    _checkFormat<String>(
      <String>['email', 'name', 'given_name', 'family_name'],
      json,
    );

    final String? givenName =
        json['given_name'] != null ? json['given_name'] as String : null;

    final String? familyName =
        json['family_name'] != null ? json['family_name'] as String : null;

    return ProfileData(
      email: json['email'] as String,
      name: json['name'] as String,
      givenName: givenName,
      familyName: familyName,
    );
  }
}

/// The class that represents successful device authorization response.
/// See: https://datatracker.ietf.org/doc/html/rfc8628#section-3.2
class _AuthorizationResponse {
  _AuthorizationResponse._({
    required this.deviceCode,
    required this.userCode,
    required this.verificationUrl,
    required this.expiresIn,
    Duration? interval,
  }) : interval = interval ?? const Duration(seconds: 5);

  static _AuthorizationResponse fromJson(Map<String, dynamic> json) {
    _checkFormat<String>(
      <String>['device_code', 'user_code', 'verification_url'],
      json,
    );
    _checkFormat<int>(
      <String>['expires_in', 'interval'],
      json,
    );

    final Duration? interval = json['interval'] != null
        ? Duration(seconds: json['interval'] as int)
        : null;

    return _AuthorizationResponse._(
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
class _TokenResponse {
  _TokenResponse._({
    required this.accessToken,
    required this.tokenType,
    required this.expiresIn,
    this.refreshToken,
    List<String>? scope,
    required this.idToken,
  }) : scope = scope ?? <String>[];

  static _TokenResponse fromJson(Map<String, dynamic> json) {
    _checkFormat<String>(
      <String>[
        'access_token',
        'refresh_token',
        'scope',
        'token_type',
        'id_token',
      ],
      json,
    );
    _checkFormat<int>(
      <String>['expires_in'],
      json,
    );

    final String? refreshToken =
        json['refresh_token'] != null ? json['refresh_token'] as String : null;
    final List<String>? scope = json['scope'] != null
        ? (json['scope'] as String).split(' ').toList()
        : null;

    return _TokenResponse._(
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

/// The class that handles OAuth 2.0 Device Authorization Grant flow for Google SignIn.
///
/// See:
///  - [Google SignIn guide for limited input devices](https://developers.google.com/identity/gsi/web/guides/devices)
///  - [Google OAuth 2.0 guide for limited input devices](https://developers.google.com/identity/protocols/oauth2/limited-input-device)
///  - [OAuth 2.0 Device Authorization Grant spec](https://datatracker.ietf.org/doc/html/rfc8628)
///  - [OpenID Connect spec](https://openid.net/specs/openid-connect-core-1_0.html)
class GoogleSignIn {
  GlobalKey<NavigatorState> navigatorKey = GlobalKey<NavigatorState>();

  List<String> _scopes = <String>[];

  late String _clientId;

  /// The currently signed in user.
  GoogleUser? _user;

  /// Returns the currently signed in [GoogleUser], returns `null` if no user is
  /// signed in.
  GoogleUser? get user => _user;

  /// Returns the [Authentication] object of the currently signed in user, returns
  /// `null` if no user is signed in.
  Authentication? get authentication => _user?.authentication;

  /// Initializes the instance with [clientId] and [scopes].
  ///
  /// This method should be called at least once before calling [signIn].
  Future<void> init(String clientId, List<String> scopes) async {
    _clientId = clientId;
    _scopes = scopes;
  }

  /// Starts the interactive sign-in flow.
  ///
  /// Returns the currently signed in user if already signed in, `null` if
  /// sign-in was cancelled.
  Future<GoogleUser?> signIn() async {
    if (_clientId == null) {
      throw Exception('GoogleSignIn instance is not initialized.');
    }

    if (_user != null) {
      return _user;
    }

    final _AuthorizationResponse authorizationResponse =
        await _requestUserCode();

    // Polls for user response.
    final Completer<void> completer = Completer<void>();
    _TokenResponse? tokenResponse;
    final Timer timer =
        Timer.periodic(authorizationResponse.interval, (Timer timer) async {
      try {
        tokenResponse =
            await _requestDeviceCodeInput(authorizationResponse.deviceCode);
        timer.cancel();
        closeUserCodeDialog(navigatorKey);
        completer.complete();
      } on AuthorizationException catch (e) {
        // The authorization request is still pending as the end user hasn't
        // yet completed the user-interaction steps.
        if (e.error != 'authorization_pending') {
          timer.cancel();
          completer.complete();
          rethrow;
        }
      }
    });

    displayUserCodeDialog(
      authorizationResponse.userCode,
      authorizationResponse.verificationUrl,
      navigatorKey,
      () {
        timer.cancel();
        completer.complete();
      },
    );

    // Waits until user inputs 'user code' or cancels sign-in flow.
    await completer.future;
    if (tokenResponse == null) {
      return null;
    }
    final _TokenResponse response = tokenResponse!;
    final Map<String, dynamic> jsonProfile = _decodeJWT(response.idToken);
    final ProfileData profile = ProfileData.fromJson(jsonProfile);

    final Authentication authentication = Authentication(
      clientId: _clientId,
      accessToken: response.accessToken,
      accessTokenExpirationDate: DateTime.now().add(response.expiresIn),
      refreshToken: response.refreshToken,
      idToken: response.idToken,
    );

    _checkFormat<String>(<String>['sub'], jsonProfile);
    _user = GoogleUser(
      userId: jsonProfile['sub'] as String,
      profile: profile,
      authentication: authentication,
      scope: response.scope,
    );

    return _user;
  }

  /// Signs out the currently signed in user.
  Future<void> signOut() async {
    if (_user != null) {
      _user = null;
    }
  }

  /// Signs out the currently signed in user and revoke its authentication.
  Future<void> disconnect() async {
    if (_user == null) {
      return;
    }

    final Map<String, String> body = <String, String>{
      'revoke': _user!.authentication.accessToken,
    };
    signOut();

    final http.Response response = await http.post(_revokeEndPoint, body: body);
    final Map<String, dynamic> jsonResponse =
        convert.jsonDecode(response.body) as Map<String, dynamic>;
    if (response.statusCode != 200) {
      _handleErrorResponse(jsonResponse);
    }
  }

  /// Returns `true` if there is a signed in user, otherwise returns `false`.
  Future<bool> isSignedIn() async => _user != null;

  Future<_AuthorizationResponse> _requestUserCode() async {
    final Map<String, String> body = <String, String>{
      'client_id': _clientId,
    };
    body['scope'] = _scopes.join(' ');

    final http.Response response =
        await http.post(_authorizationEndPoint, body: body);
    final Map<String, dynamic> jsonResponse =
        convert.jsonDecode(response.body) as Map<String, dynamic>;
    if (response.statusCode != 200) {
      _handleErrorResponse(jsonResponse);
    }

    return _AuthorizationResponse.fromJson(jsonResponse);
  }

  Future<_TokenResponse> _requestDeviceCodeInput(String deviceCode) async {
    final Map<String, String> body = <String, String>{
      'grant_type': 'http://oauth.net/grant_type/device/1.0',
      'client_id': _clientId,
      'client_secret': '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',
      'code': deviceCode,
    };

    final http.Response response = await http.post(_tokenEndPoint, body: body);
    final Map<String, dynamic> jsonResponse =
        convert.jsonDecode(response.body) as Map<String, dynamic>;

    if (response.statusCode != 200) {
      _handleErrorResponse(jsonResponse);
    }

    return _TokenResponse.fromJson(jsonResponse);
  }

  void _handleErrorResponse(Map<String, dynamic> jsonResponse) {
    _checkFormat<String>(
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

  Map<String, dynamic> _decodeJWT(String token) {
    final List<String> splitTokens = token.split('.');
    if (splitTokens.length != 3) {
      throw const FormatException('Invalid token.');
    }
    final String normalizedPayload = convert.base64.normalize(splitTokens[1]);
    final String payloadString =
        convert.utf8.decode(convert.base64.decode(normalizedPayload));
    return convert.jsonDecode(payloadString) as Map<String, dynamic>;
  }
}

void _checkFormat<T>(List<String> names, Map<String, dynamic> parameters) {
  for (final String name in names) {
    final dynamic value = parameters[name];
    if (value != null && value is! T) {
      throw FormatException('parameter "$name" is not a $T, is "$value".');
    }
  }
}
