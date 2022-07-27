// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter/services.dart';
import 'package:flutter/widgets.dart';
import 'package:google_sign_in_platform_interface/google_sign_in_platform_interface.dart';

import 'src/device_flow_widget.dart' as device_flow_widget;
import 'src/google_sign_in.dart';

export 'src/authorization_exception.dart';

/// Sets a custom [GlobalKey\<NavigatorState>] object used for pushing a Flutter
/// widget that displays "user_code" and "verification_uri".
void setNavigatorKey(GlobalKey<NavigatorState> key) =>
    device_flow_widget.navigatorKey = key;

/// Returns a [GlobalKey\<NavigatorState>] object currently used for pushing a
/// Flutter widget that displays "user_code" and "verification_uri".
///
/// A default key used by the plugin will be returned unless a custom key was
/// provided with [setNavigatorKey].
GlobalKey<NavigatorState> getNavigatorKey() => device_flow_widget.navigatorKey;

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

/// Tizen implementation of [GoogleSignInPlatform].
class GoogleSignInTizen extends GoogleSignInPlatform {
  /// Registers this class as the default instance of [GoogleSignInPlatform].
  static void register() {
    GoogleSignInPlatform.instance = GoogleSignInTizen();
  }

  final GoogleSignIn _googleSignIn = GoogleSignIn();

  Configuration? _configuration;

  void _setCredentials({
    required String clientId,
    required String clientSecret,
  }) {
    _configuration =
        Configuration(clientId: clientId, clientSecret: clientSecret);
  }

  void _ensureConfigurationInitialized() {
    if (_configuration == null) {
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
            '`MaterialApp` or `CupertinoApp`. To get a default navigator key, '
            'call getNavigatorKey, to provide a custom one, call setNavigatorKey '
            'in google_sign_in_tizen.dart.',
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

    _ensureConfigurationInitialized();
    _configuration = Configuration(
      clientId: _configuration!.clientId,
      clientSecret: _configuration!.clientSecret,
      scope: scopes,
    );
  }

  @override
  Future<GoogleSignInUserData?> signInSilently() {
    throw UnimplementedError('signInSilently() has not been implemented.');
  }

  @override
  Future<GoogleSignInUserData?> signIn() async {
    _ensureConfigurationInitialized();
    _ensureNavigatorKeyInitialized();

    return await _googleSignIn.signIn(_configuration!);
  }

  @override
  Future<GoogleSignInTokenData> getTokens(
      {required String email, bool? shouldRecoverAuth = true}) async {
    _ensureConfigurationInitialized();
    final Authentication? authentication =
        await _googleSignIn.getAuthentication(
      refresh: true,
      clientSecret: _configuration!.clientSecret,
    );
    if (authentication != null) {
      return GoogleSignInTokenData(
        accessToken: authentication.accessToken,
        idToken: authentication.idToken,
      );
    }
    return GoogleSignInTokenData();
  }

  @override
  Future<void> signOut() => _googleSignIn.signOut();

  @override
  Future<void> disconnect() => _googleSignIn.disconnect();

  @override
  Future<bool> isSignedIn() => _googleSignIn.isSignedIn();

  @override
  Future<void> clearAuthCache({String? token}) {
    throw UnimplementedError('clearAuthCache() has not been implemented.');
  }

  @override
  Future<bool> requestScopes(List<String> scopes) {
    throw UnimplementedError('requestScopes() has not been implemented.');
  }
}
