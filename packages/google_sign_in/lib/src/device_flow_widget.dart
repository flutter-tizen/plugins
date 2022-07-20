// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

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

/// Displays a widget that shows [code] and [verificationUrl].
void showDeviceFlowWidget({
  required String code,
  required Uri verificationUrl,
  required Duration expiresIn,
  VoidCallback? onExpired,
  VoidCallback? onCancelled,
}) {
  showDialog<void>(
    // NavigatorKey.currentContext is non-null if plugin is properly initialized.
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
              'From a PC, phone, or tablet go to:',
            ),
            Text(
              '$verificationUrl',
              textScaleFactor: 1.5,
            ),
            const Text('Then enter the code below:'),
            Text(
              code,
              textScaleFactor: 2.5,
            ),
            const Text('Code will expire in:'),
            _CountdownTimer(
              expiresIn,
              onFinished: () {
                onExpired?.call();
                Navigator.pop(context);
              },
            ),
          ],
        ),
        actions: <Widget>[
          TextButton(
            onPressed: () {
              onCancelled?.call();
              Navigator.pop(context);
            },
            child: const Text(
              'Cancel',
              textAlign: TextAlign.end,
            ),
          )
        ],
      );
    },
  );
}

class _CountdownTimer extends StatefulWidget {
  const _CountdownTimer(
    this.duration, {
    required this.onFinished,
  });

  final Duration duration;
  final VoidCallback? onFinished;

  @override
  State<_CountdownTimer> createState() => _CountdownTimerState();
}

class _CountdownTimerState extends State<_CountdownTimer> {
  late Timer _timer;
  late Duration _remaining;

  void _countDown() {
    setState(() {
      _remaining = _remaining - const Duration(seconds: 1);
      if (_remaining.inSeconds == 0) {
        _timer.cancel();
        widget.onFinished?.call();
      }
    });
  }

  @override
  void initState() {
    super.initState();
    _remaining = Duration(seconds: widget.duration.inSeconds);
    _timer = Timer.periodic(const Duration(seconds: 1), (_) => _countDown());
  }

  @override
  Widget build(BuildContext context) {
    final String minutes =
        (_remaining.inSeconds / 60).floor().toString().padLeft(2, '0');
    final String seconds =
        (_remaining.inSeconds % 60).toString().padLeft(2, '0');
    return Text('$minutes:$seconds');
  }

  @override
  void dispose() {
    _timer.cancel();
    super.dispose();
  }
}
