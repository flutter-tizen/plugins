// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:flutter/services.dart';
import 'package:flutter/widgets.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';
import 'package:tizen_app_control/app_control.dart';
import 'package:tizen_app_control/app_manager.dart';

const String kAppId = 'org.tizen.tizen_app_control_example';
const String kServiceAppId = 'org.tizen.tizen_app_control_example_service';
const Timeout kTimeout = Timeout(Duration(seconds: 10));

@pragma('vm:entry-point')
void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  testWidgets('Can receive request from platform', (WidgetTester _) async {
    // The very first message is a launch request from the platform.
    final ReceivedAppControl received = await AppControl.onAppControl.first;
    expect(received.appId, kAppId);
    expect(received.operation, 'http://tizen.org/appcontrol/operation/default');
  }, timeout: kTimeout);

  testWidgets('Can send and receive request', (WidgetTester _) async {
    // Send a request to this app (the test runner itself).
    final AppControl request = AppControl(
      appId: kAppId,
      operation: 'operation_1',
    );
    await request.sendLaunchRequest();

    final ReceivedAppControl received = await AppControl.onAppControl.first;
    expect(received.appId, kAppId);
    expect(received.operation, 'operation_1');
    expect(received.uri, isNull);
    expect(received.mime, isNull);
    expect(received.category, isNull);
    expect(received.launchMode, LaunchMode.single);
    expect(received.extraData, isEmpty);
    expect(received.shouldReply, isFalse);
  }, timeout: kTimeout);

  testWidgets('Omit invalid extra data', (WidgetTester _) async {
    final AppControl request = AppControl(
      appId: kAppId,
      extraData: <String, dynamic>{
        'STRING_DATA': 'string',
        'STRING_LIST_DATA': <String>['string', 'list'],
        'INTEGER_DATA': 1,
      },
    );
    await request.sendLaunchRequest();

    final ReceivedAppControl received = await AppControl.onAppControl.first;
    expect(received.extraData.length, 2);
    expect(received.extraData['STRING_DATA'], 'string');
    expect(received.extraData['STRING_LIST_DATA'], isNotEmpty);
    expect(received.extraData['STRING_LIST_DATA'][0], 'string');
  }, timeout: kTimeout);

  testWidgets('Can send and receive reply', (WidgetTester _) async {
    // This time, the request is sent to the service app instead of the test
    // runner, because the platform doesn't allow sending a reply back when
    // caller = callee.
    final AppControl request = AppControl(
      appId: kServiceAppId,
      operation: 'operation_2',
    );
    await request.sendLaunchRequest(
      replyCallback: (
        AppControl request,
        AppControl reply,
        AppControlReplyResult result,
      ) {
        expect(result, AppControlReplyResult.canceled);
        expect(reply.extraData['STRING_DATA'], 'string');
      },
    );
  }, timeout: kTimeout);

  testWidgets('Cannot find target applications', (WidgetTester _) async {
    final AppControl request1 = AppControl(appId: 'unknown_app');
    expect(
      request1.sendLaunchRequest,
      throwsA(isInstanceOf<PlatformException>()),
    );

    final AppControl request2 = AppControl(operation: 'unknown_operation');
    expect(
      request2.sendLaunchRequest,
      throwsA(isInstanceOf<PlatformException>()),
    );
  }, timeout: kTimeout);

  testWidgets('Can terminate service application', (WidgetTester _) async {
    expect(AppManager.isRunning(kServiceAppId), isFalse);

    final AppControl request = AppControl(appId: kServiceAppId);
    await request.sendLaunchRequest();
    await Future<void>.delayed(const Duration(seconds: 1));
    expect(AppManager.isRunning(kServiceAppId), isTrue);

    AppManager.terminateBackgroundApplication(kServiceAppId);
    await Future<void>.delayed(const Duration(seconds: 1));
    expect(AppManager.isRunning(kServiceAppId), isFalse);
  }, timeout: kTimeout);
}

@pragma('vm:entry-point')
void serviceMain() {
  WidgetsFlutterBinding.ensureInitialized();

  AppControl.onAppControl.listen((ReceivedAppControl request) async {
    if (request.shouldReply) {
      final AppControl reply = AppControl(
        extraData: <String, dynamic>{'STRING_DATA': 'string'},
      );
      await request.reply(reply, AppControlReplyResult.canceled);
      await SystemNavigator.pop();
    }
  });

  Future<void>.delayed(kTimeout.duration!).whenComplete(() async {
    await SystemNavigator.pop();
  });
}
