// Copyright 2021 Invertase Limited. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file.

library firebase_core_tizen;

import 'package:firebase_core_dart/firebase_core_dart.dart' as core_dart;
import 'package:firebase_core_platform_interface/firebase_core_platform_interface.dart';

part 'firebase_app_desktop.dart';

/// Desktop implementation of FirebaseCore for managing Firebase app
/// instances.
class FirebaseCore extends FirebasePlatform {
  /// Called by PluginRegistry to register this plugin as the implementation for Desktop
  static void register() {
    FirebasePlatform.instance = FirebaseCore();
  }

  FirebaseApp _mapDartToPlatfromApp(core_dart.FirebaseApp app) {
    final core_dart.FirebaseOptions options = app.options;

    return FirebaseApp._(
      app.name,
      FirebaseOptions(
        apiKey: options.apiKey,
        appId: options.appId,
        messagingSenderId: options.messagingSenderId,
        authDomain: options.authDomain,
        projectId: options.projectId,
        databaseURL: options.databaseURL,
        measurementId: options.measurementId,
        storageBucket: options.storageBucket,
        trackingId: options.trackingId,
        appGroupId: options.appGroupId,
        deepLinkURLScheme: options.deepLinkURLScheme,
      ),
    );
  }

  @override
  List<FirebaseApp> get apps {
    return core_dart.Firebase.apps
        .map(_mapDartToPlatfromApp)
        .toList(growable: false);
  }

  @override
  Future<FirebaseApp> initializeApp({
    String? name,
    FirebaseOptions? options,
  }) async {
    assert(
      options != null,
      'options should be provided to initialize the default app.',
    );

    /// Ensures the name isn't null, in case no name
    /// passed, [defaultFirebaseAppName] will be used
    final String _name = name ?? defaultFirebaseAppName;

    try {
      // Initialize the app in firebase_core_dart
      final core_dart.FirebaseOptions _dartOptions =
          core_dart.FirebaseOptions.fromMap(options!.asMap);
      final core_dart.FirebaseApp _dartApp =
          await core_dart.Firebase.initializeApp(
        name: _name,
        options: _dartOptions,
      );

      return _mapDartToPlatfromApp(_dartApp);
    } on core_dart.FirebaseException catch (e) {
      switch (e.code) {
        case 'no-app':
          throw noAppExists(_name);

        case 'duplicate-app':
          throw duplicateApp(_name);
      }

      rethrow;
    } catch (e) {
      rethrow;
    }
  }

  @override
  FirebaseApp app([String name = defaultFirebaseAppName]) {
    try {
      return _mapDartToPlatfromApp(core_dart.Firebase.app(name));
    } catch (_) {
      throw noAppExists(name);
    }
  }
}
