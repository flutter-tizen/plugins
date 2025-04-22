// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:convert';
import 'dart:io' as io;

import 'package:file/file.dart';
import 'package:flutter_plugin_tools/src/common/core.dart';
import 'package:flutter_plugin_tools/src/common/process_runner.dart';

import 'process_runner_apis.dart';
import 'tizen_sdk.dart';

/// A reference to a Tizen device (either physical or emulator) that can run
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
  }) : _tizenSdk = tizenSdk,
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
    device._serial = device._findSerial();
    if (device._serial == null) {
      throw Exception(
        "$name ($profile)'s serial is null. "
        'Physical device references must be connected to host PC.',
      );
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

  String? _serial;

  final ProcessRunner _processRunner;

  /// The unqiue identifier assigned to a connected device.
  ///
  /// Physical devices have baked in serial numbers while emulators
  /// are assigned with a port number when they're launched.
  String? get serial => _serial;

  /// Whether this device is an emulator.
  bool get isEmulator => false;

  /// Whether this device is connected to host PC.
  bool get isConnected {
    _serial = _findSerial();
    return _serial != null;
  }

  String? _findSerial() {
    final List<SdbDeviceInfo> deviceInfos = _tizenSdk.sdbDevices();
    for (final SdbDeviceInfo deviceInfo in deviceInfos) {
      if (deviceInfo.name == name) {
        return deviceInfo.serial;
      }
    }
    return null;
  }

  /// Runs integration test in [workingDir].
  ///
  /// [workingDir] must be a valid existing Flutter project directory where
  /// `flutter pub get` has already been run succesfully. For an app project,
  /// [workingDir] is the app's source root. For a plugin project, [workingDir]
  /// is one of the example directories.
  ///
  /// Returns null if all tests have passed successfully before [timeout],
  /// otherwise returns a string with the error details.
  Future<String?> runIntegrationTest(
    Directory workingDir,
    Duration timeout,
  ) async {
    if (!isConnected) {
      return 'Device $name ($profile) is not connected.';
    }

    final io.Process process = await _processRunner.start(
      'flutter-tizen',
      <String>['-d', serial!, 'test', '--no-dds', 'integration_test'],
      workingDirectory: workingDir,
    );

    bool timedOut = false;
    final Stream<String> streamLines = process.stdout
        .transform(const Utf8Decoder())
        .transform(const LineSplitter());

    final Future<int> timedExitCode = process.exitCode.timeout(
      timeout,
      onTimeout: () {
        timedOut = true;
        return 1;
      },
    );

    String lastLine = '';
    final Completer<int> completer = Completer<int>();
    streamLines.listen((String line) {
      lastLine = line;
      print(line);
    }, onDone: () async => completer.complete(await timedExitCode));
    // Waits for the done event as finishing `Process.exitCode` future does not
    // guarantee that all buffered outputs of the process have returned.
    await completer.future;

    String? error;
    if (timedOut) {
      error =
          'Timeout expired. The test may need more time to finish. '
          'If you expect the test to finish before timeout, check if the tests '
          'require device screen to be awake or if they require manually '
          'clicking the UI button for permissions.';
    } else if (lastLine.contains('No tests ran')) {
      error =
          'Missing integration tests (use --exclude if this is intentional).';
    } else if (lastLine.contains('No devices found')) {
      error = 'Device was disconnected during test.';
    } else if (lastLine.contains('failed')) {
      error =
          'flutter-tizen test integration_test failed, see the output '
          'above for details.';
    } else if (!lastLine.contains('passed')) {
      error = 'Could not parse the log output.';
    }
    return error;
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
    emulatorDevice._serial = emulatorDevice._findSerial();
    emulatorDevice._pid = findEmulatorPid(name);
    return emulatorDevice;
  }

  EmulatorDevice._(
    super.name,
    super.profile, {
    required super.tizenSdk,
    super.processRunner = const ProcessRunner(),
  }) : super._();

  String? _pid;

  @override
  bool get isConnected {
    _serial = _findSerial();
    _pid = findEmulatorPid(name);
    return _serial != null && _pid != null;
  }

  @override
  bool get isEmulator => true;

  /// Checks whether the emulator with [name] and [profile] exists
  /// in the Emulator Manager.
  bool get exists {
    final io.ProcessResult result = _processRunner.runSync(
      _tizenSdk.emCli.path,
      <String>['list-vm'],
    );
    if (result.exitCode != 0) {
      print('Error: Unable to list available emulators.');
      throw ToolExit(result.exitCode);
    }

    final List<String> emulatorNames =
        LineSplitter.split(
          result.stdout as String,
        ).map((String name) => name.trim()).toList();
    return emulatorNames.contains(name);
  }

  /// Creates this emulator.
  Future<void> create() async {
    late final String platform;
    switch (profile.deviceType) {
      case DeviceType.tv:
        platform =
            '${profile.deviceType}-samsung-${profile.version.toString().substring(0, 3)}-x86';
        break;
      case DeviceType.mobile:
        platform = '$profile-x86';
        break;
    }
    await _processRunner.runAndStream(_tizenSdk.emCli.path, <String>[
      'create',
      '-n',
      name,
      '-p',
      platform,
    ], exitOnError: true);
  }

  /// Grants all privacy-related permissions to apps by default.
  ///
  /// This only applies to apps newly installed to the device after this call.
  bool _disablePermissionPopups() {
    if (!isConnected) {
      return false;
    }
    final Map<String, String> capability = _tizenSdk.sdbCapability(serial!);
    final bool rooted = capability['rootonoff_support'] == 'enabled';
    if (!rooted) {
      return false;
    }

    io.ProcessResult result = _processRunner.runSync(
      _tizenSdk.sdb.path,
      <String>['-s', serial!, 'root', 'on'],
    );
    if (result.exitCode != 0) {
      print('Error: running "sdb root on" failed.');
      return false;
    }

    result = _processRunner.runSync(_tizenSdk.sdb.path, <String>[
      '-s',
      serial!,
      'shell',
      'touch',
      '/opt/share/askuser_disable',
    ]);
    final String stdout = result.stdout as String;
    if (result.exitCode != 0 || stdout.trim().isNotEmpty) {
      print('Error: running sdb shell command failed: $stdout');
      return false;
    }

    _processRunner.runSync(_tizenSdk.sdb.path, <String>[
      '-s',
      serial!,
      'root',
      'off',
    ]);
    return true;
  }

  /// Deletes this emulator.
  Future<void> delete() async {
    await _processRunner.runAndStream(_tizenSdk.emCli.path, <String>[
      'delete',
      '-n',
      name,
    ]);
  }

  /// Launches this emualtor.
  Future<void> launch() async {
    if (isConnected) {
      print('Device $name ($profile) is already launched.');
      return;
    }
    await _processRunner.runAndStream(_tizenSdk.emCli.path, <String>[
      'launch',
      '-n',
      name,
    ], exitOnError: true);

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
    _serial = _findSerial();
  }

  /// Closes this emulator.
  Future<void> close() async {
    if (!isConnected) {
      print('Device $name ($profile) is already closed.');
      return;
    }
    // TODO(HakkyuKim): Support Windows.
    await _processRunner.run('kill', <String>['-9', _pid!], exitOnError: true);

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
    _serial = null;
  }

  Future<void> _poll(
    FutureOr<bool> Function() function, {
    Duration interval = const Duration(seconds: 1),
    Duration timeout = const Duration(minutes: 5),
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
  Future<String?> runIntegrationTest(
    Directory workingDir,
    Duration timeout,
  ) async {
    bool autoLaunched = false;
    bool autoCreated = false;
    try {
      if (!exists) {
        autoCreated = true;
        await create();
      }
      if (!isConnected) {
        autoLaunched = true;
        await launch();
      }
      _disablePermissionPopups();
      return await super.runIntegrationTest(workingDir, timeout);
    } finally {
      if (autoLaunched) {
        await close();
      }
      if (autoCreated) {
        await delete();
      }
    }
  }
}
