// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:package_info_plus/package_info_plus.dart';
import 'package:package_info_plus_platform_interface/package_info_platform_interface.dart';

/// The Tizen implementation of [PackageInfoPlatform].
///
/// This class disables platform overriding so that it always uses the default
/// implementation of [PackageInfoPlatform].
///
/// Although [PackageInfo.disablePackageInfoPlatformOverride] is meant to be
/// used for testing, it is required for Tizen to avoid platform interface
/// being set to Linux's.
/// https://github.com/fluttercommunity/plus_plugins/blob/c7c942d1d19595a536c6ae18168d3781ddef7785/packages/package_info_plus/lib/package_info_plus.dart#L43-L55
class PackageInfoPlugin extends PackageInfoPlatform {
  /// Register dart plugin to inject Tizen plugin.
  static void register() {
    // ignore: invalid_use_of_visible_for_testing_member
    PackageInfo.disablePackageInfoPlatformOverride = true;
  }
}
