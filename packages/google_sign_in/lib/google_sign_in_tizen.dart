// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:google_sign_in_platform_interface/google_sign_in_platform_interface.dart';

/// Tizen implementation of [GoogleSignInPlatform].
class GoogleSignInTizen extends GoogleSignInPlatform {
  /// Registers this class as the default instance of [GoogleSignInPlatform].
  static void register() {
    GoogleSignInPlatform.instance = GoogleSignInTizen();
  }

  @override
  Future<void> init({
    List<String> scopes = const <String>[],
    SignInOption signInOption = SignInOption.standard,
    String? hostedDomain,
    String? clientId,
  }) {}

  @override
  Future<GoogleSignInUserData?> signInSilently() {}

  @override
  Future<GoogleSignInUserData?> signIn() {}

  @override
  Future<GoogleSignInTokenData> getTokens(
      {required String email, bool? shouldRecoverAuth = true}) {}

  @override
  Future<void> signOut() {}

  @override
  Future<void> disconnect() {}

  @override
  Future<bool> isSignedIn() async {}

  @override
  Future<void> clearAuthCache({String? token}) {}

  @override
  Future<bool> requestScopes(List<String> scopes) async {}
}
