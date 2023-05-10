// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
