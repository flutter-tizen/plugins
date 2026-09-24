// Copyright 2021 Invertase Limited. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file.

library firebase_core_tizen;

import 'package:firebase_core_dart/firebase_core_dart.dart' as core_dart;
import 'package:firebase_core_platform_interface/firebase_core_platform_interface.dart';

part 'firebase_app_tizen.dart';

/// Tizen implementation of FirebaseCore for managing Firebase app instances.
class FirebaseCore extends FirebasePlatform {
  /// Registers this class as the default instance of [FirebasePlatform].
  static void register() {
    FirebasePlatform.instance = FirebaseCore();
  }

  FirebaseApp _mapDartToPlatformApp(core_dart.FirebaseApp app) {
    final core_dart.FirebaseOptions options = app.options;

    return FirebaseApp._(
      app,
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
        androidClientId: options.androidClientId,
        iosClientId: options.iosClientId,
        iosBundleId: options.iosBundleId,
      ),
    );
  }

  @override
  List<FirebaseApp> get apps {
    return core_dart.Firebase.apps.map(_mapDartToPlatformApp).toList(growable: false);
  }

  @override
  Future<FirebaseApp> initializeApp({
    String? name,
    FirebaseOptions? options,
  }) async {
    name ??= defaultFirebaseAppName;
    if (name == defaultFirebaseAppName && options == null) {
      if (core_dart.Firebase.apps.any((app) => app.name == name)) {
        return app(name);
      }
      throw coreNotInitialized();
    }

    assert(
      options != null,
      'options should be provided to initialize a new app.',
    );

    try {
      // Initialize the app in firebase_core_dart
      final dartOptions = core_dart.FirebaseOptions.fromMap(options!.asMap);
      final core_dart.FirebaseApp dartApp =
          await core_dart.Firebase.initializeApp(name: name, options: dartOptions);

      return _mapDartToPlatformApp(dartApp);
    } on core_dart.FirebaseException catch (e) {
      switch (e.code) {
        case 'no-app':
          throw noAppExists(name);

        case 'duplicate-app':
          throw duplicateApp(name);
      }

      rethrow;
    } catch (e) {
      rethrow;
    }
  }

  @override
  FirebaseApp app([String name = defaultFirebaseAppName]) {
    try {
      return _mapDartToPlatformApp(core_dart.Firebase.app(name));
    } catch (_) {
      throw noAppExists(name);
    }
  }
}
