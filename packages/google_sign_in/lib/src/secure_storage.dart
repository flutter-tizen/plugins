// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:convert' as convert;
import 'dart:math';
import 'dart:typed_data';

import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

/// Storage that encrypts data using AES algorithm.
///
/// AES key is automatically created and removed internally.
@visibleForTesting
class SecureStorage {
  final MethodChannel _channel = const MethodChannel('tizen/secure_storage');

  final Random _random = Random.secure();

  /// Saves json data with [name] to storage, data will be overwritten if it
  /// already exists.
  Future<void> saveJson(String name, Map<String, Object?> json) async {
    final Uint8List bytes =
        Uint8List.fromList(convert.jsonEncode(json).codeUnits);
    final Uint8List initializationVector =
        Uint8List.fromList(List<int>.generate(16, (_) => _random.nextInt(256)));
    await _channel.invokeMethod<void>('save', <String, Object>{
      'name': name,
      'data': bytes,
      'initialization_vector': initializationVector,
    });
  }

  /// Gets the data [name] from storage, returns `null` if no data is present.
  Future<Map<String, Object?>?> getJson(String name) async {
    final Uint8List? bytes = await _channel
        .invokeMethod<Uint8List>('get', <String, String>{'name': name});
    if (bytes != null) {
      return convert.jsonDecode(String.fromCharCodes(bytes.toList()))
          as Map<String, dynamic>;
    }
    return null;
  }

  /// Removes the data [name] from storage.
  ///
  /// This method is safe to call multiple times, nothing will happen if [name]
  /// doesn't exist in storage.
  Future<void> remove(String name) =>
      _channel.invokeMethod<void>('remove', <String, String>{'name': name});

  /// Removes all data and key from storage.
  Future<void> destroy() => _channel.invokeMethod<void>('destroy');
}
