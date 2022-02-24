// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:convert';
import 'dart:io' as io;

import 'package:file/file.dart';
import 'package:flutter_plugin_tools/src/common/core.dart';
import 'package:flutter_plugin_tools/src/common/package_looping_command.dart';
import 'package:flutter_plugin_tools/src/common/process_runner.dart';

import 'process_runner_apis.dart';
import 'tizen_sdk.dart';

export 'package:flutter_plugin_tools/src/common/package_looping_command.dart'
    show PackageResult, RunState;

/// A reference to a Tizen device(either physical or emulator) that can run
/// Flutter applications.
///
/// Physical devices are connected when they're connected to host PC either
/// with a cable or wirelessly. Emulators are connected when they're launched on
/// host PC.
class Device {
  Device._(
    this.name,
    this.profile, {
    required TizenSdk tizenSdk,
    ProcessRunner processRunner = const ProcessRunner(),
  })  : _tizenSdk = tizenSdk,
        _processRunner = processRunner;

  /// Creates a new physical Tizen device reference.
  ///
  /// Valid characters for a device name are [A-Za-z0-9-_].
  factory Device.physical(
    String name,
    Profile profile, {
    required TizenSdk tizenSdk,
    ProcessRunner processRunner = const ProcessRunner(),
  }) {
    final Device device = Device._(
      name,
      profile,
      tizenSdk: tizenSdk,
      processRunner: processRunner,
    );
    device._id = device._findId();
    if (device._id == null) {
      throw Exception('$name($profile)\'s id is null. '
          'Physical device references must be connected to host PC.');
    }
    return device;
  }

  /// Creates a new reference to a Tizen emulator device.
  ///
  /// Valid characters for a device name are [A-Za-z0-9-_].
  factory Device.emulator(
    String name,
    Profile profile, {
    required TizenSdk tizenSdk,
    ProcessRunner processRunner = const ProcessRunner(),
  }) {
    return EmulatorDevice(
      name,
      profile,
      tizenSdk: tizenSdk,
      processRunner: processRunner,
    );
  }

  /// The name of the device.
  final String name;

  /// The profile of the device.
  final Profile profile;

  final TizenSdk _tizenSdk;

  String? _id;

  final ProcessRunner _processRunner;

  /// The unqiue identifier assigned to a connected device.
  String? get id => _id;

  /// Whether this device is an emulator.
  bool get isEmulator => false;

  /// Whether this device is connected to host PC.
  bool get isConnected {
    _id = _findId();
    return _id != null;
  }

  static final RegExp _logPattern =
      RegExp(r'\d\d:\d\d\s+([(\+\d+\s+)|(~\d+\s+)|(\-\d+\s+)]+):\s+(.*)');

  String? _findId() {
    final List<SdbDeviceInfo> deviceInfos = _tizenSdk.sdbDevices();
    for (final SdbDeviceInfo deviceInfo in deviceInfos) {
      if (deviceInfo.name == name) {
        return deviceInfo.id;
      }
    }
    return null;
  }

  /// Runs integration test in [workingDir].
  ///
  /// [workingDir] must be a valid exisiting flutter directory where
  /// `flutter pub get` has already been ran succesfully. For an app project,
  /// [workingDir] is the app's source root. For a plugin project, [workingDir]
  /// is one of the example directories.
  ///
  /// If test doesn't finish after [timeout], it's considered a failure and the
  /// function will return [PackageResult.fail] with time expired log.
  ///
  /// Returns [PackageResult.success] when every test passes succesfully,
  /// otherwise returns [PackageResult.fail]. Never returns [PackageResult.skip]
  /// nor [PackageResult.exclude].
  Future<PackageResult> runIntegrationTest(
    Directory workingDir,
    Duration timeout,
  ) async {
    if (!isConnected) {
      return PackageResult.fail(
          <String>['Device $name($profile) is not connected.']);
    }

    final io.Process process = await _processRunner.start(
      'flutter-tizen',
      <String>['-d', id!, 'test', 'integration_test'],
      workingDirectory: workingDir,
    );

    bool timedOut = false;
    final Stream<String> streamLines = process.stdout
        .transform(const Utf8Decoder())
        .transform(const LineSplitter())
        .timeout(
      timeout,
      onTimeout: (EventSink<String> sink) {
        timedOut = true;
        sink.close();
      },
    );

    String lastLine = '';
    final DateTime start = DateTime.now();
    await for (final String line in streamLines) {
      lastLine = line;
      io.stdout.add('$line\n'.codeUnits);
      if (DateTime.now().difference(start) > timeout) {
        process.kill();
        timedOut = true;
        break;
      }
    }

    final List<String> errors = <String>[];
    if (timedOut) {
      errors.add('Timeout expired. The test may need more time to finish. '
          'If you expect the test to finish before timeout, check if the tests '
          'require device screen to be awake or if they require manually '
          'clicking the UI button for permissions.');
    } else if (lastLine.startsWith('No tests ran')) {
      errors.add(
          'Missing integration tests (use --exclude if this is intentional).');
    } else if (lastLine.startsWith('No devices found')) {
      errors.add('Device was disconnected during test.');
    } else {
      final RegExpMatch? match = _logPattern.firstMatch(lastLine);
      if (match == null || match.group(2) == null) {
        throw Exception('Log message is not parsed correctly.');
      } else if (!match.group(2)!.startsWith('All tests passed!')) {
        errors.add('flutter-tizen test integration_test failed, see the output '
            'above for details.');
      }
    }

    return errors.isEmpty
        ? PackageResult.success()
        : PackageResult.fail(errors);
  }
}

/// A reference to a Tizen emulator device.
class EmulatorDevice extends Device {
  /// Creates a new reference to a Tizen emulator device.
  ///
  /// Valid characters for a device name are [A-Za-z0-9-_].
  factory EmulatorDevice(
    String name,
    Profile profile, {
    required TizenSdk tizenSdk,
    ProcessRunner processRunner = const ProcessRunner(),
  }) {
    final EmulatorDevice emulatorDevice = EmulatorDevice._(
      name,
      profile,
      tizenSdk: tizenSdk,
      processRunner: processRunner,
    );
    emulatorDevice._id = emulatorDevice._findId();
    emulatorDevice._pid = findEmulatorPid(name);
    return emulatorDevice;
  }

  EmulatorDevice._(
    String name,
    Profile profile, {
    required TizenSdk tizenSdk,
    ProcessRunner processRunner = const ProcessRunner(),
  }) : super._(
          name,
          profile,
          tizenSdk: tizenSdk,
          processRunner: processRunner,
        );

  String? _pid;

  @override
  bool get isConnected {
    _id = _findId();
    _pid = findEmulatorPid(name);
    return _id != null && _pid != null;
  }

  @override
  bool get isEmulator => true;

  /// Checks whether the emulator with [name] and [profile] exists
  /// in the Emulator Manager.
  bool get exists {
    final io.ProcessResult result =
        _processRunner.runSync(_tizenSdk.emCli.path, <String>['list-vm']);
    if (result.exitCode != 0) {
      throw ToolExit(result.exitCode);
    }

    final List<String> emulatorNames =
        LineSplitter.split((result.stdout as String).trim())
            .map((String name) => name.trim())
            .toList();
    return emulatorNames.contains(name);
  }

  /// Creates this emulator.
  Future<void> create() async {
    late final String platform;
    switch (profile.deviceType) {
      case DeviceType.wearable:
        platform = '$profile-circle-x86';
        break;
      case DeviceType.tv:
        platform =
            '${profile.deviceType}-samsung-${profile.version.toString().substring(0, 3)}-x86';
        break;
      case DeviceType.mobile:
        platform = '$profile-x86';
        break;
    }
    await _processRunner.runAndStream(
      _tizenSdk.emCli.path,
      <String>['create', '-n', name, '-p', platform],
      exitOnError: true,
    );
  }

  /// Deletes this emulator.
  Future<void> delete() async => await _processRunner.runAndStream(
        _tizenSdk.emCli.path,
        <String>['delete', '-n', name],
        exitOnError: true,
      );

  /// Launches this emualtor.
  Future<void> launch() async {
    if (isConnected) {
      print('Device $name($profile) is already launched.');
      return;
    }
    await _processRunner.runAndStream(
      _tizenSdk.emCli.path,
      <String>['launch', '-n', name],
      exitOnError: true,
    );

    await _poll(() {
      final List<SdbDeviceInfo> deviceInfos = _tizenSdk.sdbDevices();
      for (final SdbDeviceInfo deviceInfo in deviceInfos) {
        if (deviceInfo.name == name && deviceInfo.status == 'device') {
          return true;
        }
      }
      return false;
    });

    await _poll(() {
      if (findEmulatorPid(name) != null) {
        return true;
      }
      return false;
    });
    _id = _findId();
  }

  /// Closes this emulator.
  Future<void> close() async {
    if (!isConnected) {
      print('Device $name($profile) is already closed.');
      return;
    }
    // TODO(HakkyuKim): Support Windows.
    await _processRunner.run(
      'kill',
      <String>['-9', _pid!],
      exitOnError: true,
    );

    await _poll(() {
      final List<SdbDeviceInfo> deviceInfos = _tizenSdk.sdbDevices();
      for (final SdbDeviceInfo deviceInfo in deviceInfos) {
        if (deviceInfo.name == name) {
          return false;
        }
      }
      return true;
    });
    _pid = null;
    _id = null;
  }

  Future<void> _poll(
    FutureOr<bool> Function() function, {
    Duration interval = const Duration(seconds: 1),
    Duration timeout = const Duration(minutes: 10),
  }) async {
    final DateTime start = DateTime.now();
    while (DateTime.now().difference(start) <= timeout) {
      if (await function()) {
        return;
      }
      await Future<void>.delayed(interval);
    }
    throw Exception('Polling failed due to timeout.');
  }

  @override
  Future<PackageResult> runIntegrationTest(
    Directory workingDir,
    Duration timeout,
  ) async {
    bool autoLaunched = false;
    bool autoCreated = false;
    late final PackageResult result;
    try {
      if (!exists) {
        autoCreated = true;
        await create();
      }
      if (!isConnected) {
        autoLaunched = true;
        await launch();
      }
      result = await super.runIntegrationTest(workingDir, timeout);
    } finally {
      if (autoLaunched) {
        await close();
      }
      if (autoCreated) {
        await delete();
      }
    }
    return result;
  }
}
