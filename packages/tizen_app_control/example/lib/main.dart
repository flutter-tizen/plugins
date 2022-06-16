// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:messageport_tizen/messageport_tizen.dart';
import 'package:tizen_app_control/tizen_app_control.dart';
import 'package:tizen_app_manager/tizen_app_manager.dart';

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
  final StreamSubscription<ReceivedAppControl> appControlListener =
      AppControl.onAppControl.listen((ReceivedAppControl request) async {
    if (request.shouldReply) {
      final AppControl reply = AppControl();
      await request.reply(reply, AppControlReplyResult.succeeded);
    }
  });

  // Connect to the UI app and send messages.
  // An exception will be thrown if the UI app is not running.
  RemotePort.connect(_kAppId, _kPortName).then((RemotePort remotePort) async {
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
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  final GlobalKey<ScaffoldMessengerState> _messengerKey =
      GlobalKey<ScaffoldMessengerState>();
  LocalPort? _localPort;
  int _messagesCount = 0;
  bool _isServiceStarted = false;

  @override
  void initState() {
    super.initState();

    // Open a message port to receive messages from the service app.
    LocalPort.create(_kPortName).then((LocalPort value) {
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

  Future<void> _sendSms() async {
    final AppControl request = AppControl(
      operation: 'http://tizen.org/appcontrol/operation/share_text',
      uri: 'sms:',
      launchMode: LaunchMode.group,
      extraData: <String, dynamic>{
        'http://tizen.org/appcontrol/data/text': 'Some text',
      },
    );
    final List<String> matches = await request.getMatchedAppIds();
    if (matches.isEmpty) {
      _messengerKey.currentState!.showSnackBar(
        const SnackBar(content: Text('No application found.')),
      );
      return;
    }
    await request.sendLaunchRequest();
  }

  Future<void> _pickImage() async {
    final AppControl request = AppControl(
      operation: 'http://tizen.org/appcontrol/operation/pick',
      mime: 'image/*',
      launchMode: LaunchMode.group,
    );
    final List<String> matches = await request.getMatchedAppIds();
    if (matches.isEmpty) {
      _messengerKey.currentState!.showSnackBar(
        const SnackBar(content: Text('No application found.')),
      );
      return;
    }
    await request.sendLaunchRequest(
      replyCallback: (
        AppControl request,
        AppControl reply,
        AppControlReplyResult result,
      ) {
        const String kAppControlDataSelected =
            'http://tizen.org/appcontrol/data/selected';
        String? imagePath;
        if (result == AppControlReplyResult.succeeded &&
            reply.extraData.containsKey(kAppControlDataSelected)) {
          imagePath = reply.extraData[kAppControlDataSelected][0] as String;
        }
        _messengerKey.currentState!.showSnackBar(
          SnackBar(content: Text(imagePath ?? 'No image selected.')),
        );
      },
    );
  }

  Future<void> _launchService() async {
    final AppControl request = AppControl(appId: _kServiceAppId);
    await request.sendLaunchRequest(
      replyCallback: (
        AppControl request,
        AppControl reply,
        AppControlReplyResult result,
      ) {
        if (result == AppControlReplyResult.succeeded) {
          setState(() {
            _isServiceStarted = true;
          });
        } else {
          _messengerKey.currentState!.showSnackBar(
            const SnackBar(content: Text('Launch failed.')),
          );
        }
      },
    );
  }

  Future<void> _terminateService() async {
    final AppRunningContext context = AppRunningContext(appId: _kServiceAppId);
    context.terminate(background: true);
    setState(() {
      _isServiceStarted = false;
    });
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      scaffoldMessengerKey: _messengerKey,
      home: Scaffold(
        appBar: AppBar(title: const Text('Tizen App Control Example')),
        body: Center(
          child: Column(
            mainAxisSize: MainAxisSize.min,
            children: <Widget>[
              ElevatedButton(
                onPressed: _sendSms,
                child: const Text('Send SMS'),
              ),
              const SizedBox(height: 10),
              ElevatedButton(
                onPressed: _pickImage,
                child: const Text('Pick image'),
              ),
              const SizedBox(height: 10),
              if (_isServiceStarted)
                ElevatedButton(
                  onPressed: _terminateService,
                  style: ElevatedButton.styleFrom(primary: Colors.redAccent),
                  child: const Text('Terminate service'),
                )
              else
                ElevatedButton(
                  onPressed: _launchService,
                  child: const Text('Launch service'),
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
