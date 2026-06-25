// Copyright 2024 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: lines_longer_than_80_chars, avoid_classes_with_only_static_members, no_default_cases, public_member_api_docs
import 'package:firebase_core/firebase_core.dart' show FirebaseOptions;
import 'package:flutter/foundation.dart'
    show TargetPlatform, defaultTargetPlatform, kIsWeb;

class DefaultFirebaseOptions {
  static FirebaseOptions get currentPlatform {
    if (kIsWeb) {
      throw UnsupportedError('Web is not supported.');
    }
    switch (defaultTargetPlatform) {
      case TargetPlatform.linux: // Tizen reports as TargetPlatform.linux
        return linux;
      default:
        throw UnsupportedError(
          'DefaultFirebaseOptions are not supported for this platform.',
        );
    }
  }

  static const FirebaseOptions linux = FirebaseOptions(
    apiKey: 'AIzaSyAgUhHU8wSJgO5MVNy95tMT07NEjzMOfz0',
    appId: '1:448618578101:ios:0b650370bb29e29cac3efc',
    messagingSenderId: '448618578101',
    projectId: 'react-native-firebase-testing',
  );
}
