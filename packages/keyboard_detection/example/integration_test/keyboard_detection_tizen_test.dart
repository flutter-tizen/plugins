// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:flutter/services.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';
import 'package:keyboard_detection_tizen/keyboard_detection_tizen.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  const String channelName = 'tizen/internal/inputpanel';
  const StandardMethodCodec codec = StandardMethodCodec();

  // Injects an event into the input-panel [EventChannel] the controller
  // listens to, simulating a message coming from the flutter-tizen embedder.
  Future<void> emit(WidgetTester tester, Object? payload) async {
    final ByteData data = codec.encodeSuccessEnvelope(payload);
    await tester.binding.defaultBinaryMessenger.handlePlatformMessage(
      channelName,
      data,
      (_) {},
    );
  }

  group('state reporting', () {
    testWidgets('starts in the unknown state', (WidgetTester tester) async {
      final KeyboardDetectionController controller =
          KeyboardDetectionController();
      expect(controller.state, KeyboardState.unknown);
      expect(controller.stateAsBool(), isNull);
      await controller.dispose();
    });

    testWidgets('reports visibling on will_show event',
        (WidgetTester tester) async {
      final KeyboardDetectionController controller =
          KeyboardDetectionController();
      await emit(tester, <String, Object?>{'state': 'will_show'});
      await tester.pump();
      expect(controller.state, KeyboardState.visibling);
      expect(controller.stateAsBool(), isFalse);
      expect(controller.stateAsBool(true), isTrue);
      await controller.dispose();
    });

    testWidgets('reports visible on show event', (WidgetTester tester) async {
      final KeyboardDetectionController controller =
          KeyboardDetectionController();
      await emit(tester, <String, Object?>{'state': 'show'});
      await tester.pump();
      expect(controller.state, KeyboardState.visible);
      expect(controller.stateAsBool(), isTrue);
      await controller.dispose();
    });

    testWidgets('reports hiding on will_hide event',
        (WidgetTester tester) async {
      final KeyboardDetectionController controller =
          KeyboardDetectionController();
      await emit(tester, <String, Object?>{'state': 'show'});
      await tester.pump();
      await emit(tester, <String, Object?>{'state': 'will_hide'});
      await tester.pump();
      expect(controller.state, KeyboardState.hiding);
      expect(controller.stateAsBool(), isTrue);
      expect(controller.stateAsBool(true), isFalse);
      await controller.dispose();
    });

    testWidgets('reports hidden on hide event', (WidgetTester tester) async {
      final KeyboardDetectionController controller =
          KeyboardDetectionController();
      await emit(tester, <String, Object?>{'state': 'show'});
      await tester.pump();
      await emit(tester, <String, Object?>{'state': 'hide'});
      await tester.pump();
      expect(controller.state, KeyboardState.hidden);
      expect(controller.stateAsBool(), isFalse);
      await controller.dispose();
    });

    testWidgets('falls back to unknown for an unrecognized event',
        (WidgetTester tester) async {
      final KeyboardDetectionController controller =
          KeyboardDetectionController();
      await emit(tester, <String, Object?>{'state': 'show'});
      await tester.pump();
      await emit(tester, <String, Object?>{'state': 'something_else'});
      await tester.pump();
      expect(controller.state, KeyboardState.unknown);
      await controller.dispose();
    });

    testWidgets('ignores events without a valid state field',
        (WidgetTester tester) async {
      final KeyboardDetectionController controller =
          KeyboardDetectionController();
      // Reach a known state first, so the assertions verify that the invalid
      // events are ignored (state preserved) rather than merely matching the
      // initial unknown state.
      await emit(tester, <String, Object?>{'state': 'show'});
      await tester.pump();
      expect(controller.state, KeyboardState.visible);
      await emit(tester, <String, Object?>{'noState': true});
      await tester.pump();
      expect(controller.state, KeyboardState.visible);
      await emit(tester, <String, Object?>{'state': 123});
      await tester.pump();
      expect(controller.state, KeyboardState.visible);
      await controller.dispose();
    });
  });

  group('keyboard metrics', () {
    testWidgets('size, width and position are zero before any event',
        (WidgetTester tester) async {
      final KeyboardDetectionController controller =
          KeyboardDetectionController();
      expect(controller.size, 0);
      expect(controller.width, 0);
      expect(controller.position, Offset.zero);
      expect(controller.isSizeLoaded, isFalse);
      await controller.dispose();
    });

    testWidgets('updates metrics from a show event carrying dimensions',
        (WidgetTester tester) async {
      final KeyboardDetectionController controller =
          KeyboardDetectionController();
      await emit(tester, <String, Object?>{
        'state': 'show',
        'width': 1080.0,
        'height': 420.0,
        'x': 0.0,
        'y': 1500.0,
      });
      await tester.pump();
      expect(controller.width, 1080.0);
      expect(controller.size, 420.0);
      expect(controller.position, const Offset(0, 1500));
      expect(controller.isSizeLoaded, isTrue);
      await controller.dispose();
    });

    testWidgets('ensureSizeLoaded completes once metrics arrive',
        (WidgetTester tester) async {
      final KeyboardDetectionController controller =
          KeyboardDetectionController();
      final Future<void> sizeLoaded = controller.ensureSizeLoaded;
      await emit(tester, <String, Object?>{
        'state': 'show',
        'width': 1080.0,
        'height': 420.0,
      });
      await tester.pump();
      await expectLater(sizeLoaded, completes);
      expect(controller.isSizeLoaded, isTrue);
      await controller.dispose();
    });

    testWidgets('resets metrics to zero on hide', (WidgetTester tester) async {
      final KeyboardDetectionController controller =
          KeyboardDetectionController();
      await emit(tester, <String, Object?>{
        'state': 'show',
        'width': 1080.0,
        'height': 420.0,
        'x': 0.0,
        'y': 1500.0,
      });
      await tester.pump();
      await emit(tester, <String, Object?>{'state': 'hide'});
      await tester.pump();
      expect(controller.width, 0);
      expect(controller.size, 0);
      expect(controller.position, Offset.zero);
      await controller.dispose();
    });
  });

  group('notifications', () {
    testWidgets('stream emits state changes in order',
        (WidgetTester tester) async {
      final KeyboardDetectionController controller =
          KeyboardDetectionController();
      final List<KeyboardState> seen = <KeyboardState>[];
      final StreamSubscription<KeyboardState> subscription =
          controller.stream.listen(seen.add);
      await emit(tester, <String, Object?>{'state': 'will_show'});
      await tester.pump();
      await emit(tester, <String, Object?>{'state': 'show'});
      await tester.pump();
      await emit(tester, <String, Object?>{'state': 'hide'});
      await tester.pump();
      expect(seen, <KeyboardState>[
        KeyboardState.visibling,
        KeyboardState.visible,
        KeyboardState.hidden,
      ]);
      await subscription.cancel();
      await controller.dispose();
    });

    testWidgets('onChanged is invoked on every state change',
        (WidgetTester tester) async {
      final List<KeyboardState> seen = <KeyboardState>[];
      final KeyboardDetectionController controller =
          KeyboardDetectionController(onChanged: seen.add);
      await emit(tester, <String, Object?>{'state': 'show'});
      await tester.pump();
      await emit(tester, <String, Object?>{'state': 'hide'});
      await tester.pump();
      expect(seen, <KeyboardState>[
        KeyboardState.visible,
        KeyboardState.hidden,
      ]);
      await controller.dispose();
    });
  });

  group('registered callbacks', () {
    testWidgets('a registered callback receives state changes',
        (WidgetTester tester) async {
      final KeyboardDetectionController controller =
          KeyboardDetectionController();
      final List<KeyboardState> seen = <KeyboardState>[];
      controller.registerCallback((KeyboardState state) {
        seen.add(state);
        return true;
      });
      await emit(tester, <String, Object?>{'state': 'show'});
      await tester.pump();
      await emit(tester, <String, Object?>{'state': 'hide'});
      await tester.pump();
      expect(seen, <KeyboardState>[
        KeyboardState.visible,
        KeyboardState.hidden,
      ]);
      await controller.dispose();
    });

    testWidgets('a callback returning false unregisters itself',
        (WidgetTester tester) async {
      final KeyboardDetectionController controller =
          KeyboardDetectionController();
      int calls = 0;
      controller.registerCallback((KeyboardState state) {
        calls++;
        return false;
      });
      await emit(tester, <String, Object?>{'state': 'show'});
      await tester.pump();
      await emit(tester, <String, Object?>{'state': 'hide'});
      await tester.pump();
      expect(calls, 1);
      await controller.dispose();
    });

    testWidgets('unregisterCallback stops further invocations',
        (WidgetTester tester) async {
      final KeyboardDetectionController controller =
          KeyboardDetectionController();
      final List<KeyboardState> seen = <KeyboardState>[];
      bool callback(KeyboardState state) {
        seen.add(state);
        return true;
      }

      controller.registerCallback(callback);
      await emit(tester, <String, Object?>{'state': 'show'});
      await tester.pump();
      controller.unregisterCallback(callback);
      await emit(tester, <String, Object?>{'state': 'hide'});
      await tester.pump();
      expect(seen, <KeyboardState>[KeyboardState.visible]);
      await controller.dispose();
    });

    testWidgets('unregisterAllCallbacks removes every callback',
        (WidgetTester tester) async {
      final KeyboardDetectionController controller =
          KeyboardDetectionController();
      int first = 0;
      int second = 0;
      controller.registerCallback((KeyboardState state) {
        first++;
        return true;
      });
      controller.registerCallback((KeyboardState state) {
        second++;
        return true;
      });
      await emit(tester, <String, Object?>{'state': 'show'});
      await tester.pump();
      controller.unregisterAllCallbacks();
      await emit(tester, <String, Object?>{'state': 'hide'});
      await tester.pump();
      expect(first, 1);
      expect(second, 1);
      await controller.dispose();
    });
  });

  group('lifecycle', () {
    testWidgets('does not report state changes after dispose',
        (WidgetTester tester) async {
      final KeyboardDetectionController controller =
          KeyboardDetectionController();
      await emit(tester, <String, Object?>{'state': 'show'});
      await tester.pump();
      expect(controller.state, KeyboardState.visible);
      await controller.dispose();
      await emit(tester, <String, Object?>{'state': 'hide'});
      await tester.pump();
      expect(controller.state, KeyboardState.visible);
    });
  });
}
