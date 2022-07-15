// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:convert' as convert;
import 'dart:io';

import 'package:flutter/widgets.dart';
import 'package:google_sign_in_platform_interface/google_sign_in_platform_interface.dart';
import 'package:google_sign_in_tizen/src/google_sign_in.dart';
import 'package:path_provider/path_provider.dart';

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
    // TODO(HakkyuKim): Throw if navigator has not been set.

    // Parse client id and client secret from configuration file.
    final Directory dataDir = await getApplicationDocumentsDirectory();

    // The 'data' directory for storing application's private data is deprecated
    // by the SDK team. See: https://github.sec.samsung.net/RS-TizenStudio/home/issues/238.
    // This is a workaround until path_provider is fixed.
    final Directory resDir = dataDir.parent
        .listSync()
        .whereType<Directory>()
        .firstWhere(
            (Directory dir) =>
                dir.path.endsWith('res') || dir.path.endsWith('res/'),
            orElse: () => throw Exception("The res directory doesn't exist."));

    final File file = resDir.listSync().whereType<File>().firstWhere(
        (File file) => file.path.endsWith('google-services.json'),
        orElse: () =>
            throw Exception('The google-services.json file not found.'));
    final Map<String, dynamic> googleServicesJson =
        convert.jsonDecode(file.readAsStringSync()) as Map<String, dynamic>;

    if (googleServicesJson['client_id'] == null ||
        googleServicesJson['client_secret'] == null) {
      throw Exception('The client_id or client_secret is missing.');
    }

    _configuration = Configuration(
      clientId: googleServicesJson['client_id'] as String,
      clientSecret: googleServicesJson['client_secret'] as String,
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
    // TODO(HakkyuKim): Handle token refresh if expired.
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
