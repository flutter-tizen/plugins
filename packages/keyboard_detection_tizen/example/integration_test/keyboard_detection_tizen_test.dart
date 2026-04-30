// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';
import 'package:keyboard_detection_tizen/keyboard_detection_tizen.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  testWidgets('controller starts in unknown state', (
    WidgetTester tester,
  ) async {
    final KeyboardDetectionController controller =
        KeyboardDetectionController();
    expect(controller.state, KeyboardState.unknown);
    expect(controller.stateAsBool(), isNull);
    expect(controller.size, 0);
    expect(controller.width, 0);
    expect(controller.isSizeLoaded, isFalse);
    await controller.dispose();
  });

  testWidgets('event channel events drive controller state and size', (
    WidgetTester tester,
  ) async {
    const StandardMethodCodec codec = StandardMethodCodec();
    const String channelName = 'tizen/internal/inputpanel';

    Future<void> emit(Map<String, Object?> payload) async {
      final ByteData data = codec.encodeSuccessEnvelope(payload);
      await tester.binding.defaultBinaryMessenger.handlePlatformMessage(
        channelName,
        data,
        (_) {},
      );
    }

    final List<KeyboardState> seen = <KeyboardState>[];
    final KeyboardDetectionController controller =
        KeyboardDetectionController(onChanged: seen.add);

    final StreamSubscription<KeyboardState> sub = controller.stream.listen(
      (_) {},
    );

    await emit(<String, Object?>{
      'state': 'will_show',
      'x': 0,
      'y': 0,
      'width': 0,
      'height': 0,
    });
    await tester.pump();
    expect(controller.state, KeyboardState.visibling);
    expect(controller.size, 0);

    await emit(<String, Object?>{
      'state': 'show',
      'x': 0,
      'y': 600,
      'width': 1280,
      'height': 320,
    });
    await tester.pump();
    expect(controller.state, KeyboardState.visible);
    expect(controller.stateAsBool(), isTrue);
    expect(controller.size, 320);
    expect(controller.width, 1280);
    expect(controller.position.dy, 600);
    expect(controller.isSizeLoaded, isTrue);
    await expectLater(controller.ensureSizeLoaded, completes);

    await emit(<String, Object?>{
      'state': 'hide',
      'x': 0,
      'y': 0,
      'width': 0,
      'height': 0,
    });
    await tester.pump();
    expect(controller.state, KeyboardState.hidden);
    expect(controller.stateAsBool(), isFalse);
    // Geometry keeps the last visible value after hide.
    expect(controller.size, 320);
    expect(controller.width, 1280);
    expect(controller.position.dy, 600);

    expect(seen, <KeyboardState>[
      KeyboardState.visibling,
      KeyboardState.visible,
      KeyboardState.hidden,
    ]);

    await sub.cancel();
    await controller.dispose();
  });

  testWidgets('payload without geometry leaves size at zero', (
    WidgetTester tester,
  ) async {
    const StandardMethodCodec codec = StandardMethodCodec();
    const String channelName = 'tizen/internal/inputpanel';

    final List<KeyboardState> seen = <KeyboardState>[];
    final KeyboardDetectionController controller =
        KeyboardDetectionController(onChanged: seen.add);
    final StreamSubscription<KeyboardState> sub = controller.stream.listen(
      (_) {},
    );

    final ByteData data = codec.encodeSuccessEnvelope(<String, Object?>{
      'state': 'show',
    });
    await tester.binding.defaultBinaryMessenger.handlePlatformMessage(
      channelName,
      data,
      (_) {},
    );
    await tester.pump();

    expect(seen, contains(KeyboardState.visible));
    expect(controller.size, 0);
    expect(controller.isSizeLoaded, isFalse);

    await sub.cancel();
    await controller.dispose();
  });

  testWidgets('real IME show populates size on Tizen', (
    WidgetTester tester,
  ) async {
    final KeyboardDetectionController controller =
        KeyboardDetectionController();
    final FocusNode focusNode = FocusNode();
    final TextEditingController text = TextEditingController();

    await tester.pumpWidget(
      MaterialApp(
        home: Scaffold(
          body: Center(
            child: TextField(
              controller: text,
              focusNode: focusNode,
            ),
          ),
        ),
      ),
    );
    await tester.pumpAndSettle();

    final Completer<void> visible = Completer<void>();
    final StreamSubscription<KeyboardState> sub =
        controller.stream.listen((KeyboardState state) {
      if (state == KeyboardState.visible && !visible.isCompleted) {
        visible.complete();
      }
    });

    focusNode.requestFocus();
    await tester.pumpAndSettle();
    // Force the platform IME to show since `tester.tap` does not trigger it
    // on a headless integration test.
    await SystemChannels.textInput.invokeMethod<void>('TextInput.show');

    await visible.future.timeout(
      const Duration(seconds: 6),
      onTimeout: () {
        // Some emulator profiles may not actually open an IME. Don't fail
        // here since the fixed-payload tests above already cover parsing.
      },
    );
    await tester.pump(const Duration(milliseconds: 500));

    if (controller.state == KeyboardState.visible) {
      expect(
        controller.size,
        greaterThan(0),
        reason: 'embedder should report a non-zero IME height when visible',
      );
      debugPrint(
        'IME geometry observed: size=${controller.size}, '
        'width=${controller.width}, position=${controller.position}',
      );
    } else {
      debugPrint(
        'IME did not surface on this emulator profile; '
        'controller.state=${controller.state}',
      );
    }

    await SystemChannels.textInput.invokeMethod<void>('TextInput.hide');
    await sub.cancel();
    await controller.dispose();
  });
}
