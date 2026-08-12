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

  Future<void> emit(Map<String, Object?> payload) async {
    final ByteData data = codec.encodeSuccessEnvelope(payload);
    await TestDefaultBinaryMessengerBinding.instance.defaultBinaryMessenger
        .handlePlatformMessage(
      channelName,
      data,
      (_) {},
    );
  }

  group('state reporting', () {
    test('starts in the unknown state', () async {
      final KeyboardDetectionController controller =
          KeyboardDetectionController();
      expect(controller.state, KeyboardState.unknown);
      expect(controller.stateAsBool(), isNull);
      await controller.dispose();
    });

    test('reports visibling on will_show event', () async {
      final KeyboardDetectionController controller =
          KeyboardDetectionController();
      await emit(<String, Object?>{'state': 'will_show'});
      await Future<void>.delayed(Duration.zero);
      expect(controller.state, KeyboardState.visibling);
      expect(controller.stateAsBool(), isFalse);
      expect(controller.stateAsBool(true), isTrue);
      await controller.dispose();
    });

    test('reports visible on show event', () async {
      final KeyboardDetectionController controller =
          KeyboardDetectionController();
      await emit(<String, Object?>{'state': 'show'});
      await Future<void>.delayed(Duration.zero);
      expect(controller.state, KeyboardState.visible);
      expect(controller.stateAsBool(), isTrue);
      await controller.dispose();
    });

    test('reports hiding on will_hide event', () async {
      final KeyboardDetectionController controller =
          KeyboardDetectionController();
      await emit(<String, Object?>{'state': 'show'});
      await Future<void>.delayed(Duration.zero);
      await emit(<String, Object?>{'state': 'will_hide'});
      await Future<void>.delayed(Duration.zero);
      expect(controller.state, KeyboardState.hiding);
      expect(controller.stateAsBool(), isTrue);
      expect(controller.stateAsBool(true), isFalse);
      await controller.dispose();
    });

    test('reports hidden on hide event', () async {
      final KeyboardDetectionController controller =
          KeyboardDetectionController();
      await emit(<String, Object?>{'state': 'show'});
      await Future<void>.delayed(Duration.zero);
      await emit(<String, Object?>{'state': 'hide'});
      await Future<void>.delayed(Duration.zero);
      expect(controller.state, KeyboardState.hidden);
      expect(controller.stateAsBool(), isFalse);
      await controller.dispose();
    });

    test('falls back to unknown for an unrecognized event', () async {
      final KeyboardDetectionController controller =
          KeyboardDetectionController();
      await emit(<String, Object?>{'state': 'show'});
      await Future<void>.delayed(Duration.zero);
      await emit(<String, Object?>{'state': 'something_else'});
      await Future<void>.delayed(Duration.zero);
      expect(controller.state, KeyboardState.unknown);
      await controller.dispose();
    });

    test('ignores events without a valid state field', () async {
      final KeyboardDetectionController controller =
          KeyboardDetectionController();
      // Reach a known state first, so the assertions verify that the invalid
      // events are ignored (state preserved) rather than merely matching the
      // initial unknown state.
      await emit(<String, Object?>{'state': 'show'});
      await Future<void>.delayed(Duration.zero);
      expect(controller.state, KeyboardState.visible);
      await emit(<String, Object?>{'noState': true});
      await Future<void>.delayed(Duration.zero);
      expect(controller.state, KeyboardState.visible);
      await emit(<String, Object?>{'state': 123});
      await Future<void>.delayed(Duration.zero);
      expect(controller.state, KeyboardState.visible);
      await controller.dispose();
    });
  });

  group('keyboard metrics', () {
    test('size, width and position are zero before any event', () async {
      final KeyboardDetectionController controller =
          KeyboardDetectionController();
      expect(controller.size, 0);
      expect(controller.width, 0);
      expect(controller.position, Offset.zero);
      expect(controller.isSizeLoaded, isFalse);
      await controller.dispose();
    });

    test('updates metrics from a show event carrying dimensions', () async {
      final KeyboardDetectionController controller =
          KeyboardDetectionController();
      await emit(<String, Object?>{
        'state': 'show',
        'width': 1080.0,
        'height': 420.0,
        'x': 0.0,
        'y': 1500.0,
      });
      await Future<void>.delayed(Duration.zero);
      expect(controller.width, 1080.0);
      expect(controller.size, 420.0);
      expect(controller.position, const Offset(0, 1500));
      expect(controller.isSizeLoaded, isTrue);
      await controller.dispose();
    });

    test('ensureSizeLoaded completes once metrics arrive', () async {
      final KeyboardDetectionController controller =
          KeyboardDetectionController();
      final Future<void> sizeLoaded = controller.ensureSizeLoaded;
      await emit(<String, Object?>{
        'state': 'show',
        'width': 1080.0,
        'height': 420.0,
      });
      await Future<void>.delayed(Duration.zero);
      await expectLater(sizeLoaded, completes);
      expect(controller.isSizeLoaded, isTrue);
      await controller.dispose();
    });

    test('resets metrics to zero on hide', () async {
      final KeyboardDetectionController controller =
          KeyboardDetectionController();
      await emit(<String, Object?>{
        'state': 'show',
        'width': 1080.0,
        'height': 420.0,
        'x': 0.0,
        'y': 1500.0,
      });
      await Future<void>.delayed(Duration.zero);
      await emit(<String, Object?>{'state': 'hide'});
      await Future<void>.delayed(Duration.zero);
      expect(controller.width, 0);
      expect(controller.size, 0);
      expect(controller.position, Offset.zero);
      await controller.dispose();
    });
  });

  group('notifications', () {
    test('stream emits state changes in order', () async {
      final KeyboardDetectionController controller =
          KeyboardDetectionController();
      final List<KeyboardState> seen = <KeyboardState>[];
      final StreamSubscription<KeyboardState> subscription =
          controller.stream.listen(seen.add);
      await emit(<String, Object?>{'state': 'will_show'});
      await Future<void>.delayed(Duration.zero);
      await emit(<String, Object?>{'state': 'show'});
      await Future<void>.delayed(Duration.zero);
      await emit(<String, Object?>{'state': 'hide'});
      await Future<void>.delayed(Duration.zero);
      expect(seen, <KeyboardState>[
        KeyboardState.visibling,
        KeyboardState.visible,
        KeyboardState.hidden,
      ]);
      await subscription.cancel();
      await controller.dispose();
    });

    test('onChanged is invoked on every state change', () async {
      final List<KeyboardState> seen = <KeyboardState>[];
      final KeyboardDetectionController controller =
          KeyboardDetectionController(onChanged: seen.add);
      await emit(<String, Object?>{'state': 'show'});
      await Future<void>.delayed(Duration.zero);
      await emit(<String, Object?>{'state': 'hide'});
      await Future<void>.delayed(Duration.zero);
      expect(seen, <KeyboardState>[
        KeyboardState.visible,
        KeyboardState.hidden,
      ]);
      await controller.dispose();
    });
  });

  group('registered callbacks', () {
    test('a registered callback receives state changes', () async {
      final KeyboardDetectionController controller =
          KeyboardDetectionController();
      final List<KeyboardState> seen = <KeyboardState>[];
      controller.registerCallback((KeyboardState state) {
        seen.add(state);
        return true;
      });
      await emit(<String, Object?>{'state': 'show'});
      await Future<void>.delayed(Duration.zero);
      await emit(<String, Object?>{'state': 'hide'});
      await Future<void>.delayed(Duration.zero);
      expect(seen, <KeyboardState>[
        KeyboardState.visible,
        KeyboardState.hidden,
      ]);
      await controller.dispose();
    });

    test('a callback returning false unregisters itself', () async {
      final KeyboardDetectionController controller =
          KeyboardDetectionController();
      int calls = 0;
      controller.registerCallback((KeyboardState state) {
        calls++;
        return false;
      });
      await emit(<String, Object?>{'state': 'show'});
      await Future<void>.delayed(Duration.zero);
      await emit(<String, Object?>{'state': 'hide'});
      await Future<void>.delayed(Duration.zero);
      expect(calls, 1);
      await controller.dispose();
    });

    test('unregisterCallback stops further invocations', () async {
      final KeyboardDetectionController controller =
          KeyboardDetectionController();
      final List<KeyboardState> seen = <KeyboardState>[];
      bool callback(KeyboardState state) {
        seen.add(state);
        return true;
      }

      controller.registerCallback(callback);
      await emit(<String, Object?>{'state': 'show'});
      await Future<void>.delayed(Duration.zero);
      controller.unregisterCallback(callback);
      await emit(<String, Object?>{'state': 'hide'});
      await Future<void>.delayed(Duration.zero);
      expect(seen, <KeyboardState>[KeyboardState.visible]);
      await controller.dispose();
    });

    test('unregisterAllCallbacks removes every callback', () async {
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
      await emit(<String, Object?>{'state': 'show'});
      await Future<void>.delayed(Duration.zero);
      controller.unregisterAllCallbacks();
      await emit(<String, Object?>{'state': 'hide'});
      await Future<void>.delayed(Duration.zero);
      expect(first, 1);
      expect(second, 1);
      await controller.dispose();
    });
  });

  group('lifecycle', () {
    test('does not report state changes after dispose', () async {
      final KeyboardDetectionController controller =
          KeyboardDetectionController();
      await emit(<String, Object?>{'state': 'show'});
      await Future<void>.delayed(Duration.zero);
      expect(controller.state, KeyboardState.visible);
      await controller.dispose();
      await emit(<String, Object?>{'state': 'hide'});
      await Future<void>.delayed(Duration.zero);
      expect(controller.state, KeyboardState.visible);
    });
  });
}
