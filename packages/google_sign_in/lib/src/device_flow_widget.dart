// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:flutter/material.dart';

/// The [GlobalKey] that identifies a [NavigatorState].
///
/// This object refers to a navigator used to push the "Device Flow Widget"
/// that shows "verification url" and "user code" which are
/// required to authorize sign-in.
///
/// The navigatorKey.currentContext is non-null if navigatorKey
/// is assigned to a valid [Navigator].
GlobalKey<NavigatorState> navigatorKey = GlobalKey<NavigatorState>();

/// Closes the widget that was shown from [showDeviceFlowWidget].
void closeDeviceFlowWidget() {
  navigatorKey.currentState!.pop();
}

/// Displays a widget that shows [code] and [verificationUrl].
void showDeviceFlowWidget({
  required String code,
  required Uri verificationUrl,
  required Duration expiresIn,
  VoidCallback? onExpired,
  VoidCallback? onCanceled,
}) {
  showDialog<void>(
    context: navigatorKey.currentContext!,
    barrierDismissible: false,
    builder: (BuildContext context) {
      return AlertDialog(
        title: const Text('Google Sign In'),
        content: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: <Widget>[
            const Text('From a PC, phone, or tablet go to:'),
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
                closeDeviceFlowWidget();
              },
            ),
          ],
        ),
        actions: <Widget>[
          TextButton(
            onPressed: () {
              onCanceled?.call();
              closeDeviceFlowWidget();
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
    });
    if (_remaining.inSeconds == 0) {
      _timer.cancel();
      widget.onFinished?.call();
    }
  }

  @override
  void initState() {
    super.initState();
    _remaining = Duration(seconds: widget.duration.inSeconds);
    _timer = Timer.periodic(const Duration(seconds: 1), (_) => _countDown());
  }

  @override
  Widget build(BuildContext context) {
    final String minutes = _remaining.inMinutes.toString().padLeft(2, '0');
    final String seconds =
        _remaining.inSeconds.remainder(60).toString().padLeft(2, '0');
    return Text('$minutes:$seconds');
  }

  @override
  void dispose() {
    _timer.cancel();
    super.dispose();
  }
}
