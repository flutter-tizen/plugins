// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:flutter/services.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:tizen_stt/tizen_stt.dart';

void main() {
  TestWidgetsFlutterBinding.ensureInitialized();
  final TestDefaultBinaryMessenger messenger =
      TestDefaultBinaryMessengerBinding.instance.defaultBinaryMessenger;
  const MethodChannel channel = MethodChannel('tizen_stt');
  const MethodChannel eventChannel = MethodChannel('tizen_stt/events');
  final TizenStt stt = TizenStt();
  final List<MethodCall> calls = <MethodCall>[];

  setUp(() {
    calls.clear();
    messenger.setMockMethodCallHandler(channel, (MethodCall call) async {
      calls.add(call);
      if (call.method == 'getLanguages') {
        return <String>['en_US', 'ko_KR'];
      }
      if (call.method == 'dispose') {
        return null;
      }
      return <String, Object?>{
        'state': <String, Object?>{'name': 'ready'}
      };
    });
    messenger.setMockMethodCallHandler(
        eventChannel, (MethodCall call) async => null);
  });
  tearDown(() {
    messenger.setMockMethodCallHandler(channel, null);
    messenger.setMockMethodCallHandler(eventChannel, null);
  });

  test('control calls forward arguments and decode state and languages',
      () async {
    expect(await stt.initialize(), TizenSttState.ready);
    expect(
        await stt.startListening(
            language: 'ko_KR',
            recognitionType: TizenSttRecognitionType.freePartial),
        TizenSttState.ready);
    expect(calls.last.arguments, <String, Object?>{
      'language': 'ko_KR',
      'type': TizenSttRecognitionType.freePartial
    });
    await stt.startListening();
    expect(calls.last.arguments, <String, Object?>{
      'language': null,
      'type': TizenSttRecognitionType.free
    });
    await stt.stopListening();
    await stt.cancel();
    await stt.getState();
    expect(await stt.getLanguages(), <String>['en_US', 'ko_KR']);
    await stt.dispose();
    expect(calls.map((MethodCall call) => call.method), <String>[
      'initialize',
      'start',
      'start',
      'stop',
      'cancel',
      'getState',
      'getLanguages',
      'dispose'
    ]);
  });

  test('initialization stays pending and propagates native failure', () async {
    final Completer<Object?> ready = Completer<Object?>();
    messenger.setMockMethodCallHandler(
        channel, (MethodCall call) => ready.future);
    bool completed = false;
    final Future<TizenSttState> initializing =
        stt.initialize().then((TizenSttState state) {
      completed = true;
      return state;
    });
    await Future<void>.delayed(Duration.zero);
    expect(completed, isFalse);
    final Future<void> expectation = expectLater(
        initializing,
        throwsA(isA<PlatformException>().having((PlatformException e) => e.code,
            'code', 'STT_ERROR_ENGINE_NOT_FOUND')));
    ready.completeError(PlatformException(code: 'STT_ERROR_ENGINE_NOT_FOUND'));
    await expectation;
  });

  test('shared event stream decodes state, results and both error forms',
      () async {
    final List<TizenSttEvent> events = <TizenSttEvent>[];
    final StreamSubscription<TizenSttEvent> first =
        stt.events.listen(events.add);
    final List<TizenSttEvent> otherEvents = <TizenSttEvent>[];
    final StreamSubscription<TizenSttEvent> second =
        TizenStt().events.listen(otherEvents.add);
    await Future<void>.delayed(Duration.zero);
    for (final Map<String, Object?> event in <Map<String, Object?>>[
      <String, Object?>{
        'type': 'state',
        'current': <String, Object?>{'name': 'ready'}
      },
      <String, Object?>{'type': 'result', 'event': 'partial', 'text': 'hello'},
      <String, Object?>{
        'type': 'result',
        'event': 'final',
        'text': 'hello world'
      },
      <String, Object?>{
        'type': 'result',
        'event': 'error',
        'message': 'no match'
      },
      <String, Object?>{'type': 'error', 'name': 'STT_ERROR_NO_SPEECH'},
    ]) {
      final ByteData data =
          const StandardMethodCodec().encodeSuccessEnvelope(event);
      await messenger.handlePlatformMessage('tizen_stt/events', data, (_) {});
    }
    expect(events.first.state, TizenSttState.ready);
    expect(events[1].isPartialResult, isTrue);
    expect(events[2].isFinalResult, isTrue);
    expect(events[2].text, 'hello world');
    expect(events[3].isError, isTrue);
    expect(events[4].errorName, 'STT_ERROR_NO_SPEECH');
    expect(otherEvents.length, events.length);
    await first.cancel();
    await second.cancel();
  });
}
