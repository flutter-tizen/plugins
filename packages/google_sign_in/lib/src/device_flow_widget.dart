// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:flutter/material.dart';
import 'package:flutter_svg/svg.dart';

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
      final TextStyle bodyStyle = Theme.of(
        context,
      ).textTheme.bodyLarge!.copyWith(color: Colors.black);
      final TextStyle titleStyle = Theme.of(context).textTheme.titleLarge!
          .copyWith(color: Colors.black, fontWeight: FontWeight.bold);

      return AlertDialog(
        scrollable: true,
        contentPadding: const EdgeInsets.all(20),
        content: Column(
          mainAxisSize: MainAxisSize.min,
          children: <Widget>[
            FittedBox(
              child: Text(
                'From a PC, phone, or tablet go to:',
                style: bodyStyle,
              ),
            ),
            const SizedBox(height: 5),
            FittedBox(child: Text('$verificationUrl', style: titleStyle)),
            const SizedBox(height: 15),
            FittedBox(
              child: Text('Or scan the following QR code:', style: bodyStyle),
            ),
            const SizedBox(height: 5),
            SvgPicture.asset(
              'assets/images/qrcode.svg',
              package: 'google_sign_in_tizen',
              width: 100,
              height: 100,
            ),
            const SizedBox(height: 15),
            FittedBox(
              child: Text('Then enter the code below:', style: bodyStyle),
            ),
            const SizedBox(height: 5),
            FittedBox(child: Text(code, style: titleStyle)),
            const SizedBox(height: 15),
            FittedBox(child: Text('Code will expire in:', style: bodyStyle)),
            const SizedBox(height: 5),
            _CountdownTimer(
              const Duration(minutes: 30),
              style: bodyStyle,
              onFinished: () {
                onExpired?.call();
                closeDeviceFlowWidget();
              },
            ),
            const SizedBox(height: 15),
            ElevatedButton(
              onPressed: () {
                onCanceled?.call();
                closeDeviceFlowWidget();
              },
              child: const Text('Cancel'),
            ),
          ],
        ),
      );
    },
  );
}

class _CountdownTimer extends StatefulWidget {
  const _CountdownTimer(
    this.duration, {
    required this.style,
    required this.onFinished,
  });

  final Duration duration;
  final TextStyle style;
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
    final String seconds = _remaining.inSeconds
        .remainder(60)
        .toString()
        .padLeft(2, '0');
    return FittedBox(child: Text('$minutes:$seconds', style: widget.style));
  }

  @override
  void dispose() {
    _timer.cancel();
    super.dispose();
  }
}
