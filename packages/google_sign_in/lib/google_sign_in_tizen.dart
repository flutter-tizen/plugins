// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:convert';

import 'package:flutter/widgets.dart';
import 'package:flutter_secure_storage/flutter_secure_storage.dart';
import 'package:google_sign_in_platform_interface/google_sign_in_platform_interface.dart';

import 'src/authorization_exception.dart';
import 'src/device_flow_widget.dart' as device_flow_widget;
import 'src/oauth2.dart';

export 'src/authorization_exception.dart';

const List<String> _authenticationScopes = <String>[
  'openid',
  'email',
  'profile',
];

/// Holds authentication data after Google sign in for Tizen.
class _GoogleSignInTokenDataTizen {
  /// Creates an instance of [_GoogleSignInTokenDataTizen].
  _GoogleSignInTokenDataTizen({
    required this.accessToken,
    required this.accessTokenExpirationDate,
    required this.idToken,
    required Iterable<String> grantedScopes,
    this.refreshToken,
  }) : grantedScopes = grantedScopes.map(_normalizeScope).toSet();

  /// The OAuth2 access token used to access Google services.
  final String accessToken;

  /// The estimated expiration date of [accessToken].
  final DateTime accessTokenExpirationDate;

  /// The OpenID Connect ID token that identifies the user.
  final String idToken;

  /// The OAuth2 refresh token to exchange for new access tokens.
  final String? refreshToken;

  /// The scopes granted to [accessToken].
  final Set<String> grantedScopes;

  /// Returns `true` if [accessToken] is expired and needs to be refreshed.
  bool get isExpired {
    const Duration minimalTimeToExpire = Duration(minutes: 1);
    return DateTime.now()
        .add(minimalTimeToExpire)
        .isAfter(accessTokenExpirationDate);
  }

  /// Returns `true` if all [scopes] are granted by [accessToken].
  bool grantsScopes(List<String> scopes) {
    return scopes
        .map(_normalizeScope)
        .every((String scope) => grantedScopes.contains(scope));
  }

  /// Returns a copy whose access token will be refreshed on next use.
  _GoogleSignInTokenDataTizen withInvalidatedAccessToken() {
    return _GoogleSignInTokenDataTizen(
      accessToken: accessToken,
      accessTokenExpirationDate: DateTime.fromMillisecondsSinceEpoch(0),
      idToken: idToken,
      refreshToken: refreshToken,
      grantedScopes: grantedScopes,
    );
  }

  /// Creates a [_GoogleSignInTokenDataTizen] from a json object.
  static _GoogleSignInTokenDataTizen fromJson(Map<String, Object?> json) {
    final Object? grantedScopesJson = json['granted_scopes'];
    return _GoogleSignInTokenDataTizen(
      accessToken: json['access_token']! as String,
      accessTokenExpirationDate: DateTime.parse(
        json['access_token_expiration_date']! as String,
      ),
      idToken: json['id_token']! as String,
      refreshToken: json['refresh_token'] as String?,
      grantedScopes:
          grantedScopesJson is List<Object?>
              ? grantedScopesJson.cast<String>()
              : _authenticationScopes,
    );
  }

  /// Creates a json object from this token data.
  Map<String, Object> toJson() {
    return <String, Object>{
      'access_token': accessToken,
      'access_token_expiration_date':
          accessTokenExpirationDate.toIso8601String(),
      'id_token': idToken,
      'granted_scopes': grantedScopes.toList(),
      if (refreshToken != null) 'refresh_token': refreshToken!,
    };
  }
}

/// The set of "Client ID" and "Client Secret" issued by the authorization server.
class _Credentials {
  const _Credentials(this.clientId, this.clientSecret);

  /// The unique public identifier for apps that is issued by the authorization
  /// server. It's analogous to a login id.
  final String clientId;

  /// The secret credential known only to the application and the authorization
  /// server, it's analogous to a password.
  final String clientSecret;
}

class _CachedTokenStorage {
  // ignore: invalid_use_of_visible_for_testing_member
  final FlutterSecureStorage _storage = const FlutterSecureStorage();

  static const String _kToken = 'token';

  /// Cached token.
  _GoogleSignInTokenDataTizen? _token;

  Future<void> saveToken(_GoogleSignInTokenDataTizen token) async {
    await _storage.write(key: _kToken, value: jsonEncode(token.toJson()));
    _token = token;
  }

  Future<_GoogleSignInTokenDataTizen?> getToken() async {
    if (_token != null) {
      return _token!;
    }
    final String? jsonString = await _storage.read(key: _kToken);
    if (jsonString == null) {
      return null;
    }
    try {
      _token = _GoogleSignInTokenDataTizen.fromJson(
        jsonDecode(jsonString) as Map<String, Object?>,
      );
      return _token;
    } catch (_) {
      await removeToken();
      return null;
    }
  }

  Future<void> removeToken() async {
    await _storage.delete(key: _kToken);
    _token = null;
  }
}

/// Tizen implementation of [GoogleSignInPlatform].
class GoogleSignInTizen extends GoogleSignInPlatform {
  /// Registers this class as the default instance of [GoogleSignInPlatform].
  static void register() {
    GoogleSignInPlatform.instance = GoogleSignInTizen();
  }

  static _Credentials? _credentials;

  final _CachedTokenStorage _storage = _CachedTokenStorage();

  final DeviceAuthClient _authClient = DeviceAuthClient(
    authorizationEndPoint: Uri.parse(
      'https://oauth2.googleapis.com/device/code',
    ),
    tokenEndPoint: Uri.parse('https://oauth2.googleapis.com/token'),
    revokeEndPoint: Uri.parse('https://oauth2.googleapis.com/revoke'),
  );

  /// Sets [clientId] and [clientSecret] to be used for GoogleSignIn authentication.
  ///
  /// This must be called before calling [GoogleSignIn.initialize].
  static void setCredentials({
    required String clientId,
    required String clientSecret,
  }) {
    _credentials = _Credentials(clientId, clientSecret);
  }

  /// Gets the [GlobalKey] that identifies a [NavigatorState].
  ///
  /// This object must be assigned to a valid [Navigator] widget to push
  /// a dialog that shows "verification url" and "user code" which are
  /// required to authorize sign-in.
  ///
  /// If [MaterialApp] or [CupertinoApp] is used, it's convinient to
  /// assign this object to their `navigatorKey` parameter.
  static GlobalKey<NavigatorState> get navigatorKey =>
      device_flow_widget.navigatorKey;

  /// Sets the [GlobalKey] that identifies a [NavigatorState].
  ///
  /// This object must be set if [GlobalKey] needs to be instantiated from
  /// client code.
  static set navigatorKey(GlobalKey<NavigatorState> navigatorKey) =>
      device_flow_widget.navigatorKey = navigatorKey;

  void _ensureSetCredentials() {
    if (_credentials == null) {
      throw const GoogleSignInException(
        code: GoogleSignInExceptionCode.clientConfigurationError,
        description:
            'Cannot initialize GoogleSignInTizen: clientId and '
            'clientSecret have not been set. Call '
            'GoogleSignInTizen.setCredentials before using GoogleSignIn.',
      );
    }
  }

  void _ensureNavigatorKeyAssigned() {
    if (device_flow_widget.navigatorKey.currentContext == null) {
      throw const GoogleSignInException(
        code: GoogleSignInExceptionCode.uiUnavailable,
        description:
            'Cannot show GoogleSignInTizen authorization UI: a '
            'navigator key must be assigned to MaterialApp or CupertinoApp.',
      );
    }
  }

  @override
  Future<void> init(InitParameters params) async {
    _ensureSetCredentials();
    if (params.clientId != null && params.clientId != _credentials!.clientId) {
      throw const GoogleSignInException(
        code: GoogleSignInExceptionCode.clientConfigurationError,
        description:
            'The clientId passed to GoogleSignIn.initialize must match '
            'the clientId passed to GoogleSignInTizen.setCredentials.',
      );
    }
    if (params.hostedDomain != null) {
      throw const GoogleSignInException(
        code: GoogleSignInExceptionCode.clientConfigurationError,
        description: 'hostedDomain is not supported by google_sign_in_tizen.',
      );
    }
    if (params.nonce != null) {
      throw const GoogleSignInException(
        code: GoogleSignInExceptionCode.clientConfigurationError,
        description: 'nonce is not supported by google_sign_in_tizen.',
      );
    }
  }

  @override
  Future<AuthenticationResults?> attemptLightweightAuthentication(
    AttemptLightweightAuthenticationParameters params,
  ) async {
    final _GoogleSignInTokenDataTizen? token = await _getValidToken();
    return token == null
        ? null
        : AuthenticationResults(
          user: _userDataFromIdToken(token.idToken),
          authenticationTokens: AuthenticationTokenData(idToken: token.idToken),
        );
  }

  @override
  bool supportsAuthenticate() => true;

  @override
  Future<AuthenticationResults> authenticate(
    AuthenticateParameters params,
  ) async {
    final _GoogleSignInTokenDataTizen token = await _signInWithDeviceFlow(
      _authenticationScopes,
    );
    await _storage.saveToken(token);
    return AuthenticationResults(
      user: _userDataFromIdToken(token.idToken),
      authenticationTokens: AuthenticationTokenData(idToken: token.idToken),
    );
  }

  @override
  bool authorizationRequiresUserInteraction() => false;

  @override
  Future<ClientAuthorizationTokenData?> clientAuthorizationTokensForScopes(
    ClientAuthorizationTokensForScopesParameters params,
  ) async {
    final AuthorizationRequestDetails request = params.request;
    final _GoogleSignInTokenDataTizen? existingToken = await _getValidToken();
    if (existingToken != null &&
        _tokenMatchesRequestedUser(existingToken, request) &&
        existingToken.grantsScopes(request.scopes)) {
      return ClientAuthorizationTokenData(
        accessToken: existingToken.accessToken,
      );
    }

    if (!request.promptIfUnauthorized) {
      return null;
    }

    final _GoogleSignInTokenDataTizen token = await _signInWithDeviceFlow(
      _scopesForAuthentication(request.scopes),
    );
    if (!_tokenMatchesRequestedUser(token, request)) {
      throw const GoogleSignInException(
        code: GoogleSignInExceptionCode.userMismatch,
        description:
            'The authorized Google account does not match the '
            'requested account.',
      );
    }
    await _storage.saveToken(token);

    return ClientAuthorizationTokenData(accessToken: token.accessToken);
  }

  /// Device Flow does not return a server auth code, so this is unsupported on
  /// Tizen.
  @override
  Future<ServerAuthorizationTokenData?> serverAuthorizationTokensForScopes(
    ServerAuthorizationTokensForScopesParameters params,
  ) async {
    return null;
  }

  @override
  Future<void> clearAuthorizationToken(
    ClearAuthorizationTokenParams params,
  ) async {
    final _GoogleSignInTokenDataTizen? existingToken =
        await _storage.getToken();
    if (existingToken == null ||
        existingToken.accessToken != params.accessToken) {
      return;
    }
    await _storage.saveToken(existingToken.withInvalidatedAccessToken());
  }

  @override
  Future<void> signOut(SignOutParams params) => _storage.removeToken();

  @override
  Future<void> disconnect(DisconnectParams params) async {
    final _GoogleSignInTokenDataTizen? existingToken =
        await _storage.getToken();
    if (existingToken == null) {
      return;
    }

    try {
      await _authClient.revokeToken(existingToken.accessToken);
    } on AuthorizationException catch (error) {
      throw _exceptionFromAuthorizationException(error);
    } catch (error) {
      throw _unknownException(error);
    } finally {
      await signOut(const SignOutParams());
    }
  }

  Future<_GoogleSignInTokenDataTizen?> _getValidToken() async {
    final _GoogleSignInTokenDataTizen? existingToken =
        await _storage.getToken();
    if (existingToken == null) {
      return null;
    }
    if (!existingToken.isExpired) {
      return existingToken;
    }
    if (existingToken.refreshToken == null) {
      await _storage.removeToken();
      return null;
    }

    try {
      final _GoogleSignInTokenDataTizen token = await _refreshToken(
        existingToken,
      );
      await _storage.saveToken(token);
      return token;
    } on AuthorizationException catch (error) {
      if (error.error == 'invalid_grant') {
        await _storage.removeToken();
        return null;
      }
      throw _exceptionFromAuthorizationException(error);
    } catch (error) {
      throw _unknownException(error);
    }
  }

  Future<_GoogleSignInTokenDataTizen> _signInWithDeviceFlow(
    List<String> scopes,
  ) async {
    _ensureSetCredentials();
    _ensureNavigatorKeyAssigned();

    bool expired = false;
    bool canceled = false;
    bool widgetShown = false;

    try {
      final AuthorizationResponse authorizationResponse = await _authClient
          .requestAuthorization(_credentials!.clientId, scopes);

      final Future<TokenResponse?> tokenResponseFuture = _authClient.pollToken(
        clientId: _credentials!.clientId,
        clientSecret: _credentials!.clientSecret,
        deviceCode: authorizationResponse.deviceCode,
        interval: authorizationResponse.interval,
      );

      widgetShown = true;
      device_flow_widget.showDeviceFlowWidget(
        code: authorizationResponse.userCode,
        verificationUrl: authorizationResponse.verificationUrl,
        expiresIn: authorizationResponse.expiresIn,
        onExpired: () {
          expired = true;
          _authClient.cancelPollToken();
        },
        onCanceled: () {
          canceled = true;
          _authClient.cancelPollToken();
        },
      );

      final TokenResponse? tokenResponse = await tokenResponseFuture;
      if (tokenResponse == null) {
        if (canceled) {
          throw const GoogleSignInException(
            code: GoogleSignInExceptionCode.canceled,
            description: 'Sign in was canceled.',
          );
        }
        throw GoogleSignInException(
          code: GoogleSignInExceptionCode.interrupted,
          description:
              expired
                  ? 'The device authorization code expired.'
                  : 'Device authorization was interrupted.',
        );
      }
      return _tokenDataFromResponse(tokenResponse, requestedScopes: scopes);
    } on AuthorizationException catch (error) {
      throw _exceptionFromAuthorizationException(error);
    } on GoogleSignInException {
      rethrow;
    } catch (error) {
      throw _unknownException(error);
    } finally {
      if (widgetShown) {
        device_flow_widget.closeDeviceFlowWidget();
      }
    }
  }

  _GoogleSignInTokenDataTizen _tokenDataFromResponse(
    TokenResponse tokenResponse, {
    required Iterable<String> requestedScopes,
    _GoogleSignInTokenDataTizen? previousToken,
  }) {
    final String? idToken = tokenResponse.idToken ?? previousToken?.idToken;
    if (idToken == null) {
      throw const GoogleSignInException(
        code: GoogleSignInExceptionCode.providerConfigurationError,
        description: 'Google token response did not include an idToken.',
      );
    }

    return _GoogleSignInTokenDataTizen(
      accessToken: tokenResponse.accessToken,
      accessTokenExpirationDate: DateTime.now().add(tokenResponse.expiresIn),
      refreshToken: tokenResponse.refreshToken ?? previousToken?.refreshToken,
      idToken: idToken,
      grantedScopes:
          tokenResponse.scope.isNotEmpty
              ? tokenResponse.scope
              : previousToken?.grantedScopes ?? requestedScopes,
    );
  }

  GoogleSignInUserData _userDataFromIdToken(String idToken) {
    final Map<String, Object?> json;
    try {
      final List<String> splitTokens = idToken.split('.');
      if (splitTokens.length != 3) {
        throw const FormatException('Invalid idToken.');
      }
      final String normalizedPayload = base64Url.normalize(splitTokens[1]);
      final String payloadString = utf8.decode(
        base64Url.decode(normalizedPayload),
      );
      json = jsonDecode(payloadString) as Map<String, Object?>;
    } catch (error) {
      throw GoogleSignInException(
        code: GoogleSignInExceptionCode.providerConfigurationError,
        description: 'Google token response included an invalid idToken.',
        details: error.toString(),
      );
    }

    final String? email = json['email'] as String?;
    final String? id = json['sub'] as String?;
    if (email == null || id == null) {
      throw const GoogleSignInException(
        code: GoogleSignInExceptionCode.providerConfigurationError,
        description: 'Google idToken did not include the required user data.',
      );
    }

    return GoogleSignInUserData(
      email: email,
      id: id,
      displayName: json['name'] as String?,
      photoUrl: json['picture'] as String?,
    );
  }

  Future<_GoogleSignInTokenDataTizen> _refreshToken(
    _GoogleSignInTokenDataTizen token,
  ) async {
    _ensureSetCredentials();

    final TokenResponse tokenResponse = await _authClient.refreshToken(
      clientId: _credentials!.clientId,
      clientSecret: _credentials!.clientSecret,
      refreshToken: token.refreshToken!,
    );

    return _tokenDataFromResponse(
      tokenResponse,
      requestedScopes: token.grantedScopes,
      previousToken: token,
    );
  }

  bool _tokenMatchesRequestedUser(
    _GoogleSignInTokenDataTizen token,
    AuthorizationRequestDetails request,
  ) {
    if (request.userId == null && request.email == null) {
      return true;
    }
    final GoogleSignInUserData user = _userDataFromIdToken(token.idToken);
    return (request.userId == null || request.userId == user.id) &&
        (request.email == null || request.email == user.email);
  }

  List<String> _scopesForAuthentication(List<String> scopes) {
    return <String>{
      ..._authenticationScopes,
      ...scopes.map(_normalizeScope),
    }.toList();
  }
}

GoogleSignInException _exceptionFromAuthorizationException(
  AuthorizationException error,
) {
  final GoogleSignInExceptionCode code = switch (error.error) {
    'access_denied' => GoogleSignInExceptionCode.canceled,
    'invalid_client' => GoogleSignInExceptionCode.clientConfigurationError,
    'invalid_scope' => GoogleSignInExceptionCode.clientConfigurationError,
    _ => GoogleSignInExceptionCode.unknownError,
  };
  return GoogleSignInException(
    code: code,
    description: error.description ?? error.error,
    details: <String, String?>{
      'error': error.error,
      'uri': error.uri?.toString(),
    },
  );
}

GoogleSignInException _unknownException(Object error) {
  return GoogleSignInException(
    code: GoogleSignInExceptionCode.unknownError,
    description: 'Google Sign-In failed.',
    details: error.toString(),
  );
}

String _normalizeScope(String scope) {
  return switch (scope) {
    'https://www.googleapis.com/auth/userinfo.email' => 'email',
    'https://www.googleapis.com/auth/userinfo.profile' => 'profile',
    _ => scope,
  };
}
