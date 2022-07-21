// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:flutter/services.dart';
import 'package:google_sign_in_tizen/src/oauth2.dart';
import 'package:http/http.dart' as http;

import 'device_flow_widget.dart';
import 'utils.dart' as utils;

/// The class that represents a Google user account.
class GoogleUser {
  /// Creates an instance of [GoogleUser].
  GoogleUser({
    required this.userId,
    required this.profile,
    required this.authentication,
    List<String>? scope,
  }) : grantedScope = scope ?? <String>[];

  /// The Google user ID.
  final String userId;

  /// The basic profile data for the user.
  final ProfileData profile;

  /// The authentication info for the user.
  final Authentication authentication;

  /// The API scopes granted to the app.
  final List<String> grantedScope;
}

/// The class that holds parameters for OAuth request.
class Configuration {
  /// Creates an instance of [Configuration].
  const Configuration({
    required this.clientId,
    required this.clientSecret,
    this.scope = const <String>[],
  });

  /// The unique public identifier for apps that is issued by the authorization
  /// server. It's analogous to a login id.
  final String clientId;

  /// The secret credential knwon only to the application and the authorization
  /// server. It's analogous to a password.
  final String clientSecret;

  /// The amount of resources to access on behalf of the user.
  final List<String> scope;
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
    this.picture,
  });

  /// The Google user's email.
  final String email;

  /// The Google user's full name.
  final String name;

  /// The Google user's given name.
  final String? givenName;

  /// The Google user's family name.
  final String? familyName;

  /// The Uri of Google user's profile picture.
  final Uri? picture;

  /// Creates a [ProfileData] from json object.
  static ProfileData fromJson(Map<String, dynamic> json) {
    print(json);

    utils.checkFormat<String>(
      <String>['email', 'name', 'given_name', 'family_name', 'picture'],
      json,
    );

    final String? givenName =
        json['given_name'] != null ? json['given_name'] as String : null;

    final String? familyName =
        json['family_name'] != null ? json['family_name'] as String : null;

    final Uri? picture =
        json['picture'] != null ? Uri.parse(json['picture'] as String) : null;

    return ProfileData(
      email: json['email'] as String,
      name: json['name'] as String,
      givenName: givenName,
      familyName: familyName,
      picture: picture,
    );
  }
}

/// The class that handles OAuth 2.0 Device Authorization Grant flow for Google SignIn.
///
/// See:
///  - [Google SignIn guide for limited input devices](https://developers.google.com/identity/gsi/web/guides/devices)
///  - [Google OAuth 2.0 guide for limited input devices](https://developers.google.com/identity/protocols/oauth2/limited-input-device)
///  - [OAuth 2.0 Device Authorization Grant spec](https://datatracker.ietf.org/doc/html/rfc8628)
///  - [OpenID Connect spec](https://openid.net/specs/openid-connect-core-1_0.html)
class GoogleSignIn {
  /// The currently signed in user.
  GoogleUser? _user;

  /// Returns the currently signed in [GoogleUser], returns `null` if no user is
  /// signed in.
  GoogleUser? get user => _user;

  final DeviceAuthClient _authClient = DeviceAuthClient(
    authorizationEndPoint:
        Uri.parse('https://oauth2.googleapis.com/device/code'),
    tokenEndPoint: Uri.parse('https://oauth2.googleapis.com/token'),
    revokeEndPoint: Uri.parse('https://oauth2.googleapis.com/revoke'),
  );

  /// Starts the interactive sign-in flow.
  ///
  /// Returns the currently signed in user if already signed in, `null` if
  /// sign-in was cancelled.
  Future<GoogleUser?> signIn(Configuration configuration) async {
    if (_user != null) {
      return _user;
    }

    final AuthorizationResponse authorizationResponse =
        await _authClient.requestAuthorization(
      configuration.clientId,
      configuration.scope,
    );

    final Future<TokenResponse?> tokenResponseFuture = _authClient.pollToken(
      clientId: configuration.clientId,
      clientSecret: configuration.clientSecret,
      deviceCode: authorizationResponse.deviceCode,
      interval: authorizationResponse.interval,
    );

    showDeviceFlowWidget(
      code: authorizationResponse.userCode,
      verificationUrl: authorizationResponse.verificationUrl,
      expiresIn: authorizationResponse.expiresIn,
      onExpired: () => _authClient.cancelPollToken(),
      onCancelled: () => _authClient.cancelPollToken(),
    );

    // Waits until user interaction on secondary device is finished, or until
    // code is expired or polling is cancelled.
    final TokenResponse? tokenResponse = await tokenResponseFuture;
    if (tokenResponse == null) {
      return null;
    }
    closeDeviceFlowWidget();

    final Map<String, dynamic> jsonProfile =
        utils.decodeJWT(tokenResponse.idToken);
    final ProfileData profile = ProfileData.fromJson(jsonProfile);

    final Authentication authentication = Authentication(
      clientId: configuration.clientId,
      accessToken: tokenResponse.accessToken,
      accessTokenExpirationDate: DateTime.now().add(tokenResponse.expiresIn),
      refreshToken: tokenResponse.refreshToken,
      idToken: tokenResponse.idToken,
    );

    utils.checkFormat<String>(<String>['sub'], jsonProfile);
    _user = GoogleUser(
      userId: jsonProfile['sub'] as String,
      profile: profile,
      authentication: authentication,
      scope: tokenResponse.scope,
    );

    return _user;
  }

  /// Returns the [Authentication] of the currently signed in user, returns
  /// `null` if no user is signed in.
  ///
  /// If [refresh] is `true`, expired tokens will be refreshed after
  /// getting new tokens from the server. [clientSecret] cannot be `null` when
  /// refresh is required.
  Future<Authentication?> getAuthentication({
    bool refresh = false,
    String? clientSecret,
  }) async {
    if (_user == null) {
      return null;
    }
    final GoogleUser user = _user!;
    final Authentication authentication = user.authentication;

    // Check access token is if expired.
    const Duration minimalTimeToExpire = Duration(minutes: 1);
    if (!refresh ||
        authentication.accessTokenExpirationDate
            .add(minimalTimeToExpire)
            .isBefore(DateTime.now())) {
      return authentication;
    }

    if (clientSecret == null) {
      throw ArgumentError(
          "Token expired: `clientSecret` can't be null when refresh is required",
          'clientSecret');
    }

    if (authentication.refreshToken == null) {
      throw PlatformException(
        code: 'refresh-token-missing',
        message: 'Cannot refresh tokens as refresh tokens are missing. '
            'Request new tokens by signing-in again.',
      );
    }

    final TokenResponse tokenResponse = await _authClient.refreshToken(
      clientId: authentication.clientId,
      clientSecret: clientSecret,
      refreshToken: authentication.refreshToken!,
    );

    final Authentication newAuthentication = Authentication(
      clientId: authentication.clientId,
      accessToken: tokenResponse.accessToken,
      accessTokenExpirationDate: DateTime.now().add(tokenResponse.expiresIn),
      refreshToken: tokenResponse.refreshToken,
      idToken: tokenResponse.idToken,
    );

    _user = GoogleUser(
      userId: user.userId,
      profile: user.profile,
      authentication: newAuthentication,
      scope: tokenResponse.scope,
    );
    return newAuthentication;
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

    final String accessToken = _user!.authentication.accessToken;
    signOut();

    _authClient.revokeToken(accessToken);
  }

  /// Returns `true` if there is a signed in user, otherwise returns `false`.
  Future<bool> isSignedIn() async => _user != null;
}
