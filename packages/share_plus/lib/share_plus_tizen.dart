// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:ui';

import 'package:share_plus_platform_interface/share_plus_platform_interface.dart';
import 'package:tizen_app_control/app_control.dart';

/// The Tizen implementation of [SharePlatform].
class SharePlugin extends SharePlatform {
  /// Registers this class as the default instance of [SharePlatform].
  static void register() {
    SharePlatform.instance = SharePlugin();
  }

  @override
  Future<void> share(
    String text, {
    String? subject,
    Rect? sharePositionOrigin,
  }) {
    final bool hasSubject = subject != null && subject.isNotEmpty;
    return AppControl(
      operation: 'http://tizen.org/appcontrol/operation/share_text',
      uri: hasSubject ? 'mailto:' : 'sms:',
      launchMode: LaunchMode.group,
      extraData: {
        'http://tizen.org/appcontrol/data/text': text,
        if (hasSubject) 'http://tizen.org/appcontrol/data/subject': subject,
      },
    ).sendLaunchRequest();
  }

  @override
  Future<void> shareFiles(
    List<String> paths, {
    List<String>? mimeTypes,
    String? subject,
    String? text,
    Rect? sharePositionOrigin,
  }) {
    final bool hasSubject = subject != null && subject.isNotEmpty;
    final bool hasText = text != null && text.isNotEmpty;
    return AppControl(
      operation: 'http://tizen.org/appcontrol/operation/share_text',
      uri: 'mailto:',
      launchMode: LaunchMode.group,
      extraData: {
        'http://tizen.org/appcontrol/data/text': hasText ? text : 'null',
        if (hasSubject) 'http://tizen.org/appcontrol/data/subject': subject,
        if (paths.isNotEmpty) 'http://tizen.org/appcontrol/data/path': paths,
      },
    ).sendLaunchRequest();
  }
}
