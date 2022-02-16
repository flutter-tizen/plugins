// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:io' as io;

import 'package:file/file.dart';
import 'package:file/local.dart';
import 'package:flutter_plugin_tools/src/common/core.dart';
import 'package:pub_semver/pub_semver.dart';

import 'syncable_process_runner.dart';

/// A data type that holds information such as id and status of
/// a connected device.
class SdbDeviceInfo {
  /// Creates a [SdbDeviceInfo] instance.
  const SdbDeviceInfo({
    required this.id,
    required this.status,
    required this.name,
  });

  /// A unique string that identifies a running device.
  final String id;

  // TODO(HakkyuKim): Document possible values of [status].
  /// Current connection state of the device.
  final String status;

  /// The name of the device.
  final String name;
}

/// A Tizen device type.
class DeviceType {
  const DeviceType._(this._value);

  /// Creates a [DeviceType] instance from a string value.
  static DeviceType fromString(String value) {
    for (final DeviceType profile in values) {
      if (profile._value == value) {
        return profile;
      }
    }
    throw ArgumentError('Device type string must be one of $values.', 'value');
  }

  final String _value;

  /// A list of all supported Tizen device types.
  static const List<DeviceType> values = <DeviceType>[wearable, tv, mobile];

  /// A wearable device type such as Galaxy Watch.
  static const DeviceType wearable = DeviceType._('wearable');

  /// A tv device type such as Samsung Smart TV.
  static const DeviceType tv = DeviceType._('tv');

  /// A mobile device type such as smartphones.
  static const DeviceType mobile = DeviceType._('mobile');

  @override
  String toString() => _value;
}

/// A combination of Tizen device type and platform version.
class Profile {
  /// Creates a [Profile] instance.
  Profile(this.deviceType, this.version);

  /// Creates a [Profile] instance from a string value.
  ///
  /// String must be in the form of device_type-platform_version, where
  /// platform_version is optional.
  ///
  /// ```dart
  /// // Wearable device.
  /// Profile.fromString('wearable')
  /// // Wearable device with Tizen 5.5.
  /// Profile.fromString('wearable-5.5')
  ///
  /// ```
  ///
  /// To see all supported device types, see [DeviceType].
  static Profile fromString(String value) {
    final RegExp regExp = RegExp(DeviceType.values.join('|'));
    final RegExpMatch? match = regExp.firstMatch(value);
    if (match == null) {
      throw ArgumentError(
          'Profile string must start with one of ${DeviceType.values}.',
          'value');
    }
    final DeviceType deviceType =
        DeviceType.fromString(value.substring(match.start, match.end));
    Version? version;
    if (value.length >= match.end && value[match.end] == '-') {
      try {
        final List<String> segments = value.substring(match.end + 1).split('.');
        while (segments.length < 3) {
          segments.add('0');
        }
        version = Version.parse(segments.join('.'));
      } on FormatException {
        throw ArgumentError(
            'Platform version cannot be parsed from profile string $value.',
            'value');
      }
    }
    return Profile(deviceType, version);
  }

  /// Tizen device type.
  final DeviceType deviceType;

  /// Tizen platform version.
  final Version? version;

  @override
  String toString() =>
      deviceType.toString() +
      (version == null ? '' : '-${version.toString().substring(0, 3)}');
}

/// A class that provides some of Tizen SDK's functionalities.
class TizenSdk {
  TizenSdk._(
    this._sdkRoot, {
    SyncableProcessRunner processRunner = const SyncableProcessRunner(),
  }) : _processRunner = processRunner;

  /// Finds the Tizen sdk's installed directory and
  /// returns an instance of [TizenSdk].
  static TizenSdk locateTizenSdk({
    FileSystem fileSystem = const LocalFileSystem(),
    SyncableProcessRunner processRunner = const SyncableProcessRunner(),
  }) {
    Directory? tizenHomeDir;
    final Map<String, String> environment = io.Platform.environment;
    if (environment.containsKey(_kTizenSdk)) {
      tizenHomeDir = fileSystem.directory(environment[_kTizenSdk]);
    } else if (io.Platform.isLinux || io.Platform.isMacOS) {
      tizenHomeDir = fileSystem
          .directory(environment['HOME'])
          .childDirectory('tizen-studio');
    } else if (io.Platform.isWindows) {
      if (environment.containsKey('SystemDrive')) {
        tizenHomeDir = fileSystem
            .directory(environment['SystemDrive'])
            .childDirectory('tizen-studio');
      }
    }
    if (tizenHomeDir == null || !tizenHomeDir.existsSync()) {
      print(
          'Error: Cannot find tizen sdk, make sure to set the path of tizen sdk '
          'to the environment variable $_kTizenSdk.');
      throw ToolExit(exitCommandFoundErrors);
    }
    return TizenSdk._(
      tizenHomeDir,
      processRunner: processRunner,
    );
  }

  static const String _kTizenSdk = 'TIZEN_SDK';

  final SyncableProcessRunner _processRunner;

  final Directory _sdkRoot;

  /// Cli tool for interacting with connected devices.
  File get sdb => _sdkRoot
      .childDirectory('tools')
      .childFile(io.Platform.isWindows ? 'sdb.exe' : 'sdb');

  /// Tizen SDK's emulator cli.
  File get emCli => _sdkRoot
      .childDirectory('tools')
      .childDirectory('emulator')
      .childDirectory('bin')
      .childFile(io.Platform.isWindows ? 'em-cli.bat' : 'em-cli');

  /// Returns information of all connected devices.
  List<SdbDeviceInfo> sdbDevices() {
    final io.ProcessResult result =
        _processRunner.runSync(sdb.path, <String>['devices']);
    if (result.exitCode != 0) {
      print('Error: running command `sdb devices` failed.');
      throw ToolExit(result.exitCode);
    }

    final List<SdbDeviceInfo> deviceInfos = <SdbDeviceInfo>[];
    final List<String> lines =
        (result.stdout as String).trim().split('\n').sublist(1);
    for (final String line in lines) {
      final List<String> tokens = line.split(RegExp(r'\s+'));
      deviceInfos.add(SdbDeviceInfo(
        id: tokens[0],
        status: tokens[1],
        name: tokens[2],
      ));
    }
    return deviceInfos;
  }

  /// Returns the result of running sdb capability as a map of key-value pairs.
  Map<String, String> sdbCapability(String id) {
    final io.ProcessResult result =
        _processRunner.runSync(sdb.path, <String>['-s', id, 'capability']);
    if (result.exitCode != 0) {
      print('Error: running command `sdb -s $id capability` failed.');
      throw ToolExit(result.exitCode);
    }

    final Map<String, String> capabilities = <String, String>{};
    final List<String> lines = (result.stdout as String).trim().split('\n');
    for (final String line in lines) {
      final int index = line.indexOf(':');
      final String key = line.substring(0, index).trim();
      final String value =
          index + 1 > line.length ? '' : line.substring(index + 1).trim();
      capabilities[key] = value;
    }
    return capabilities;
  }
}

/// Returns the pid(process id) of a running emulator instance [name].
///
/// Returns `null` if emulator [name] is not running.
String? findEmulatorPid(String name) {
  const SyncableProcessRunner processRunner = SyncableProcessRunner();
  // TODO(HakkyuKim): Support Windows.
  final io.ProcessResult result = processRunner.runSync('ps', <String>['aux']);

  if (result.exitCode != 0) {
    throw ToolExit(1);
  }

  final List<String> lines = (result.stdout as String)
      .trim()
      .split('\n')
      .map((String line) => line.trim())
      .toList();

  for (final String line in lines) {
    if (line.contains('emulator-x86_64') && line.contains(name)) {
      return line.split(RegExp(r'\s+'))[1];
    }
  }

  return null;
}
