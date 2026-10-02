// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';
import 'package:tizen_stt/tizen_stt.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();
  final TizenStt stt = TizenStt();

  setUp(() async {
    expect(await stt.initialize(), TizenSttState.ready);
  });

  tearDown(() async {
    await stt.dispose();
  });

  Future<void> expectTransition(
    Future<TizenSttState> Function() action,
    TizenSttState state,
  ) async {
    final Future<void> event = stt.events
        .timeout(const Duration(seconds: 10))
        .firstWhere(
            (TizenSttEvent event) => event.isError || event.state == state)
        .then((TizenSttEvent event) =>
            expect(event.state, state, reason: event.raw.toString()));
    await Future.wait<Object?>(<Future<Object?>>[event, action()]);
  }

  test('languages and session disposal', () async {
    expect(await stt.getLanguages(), isNotEmpty);
    expect(await stt.getState(), TizenSttState.ready);
    await stt.dispose();
    expect(await stt.getState(), TizenSttState.notCreated);
  });

  test('recording can be cancelled', () async {
    await expectTransition(stt.startListening, TizenSttState.recording);
    await expectTransition(stt.cancel, TizenSttState.ready);
    expect(await stt.getState(), TizenSttState.ready);
  });

  test('stopping recording starts processing', () async {
    await expectTransition(stt.startListening, TizenSttState.recording);
    await expectTransition(stt.stopListening, TizenSttState.processing);
  });
}
