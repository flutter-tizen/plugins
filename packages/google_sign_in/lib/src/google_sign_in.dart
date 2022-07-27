// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:flutter/services.dart';
import 'package:google_sign_in_platform_interface/google_sign_in_platform_interface.dart';

import 'device_flow_widget.dart';
import 'oauth2.dart';
import 'utils.dart' as utils;

/// The parameters to use when initializing the Google sign in process for Tizen.
class SignInInitParametersTizen extends SignInInitParameters {
  /// Creates an instance of [SignInInitParametersTizen].
  const SignInInitParametersTizen({
    required String clientId,
    required this.clientSecret,
    super.scopes,
  }) : super(clientId: clientId);

  @override
  String get clientId => super.clientId!;

  /// The secret credential known only to the application and the authorization
  /// server, it's analogous to a password.
  final String clientSecret;
}

/// Holds authentication data after Google sign in for Tizen.
class GoogleSignInTokenDataTizen extends GoogleSignInTokenData {
  /// Creates an instance of [GoogleSignInTokenDataTizen].
  GoogleSignInTokenDataTizen({
    required super.accessToken,
    required this.accessTokenExpirationDate,
    required super.idToken,
    this.refreshToken,
  });

  @override
  String get accessToken => super.accessToken!;

  /// The estimated expiration date of [accessToken].
  final DateTime accessTokenExpirationDate;

  @override
  String get idToken => super.idToken!;

  /// The OAuth2 refresh token to exchange for new access tokens.
  final String? refreshToken;

  /// Returns `true` if [accessToken] is expired and needs to be refreshed,
  /// otherwise `false`.
  bool get isExpired {
    const Duration minimalTimeToExpire = Duration(minutes: 1);
    return accessTokenExpirationDate
        .add(minimalTimeToExpire)
        .isAfter(DateTime.now());
  }
}

/// The class that handles Google SignIn.
class GoogleSignIn {
  /// The current token data.
  GoogleSignInTokenDataTizen? _tokenData;

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
    if (_tokenData != null) {
      return _createUserData(_tokenData!.idToken);
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

    _tokenData = GoogleSignInTokenDataTizen(
      accessToken: tokenResponse.accessToken,
      accessTokenExpirationDate: DateTime.now().add(tokenResponse.expiresIn),
      refreshToken: tokenResponse.refreshToken,
      idToken: tokenResponse.idToken,
    );

    return _createUserData(_tokenData!.idToken);
  }

  /// Returns the [tokenData] of the currently signed in user, returns
  /// `null` if no user is signed in.
  ///
  /// If [refresh] is `true`, expired tokens will be refreshed after
  /// getting new tokens from the server. [initParameters] cannot be `null` when
  /// refresh is required.
  Future<GoogleSignInTokenDataTizen?> getAuthentication({
    bool refresh = false,
    SignInInitParametersTizen? initParameters,
  }) async {
    if (_tokenData == null) {
      return null;
    }
    final GoogleSignInTokenDataTizen tokenData = _tokenData!;

    // Check if access token expired.
    if (!refresh || !tokenData.isExpired) {
      return tokenData;
    }

    if (initParameters == null) {
      throw ArgumentError(
          "Token expired: `initParameters` can't be null when refresh is required",
          'initParameters');
    }

    if (tokenData.refreshToken == null) {
      throw PlatformException(
        code: 'refresh-token-missing',
        message: 'Cannot refresh tokens as refresh tokens are missing. '
            'Request new tokens by signing-in again.',
      );
    }

    final TokenResponse tokenResponse = await _authClient.refreshToken(
      clientId: initParameters.clientId,
      clientSecret: initParameters.clientSecret,
      refreshToken: tokenData.refreshToken!,
    );

    _tokenData = GoogleSignInTokenDataTizen(
      accessToken: tokenResponse.accessToken,
      accessTokenExpirationDate: DateTime.now().add(tokenResponse.expiresIn),
      refreshToken: tokenResponse.refreshToken,
      idToken: tokenResponse.idToken,
    );

    return _tokenData;
  }

  /// Signs out the currently signed in user.
  Future<void> signOut() async {
    if (_tokenData != null) {
      _tokenData = null;
    }
  }

  /// Signs out the currently signed in user and revoke its authentication.
  Future<void> disconnect() async {
    if (_tokenData == null) {
      return;
    }

    final String accessToken = _tokenData!.accessToken;
    signOut();

    _authClient.revokeToken(accessToken);
  }

  /// Returns `true` if there is a signed in user, otherwise returns `false`.
  Future<bool> isSignedIn() async => _tokenData != null;

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
