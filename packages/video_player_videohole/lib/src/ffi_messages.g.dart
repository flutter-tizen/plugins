// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// FFI API for video_player_tizen - manually maintained
// This file contains FFI bindings and message types for synchronous native calls

import 'dart:ffi' as ffi;
import 'dart:ffi' show NativeCallable;
import 'dart:isolate' show RawReceivePort, ReceivePort;
import 'package:ffi/ffi.dart' show calloc;
import 'dart:typed_data' show Float64List, Int32List, Int64List, Uint8List;
import 'dart:convert' show utf8, jsonEncode, jsonDecode;

import 'package:flutter/foundation.dart'
    show ReadBuffer, WriteBuffer, debugPrint;
import 'package:flutter/services.dart';

// ===== Message Types =====

class PlayerMessage {
  PlayerMessage({required this.playerId});

  int playerId;

  Object encode() {
    return <Object?>[playerId];
  }

  static PlayerMessage decode(Object result) {
    result as List<Object?>;
    return PlayerMessage(playerId: result[0]! as int);
  }
}

class LoopingMessage {
  LoopingMessage({required this.playerId, required this.isLooping});

  int playerId;
  bool isLooping;

  Object encode() {
    return <Object?>[playerId, isLooping];
  }

  static LoopingMessage decode(Object result) {
    result as List<Object?>;
    return LoopingMessage(
      playerId: result[0]! as int,
      isLooping: result[1]! as bool,
    );
  }
}

class VolumeMessage {
  VolumeMessage({required this.playerId, required this.volume});

  int playerId;
  double volume;

  Object encode() {
    return <Object?>[playerId, volume];
  }

  static VolumeMessage decode(Object result) {
    result as List<Object?>;
    return VolumeMessage(
      playerId: result[0]! as int,
      volume: result[1]! as double,
    );
  }
}

class PlaybackSpeedMessage {
  PlaybackSpeedMessage({required this.playerId, required this.speed});

  int playerId;
  double speed;

  Object encode() {
    return <Object?>[playerId, speed];
  }

  static PlaybackSpeedMessage decode(Object result) {
    result as List<Object?>;
    return PlaybackSpeedMessage(
      playerId: result[0]! as int,
      speed: result[1]! as double,
    );
  }
}

class TrackMessage {
  TrackMessage({required this.playerId, required this.tracks});

  int playerId;
  List<Map<Object?, Object?>?> tracks;

  Object encode() {
    return <Object?>[playerId, tracks];
  }

  static TrackMessage decode(Object result) {
    result as List<Object?>;
    return TrackMessage(
      playerId: result[0]! as int,
      tracks: (result[1] as List<Object?>?)!.cast<Map<Object?, Object?>?>(),
    );
  }
}

class TrackTypeMessage {
  TrackTypeMessage({required this.playerId, required this.trackType});

  int playerId;
  String trackType;

  Object encode() {
    return <Object?>[playerId, trackType];
  }

  static TrackTypeMessage decode(Object result) {
    result as List<Object?>;
    return TrackTypeMessage(
      playerId: result[0]! as int,
      trackType: result[1]! as String,
    );
  }
}

class SelectedTracksMessage {
  SelectedTracksMessage({
    required this.playerId,
    required this.trackId,
    required this.trackType,
  });

  int playerId;
  int trackId;
  String trackType;

  Object encode() {
    return <Object?>[playerId, trackId, trackType];
  }

  static SelectedTracksMessage decode(Object result) {
    result as List<Object?>;
    return SelectedTracksMessage(
      playerId: result[0]! as int,
      trackId: result[1]! as int,
      trackType: result[2]! as String,
    );
  }
}

class PositionMessage {
  PositionMessage({required this.playerId, required this.position});

  int playerId;
  int position;

  Object encode() {
    return <Object?>[playerId, position];
  }

  static PositionMessage decode(Object result) {
    result as List<Object?>;
    return PositionMessage(
      playerId: result[0]! as int,
      position: result[1]! as int,
    );
  }
}

class CreateMessage {
  CreateMessage({
    this.asset,
    this.uri,
    this.packageName,
    this.formatHint,
    this.httpHeaders,
    this.drmConfigs,
    this.playerOptions,
  });

  String? asset;
  String? uri;
  String? packageName;
  String? formatHint;
  Map<Object?, Object?>? httpHeaders;
  Map<Object?, Object?>? drmConfigs;
  Map<Object?, Object?>? playerOptions;

  Object encode() {
    return <Object?>[
      asset,
      uri,
      packageName,
      formatHint,
      httpHeaders,
      drmConfigs,
      playerOptions,
    ];
  }

  static CreateMessage decode(Object result) {
    result as List<Object?>;
    return CreateMessage(
      asset: result[0] as String?,
      uri: result[1] as String?,
      packageName: result[2] as String?,
      formatHint: result[3] as String?,
      httpHeaders:
          (result[4] as Map<Object?, Object?>?)?.cast<Object?, Object?>(),
      drmConfigs:
          (result[5] as Map<Object?, Object?>?)?.cast<Object?, Object?>(),
      playerOptions:
          (result[6] as Map<Object?, Object?>?)?.cast<Object?, Object?>(),
    );
  }
}

class MixWithOthersMessage {
  MixWithOthersMessage({required this.mixWithOthers});

  bool mixWithOthers;

  Object encode() {
    return <Object?>[mixWithOthers];
  }

  static MixWithOthersMessage decode(Object result) {
    result as List<Object?>;
    return MixWithOthersMessage(mixWithOthers: result[0]! as bool);
  }
}

class GeometryMessage {
  GeometryMessage({
    required this.playerId,
    required this.x,
    required this.y,
    required this.width,
    required this.height,
  });

  int playerId;
  int x;
  int y;
  int width;
  int height;

  Object encode() {
    return <Object?>[playerId, x, y, width, height];
  }

  static GeometryMessage decode(Object result) {
    result as List<Object?>;
    return GeometryMessage(
      playerId: result[0]! as int,
      x: result[1]! as int,
      y: result[2]! as int,
      width: result[3]! as int,
      height: result[4]! as int,
    );
  }
}

class DurationMessage {
  DurationMessage({required this.playerId, this.durationRange});

  int playerId;
  List<int?>? durationRange;

  Object encode() {
    return <Object?>[playerId, durationRange];
  }

  static DurationMessage decode(Object result) {
    result as List<Object?>;
    return DurationMessage(
      playerId: result[0]! as int,
      durationRange: (result[1] as List<Object?>?)?.cast<int?>(),
    );
  }
}

class RotationMessage {
  RotationMessage({required this.playerId, required this.rotation});

  int playerId;
  int rotation;

  Object encode() {
    return <Object?>[playerId, rotation];
  }

  static RotationMessage decode(Object result) {
    result as List<Object?>;
    return RotationMessage(
      playerId: result[0]! as int,
      rotation: result[1]! as int,
    );
  }
}

// ===== FFI Type Definitions =====

typedef _FFIInitializeNative = ffi.Int32 Function();
typedef _FFIInitializeDart = int Function();

typedef _FFICreateNative = ffi.Int64 Function(ffi.Pointer<ffi.Char>);
typedef _FFICreateDart = int Function(ffi.Pointer<ffi.Char>);

typedef _FFIDisposeNative = ffi.Int32 Function(ffi.Int64);
typedef _FFIDisposeDart = int Function(int);

typedef _FFIPlayNative = ffi.Int32 Function(ffi.Int64);
typedef _FFIPlayDart = int Function(int);

typedef _FFIPauseNative = ffi.Int32 Function(ffi.Int64);
typedef _FFIPauseDart = int Function(int);

typedef _FFISeekToNative = ffi.Int32 Function(ffi.Int64, ffi.Int64);
typedef _FFISeekToDart = int Function(int, int);

typedef _FFIGetPositionNative = ffi.Int64 Function(ffi.Int64);
typedef _FFIGetPositionDart = int Function(int);

typedef _FFIGetDurationNative = ffi.Pointer<ffi.Char> Function(ffi.Int64);
typedef _FFIGetDurationDart = ffi.Pointer<ffi.Char> Function(int);

typedef _FFISetVolumeNative = ffi.Int32 Function(ffi.Int64, ffi.Double);
typedef _FFISetVolumeDart = int Function(int, double);

typedef _FFISetPlaybackSpeedNative = ffi.Int32 Function(ffi.Int64, ffi.Double);
typedef _FFISetPlaybackSpeedDart = int Function(int, double);

typedef _FFISetLoopingNative = ffi.Int32 Function(ffi.Int64, ffi.Bool);
typedef _FFISetLoopingDart = int Function(int, bool);

typedef _FFIGetTrackInfoNative = ffi.Pointer<ffi.Char> Function(
    ffi.Int64, ffi.Pointer<ffi.Char>);
typedef _FFIGetTrackInfoDart = ffi.Pointer<ffi.Char> Function(
    int, ffi.Pointer<ffi.Char>);

typedef _FFISetTrackSelectionNative = ffi.Int32 Function(
    ffi.Int64, ffi.Int64, ffi.Pointer<ffi.Char>);
typedef _FFISetTrackSelectionDart = int Function(
    int, int, ffi.Pointer<ffi.Char>);

typedef _FFISetDisplayGeometryNative = ffi.Int32 Function(
    ffi.Int64, ffi.Int32, ffi.Int32, ffi.Int32, ffi.Int32);
typedef _FFISetDisplayGeometryDart = int Function(int, int, int, int, int);

typedef _FFISetDisplayRotateNative = ffi.Int32 Function(ffi.Int64, ffi.Int32);
typedef _FFISetDisplayRotateDart = int Function(int, int);

typedef _FFISetActivateNative = ffi.Int32 Function(ffi.Int64);
typedef _FFISetActivateDart = int Function(int);

typedef _FFISetDeactivateNative = ffi.Int32 Function(ffi.Int64);
typedef _FFISetDeactivateDart = int Function(int);

typedef _FFISetMixWithOthersNative = ffi.Int32 Function(ffi.Bool);
typedef _FFISetMixWithOthersDart = int Function(bool);

typedef _FFISuspendNative = ffi.Int32 Function(ffi.Int64);
typedef _FFISuspendDart = int Function(int);

typedef _FFIRestoreNative = ffi.Int32 Function(
    ffi.Int64, ffi.Pointer<ffi.Char>, ffi.Int64);
typedef _FFIRestoreDart = int Function(int, ffi.Pointer<ffi.Char>, int);

// ===== Helper Functions =====

ffi.Pointer<ffi.Char> _toPointer(String? str) {
  if (str == null) return ffi.nullptr;
  final units = utf8.encode(str);
  final result = calloc.allocate<ffi.Uint8>(units.length + 1);
  final Uint8List nativeString = result.asTypedList(units.length + 1);
  nativeString.setAll(0, units);
  nativeString[units.length] = 0; // null terminator
  return result.cast<ffi.Char>();
}

void _freePointer(ffi.Pointer<ffi.Char> ptr) {
  if (ptr != ffi.nullptr) {
    calloc.free(ptr);
  }
}

// ===== FFI Bindings =====

class VideoPlayerFFIBindings {
  static VideoPlayerFFIBindings? _instance;
  ffi.DynamicLibrary? _lib;

  late int Function() _ffiInitialize;
  late int Function(ffi.Pointer<ffi.Char>) _ffiCreate;

  late int Function(int) _ffiDispose;
  late int Function(int) _ffiPlay;
  late int Function(int) _ffiPause;
  late int Function(int, int) _ffiSeekTo;
  late int Function(int) _ffiGetPosition;
  late ffi.Pointer<ffi.Char> Function(int) _ffiGetDuration;
  late int Function(int, double) _ffiSetVolume;
  late int Function(int, double) _ffiSetPlaybackSpeed;
  late int Function(int, bool) _ffiSetLooping;
  late ffi.Pointer<ffi.Char> Function(int, ffi.Pointer<ffi.Char>)
      _ffiGetTrackInfo;
  late int Function(int, int, ffi.Pointer<ffi.Char>) _ffiSetTrackSelection;
  late int Function(int, int, int, int, int) _ffiSetDisplayGeometry;
  late int Function(int, int) _ffiSetDisplayRotate;
  late int Function(int) _ffiSuspend;
  late int Function(int, ffi.Pointer<ffi.Char>, int) _ffiRestore;
  late int Function(int) _ffiSetActivate;
  late int Function(int) _ffiSetDeactivate;
  late int Function(bool) _ffiSetMixWithOthers;

  static VideoPlayerFFIBindings get instance {
    _instance ??= VideoPlayerFFIBindings._();
    return _instance!;
  }

  VideoPlayerFFIBindings._();

  /// Load the native library - must be called before using FFI functions
  void load() {
    if (_lib != null) return;

    try {
      // On Tizen, the plugin is statically linked, so we use process() to access
      // symbols from the main executable
      _lib = ffi.DynamicLibrary.process();

      _ffiInitialize = _lib!
          .lookup<ffi.NativeFunction<_FFIInitializeNative>>('ffi_initialize')
          .asFunction<_FFIInitializeDart>();

      _ffiCreate = _lib!
          .lookup<ffi.NativeFunction<_FFICreateNative>>('ffi_create')
          .asFunction<_FFICreateDart>();

      _ffiDispose = _lib!
          .lookup<ffi.NativeFunction<_FFIDisposeNative>>('ffi_dispose')
          .asFunction<_FFIDisposeDart>();

      _ffiPlay = _lib!
          .lookup<ffi.NativeFunction<_FFIPlayNative>>('ffi_play')
          .asFunction<_FFIPlayDart>();

      _ffiPause = _lib!
          .lookup<ffi.NativeFunction<_FFIPauseNative>>('ffi_pause')
          .asFunction<_FFIPauseDart>();

      _ffiSeekTo = _lib!
          .lookup<ffi.NativeFunction<_FFISeekToNative>>('ffi_seek_to')
          .asFunction<_FFISeekToDart>();

      _ffiGetPosition = _lib!
          .lookup<ffi.NativeFunction<_FFIGetPositionNative>>('ffi_get_position')
          .asFunction<_FFIGetPositionDart>();

      _ffiGetDuration = _lib!
          .lookup<ffi.NativeFunction<_FFIGetDurationNative>>('ffi_get_duration')
          .asFunction<_FFIGetDurationDart>();

      _ffiSetVolume = _lib!
          .lookup<ffi.NativeFunction<_FFISetVolumeNative>>('ffi_set_volume')
          .asFunction<_FFISetVolumeDart>();

      _ffiSetPlaybackSpeed = _lib!
          .lookup<ffi.NativeFunction<_FFISetPlaybackSpeedNative>>(
              'ffi_set_playback_speed')
          .asFunction<_FFISetPlaybackSpeedDart>();

      _ffiSetLooping = _lib!
          .lookup<ffi.NativeFunction<_FFISetLoopingNative>>('ffi_set_looping')
          .asFunction<_FFISetLoopingDart>();

      _ffiGetTrackInfo = _lib!
          .lookup<ffi.NativeFunction<_FFIGetTrackInfoNative>>(
              'ffi_get_track_info')
          .asFunction<_FFIGetTrackInfoDart>();

      _ffiSetTrackSelection = _lib!
          .lookup<ffi.NativeFunction<_FFISetTrackSelectionNative>>(
              'ffi_set_track_selection')
          .asFunction<_FFISetTrackSelectionDart>();

      _ffiSetDisplayGeometry = _lib!
          .lookup<ffi.NativeFunction<_FFISetDisplayGeometryNative>>(
              'ffi_set_display_geometry')
          .asFunction<_FFISetDisplayGeometryDart>();

      _ffiSetDisplayRotate = _lib!
          .lookup<ffi.NativeFunction<_FFISetDisplayRotateNative>>(
              'ffi_set_display_rotate')
          .asFunction<_FFISetDisplayRotateDart>();

      _ffiSuspend = _lib!
          .lookup<ffi.NativeFunction<_FFISuspendNative>>('ffi_suspend')
          .asFunction<_FFISuspendDart>();

      _ffiRestore = _lib!
          .lookup<ffi.NativeFunction<_FFIRestoreNative>>('ffi_restore')
          .asFunction<_FFIRestoreDart>();

      _ffiSetActivate = _lib!
          .lookup<ffi.NativeFunction<_FFISetActivateNative>>('ffi_set_activate')
          .asFunction<_FFISetActivateDart>();

      _ffiSetDeactivate = _lib!
          .lookup<ffi.NativeFunction<_FFISetDeactivateNative>>(
              'ffi_set_deactivate')
          .asFunction<_FFISetDeactivateDart>();

      _ffiSetMixWithOthers = _lib!
          .lookup<ffi.NativeFunction<_FFISetMixWithOthersNative>>(
              'ffi_set_mix_with_others')
          .asFunction<_FFISetMixWithOthersDart>();

      debugPrint('FFI bindings loaded successfully');
    } catch (e) {
      debugPrint('Failed to load FFI bindings: $e');
      rethrow;
    }
  }

  bool get isLoaded => _lib != null;

  /// FFI initialize function
  int ffiInitialize() {
    if (!isLoaded) {
      throw StateError('FFI bindings not loaded. Call load() first.');
    }
    return _ffiInitialize();
  }

  /// FFI create function - takes a single JSON string for all parameters
  int ffiCreate({
    String? uri,
    String? asset,
    String? packageName,
    String? formatHint,
    Map<Object?, Object?>? httpHeaders,
    Map<Object?, Object?>? drmConfigs,
    Map<Object?, Object?>? playerOptions,
  }) {
    if (!isLoaded) {
      throw StateError('FFI bindings not loaded. Call load() first.');
    }

    // Build JSON string from individual parameters
    final Map<String, dynamic> jsonMap = <String, dynamic>{};
    if (uri != null && uri.isNotEmpty) jsonMap['uri'] = uri;
    if (asset != null && asset.isNotEmpty) jsonMap['asset'] = asset;
    if (packageName != null && packageName.isNotEmpty)
      jsonMap['packageName'] = packageName;
    if (formatHint != null && formatHint.isNotEmpty)
      jsonMap['formatHint'] = formatHint;
    if (httpHeaders != null && httpHeaders.isNotEmpty)
      jsonMap['httpHeaders'] = httpHeaders;
    if (drmConfigs != null && drmConfigs.isNotEmpty)
      jsonMap['drmConfigs'] = drmConfigs;
    if (playerOptions != null && playerOptions.isNotEmpty)
      jsonMap['playerOptions'] = playerOptions;

    final String jsonString = jsonEncode(jsonMap);
    final jsonPtr = _toPointer(jsonString);

    try {
      return _ffiCreate(jsonPtr);
    } finally {
      _freePointer(jsonPtr);
    }
  }
}

// ===== FFI API Class =====

class VideoPlayerFFIApi {
  int initialize() {
    return ffiInitialize();
  }

  int create({
    String? uri,
    String? asset,
    String? packageName,
    String? formatHint,
    Map<Object?, Object?>? httpHeaders,
    Map<Object?, Object?>? drmConfigs,
    Map<Object?, Object?>? playerOptions,
  }) {
    return ffiCreate(
      uri: uri,
      asset: asset,
      packageName: packageName,
      formatHint: formatHint,
      httpHeaders: httpHeaders,
      drmConfigs: drmConfigs,
      playerOptions: playerOptions,
    );
  }

  int dispose(int playerId) {
    return ffiDispose(playerId);
  }

  int play(int playerId) {
    return ffiPlay(playerId);
  }

  int pause(int playerId) {
    return ffiPause(playerId);
  }

  int seekTo(int playerId, int positionMs) {
    return ffiSeekTo(playerId, positionMs);
  }

  int getPosition(int playerId) {
    return ffiGetPosition(playerId);
  }

  DurationMessage duration(int playerId) {
    return ffiGetDuration(playerId);
  }

  int setVolume(int playerId, double volume) {
    return ffiSetVolume(playerId, volume);
  }

  int setPlaybackSpeed(int playerId, double speed) {
    return ffiSetPlaybackSpeed(playerId, speed);
  }

  int setLooping(int playerId, bool isLooping) {
    return ffiSetLooping(playerId, isLooping);
  }

  int setDisplayGeometry(int playerId, int x, int y, int width, int height) {
    return ffiSetDisplayGeometry(playerId, x, y, width, height);
  }

  int setDisplayRotate(int playerId, int rotation) {
    return ffiSetDisplayRotate(playerId, rotation);
  }

  int suspend(int playerId) {
    return ffiSuspend(playerId);
  }

  int restore(int playerId, String? createMessageJson, int resumeTime) {
    return ffiRestore(playerId, createMessageJson, resumeTime);
  }

  int setActivate(int playerId) {
    return ffiSetActivate(playerId);
  }

  int setDeactivate(int playerId) {
    return ffiSetDeactivate(playerId);
  }

  String getTrackInfo(int playerId, String trackType) {
    return ffiGetTrackInfo(playerId, trackType);
  }

  int setTrackSelection(int playerId, int trackId, String trackType) {
    return ffiSetTrackSelection(playerId, trackId, trackType);
  }

  int setMixWithOthers(bool mixWithOthers) {
    return ffiSetMixWithOthers(mixWithOthers);
  }
}

// ===== Top-level FFI Functions =====

int ffiInitialize() {
  final bindings = VideoPlayerFFIBindings.instance;
  if (!bindings.isLoaded) {
    bindings.load();
  }
  return bindings.ffiInitialize();
}

int ffiCreate({
  String? uri,
  String? asset,
  String? packageName,
  String? formatHint,
  Map<Object?, Object?>? httpHeaders,
  Map<Object?, Object?>? drmConfigs,
  Map<Object?, Object?>? playerOptions,
}) {
  final bindings = VideoPlayerFFIBindings.instance;
  if (!bindings.isLoaded) {
    bindings.load();
  }
  return bindings.ffiCreate(
    uri: uri,
    asset: asset,
    packageName: packageName,
    formatHint: formatHint,
    httpHeaders: httpHeaders,
    drmConfigs: drmConfigs,
    playerOptions: playerOptions,
  );
}

int ffiDispose(int playerId) {
  final bindings = VideoPlayerFFIBindings.instance;
  if (!bindings.isLoaded) {
    bindings.load();
  }
  return bindings._ffiDispose(playerId);
}

int ffiPlay(int playerId) {
  final bindings = VideoPlayerFFIBindings.instance;
  if (!bindings.isLoaded) {
    bindings.load();
  }
  return bindings._ffiPlay(playerId);
}

int ffiPause(int playerId) {
  final bindings = VideoPlayerFFIBindings.instance;
  if (!bindings.isLoaded) {
    bindings.load();
  }
  return bindings._ffiPause(playerId);
}

int ffiSeekTo(int playerId, int positionMs) {
  final bindings = VideoPlayerFFIBindings.instance;
  if (!bindings.isLoaded) {
    bindings.load();
  }
  return bindings._ffiSeekTo(playerId, positionMs);
}

int ffiGetPosition(int playerId) {
  final bindings = VideoPlayerFFIBindings.instance;
  if (!bindings.isLoaded) {
    bindings.load();
  }
  return bindings._ffiGetPosition(playerId);
}

DurationMessage ffiGetDuration(int playerId) {
  final bindings = VideoPlayerFFIBindings.instance;
  if (!bindings.isLoaded) {
    bindings.load();
  }
  final ptr = bindings._ffiGetDuration(playerId);
  if (ptr == ffi.nullptr) {
    throw Exception('FFI getDuration failed - returned null pointer');
  }
  try {
    final bytes = ptr.cast<ffi.Uint8>();
    int length = 0;
    while (bytes[length] != 0) {
      length++;
    }
    final jsonString = utf8.decode(bytes.asTypedList(length));
    if (jsonString == '-1') {
      throw Exception('FFI getDuration failed');
    }
    final Map<String, dynamic> jsonMap = jsonDecode(jsonString);
    return DurationMessage(
      playerId: playerId,
      durationRange: [
        jsonMap['start'] as int,
        jsonMap['end'] as int,
      ],
    );
  } finally {
    calloc.free(ptr);
  }
}

int ffiSetVolume(int playerId, double volume) {
  final bindings = VideoPlayerFFIBindings.instance;
  if (!bindings.isLoaded) {
    bindings.load();
  }
  return bindings._ffiSetVolume(playerId, volume);
}

int ffiSetPlaybackSpeed(int playerId, double speed) {
  final bindings = VideoPlayerFFIBindings.instance;
  if (!bindings.isLoaded) {
    bindings.load();
  }
  return bindings._ffiSetPlaybackSpeed(playerId, speed);
}

int ffiSetLooping(int playerId, bool isLooping) {
  final bindings = VideoPlayerFFIBindings.instance;
  if (!bindings.isLoaded) {
    bindings.load();
  }
  return bindings._ffiSetLooping(playerId, isLooping);
}

int ffiSetDisplayGeometry(int playerId, int x, int y, int width, int height) {
  final bindings = VideoPlayerFFIBindings.instance;
  if (!bindings.isLoaded) {
    bindings.load();
  }
  return bindings._ffiSetDisplayGeometry(playerId, x, y, width, height);
}

int ffiSetDisplayRotate(int playerId, int rotation) {
  final bindings = VideoPlayerFFIBindings.instance;
  if (!bindings.isLoaded) {
    bindings.load();
  }
  return bindings._ffiSetDisplayRotate(playerId, rotation);
}

int ffiSuspend(int playerId) {
  final bindings = VideoPlayerFFIBindings.instance;
  if (!bindings.isLoaded) {
    bindings.load();
  }
  return bindings._ffiSuspend(playerId);
}

int ffiRestore(int playerId, String? createMessageJson, int resumeTime) {
  final bindings = VideoPlayerFFIBindings.instance;
  if (!bindings.isLoaded) {
    bindings.load();
  }
  final createMessagePtr = _toPointer(createMessageJson);
  try {
    return bindings._ffiRestore(playerId, createMessagePtr, resumeTime);
  } finally {
    _freePointer(createMessagePtr);
  }
}

int ffiSetActivate(int playerId) {
  final bindings = VideoPlayerFFIBindings.instance;
  if (!bindings.isLoaded) {
    bindings.load();
  }
  return bindings._ffiSetActivate(playerId);
}

int ffiSetDeactivate(int playerId) {
  final bindings = VideoPlayerFFIBindings.instance;
  if (!bindings.isLoaded) {
    bindings.load();
  }
  return bindings._ffiSetDeactivate(playerId);
}

String ffiGetTrackInfo(int playerId, String trackType) {
  final bindings = VideoPlayerFFIBindings.instance;
  if (!bindings.isLoaded) {
    bindings.load();
  }
  final trackTypePtr = _toPointer(trackType);
  ffi.Pointer<ffi.Char>? ptr;
  try {
    ptr = bindings._ffiGetTrackInfo(playerId, trackTypePtr);
    if (ptr == ffi.nullptr) {
      throw Exception('FFI getTrackInfo failed - returned null pointer');
    }
    final bytes = ptr.cast<ffi.Uint8>();
    int length = 0;
    while (bytes[length] != 0) {
      length++;
    }
    final jsonString = utf8.decode(bytes.asTypedList(length));
    if (jsonString == '-1') {
      throw Exception('FFI getTrackInfo failed');
    }
    return jsonString;
  } finally {
    if (ptr != null) {
      calloc.free(ptr.cast());
    }
    _freePointer(trackTypePtr);
  }
}

int ffiSetTrackSelection(int playerId, int trackId, String trackType) {
  final bindings = VideoPlayerFFIBindings.instance;
  if (!bindings.isLoaded) {
    bindings.load();
  }
  final trackTypePtr = _toPointer(trackType);
  try {
    return bindings._ffiSetTrackSelection(playerId, trackId, trackTypePtr);
  } finally {
    _freePointer(trackTypePtr);
  }
}

int ffiSetMixWithOthers(bool mixWithOthers) {
  final bindings = VideoPlayerFFIBindings.instance;
  if (!bindings.isLoaded) {
    bindings.load();
  }
  return bindings._ffiSetMixWithOthers(mixWithOthers);
}

// ===== FFI Event Port Section - Using Dart_PostCObject_DL =====

typedef _FFIInitializeApiDlNative = ffi.Int32 Function(ffi.Pointer<ffi.Void>);
typedef _FFIInitializeApiDlDart = int Function(ffi.Pointer<ffi.Void>);

ffi.Pointer<ffi.NativeFunction<_FFIInitializeApiDlNative>>?
    _ffiInitializeApiDlPtr;
bool _apiDlInitialized = false;

void ffiInitializeApiDL() {
  if (_apiDlInitialized) return;

  final lib = VideoPlayerFFIBindings.instance._lib;
  if (lib == null) return;

  try {
    _ffiInitializeApiDlPtr =
        lib.lookup<ffi.NativeFunction<_FFIInitializeApiDlNative>>(
            'ffi_initialize_api_dl');

    if (_ffiInitializeApiDlPtr != null) {
      _ffiInitializeApiDlPtr!
              .cast<ffi.NativeFunction<_FFIInitializeApiDlNative>>()
              .asFunction<int Function(ffi.Pointer<ffi.Void>)>()(
          ffi.NativeApi.initializeApiDLData);
      _apiDlInitialized = true;
      debugPrint('Dart API DL initialized successfully');
    }
  } catch (e) {
    debugPrint('Failed to initialize Dart API DL: $e');
  }
}

extension RawReceivePortNativePort on RawReceivePort {
  int get nativePort => _rawReceivePortNativePort(this);
}

extension ReceivePortNativePort on ReceivePort {
  int get nativePort => _receivePortNativePort(this);
}

@pragma('vm:never-inline')
int _rawReceivePortNativePort(RawReceivePort port) {
  return port.sendPort.nativePort;
}

@pragma('vm:never-inline')
int _receivePortNativePort(ReceivePort port) {
  return port.sendPort.nativePort;
}

typedef _FFIRegisterEventPortNative = ffi.Void Function(ffi.Int64);
typedef _FFIRegisterEventPortDart = void Function(int);

typedef _FFIUnregisterEventPortNative = ffi.Void Function();
typedef _FFIUnregisterEventPortDart = void Function();

ffi.Pointer<ffi.NativeFunction<_FFIRegisterEventPortNative>>?
    _ffiRegisterEventPortPtr;
ffi.Pointer<ffi.NativeFunction<_FFIUnregisterEventPortNative>>?
    _ffiUnregisterEventPortPtr;
bool _eventPortLoaded = false;

void _loadEventPortBindings(ffi.DynamicLibrary? lib) {
  if (lib == null || _eventPortLoaded) return;

  try {
    _ffiRegisterEventPortPtr =
        lib.lookup<ffi.NativeFunction<_FFIRegisterEventPortNative>>(
            'ffi_register_event_port');

    _ffiUnregisterEventPortPtr =
        lib.lookup<ffi.NativeFunction<_FFIUnregisterEventPortNative>>(
            'ffi_unregister_event_port');

    _eventPortLoaded = true;
    debugPrint('FFI event port bindings loaded successfully');
  } catch (e) {
    debugPrint('Failed to load FFI event port bindings: $e');
  }
}

void ffiRegisterEventPort(int port) {
  if (!_eventPortLoaded) {
    _loadEventPortBindings(VideoPlayerFFIBindings.instance._lib);
  }
  if (_eventPortLoaded && _ffiRegisterEventPortPtr != null) {
    _ffiRegisterEventPortPtr!
        .cast<ffi.NativeFunction<ffi.Void Function(ffi.Int64)>>()
        .asFunction<void Function(int)>()(port);
  }
}

void ffiUnregisterEventPort() {
  if (_eventPortLoaded && _ffiUnregisterEventPortPtr != null) {
    _ffiUnregisterEventPortPtr!
        .cast<ffi.NativeFunction<ffi.Void Function()>>()
        .asFunction<void Function()>()();
  }
}
