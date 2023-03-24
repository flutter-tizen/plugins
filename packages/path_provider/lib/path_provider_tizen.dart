// Copyright 2020 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:path_provider_platform_interface/path_provider_platform_interface.dart';

import 'src/app_common.dart';
import 'src/storage.dart';

/// The Tizen implementation of [PathProviderPlatform].
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
  Future<List<String>> getExternalStoragePaths({StorageDirectory? type}) async {
    int dirType;
    switch (type) {
      case StorageDirectory.music:
        dirType = storage_directory_e.STORAGE_DIRECTORY_MUSIC;
        break;
      case StorageDirectory.ringtones:
        dirType = storage_directory_e.STORAGE_DIRECTORY_SYSTEM_RINGTONES;
        break;
      case StorageDirectory.pictures:
        dirType = storage_directory_e.STORAGE_DIRECTORY_IMAGES;
        break;
      case StorageDirectory.movies:
        dirType = storage_directory_e.STORAGE_DIRECTORY_VIDEOS;
        break;
      case StorageDirectory.downloads:
        dirType = storage_directory_e.STORAGE_DIRECTORY_DOWNLOADS;
        break;
      case StorageDirectory.dcim:
        dirType = storage_directory_e.STORAGE_DIRECTORY_CAMERA;
        break;
      case StorageDirectory.documents:
        dirType = storage_directory_e.STORAGE_DIRECTORY_DOCUMENTS;
        break;
      case StorageDirectory.podcasts:
      case StorageDirectory.alarms:
      case StorageDirectory.notifications:
      case null:
        dirType = storage_directory_e.STORAGE_DIRECTORY_OTHERS;
        break;
    }
    return <String>[await storage.getDirectory(dirType)];
  }
}
