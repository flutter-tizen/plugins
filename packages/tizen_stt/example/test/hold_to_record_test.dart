// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:tizen_stt_example/main.dart';

void main() {
  TestWidgetsFlutterBinding.ensureInitialized();
  final TestDefaultBinaryMessenger messenger =
      TestDefaultBinaryMessengerBinding.instance.defaultBinaryMessenger;
  const MethodChannel channel = MethodChannel('tizen_stt');
  const MethodChannel events = MethodChannel('tizen_stt/events');
  final List<MethodCall> calls = <MethodCall>[];
  String state = 'notCreated';
  Completer<void>? startPending;
  bool delayRecording = false;
  bool failStart = false;

  setUp(() {
    calls.clear();
    state = 'notCreated';
    startPending = null;
    delayRecording = false;
    failStart = false;
    messenger.setMockMethodCallHandler(events, (_) async => null);
    messenger.setMockMethodCallHandler(channel, (MethodCall call) async {
      calls.add(call);
      switch (call.method) {
        case 'initialize':
        case 'cancel':
          state = 'ready';
        case 'start':
          if (failStart) {
            throw PlatformException(code: 'STT_ERROR_NOT_SUPPORTED_FEATURE');
          }
          if (startPending != null) {
            await startPending!.future;
          }
          if (!delayRecording) {
            state = 'recording';
          }
        case 'stop':
          state = 'processing';
        case 'dispose':
          state = 'notCreated';
          return null;
        case 'getLanguages':
          return <String>['en_US'];
      }
      return <String, Object?>{
        'state': <String, Object?>{'name': state}
      };
    });
  });

  Future<void> emit(Map<String, Object?> event) async {
    await messenger.handlePlatformMessage('tizen_stt/events',
        const StandardMethodCodec().encodeSuccessEnvelope(event), (_) {});
  }

  Future<void> setState(String value) async {
    state = value;
    await emit(<String, Object?>{
      'type': 'state',
      'current': <String, Object?>{'name': value}
    });
  }

  int count(String method) =>
      calls.where((MethodCall call) => call.method == method).length;

  Future<void> prepare(WidgetTester tester) async {
    await tester.pumpWidget(const MyApp());
    await tester.tap(find.text('Initialize'));
    await tester.pumpAndSettle();
    Focus.of(tester.element(find.text('Hold Select to record'))).requestFocus();
    await tester.pumpAndSettle();
  }

  for (final LogicalKeyboardKey key in <LogicalKeyboardKey>[
    LogicalKeyboardKey.select,
    LogicalKeyboardKey.enter,
    LogicalKeyboardKey.numpadEnter,
  ]) {
    testWidgets('${key.keyLabel}: hold records, release transcribes',
        (WidgetTester tester) async {
      await prepare(tester);
      await tester.sendKeyDownEvent(key);
      await tester.pumpAndSettle();
      await tester.sendKeyRepeatEvent(key);
      await tester.pumpAndSettle();
      expect(count('start'), 1);
      expect(count('stop'), 0);
      expect(
          calls
              .singleWhere((MethodCall call) => call.method == 'start')
              .arguments,
          containsPair('silenceDetection', false));
      await tester.sendKeyUpEvent(key);
      await tester.pumpAndSettle();
      expect(count('stop'), 1);
      await emit(<String, Object?>{
        'type': 'result',
        'event': 'final',
        'text': 'Hello from Select'
      });
      await setState('ready');
      await tester.pumpAndSettle();
      expect(find.text('Hello from Select'), findsOneWidget);
      await tester.sendKeyDownEvent(key);
      await tester.sendKeyUpEvent(key);
      await tester.pumpAndSettle();
      expect(count('start'), 2);
      expect(count('stop'), 2);
    });
  }

  for (final bool pendingReply in <bool>[true, false]) {
    testWidgets('early release is retained (pending reply: $pendingReply)',
        (WidgetTester tester) async {
      await prepare(tester);
      if (pendingReply) {
        startPending = Completer<void>();
      } else {
        delayRecording = true;
      }
      await tester.sendKeyDownEvent(LogicalKeyboardKey.select);
      await tester.pump();
      await tester.sendKeyUpEvent(LogicalKeyboardKey.select);
      await tester.pump();
      expect(count('stop'), 0);
      if (pendingReply) {
        startPending!.complete();
      } else {
        await setState('recording');
      }
      await tester.pumpAndSettle();
      expect(count('start'), 1);
      expect(count('stop'), 1);
    });
  }

  testWidgets('losing focus stops; leaving the app disposes the session',
      (WidgetTester tester) async {
    await prepare(tester);
    await tester.sendKeyDownEvent(LogicalKeyboardKey.select);
    await tester.pumpAndSettle();
    FocusManager.instance.primaryFocus!.unfocus();
    await tester.pumpAndSettle();
    expect(count('stop'), 1);
    await tester.sendKeyUpEvent(LogicalKeyboardKey.select);
    await setState('ready');
    await tester.pumpAndSettle();
    Focus.of(tester.element(find.text('Hold Select to record'))).requestFocus();
    await tester.pumpAndSettle();
    await tester.sendKeyDownEvent(LogicalKeyboardKey.select);
    await tester.pumpAndSettle();
    final int disposed = count('dispose');
    tester.binding.handleAppLifecycleStateChanged(AppLifecycleState.inactive);
    await tester.pumpAndSettle();
    expect(count('dispose'), disposed + 1);
    tester.binding.handleAppLifecycleStateChanged(AppLifecycleState.resumed);
    await tester.sendKeyUpEvent(LogicalKeyboardKey.select);
    await tester.pumpAndSettle();
    expect(count('stop'), 1);
  });

  testWidgets('engine completion while held does not restart or leave a stop',
      (WidgetTester tester) async {
    await prepare(tester);
    await tester.sendKeyDownEvent(LogicalKeyboardKey.select);
    await tester.pumpAndSettle();
    await setState('processing');
    await setState('ready');
    await tester.pumpAndSettle();
    await tester.sendKeyRepeatEvent(LogicalKeyboardKey.select);
    await tester.sendKeyUpEvent(LogicalKeyboardKey.select);
    await tester.pumpAndSettle();
    expect(count('start'), 1);
    expect(count('stop'), 0);
    await tester.sendKeyDownEvent(LogicalKeyboardKey.select);
    await tester.sendKeyUpEvent(LogicalKeyboardKey.select);
    await tester.pumpAndSettle();
    expect(count('start'), 2);
    expect(count('stop'), 1);
  });

  testWidgets('failed start reports the error and permits a new press',
      (WidgetTester tester) async {
    await prepare(tester);
    failStart = true;
    await tester.sendKeyDownEvent(LogicalKeyboardKey.select);
    await tester.sendKeyUpEvent(LogicalKeyboardKey.select);
    await tester.pumpAndSettle();
    expect(
        find.textContaining('STT_ERROR_NOT_SUPPORTED_FEATURE'), findsOneWidget);
    expect(count('stop'), 0);
    failStart = false;
    await tester.sendKeyDownEvent(LogicalKeyboardKey.select);
    await tester.sendKeyUpEvent(LogicalKeyboardKey.select);
    await tester.pumpAndSettle();
    expect(count('start'), 2);
    expect(count('stop'), 1);
  });
}
