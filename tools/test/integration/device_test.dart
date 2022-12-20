// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/// This file contains integration tests that depends on the running system.
/// See [_checkSystemRequirements].
@Timeout(Duration(seconds: 120))

import 'dart:convert';
import 'dart:io' as io;
import 'dart:math';

import 'package:file/file.dart';
import 'package:file/local.dart';
import 'package:flutter_tizen_plugin_tools/src/device.dart';
import 'package:flutter_tizen_plugin_tools/src/tizen_sdk.dart';
import 'package:test/test.dart';

Future<void> _checkSystemRequirements(String emulatorName) async {
  final TizenSdk tizenSdk = TizenSdk.locateTizenSdk();
  io.ProcessResult result = await io.Process.run(
      tizenSdk.emCli.absolute.path, <String>['list-platform']);

  final List<String> platforms = LineSplitter.split(result.stdout as String)
      .map((String token) => token.trim())
      .toList();
  if (!platforms.contains('wearable-5.5-circle-x86')) {
    throw Exception('Tizen wearable-5.5 emulator package is not installed.');
  }

  result =
      await io.Process.run(tizenSdk.emCli.absolute.path, <String>['list-vm']);
  final List<String> names = LineSplitter.split(result.stdout as String)
      .map((String token) => token.trim())
      .toList();
  if (names.contains(emulatorName)) {
    throw Exception(
        'Emulator name used for test already exists: $emulatorName. '
        'Emulator name is randomly generated for each test. '
        'Rerun the test to choose a different name.');
  }

  result = await io.Process.run('flutter-tizen', <String>['-v']);
  if (result.exitCode != 0) {
    throw Exception(result.stderr as String);
  }
}

String _getRandomString(int length) {
  const String chars =
      'AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz1234567890';
  final Random random = Random();
  return String.fromCharCodes(List<int>.generate(
      length, (_) => chars.codeUnitAt(random.nextInt(chars.length))));
}

void main() {
  late TizenSdk tizenSdk;
  late EmulatorDevice device;
  late String emulatorName;

  setUp(() async {
    emulatorName = _getRandomString(10);
    await _checkSystemRequirements(emulatorName);
    tizenSdk = TizenSdk.locateTizenSdk();
    device = EmulatorDevice(
      emulatorName,
      Profile.fromString('wearable-5.5'),
      tizenSdk: tizenSdk,
    );
  });

  test('can create, delete, launch, and close emulators', () async {
    expect(device.exists, false);
    await device.create();
    expect(device.exists, true);

    expect(device.isConnected, false);
    await device.launch();
    expect(device.isConnected, true);

    await device.close();
    expect(device.isConnected, false);

    await device.delete();
    expect(device.exists, false);
  });

  test('can run integration test', () async {
    const FileSystem fileSystem = LocalFileSystem();
    final Directory testDataDir = fileSystem.currentDirectory
        .childDirectory('test_data')
        .childDirectory('foo')
        .childDirectory('example');

    await io.Process.run('flutter-tizen', <String>['pub', 'get'],
        workingDirectory: testDataDir.parent.absolute.path);

    final String? error = await device.runIntegrationTest(
        testDataDir, const Duration(seconds: 60));

    await io.Process.run('flutter-tizen', <String>['clean'],
        workingDirectory: testDataDir.parent.absolute.path);
    expect(error, isNull);
  });

  tearDown(() async {
    final List<SdbDeviceInfo> deviceInfos = tizenSdk.sdbDevices();
    for (final SdbDeviceInfo deviceInfo in deviceInfos) {
      if (device.name == deviceInfo.name) {
        final String? pid = findEmulatorPid(device.name);
        if (pid == null) {
          break;
        }
        await io.Process.run('kill', <String>['-9', pid]);
      }
    }

    final io.ProcessResult result =
        await io.Process.run(tizenSdk.emCli.absolute.path, <String>['list-vm']);
    final List<String> names = LineSplitter.split(result.stdout as String)
        .map((String token) => token.trim())
        .toList();
    for (final String name in names) {
      if (name == device.name) {
        await io.Process.run(tizenSdk.emCli.absolute.path,
            <String>['delete', '-n', device.name]);
      }
    }
  });
}
