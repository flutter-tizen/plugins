// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter/material.dart';

/// The [GlobalKey\<NavigatorState>] object used for pushing a Flutter widget
/// that displays "user_code" and "verification_uri".
GlobalKey<NavigatorState> navigatorKey = GlobalKey<NavigatorState>();

class _DeviceFlowWidgetKey extends GlobalObjectKey {
  const _DeviceFlowWidgetKey(super.value);
}

/// Closes the widget that was shown from [showDeviceFlowWidget].
void closeDeviceFlowWidget() {
  if (navigatorKey.currentWidget?.key ==
      const _DeviceFlowWidgetKey('device-flow')) {
    Navigator.pop(navigatorKey.currentContext!);
  }
}

/// Displays a widget that shows [userCode] and [verificationUrl].
void showDeviceFlowWidget(
  String userCode,
  Uri verificationUrl,
  Function onCancel,
) {
  showDialog<void>(
    // NavigatorKey.currentContext is non-null if plugin is initialized.
    context: navigatorKey.currentContext!,
    barrierDismissible: false,
    builder: (BuildContext context) {
      return AlertDialog(
        key: const _DeviceFlowWidgetKey('device-flow'),
        title: const Text('Google SignIn'),
        content: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: <Widget>[
            const Text(
              'Input the user code in the following URL.',
              textScaleFactor: 3.0,
            ),
            Text(
              'User code: $userCode',
              textScaleFactor: 3.0,
            ),
            Text(
              'URL: $verificationUrl',
              textScaleFactor: 3.0,
            ),
            ElevatedButton(
              onPressed: () {
                Navigator.pop(context);
                onCancel();
              },
              child: const Text('Cancel'),
            )
          ],
        ),
      );
    },
  );
}
