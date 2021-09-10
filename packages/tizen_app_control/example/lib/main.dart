// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:messageport_tizen/messageport_tizen.dart';
import 'package:tizen_app_control/app_control.dart';
import 'package:tizen_app_control/app_manager.dart';

const String _kAppId = 'org.tizen.tizen_app_control_example';
const String _kServiceAppId = 'org.tizen.tizen_app_control_example_service';
const String _kPortName = 'service_port';

/// The main entry point for the UI app.
void main() {
  runApp(const MyApp());
}

/// The main entry point for the service app.
@pragma('vm:entry-point')
void serviceMain() {
  // This call is required to use platform channels.
  WidgetsFlutterBinding.ensureInitialized();

  // Listen for incoming AppControls.
  final StreamSubscription<AppControl> appControlListener =
      AppControl.onAppControl.listen((ReceivedAppControl request) async {
    if (request.shouldReply) {
      final AppControl reply = AppControl();
      await request.reply(reply, AppControlReplyResult.succeeded);
    }
  });

  // Connect to the UI app and send messages.
  // An exception will be thrown if the UI app is not running.
  TizenMessagePort.connectToRemotePort(_kAppId, _kPortName)
      .then((RemotePort remotePort) async {
    while (true) {
      if (await remotePort.check()) {
        await remotePort.send(null);
      } else {
        break;
      }
      await Future<void>.delayed(const Duration(seconds: 1));
    }
  }).whenComplete(() async {
    await appControlListener.cancel();
    await SystemNavigator.pop();
  });
}

/// The main UI app widget.
class MyApp extends StatefulWidget {
  /// The main UI app widget.
  const MyApp({Key? key}) : super(key: key);

  @override
  _MyAppState createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  LocalPort? _localPort;
  int _messagesCount = 0;
  bool _isServiceStarted = false;

  @override
  void initState() {
    super.initState();

    // Open a message port to receive messages from the service app.
    TizenMessagePort.createLocalPort(_kPortName).then((LocalPort value) {
      _localPort = value;
      _localPort?.register((dynamic message, [RemotePort? remotePort]) {
        setState(() {
          _messagesCount++;
        });
      });
    });
  }

  @override
  void dispose() {
    super.dispose();

    _localPort?.unregister();
  }

  Future<void> _launchService() async {
    // Send a launch request to the service app.
    final AppControl request = AppControl(appId: _kServiceAppId);
    return request.sendLaunchRequest(
      replyCallback: (
        AppControl request,
        AppControl reply,
        AppControlReplyResult result,
      ) async {
        if (result == AppControlReplyResult.succeeded) {
          setState(() {
            _isServiceStarted = true;
          });
        } else {
          ScaffoldMessenger.of(context).showSnackBar(
            SnackBar(content: Text('Launch $result.')),
          );
        }
      },
    );
  }

  Future<void> _terminateService() async {
    // Send a terminate request to the service app.
    AppManager.terminateBackgroundApplication(_kServiceAppId);
    setState(() {
      _isServiceStarted = AppManager.isRunning(_kServiceAppId);
    });
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(title: const Text('Tizen App Control Example')),
        body: Center(
          child: Column(
            mainAxisSize: MainAxisSize.min,
            children: <Widget>[
              ElevatedButton(
                onPressed: _isServiceStarted ? null : _launchService,
                child: const Text('Launch service'),
              ),
              const SizedBox(height: 10),
              ElevatedButton(
                onPressed: _isServiceStarted ? _terminateService : null,
                child: const Text('Terminate service'),
              ),
              const SizedBox(height: 10),
              Text('Received messages: $_messagesCount'),
            ],
          ),
        ),
      ),
    );
  }
}
