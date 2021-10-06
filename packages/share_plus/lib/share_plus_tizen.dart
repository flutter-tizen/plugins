// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:share_plus/share_plus.dart';
import 'package:share_plus_platform_interface/share_plus_platform_interface.dart';

/// The Tizen implementation of [SharePlatform].
///
/// This class disables platform overriding so that it always uses the default
/// implementation of [SharePlatform].
///
/// Although [Share.disableSharePlatformOverride] is meant to be
/// used for testing, it is required for Tizen to avoid platform interface
/// being set to Linux's.
/// https://github.com/fluttercommunity/plus_plugins/blob/fc6864f21cf1d1f6db47a3c938f24370362d00cd/packages/share_plus/lib/share_plus.dart#L30-L42
class SharePlugin extends SharePlatform {
  /// Inject Tizen's implementation of the plugin.
  static void register() {
    // ignore: invalid_use_of_visible_for_testing_member
    Share.disableSharePlatformOverride = true;
  }
}
