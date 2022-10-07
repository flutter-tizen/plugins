// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Copyright 2021 Invertase Limited. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file.

part of firebase_core_tizen;

/// A Dart only implementation of a Firebase app instance.
class FirebaseApp extends FirebaseAppPlatform {
  FirebaseApp._(String name, FirebaseOptions options) : super(name, options);

  bool _isAutomaticDataCollectionEnabled = false;

  @override
  Future<void> delete() {
    core_dart.Firebase.app(name).delete();
    return Future.value();
  }

  @override
  bool get isAutomaticDataCollectionEnabled =>
      _isAutomaticDataCollectionEnabled;

  @override
  Future<void> setAutomaticDataCollectionEnabled(bool enabled) {
    _isAutomaticDataCollectionEnabled = enabled;
    return Future.value();
  }

  /// Sets whether automatic resource management is enabled or disabled.
  @override
  Future<void> setAutomaticResourceManagementEnabled(bool enabled) {
    // ignore: todo
    // TODO(HakkyuKim): Check if this feature needs to be implemented for Tizen.
    return Future.value();
  }
}
