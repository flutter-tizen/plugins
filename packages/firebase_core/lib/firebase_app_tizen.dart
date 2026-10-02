// Copyright 2021 Invertase Limited. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file.

part of 'firebase_core_tizen.dart';

/// A Dart only implementation of a Firebase app instance.
class FirebaseApp extends FirebaseAppPlatform {
  FirebaseApp._(this._app, FirebaseOptions options) : super(_app.name, options);

  final core_dart.FirebaseApp _app;

  @override
  Future<void> delete() async {
    if (name == defaultFirebaseAppName) {
      throw noDefaultAppDelete();
    }
    if (core_dart.Firebase.apps.any((app) => identical(app, _app))) {
      _app.delete();
    }
  }

  @override
  bool get isAutomaticDataCollectionEnabled => _app.isAutomaticDataCollectionEnabled;

  @override
  Future<void> setAutomaticDataCollectionEnabled(bool enabled) {
    return _app.setAutomaticDataCollectionEnabled(enabled);
  }

  /// Sets whether automatic resource management is enabled or disabled.
  /// This has no effect on Dart.
  @override
  Future<void> setAutomaticResourceManagementEnabled(bool enabled) {
    return Future<void>.value();
  }
}
