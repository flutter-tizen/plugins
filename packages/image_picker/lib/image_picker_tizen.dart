// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:flutter/services.dart';
import 'package:image_picker_platform_interface/image_picker_platform_interface.dart';

const MethodChannel _channel =
    MethodChannel('plugins.flutter.io/image_picker_tizen');

/// A Tizen implementation of [ImagePickerPlatform].
class ImagePickerTizen extends ImagePickerPlatform {
  /// Registers this class as the default platform implementation.
  static void register() {
    ImagePickerPlatform.instance = ImagePickerTizen();
  }

  @override
  Future<XFile?> getImageFromSource({
    required ImageSource source,
    ImagePickerOptions options = const ImagePickerOptions(),
  }) async {
    final String? path = await _getImagePath(
      source: source,
      maxHeight: options.maxHeight,
      maxWidth: options.maxWidth,
      imageQuality: options.imageQuality,
      preferredCameraDevice: options.preferredCameraDevice,
      requestFullMetadata: options.requestFullMetadata,
    );
    return path != null ? XFile(path) : null;
  }

  Future<String?> _getImagePath({
    required ImageSource source,
    double? maxWidth,
    double? maxHeight,
    int? imageQuality,
    CameraDevice preferredCameraDevice = CameraDevice.rear,
    bool requestFullMetadata = true,
  }) {
    if (imageQuality != null && (imageQuality < 0 || imageQuality > 100)) {
      throw ArgumentError.value(
          imageQuality, 'imageQuality', 'must be between 0 and 100');
    }

    if (maxWidth != null && maxWidth < 0) {
      throw ArgumentError.value(maxWidth, 'maxWidth', 'cannot be negative');
    }

    if (maxHeight != null && maxHeight < 0) {
      throw ArgumentError.value(maxHeight, 'maxHeight', 'cannot be negative');
    }

    return _channel.invokeMethod<String>(
      'pickImage',
      <String, dynamic>{
        'source': source.index,
        'maxWidth': maxWidth,
        'maxHeight': maxHeight,
        'imageQuality': imageQuality,
        'cameraDevice': preferredCameraDevice.index,
        'requestFullMetadata': requestFullMetadata,
      },
    );
  }

  @override
  Future<List<XFile>> getMultiImageWithOptions({
    MultiImagePickerOptions options = const MultiImagePickerOptions(),
  }) async {
    final List<dynamic>? paths = await _getMultiImagePath(
      maxWidth: options.imageOptions.maxWidth,
      maxHeight: options.imageOptions.maxHeight,
      imageQuality: options.imageOptions.imageQuality,
      requestFullMetadata: options.imageOptions.requestFullMetadata,
    );
    if (paths == null) {
      return <XFile>[];
    }

    return paths.map((dynamic path) => XFile(path as String)).toList();
  }

  Future<List<dynamic>?> _getMultiImagePath({
    double? maxWidth,
    double? maxHeight,
    int? imageQuality,
    bool requestFullMetadata = true,
  }) {
    if (imageQuality != null && (imageQuality < 0 || imageQuality > 100)) {
      throw ArgumentError.value(
          imageQuality, 'imageQuality', 'must be between 0 and 100');
    }

    if (maxWidth != null && maxWidth < 0) {
      throw ArgumentError.value(maxWidth, 'maxWidth', 'cannot be negative');
    }

    if (maxHeight != null && maxHeight < 0) {
      throw ArgumentError.value(maxHeight, 'maxHeight', 'cannot be negative');
    }

    return _channel.invokeMethod<List<dynamic>?>(
      'pickMultiImage',
      <String, dynamic>{
        'maxWidth': maxWidth,
        'maxHeight': maxHeight,
        'imageQuality': imageQuality,
        'requestFullMetadata': requestFullMetadata,
      },
    );
  }

  @override
  Future<XFile?> getVideo({
    required ImageSource source,
    CameraDevice preferredCameraDevice = CameraDevice.rear,
    Duration? maxDuration,
  }) async {
    final String? path = await _getVideoPath(
      source: source,
      maxDuration: maxDuration,
      preferredCameraDevice: preferredCameraDevice,
    );
    return path != null ? XFile(path) : null;
  }

  Future<String?> _getVideoPath({
    required ImageSource source,
    CameraDevice preferredCameraDevice = CameraDevice.rear,
    Duration? maxDuration,
  }) {
    return _channel.invokeMethod<String>(
      'pickVideo',
      <String, dynamic>{
        'source': source.index,
        'maxDuration': maxDuration?.inSeconds,
        'cameraDevice': preferredCameraDevice.index
      },
    );
  }
}
