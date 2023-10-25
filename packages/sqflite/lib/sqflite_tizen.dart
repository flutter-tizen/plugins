// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: avoid_classes_with_only_static_members

import 'package:sqflite/sqflite.dart';

/// A class to initialize the plugin.
///
/// This class is not intended for use by user code.
class SqfliteTizen {
  /// Sets the default factory implementation.
  static void register() {
    databaseFactory = databaseFactorySqflitePlugin;
  }
}
