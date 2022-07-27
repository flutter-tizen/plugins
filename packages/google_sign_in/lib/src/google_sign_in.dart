// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:flutter/services.dart';
import 'package:google_sign_in_platform_interface/google_sign_in_platform_interface.dart';

import 'device_flow_widget.dart';
import 'oauth2.dart';
import 'utils.dart' as utils;

/// The parameters to use when initializing the sign in process for Tizen.
class SignInInitParametersTizen extends SignInInitParameters {
  /// Creates an instance of [SignInInitParametersTizen].
  const SignInInitParametersTizen({
    required String clientId,
    required this.clientSecret,
    super.scopes,
  }) : super(clientId: clientId);

  @override
  String get clientId => super.clientId!;

  /// The secret credential knwon only to the application and the authorization
  /// server. It's analogous to a password.
  final String clientSecret;
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
  Future<GoogleSignInUserData?> signIn(
      SignInInitParametersTizen initParameters) async {
    if (_authentication != null) {
      return _createUserData(_authentication!.idToken);
    }

    final AuthorizationResponse authorizationResponse =
        await _authClient.requestAuthorization(
      initParameters.clientId,
      initParameters.scopes,
    );

    final Future<TokenResponse?> tokenResponseFuture = _authClient.pollToken(
      clientId: initParameters.clientId,
      clientSecret: initParameters.clientSecret,
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
      clientId: initParameters.clientId,
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
    final Map<String, dynamic> json = utils.decodeJWT(idToken);

    utils
        .checkFormat<String>(<String>['email', 'sub', 'name', 'picture'], json);

    return GoogleSignInUserData(
      email: json['email'] as String,
      id: json['sub'] as String,
      displayName: json['name'] as String?,
      idToken: idToken,
      photoUrl: json['picture'] as String?,
    );
  }
}
