// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:flutter/services.dart';

/// Enumeration for the package type.
enum PackageType {
  /// A special application package installed using the RPM spec.
  /// Only some preloaded packages can have this type.
  rpm,

  /// Tizen native application pacakge.
  tpk,

  /// Tizen web/hybrid application package.
  wgt,

  /// Unknown package.
  unknown,
}

/// Enumeration for the package manager event types.
enum PackageEventType {
  /// Install event.
  install,

  /// Uninstall event.
  uninstall,

  /// Update event.
  update,

  /// Move event.
  move,

  /// Clear data event.
  clearData,
}

/// Enumeration for the package manager event state.
enum PackageEventState {
  ///	Processing started.
  started,

  /// Processing state.
  processing,

  ///	Processing completed.
  completed,

  /// Processing failed.
  failed,
}

/// The package manager provides information about installed packages.
/// This information includes the pacakge name, label, path of icon, version,
/// type and installed storage.
///
/// For detailed information on Tizen's application manager, see:
/// https://docs.tizen.org/application/dotnet/guides/app-management/package-manager/
class PackageManager {
  PackageManager._();

  static const MethodChannel _channel = MethodChannel('tizen/package_manager');

  static const EventChannel _installEventChannel =
      EventChannel('tizen/package_manager/install_event');

  static const EventChannel _uninstallEventChannel =
      EventChannel('tizen/package_manager/uninstall_event');

  static const EventChannel _updateEventChannel =
      EventChannel('tizen/package_manager/update_event');

  /// Gets the package information for the given package ID.
  static Future<PackageInfo> getPackageInfo(String packageId) async {
    if (packageId.isEmpty) {
      throw ArgumentError('Must not be empty', 'packageId');
    }

    final Map<String, dynamic> map = await _channel
            .invokeMapMethod<String, dynamic>(
                'getPackage', <String, String>{'packageId': packageId}) ??
        <String, dynamic>{};

    return PackageInfo.fromMap(map);
  }

  /// Retrieves the package information of all installed packages.
  static Future<List<PackageInfo>> getPackagesInfo() async {
    final List<dynamic>? packages =
        await _channel.invokeMethod<List<dynamic>>('getPackages');

    final List<PackageInfo> list = <PackageInfo>[];
    if (packages != null) {
      for (final dynamic package in packages) {
        list.add(PackageInfo.fromMap(package));
      }
    }
    return list;
  }

  /// Installs the package located at the given path.
  ///
  /// The `http://tizen.org/privilege/packagemanager.admin` platform privilege
  /// is required to use this API.
  static Future<bool> install(String packagePath) async {
    if (packagePath.isEmpty) {
      throw ArgumentError('Must not be empty', 'packagePath');
    }

    final bool ret = await _channel.invokeMethod<bool>(
            'install', <String, String>{'path': packagePath}) ??
        false;
    return ret;
  }

  /// Uninstalls the package with the given package ID.
  ///
  /// The `http://tizen.org/privilege/packagemanager.admin` platform privilege
  /// is required to use this API.
  static Future<bool> uninstall(String packageId) async {
    if (packageId.isEmpty) {
      throw ArgumentError('Must not be empty', 'packageId');
    }

    final bool ret = await _channel.invokeMethod<bool>(
            'uninstall', <String, String>{'packageId': packageId}) ??
        false;
    return ret;
  }

  /// A stream of events occurring when a package is getting installed
  /// and the progress of the request to the package manager is changed.
  static Stream<PackageEvent> get onInstallProgressChanged =>
      _installEventChannel
          .receiveBroadcastStream()
          .map((dynamic event) => PackageEvent.fromMap(event));

  /// A stream of events occurring when a package is getting uninstalled
  /// and the progress of the request to the package manager is changed.
  static Stream<PackageEvent> get onUninstallProgressChanged =>
      _uninstallEventChannel
          .receiveBroadcastStream()
          .map((dynamic event) => PackageEvent.fromMap(event));

  /// A stream of events occurring when a package is getting updated
  /// and the progress of the request to the package manager is changed.
  static Stream<PackageEvent> get onUpdateProgressChanged => _updateEventChannel
      .receiveBroadcastStream()
      .map((dynamic event) => PackageEvent.fromMap(event));
}

/// Represents information of specific package.
class PackageInfo {
  /// Creates an instance of [PackageInfo] with the given parameters.
  PackageInfo({
    required this.packageId,
    required this.label,
    required this.packageType,
    required this.iconPath,
    required this.version,
    required this.installedStorageType,
    required this.isSystem,
    required this.isPreloaded,
    required this.isRemovable,
  });

  /// The package ID.
  final String packageId;

  /// Label of the package.
  final String label;

  /// Type of the package.
  final PackageType packageType;

  /// The path to the icon image.
  final String? iconPath;

  /// Version of the package.
  final String version;

  /// Installed storage type for the package.
  /// the return value is either internal storage or external storage.
  final String installedStorageType;

  /// Whether the package is a system package.
  final bool isSystem;

  /// Whether the package is a preload package.
  final bool isPreloaded;

  /// Whether the package is a removable package.
  final bool isRemovable;

  /// Creates an instance of [PackageInfo] with the map parameter.
  static PackageInfo fromMap(dynamic map) {
    final Object? packageType = map['type'];
    PackageType type = PackageType.unknown;
    switch (packageType) {
      case 'rpm':
        type = PackageType.rpm;
        break;
      case 'wgt':
        type = PackageType.wgt;
        break;
      case 'tpk':
        type = PackageType.tpk;
        break;
      case 'unknown':
        type = PackageType.unknown;
    }

    return PackageInfo(
      packageId: map['packageId'] as String,
      label: map['label'] as String,
      iconPath: map['iconPath'] as String?,
      packageType: type,
      version: map['version'] as String,
      installedStorageType: map['installedStorageType'] as String,
      isSystem: map['isSystem'] as bool,
      isPreloaded: map['isPreloaded'] as bool,
      isRemovable: map['isRemovable'] as bool,
    );
  }
}

/// Represents the event arguments of [PackageManager] events.
class PackageEvent {
  /// Creates an instance of [PackageEvent] with the given parameters.
  PackageEvent({
    required this.packageId,
    required this.packageType,
    required this.eventType,
    required this.eventState,
    required this.progress,
  });

  /// The package ID.
  final String packageId;

  /// Type of the package.
  final String packageType;

  /// The package manager event types
  final PackageEventType eventType;

  /// The package manager event state.
  final PackageEventState eventState;

  /// Progress for the request being processed by the package manager (in percent).
  final int progress;

  /// Creates an instance of [PackageEvent] with the map parameter.
  static PackageEvent fromMap(dynamic map) {
    PackageEventType type = PackageEventType.install;
    PackageEventState state = PackageEventState.started;
    final String eventType = map['eventType'] as String;
    final String eventState = map['eventState'] as String;

    switch (eventType) {
      case 'install':
        type = PackageEventType.install;
        break;
      case 'uninstall':
        type = PackageEventType.uninstall;
        break;
      case 'update':
        type = PackageEventType.update;
        break;
      case 'cleardata':
        type = PackageEventType.clearData;
        break;
      case 'move':
        type = PackageEventType.move;
    }

    switch (eventState) {
      case 'started':
        state = PackageEventState.started;
        break;
      case 'processing':
        state = PackageEventState.processing;
        break;
      case 'completed':
        state = PackageEventState.completed;
        break;
      case 'failed':
        state = PackageEventState.failed;
    }

    return PackageEvent(
      packageId: map['packageId'] as String,
      packageType: map['type'] as String,
      eventType: type,
      eventState: state,
      progress: map['progress'] as int,
    );
  }
}
