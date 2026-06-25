// Copyright 2024 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:typed_data';

import 'package:flutter_test/flutter_test.dart';
import 'package:flutter_webrtc/flutter_webrtc.dart';
import 'package:integration_test/integration_test.dart';

const Map<String, dynamic> _kConfig = {
  'iceServers': <Map<String, dynamic>>[],
};

Future<void> _negotiate(
  RTCPeerConnection pc1,
  RTCPeerConnection pc2,
) async {
  pc1.onIceCandidate = (c) => pc2.addCandidate(c);
  pc2.onIceCandidate = (c) => pc1.addCandidate(c);

  final offer = await pc1.createOffer();
  await pc1.setLocalDescription(offer);
  await pc2.setRemoteDescription(offer);
  final answer = await pc2.createAnswer();
  await pc2.setLocalDescription(answer);
  await pc1.setRemoteDescription(answer);
}

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  group('RTCPeerConnection', () {
    testWidgets('creates with empty ice servers', (WidgetTester tester) async {
      final pc = await createPeerConnection(_kConfig);
      expect(pc, isNotNull);
      await pc.close();
    }, timeout: const Timeout(Duration(seconds: 10)));

    testWidgets('createOffer returns valid SDP', (WidgetTester tester) async {
      final pc = await createPeerConnection(_kConfig);
      final offer = await pc.createOffer();
      expect(offer.type, equals('offer'));
      expect(offer.sdp, isNotEmpty);
      await pc.close();
    }, timeout: const Timeout(Duration(seconds: 10)));

    testWidgets('setLocalDescription succeeds', (WidgetTester tester) async {
      final pc = await createPeerConnection(_kConfig);
      final offer = await pc.createOffer();
      await expectLater(pc.setLocalDescription(offer), completes);
      await pc.close();
    }, timeout: const Timeout(Duration(seconds: 10)));

    testWidgets('createAnswer returns valid SDP', (WidgetTester tester) async {
      final pc1 = await createPeerConnection(_kConfig);
      final pc2 = await createPeerConnection(_kConfig);
      final offer = await pc1.createOffer();
      await pc2.setRemoteDescription(offer);
      final answer = await pc2.createAnswer();
      expect(answer.type, equals('answer'));
      expect(answer.sdp, isNotEmpty);
      await pc1.close();
      await pc2.close();
    }, timeout: const Timeout(Duration(seconds: 10)));

    testWidgets('close is idempotent', (WidgetTester tester) async {
      final pc = await createPeerConnection(_kConfig);
      await pc.close();
      await expectLater(pc.close(), completes);
    }, timeout: const Timeout(Duration(seconds: 10)));
  });

  group('RTCDataChannel', () {
    testWidgets('createDataChannel returns channel with correct label',
        (WidgetTester tester) async {
      final pc = await createPeerConnection(_kConfig);
      final dc = await pc.createDataChannel('test-label', RTCDataChannelInit());
      expect(dc.label, equals('test-label'));
      await dc.close();
      await pc.close();
    }, timeout: const Timeout(Duration(seconds: 10)));

    testWidgets('loopback text message exchange', (WidgetTester tester) async {
      final pc1 = await createPeerConnection(_kConfig);
      final pc2 = await createPeerConnection(_kConfig);

      final dc2Completer = Completer<RTCDataChannel>();
      pc2.onDataChannel = (channel) {
        if (!dc2Completer.isCompleted) dc2Completer.complete(channel);
      };

      final dc1 = await pc1.createDataChannel(
        'loopback',
        RTCDataChannelInit()..ordered = true,
      );

      await _negotiate(pc1, pc2);

      final dc2 =
          await dc2Completer.future.timeout(const Duration(seconds: 10));

      final dc1Open = Completer<void>();
      final dc2Open = Completer<void>();
      dc1.onDataChannelState = (s) {
        if (s == RTCDataChannelState.RTCDataChannelOpen &&
            !dc1Open.isCompleted) {
          dc1Open.complete();
        }
      };
      dc2.onDataChannelState = (s) {
        if (s == RTCDataChannelState.RTCDataChannelOpen &&
            !dc2Open.isCompleted) {
          dc2Open.complete();
        }
      };

      // Channels may already be open if state change fired before callback set.
      await Future.wait([
        dc1Open.future.timeout(const Duration(seconds: 10),
            onTimeout: () {}),
        dc2Open.future.timeout(const Duration(seconds: 10),
            onTimeout: () {}),
      ]);

      final dc2Received = Completer<String>();
      dc2.onMessage = (msg) {
        if (!dc2Received.isCompleted) dc2Received.complete(msg.text);
      };
      await dc1.send(RTCDataChannelMessage('hello from dc1'));
      expect(
        await dc2Received.future.timeout(const Duration(seconds: 5)),
        equals('hello from dc1'),
      );

      final dc1Received = Completer<String>();
      dc1.onMessage = (msg) {
        if (!dc1Received.isCompleted) dc1Received.complete(msg.text);
      };
      await dc2.send(RTCDataChannelMessage('hello from dc2'));
      expect(
        await dc1Received.future.timeout(const Duration(seconds: 5)),
        equals('hello from dc2'),
      );

      await dc1.close();
      await dc2.close();
      await pc1.close();
      await pc2.close();
    }, timeout: const Timeout(Duration(seconds: 60)));

    testWidgets('loopback binary message exchange', (WidgetTester tester) async {
      final pc1 = await createPeerConnection(_kConfig);
      final pc2 = await createPeerConnection(_kConfig);

      final dc2Completer = Completer<RTCDataChannel>();
      pc2.onDataChannel = (channel) {
        if (!dc2Completer.isCompleted) dc2Completer.complete(channel);
      };

      final dc1 = await pc1.createDataChannel(
        'binary',
        RTCDataChannelInit()..ordered = true,
      );

      await _negotiate(pc1, pc2);

      final dc2 =
          await dc2Completer.future.timeout(const Duration(seconds: 10));

      final dc1Open = Completer<void>();
      final dc2Open = Completer<void>();
      dc1.onDataChannelState = (s) {
        if (s == RTCDataChannelState.RTCDataChannelOpen &&
            !dc1Open.isCompleted) {
          dc1Open.complete();
        }
      };
      dc2.onDataChannelState = (s) {
        if (s == RTCDataChannelState.RTCDataChannelOpen &&
            !dc2Open.isCompleted) {
          dc2Open.complete();
        }
      };

      await Future.wait([
        dc1Open.future.timeout(const Duration(seconds: 10),
            onTimeout: () {}),
        dc2Open.future.timeout(const Duration(seconds: 10),
            onTimeout: () {}),
      ]);

      final payload = Uint8List.fromList([1, 2, 3, 4, 5]);

      final dc2Received = Completer<Uint8List>();
      dc2.onMessage = (msg) {
        if (!dc2Received.isCompleted) dc2Received.complete(msg.binary);
      };
      await dc1.send(RTCDataChannelMessage.fromBinary(payload));
      expect(
        await dc2Received.future.timeout(const Duration(seconds: 5)),
        equals(payload),
      );

      await dc1.close();
      await dc2.close();
      await pc1.close();
      await pc2.close();
    }, timeout: const Timeout(Duration(seconds: 60)));
  });
}
