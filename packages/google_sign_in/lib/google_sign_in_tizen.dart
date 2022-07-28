// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:convert' as convert;

import 'package:flutter/services.dart';
import 'package:google_sign_in_platform_interface/google_sign_in_platform_interface.dart';

import 'src/device_flow_widget.dart' as device_flow_widget;
import 'src/oauth2.dart';

export 'src/authorization_exception.dart';
export 'src/device_flow_widget.dart' show navigatorKey;

/// Sets [clientId] and [clientSecret] to be used for GoogleSignIn authentication.
///
/// This must be called before calling the GoogleSignIn's signIn API.
void setCredentials({
  required String clientId,
  required String clientSecret,
}) {
  (GoogleSignInPlatform.instance as GoogleSignInTizen)
      ._setCredentials(clientId: clientId, clientSecret: clientSecret);
}

/// The parameters to use when initializing the Google sign in process for Tizen.
class _SignInInitParametersTizen extends SignInInitParameters {
  /// Creates an instance of [_SignInInitParametersTizen].
  const _SignInInitParametersTizen({
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
class _GoogleSignInTokenDataTizen extends GoogleSignInTokenData {
  /// Creates an instance of [_GoogleSignInTokenDataTizen].
  _GoogleSignInTokenDataTizen({
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

/// Tizen implementation of [GoogleSignInPlatform].
class GoogleSignInTizen extends GoogleSignInPlatform {
  /// Registers this class as the default instance of [GoogleSignInPlatform].
  static void register() {
    GoogleSignInPlatform.instance = GoogleSignInTizen();
  }

  _SignInInitParametersTizen? _initParameters;

  /// The current token data.
  _GoogleSignInTokenDataTizen? _tokenData;

  final DeviceAuthClient _authClient = DeviceAuthClient(
    authorizationEndPoint:
        Uri.parse('https://oauth2.googleapis.com/device/code'),
    tokenEndPoint: Uri.parse('https://oauth2.googleapis.com/token'),
    revokeEndPoint: Uri.parse('https://oauth2.googleapis.com/revoke'),
  );

  void _setCredentials({
    required String clientId,
    required String clientSecret,
  }) {
    _initParameters = _SignInInitParametersTizen(
        clientId: clientId, clientSecret: clientSecret);
  }

  void _ensureInitParametersInitialized() {
    if (_initParameters == null) {
      throw PlatformException(
        code: 'credentials-missing',
        message: 'Cannot initialize GoogleSignInTizen: ClientID and '
            'ClientSecret has not been set, first call `setCredentials` '
            "in google_sign_in_tizen.dart before calling GoogleSignIn's signIn API.",
      );
    }
  }

  void _ensureNavigatorKeyInitialized() {
    if (device_flow_widget.navigatorKey.currentContext == null) {
      throw PlatformException(
        code: 'navigatorkey-unassigned',
        message: 'Cannot initialize GoogleSignInTizen: a default or custom '
            'navigator key must be assigned to `navigatorKey` parameter in '
            '`MaterialApp` or `CupertinoApp`.',
      );
    }
  }

  @override
  Future<void> init({
    List<String> scopes = const <String>[],
    SignInOption signInOption = SignInOption.standard,
    String? hostedDomain,
    String? clientId,
  }) async {
    if (signInOption == SignInOption.games) {
      throw PlatformException(
        code: 'unsupported-options',
        message: 'Games sign in is not supported on Tizen.',
      );
    }

    if (clientId != null) {
      throw PlatformException(
        code: 'invalid-parameter',
        message:
            'ClientID cannot be set when initializing GoogleSignIn for Tizen, '
            'instead call `setCredentials` in google_sign_in_tizen.dart.',
      );
    }

    _ensureInitParametersInitialized();
    _initParameters = _SignInInitParametersTizen(
      clientId: _initParameters!.clientId,
      clientSecret: _initParameters!.clientSecret,
      scopes: scopes,
    );
  }

  @override
  Future<GoogleSignInUserData?> signInSilently() {
    throw UnimplementedError('signInSilently() has not been implemented.');
  }

  @override
  Future<GoogleSignInUserData?> signIn() async {
    _ensureInitParametersInitialized();
    _ensureNavigatorKeyInitialized();

    if (_tokenData != null) {
      return _createUserData(_tokenData!.idToken);
    }

    final AuthorizationResponse authorizationResponse =
        await _authClient.requestAuthorization(
      _initParameters!.clientId,
      _initParameters!.scopes,
    );

    final Future<TokenResponse?> tokenResponseFuture = _authClient.pollToken(
      clientId: _initParameters!.clientId,
      clientSecret: _initParameters!.clientSecret,
      deviceCode: authorizationResponse.deviceCode,
      interval: authorizationResponse.interval,
    );

    device_flow_widget.showDeviceFlowWidget(
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
    device_flow_widget.closeDeviceFlowWidget();

    _tokenData = _GoogleSignInTokenDataTizen(
      accessToken: tokenResponse.accessToken,
      accessTokenExpirationDate: DateTime.now().add(tokenResponse.expiresIn),
      refreshToken: tokenResponse.refreshToken,
      idToken: tokenResponse.idToken,
    );

    return _createUserData(_tokenData!.idToken);
  }

  @override
  Future<GoogleSignInTokenData> getTokens(
      {required String email, bool? shouldRecoverAuth = true}) async {
    if (_tokenData == null) {
      throw PlatformException(
          code: 'not-signed-in',
          message: 'Cannot get tokens as there are no signed in user.');
    }
    final _GoogleSignInTokenDataTizen tokenData = _tokenData!;

    // Check if access token expired.
    if (!tokenData.isExpired) {
      return tokenData;
    }

    _ensureInitParametersInitialized();

    if (tokenData.refreshToken == null) {
      throw PlatformException(
        code: 'refresh-token-missing',
        message: 'Cannot refresh tokens as refresh tokens are missing. '
            'Request new tokens by signing-in again.',
      );
    }

    final TokenResponse tokenResponse = await _authClient.refreshToken(
      clientId: _initParameters!.clientId,
      clientSecret: _initParameters!.clientSecret,
      refreshToken: tokenData.refreshToken!,
    );

    _tokenData = _GoogleSignInTokenDataTizen(
      accessToken: tokenResponse.accessToken,
      accessTokenExpirationDate: DateTime.now().add(tokenResponse.expiresIn),
      refreshToken: tokenResponse.refreshToken,
      idToken: tokenResponse.idToken,
    );

    return _tokenData!;
  }

  @override
  Future<void> signOut() async {
    if (_tokenData != null) {
      _tokenData = null;
    }
  }

  @override
  Future<void> disconnect() async {
    if (_tokenData == null) {
      return;
    }

    final String accessToken = _tokenData!.accessToken;
    await signOut();

    _authClient.revokeToken(accessToken);
  }

  @override
  Future<bool> isSignedIn() async => _tokenData != null;

  @override
  Future<void> clearAuthCache({String? token}) {
    throw UnimplementedError('clearAuthCache() has not been implemented.');
  }

  @override
  Future<bool> requestScopes(List<String> scopes) {
    throw UnimplementedError('requestScopes() has not been implemented.');
  }

  GoogleSignInUserData _createUserData(String idToken) {
    // Decodes JWT payload as a json object.
    final List<String> splitTokens = idToken.split('.');
    if (splitTokens.length != 3) {
      throw const FormatException('Invalid idToken.');
    }
    final String normalizedPayload = convert.base64.normalize(splitTokens[1]);
    final String payloadString =
        convert.utf8.decode(convert.base64.decode(normalizedPayload));
    final Map<String, dynamic> json =
        convert.jsonDecode(payloadString) as Map<String, dynamic>;

    return GoogleSignInUserData(
      email: json['email'] as String,
      id: json['sub'] as String,
      displayName: json['name'] as String?,
      idToken: idToken,
      photoUrl: json['picture'] as String?,
    );
  }
}
