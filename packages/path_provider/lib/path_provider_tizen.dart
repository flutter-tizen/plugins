// Copyright 2020 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:path_provider_platform_interface/path_provider_platform_interface.dart';

import 'src/app_common.dart';
import 'src/storage.dart';

/// The Tizen implementation of [PathProviderPlatform].
///
/// This class implements the `package:path_provider` functionality for Tizen.
class PathProviderPlugin extends PathProviderPlatform {
  /// Registers this class as the default instance of [PathProviderPlatform].
  static void register() {
    PathProviderPlatform.instance = PathProviderPlugin();
  }

  @override
  Future<String> getTemporaryPath() async => appCommon.getCachePath();

  @override
  Future<String> getApplicationDocumentsPath() async => appCommon.getDataPath();

  @override
  Future<String> getApplicationSupportPath() async => appCommon.getDataPath();

  @override
  Future<String> getExternalStoragePath() async =>
      appCommon.getExternalDataPath();

  @override
  Future<List<String>> getExternalCachePaths() async =>
      <String>[appCommon.getExternalCachePath()];

  @override
  Future<List<String>> getExternalStoragePaths({
    StorageDirectory? type,
  }) async {
    StorageDirectoryType dirType;
    switch (type) {
      case StorageDirectory.music:
        dirType = StorageDirectoryType.music;
        break;
      case StorageDirectory.ringtones:
        dirType = StorageDirectoryType.system_ringtones;
        break;
      case StorageDirectory.pictures:
        dirType = StorageDirectoryType.images;
        break;
      case StorageDirectory.movies:
        dirType = StorageDirectoryType.videos;
        break;
      case StorageDirectory.downloads:
        dirType = StorageDirectoryType.downloads;
        break;
      case StorageDirectory.dcim:
        dirType = StorageDirectoryType.camera;
        break;
      case StorageDirectory.documents:
        dirType = StorageDirectoryType.documents;
        break;
      case StorageDirectory.podcasts:
      case StorageDirectory.alarms:
      case StorageDirectory.notifications:
      default:
        dirType = StorageDirectoryType.others;
        break;
    }
    return <String>[await storage.getDirectory(type: dirType)];
  }

  @override
  Future<String> getDownloadsPath() async =>
      await storage.getDirectory(type: StorageDirectoryType.downloads);
}
