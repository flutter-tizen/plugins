// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter/widgets.dart';
import 'package:google_sign_in_platform_interface/google_sign_in_platform_interface.dart';
import 'package:google_sign_in_tizen/src/google_sign_in.dart';

void setGoogleSignInTizenNavigatorKey(GlobalKey<NavigatorState> key) {
  (GoogleSignInPlatform.instance as GoogleSignInTizen).navigatorKey = key;
}

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

  GlobalKey<NavigatorState> get navigatorKey => _googleSignIn.navigatorKey;

  set navigatorKey(GlobalKey<NavigatorState> key) {
    _googleSignIn.navigatorKey = key;
  }

  @override
  Future<void> init({
    List<String> scopes = const <String>[],
    SignInOption signInOption = SignInOption.standard,
    String? hostedDomain,
    String? clientId,
  }) =>
      _googleSignIn.init(clientId!, scopes);

  @override
  Future<GoogleSignInUserData?> signInSilently() {
    throw UnimplementedError('signInSilently() has not been implemented.');
  }

  @override
  Future<GoogleSignInUserData?> signIn() async {
    final GoogleUser? user = await _googleSignIn.signIn();
    if (user != null) {
      return GoogleSignInUserData(
        email: user.profile!.email,
        id: user.userId!,
        displayName: user.profile!.name,
        idToken: user.authentication.idToken,
      );
    }
    return null;
  }

  @override
  Future<GoogleSignInTokenData> getTokens(
      {required String email, bool? shouldRecoverAuth = true}) async {
    if (_googleSignIn.authentication == null) {
      return GoogleSignInTokenData();
    }
    return GoogleSignInTokenData(
      accessToken: _googleSignIn.authentication!.accessToken,
      idToken: _googleSignIn.authentication!.idToken,
    );
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
