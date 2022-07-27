// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:flutter/services.dart';
import 'package:google_sign_in_platform_interface/google_sign_in_platform_interface.dart';

import 'device_flow_widget.dart';
import 'oauth2.dart';
import 'utils.dart' as utils;

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

/// The class that represents OAuth 2.0 entities after sign-in.
class Authentication {
  /// Creates an instance of [Authentication].
  const Authentication({
    required this.clientId,
    required this.accessToken,
    required this.accessTokenExpirationDate,
    required this.idToken,
    this.refreshToken,
    this.idTokenExpirationDate,
  });

  /// The client ID associated with the authentication.
  final String clientId;

  /// The OAuth2 access token to access Google services.
  final String accessToken;

  /// The estimated expiration date of [accessToken].
  final DateTime accessTokenExpirationDate;

  /// The OpenID Connect ID token that identifies the user.
  final String idToken;

  /// The OAuth2 refresh token to exchange for new access tokens.
  final String? refreshToken;

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

    return ProfileData(
      email: json['email'] as String,
      name: json['name'] as String,
      givenName: json['given_name'] as String?,
      familyName: json['family_name'] as String?,
      picture:
          json['picture'] != null ? Uri.parse(json['picture'] as String) : null,
    );
  }
}

/// The class that handles Google SignIn.
class GoogleSignIn {
  /// The current authentication.
  Authentication? _authentication;

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
  Future<GoogleSignInUserData?> signIn(Configuration configuration) async {
    if (_authentication != null) {
      return _createUserData(_authentication!.idToken);
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

    _authentication = Authentication(
      clientId: configuration.clientId,
      accessToken: tokenResponse.accessToken,
      accessTokenExpirationDate: DateTime.now().add(tokenResponse.expiresIn),
      refreshToken: tokenResponse.refreshToken,
      idToken: tokenResponse.idToken,
    );

    return _createUserData(_authentication!.idToken);
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
    if (_authentication == null) {
      return null;
    }
    final Authentication authentication = _authentication!;

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

    _authentication = Authentication(
      clientId: authentication.clientId,
      accessToken: tokenResponse.accessToken,
      accessTokenExpirationDate: DateTime.now().add(tokenResponse.expiresIn),
      refreshToken: tokenResponse.refreshToken,
      idToken: tokenResponse.idToken,
    );

    return _authentication;
  }

  /// Signs out the currently signed in user.
  Future<void> signOut() async {
    if (_authentication != null) {
      _authentication = null;
    }
  }

  /// Signs out the currently signed in user and revoke its authentication.
  Future<void> disconnect() async {
    if (_authentication == null) {
      return;
    }

    final String accessToken = _authentication!.accessToken;
    signOut();

    _authClient.revokeToken(accessToken);
  }

  /// Returns `true` if there is a signed in user, otherwise returns `false`.
  Future<bool> isSignedIn() async => _authentication != null;

  GoogleSignInUserData _createUserData(String idToken) {
    final Map<String, dynamic> jsonProfile = utils.decodeJWT(idToken);
    final ProfileData profile = ProfileData.fromJson(jsonProfile);

    utils.checkFormat<String>(<String>['sub'], jsonProfile);

    return GoogleSignInUserData(
      email: profile.email,
      id: jsonProfile['sub'] as String,
      displayName: profile.name,
      idToken: idToken,
      photoUrl: profile.picture.toString(),
    );
  }
}
