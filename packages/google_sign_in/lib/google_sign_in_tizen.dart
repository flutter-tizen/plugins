// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter/widgets.dart';
import 'package:google_sign_in_platform_interface/google_sign_in_platform_interface.dart';
import 'package:google_sign_in_tizen/src/google_sign_in.dart';

// TODO(HakkyuKim): Add documentation.
void setGoogleSignInTizenNavigatorKey(GlobalKey<NavigatorState> key) {
  (GoogleSignInPlatform.instance as GoogleSignInTizen).navigatorKey = key;
}

// TODO(HakkyuKim): Add documentation.
GlobalKey<NavigatorState> getGoogleSignInTizenNavigatorKey() {
  return (GoogleSignInPlatform.instance as GoogleSignInTizen).navigatorKey;
}

/// Tizen implementation of [GoogleSignInPlatform].
class GoogleSignInTizen extends GoogleSignInPlatform {
  /// Registers this class as the default instance of [GoogleSignInPlatform].
  static void register() {
    GoogleSignInPlatform.instance = GoogleSignInTizen();
  }

  final GoogleSignIn _googleSignIn = GoogleSignIn();

  Configuration? _configuration;

  // TODO(HakkyuKim): Add documentation.
  GlobalKey<NavigatorState> get navigatorKey => _googleSignIn.navigatorKey;

  // TODO(HakkyuKim): Add documentation.
  set navigatorKey(GlobalKey<NavigatorState> key) {
    _googleSignIn.navigatorKey = key;
  }

  @override
  Future<void> init({
    List<String> scopes = const <String>[],
    SignInOption signInOption = SignInOption.standard,
    String? hostedDomain,
    String? clientId,
  }) async {
    // TODO(HakkyuKim): Check if navigator has been set.

    // TODO(HakkyuKim): Parse client id and client secret from configuration file.
    //
    // Read "client id" and "client secret" from configuration file, such as
    // google-services.json for Android or GoogleService-Info.plist for iOS.
    //
    String clientIdentifier = ''; // readClientId(configurationFile);
    String clientSecret = ''; // readClientSecret(configurationFile);

    _configuration = Configuration(
      clientId: clientIdentifier,
      clientSecret: clientSecret,
      scope: scopes,
    );
  }

  @override
  Future<GoogleSignInUserData?> signInSilently() {
    throw UnimplementedError('signInSilently() has not been implemented.');
  }

  @override
  Future<GoogleSignInUserData?> signIn() async {
    if (_configuration == null) {
      throw Exception('GoogleSignIn Tizen has not been initialized.');
    }

    final GoogleUser? user = await _googleSignIn.signIn(_configuration!);
    if (user != null) {
      return GoogleSignInUserData(
        email: user.profile.email,
        id: user.userId,
        displayName: user.profile.name,
        idToken: user.authentication.idToken,
      );
    }
    return null;
  }

  @override
  Future<GoogleSignInTokenData> getTokens(
      {required String email, bool? shouldRecoverAuth = true}) async {
    final Authentication? authentication = _googleSignIn.authentication;
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
