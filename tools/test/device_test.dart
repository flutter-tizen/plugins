// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:io';

import 'package:file/file.dart';
import 'package:file/memory.dart';
import 'package:flutter_plugin_tools/src/common/package_looping_command.dart';
import 'package:flutter_plugin_tools/src/common/process_runner.dart';
import 'package:flutter_tizen_plugin_tools/src/device.dart';
import 'package:flutter_tizen_plugin_tools/src/tizen_sdk.dart';
import 'package:mocktail/mocktail.dart';
import 'package:test/test.dart';

class MockProcessRunner extends Mock implements ProcessRunner {}

class MockProcess extends Mock implements Process {}

class MockTizenSdk extends Mock implements TizenSdk {}

void main() {
  group('log test:', () {
    late ProcessRunner processRunner;
    late Device device;
    late FileSystem fileSystem;
    late Process process;
    late StreamController<String> controller;
    late Completer<int> completer;
    late TizenSdk tizenSdk;

    const Duration timeoutLimit = Duration(seconds: 3);

    setUp(() {
      processRunner = MockProcessRunner();
      fileSystem = MemoryFileSystem();
      process = MockProcess();
      controller = StreamController<String>();
      completer = Completer<int>();
      when(() => process.stdout).thenAnswer((_) =>
          controller.stream.map<List<int>>((String event) => event.codeUnits));
      when(() => process.exitCode).thenAnswer((_) => completer.future);
      when(() => processRunner.start(any(), any(),
              workingDirectory: any(named: 'workingDirectory')))
          .thenAnswer((_) => Future<Process>(() => process));
      tizenSdk = MockTizenSdk();
      when(() => tizenSdk.sdbDevices()).thenReturn(<SdbDeviceInfo>[
        const SdbDeviceInfo(
          id: 'some_id',
          status: 'device',
          name: 'some_name',
        )
      ]);
      device = Device.physical(
        'some_name',
        Profile.fromString('wearable-5.5'),
        tizenSdk: tizenSdk,
        processRunner: processRunner,
      );
    });

    test('timeout when stdout is silent.', () async {
      final PackageResult result = await device.runIntegrationTest(
        fileSystem.systemTempDirectory,
        timeoutLimit,
      );
      expect(result.state, RunState.failed);
      expect(result.details.first.contains('Timeout expired'), true);
    });

    test('timeout when stdout doesn\'t finish.', () async {
      final Timer timer = Timer.periodic(
        const Duration(seconds: 1),
        (Timer timer) => controller.add('some log'),
      );
      final PackageResult result = await device.runIntegrationTest(
        fileSystem.systemTempDirectory,
        timeoutLimit,
      );
      timer.cancel();
      expect(result.state, RunState.failed);
      expect(result.details.first.contains('Timeout expired'), true);
    });

    test('correctly parses log \'No tests ran\'', () async {
      Future<void>.delayed(
        const Duration(seconds: 1),
        () {
          controller.add('No tests ran');
          controller.close();
          completer.complete(0);
        },
      );
      final PackageResult result = await device.runIntegrationTest(
        fileSystem.systemTempDirectory,
        timeoutLimit,
      );
      expect(result.state, RunState.failed);
      expect(result.details.first.contains('Missing integration tests'), true);
    });

    test('correctly parses log \'No devices found\'', () async {
      Future<void>.delayed(
        const Duration(seconds: 1),
        () {
          controller.add('No devices found');
          controller.close();
          completer.complete(0);
        },
      );
      final PackageResult result = await device.runIntegrationTest(
        fileSystem.systemTempDirectory,
        timeoutLimit,
      );
      expect(result.state, RunState.failed);
      expect(
          result.details.first.contains('Device was disconnected during test'),
          true);
    });

    test('correctly parses log \'All tests passed!\'', () async {
      Future<void>.delayed(
        const Duration(seconds: 1),
        () {
          controller.add('00:00 +0: All tests passed!');
          controller.close();
          completer.complete(0);
        },
      );
      final PackageResult result = await device.runIntegrationTest(
        fileSystem.systemTempDirectory,
        timeoutLimit,
      );
      expect(result.state, RunState.succeeded);
    });

    test('correctly parses log \'Some tests failed\'', () async {
      Future<void>.delayed(
        const Duration(seconds: 1),
        () {
          controller.add('00:00 +0 -1: Some tests failed');
          controller.close();
          completer.complete(0);
        },
      );
      final PackageResult result = await device.runIntegrationTest(
        fileSystem.systemTempDirectory,
        timeoutLimit,
      );
      expect(result.state, RunState.failed);
      expect(
        result.details.first
            .contains('flutter-tizen test integration_test failed'),
        true,
      );
    });
  });
}
