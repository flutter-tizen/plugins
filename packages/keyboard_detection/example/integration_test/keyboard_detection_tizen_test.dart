// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter/services.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';
import 'package:keyboard_detection_tizen/keyboard_detection_tizen.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  const String channelName = 'tizen/internal/inputpanel';
  const StandardMethodCodec codec = StandardMethodCodec();

  Future<void> emit(WidgetTester tester, Map<String, Object?> payload) async {
    final ByteData data = codec.encodeSuccessEnvelope(payload);
    await tester.binding.defaultBinaryMessenger.handlePlatformMessage(
      channelName,
      data,
      (_) {},
    );
  }

  testWidgets('reports visible on show event', (WidgetTester tester) async {
    final KeyboardDetectionController controller =
        KeyboardDetectionController();
    await emit(tester, <String, Object?>{'state': 'show'});
    await tester.pump();
    expect(controller.state, KeyboardState.visible);
    expect(controller.stateAsBool(), isTrue);
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
}
