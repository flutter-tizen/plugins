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

class TrackMessage {
  TrackMessage({required this.playerId, required this.tracks});

  int playerId;
  List<Map<Object?, Object?>?> tracks;

  /// Convert to JSON string for FFI call
  String toJson() {
    final Map<String, dynamic> jsonMap = <String, dynamic>{
      'playerId': playerId,
      'tracks': tracks,
    };
    return jsonEncode(jsonMap);
  }

  /// Create from JSON string
  static TrackMessage fromJson(String jsonString) {
    final Map<String, dynamic> jsonMap = jsonDecode(jsonString);
    return TrackMessage(
      playerId: jsonMap['playerId'] as int,
      tracks: (jsonMap['tracks'] as List<dynamic>)
          .map((e) => (e as Map<String, dynamic>).cast<Object?, Object?>())
          .toList(),
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

  /// Convert to JSON string for FFI call
  String toJson() {
    final Map<String, dynamic> jsonMap = <String, dynamic>{};
    if (asset != null && asset!.isNotEmpty) jsonMap['asset'] = asset;
    if (uri != null && uri!.isNotEmpty) jsonMap['uri'] = uri;
    if (packageName != null && packageName!.isNotEmpty)
      jsonMap['packageName'] = packageName;
    if (formatHint != null && formatHint!.isNotEmpty)
      jsonMap['formatHint'] = formatHint;
    if (httpHeaders != null && httpHeaders!.isNotEmpty)
      jsonMap['httpHeaders'] = httpHeaders;
    if (drmConfigs != null && drmConfigs!.isNotEmpty)
      jsonMap['drmConfigs'] = drmConfigs;
    if (playerOptions != null && playerOptions!.isNotEmpty)
      jsonMap['playerOptions'] = playerOptions;
    return jsonEncode(jsonMap);
  }

  /// Create from JSON string
  static CreateMessage fromJson(String jsonString) {
    final Map<String, dynamic> jsonMap = jsonDecode(jsonString);
    return CreateMessage(
      asset: jsonMap['asset'] as String?,
      uri: jsonMap['uri'] as String?,
      packageName: jsonMap['packageName'] as String?,
      formatHint: jsonMap['formatHint'] as String?,
      httpHeaders: (jsonMap['httpHeaders'] as Map?)?.cast<Object?, Object?>(),
      drmConfigs: (jsonMap['drmConfigs'] as Map?)?.cast<Object?, Object?>(),
      playerOptions:
          (jsonMap['playerOptions'] as Map?)?.cast<Object?, Object?>(),
    );
  }
}

class DurationMessage {
  DurationMessage({required this.playerId, this.durationRange});

  int playerId;
  List<int?>? durationRange;

  /// Convert to JSON string for FFI call
  String toJson() {
    final Map<String, dynamic> jsonMap = <String, dynamic>{
      'playerId': playerId,
      if (durationRange != null) 'durationRange': durationRange,
    };
    return jsonEncode(jsonMap);
  }

  /// Create from JSON string
  /// C++ returns: {"playerId": <int>, "durationRange": [<start>, <end>]}
  static DurationMessage fromJson(String jsonString) {
    final Map<String, dynamic> jsonMap = jsonDecode(jsonString);
    return DurationMessage(
      playerId: jsonMap['playerId'] as int,
      durationRange: (jsonMap['durationRange'] as List<dynamic>?)
          ?.map((e) => (e as num).toInt())
          .toList(),
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

// Check if video is live stream - returns 1 if live, 0 if VOD
typedef _FFIIsLiveNative = ffi.Int32 Function(ffi.Int64);
typedef _FFIIsLiveDart = int Function(int);

typedef _FFISuspendNative = ffi.Int32 Function(ffi.Int64);
typedef _FFISuspendDart = int Function(int);

// P0-3 fix: restore returns int (0 on success, -1 on failure)
// Player ID remains unchanged after restore
typedef _FFIRestoreNative = ffi.Int32 Function(
    ffi.Int64, ffi.Pointer<ffi.Char>, ffi.Int64);
typedef _FFIRestoreDart = int Function(int, ffi.Pointer<ffi.Char>, int);

// P0-1 fix: FFI string memory management
typedef _FFIFreeStringNative = ffi.Void Function(ffi.Pointer<ffi.Char>);
typedef _FFIFreeStringDart = void Function(ffi.Pointer<ffi.Char>);

// P0-2 fix: Per-player event port registration
typedef _FFIRegisterPlayerEventPortNative = ffi.Void Function(
    ffi.Int64, ffi.Int64);
typedef _FFIRegisterPlayerEventPortDart = void Function(int, int);

typedef _FFIUnregisterPlayerEventPortNative = ffi.Void Function(ffi.Int64);
typedef _FFIUnregisterPlayerEventPortDart = void Function(int);

// P1-2 fix: Unregister all player event ports (for hot restart cleanup)
typedef _FFIUnregisterAllPlayerEventPortsNative = ffi.Void Function();
typedef _FFIUnregisterAllPlayerEventPortsDart = void Function();

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
  // P0-3 fix: restore returns int (0 on success, -1 on failure)
  late int Function(int, ffi.Pointer<ffi.Char>, int) _ffiRestore;
  late int Function(int) _ffiSetActivate;
  late int Function(int) _ffiSetDeactivate;
  late int Function(bool) _ffiSetMixWithOthers;
  late int Function(int) _ffiIsLive;
  // P0-1 fix: FFI string memory management
  late void Function(ffi.Pointer<ffi.Char>) _ffiFreeString;
  // P0-2 fix: Per-player event port registration
  late void Function(int, int) _ffiRegisterPlayerEventPort;
  late void Function(int) _ffiUnregisterPlayerEventPort;
  // P1-2 fix: Unregister all player event ports
  late void Function() _ffiUnregisterAllPlayerEventPorts;

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

      // P0-1 fix: FFI string memory management
      _ffiFreeString = _lib!
          .lookup<ffi.NativeFunction<_FFIFreeStringNative>>('ffi_free_string')
          .asFunction<_FFIFreeStringDart>();

      // P0-2 fix: Per-player event port registration
      _ffiRegisterPlayerEventPort = _lib!
          .lookup<ffi.NativeFunction<_FFIRegisterPlayerEventPortNative>>(
              'ffi_register_player_event_port')
          .asFunction<_FFIRegisterPlayerEventPortDart>();
      _ffiUnregisterPlayerEventPort = _lib!
          .lookup<ffi.NativeFunction<_FFIUnregisterPlayerEventPortNative>>(
              'ffi_unregister_player_event_port')
          .asFunction<_FFIUnregisterPlayerEventPortDart>();

      // P1-2 fix: Unregister all player event ports
      _ffiUnregisterAllPlayerEventPorts = _lib!
          .lookup<ffi.NativeFunction<_FFIUnregisterAllPlayerEventPortsNative>>(
              'ffi_unregister_all_player_event_ports')
          .asFunction<_FFIUnregisterAllPlayerEventPortsDart>();

      _ffiIsLive = _lib!
          .lookup<ffi.NativeFunction<_FFIIsLiveNative>>('ffi_is_live')
          .asFunction<_FFIIsLiveDart>();

      debugPrint('FFI bindings loaded successfully');
    } catch (e) {
      debugPrint('Failed to load FFI bindings: $e');
      rethrow;
    }
  }

  bool get isLoaded => _lib != null;
}

// ===== FFI API Class =====

class VideoPlayerVideoholeFFIApi {
  /// Initialize the FFI bindings
  int initialize() {
    final bindings = VideoPlayerFFIBindings.instance;
    if (!bindings.isLoaded) {
      bindings.load();
    }
    return bindings._ffiInitialize();
  }

  /// Create using CreateMessage object
  int create(CreateMessage message) {
    final bindings = VideoPlayerFFIBindings.instance;
    if (!bindings.isLoaded) {
      bindings.load();
    }
    final String jsonString = message.toJson();
    final jsonPtr = _toPointer(jsonString);
    try {
      return bindings._ffiCreate(jsonPtr);
    } finally {
      _freePointer(jsonPtr);
    }
  }

  /// Restore using CreateMessage object
  int restore(int playerId, CreateMessage? message, int resumeTime) {
    final bindings = VideoPlayerFFIBindings.instance;
    if (!bindings.isLoaded) {
      bindings.load();
    }
    final String? jsonString = message?.toJson();
    final createMessagePtr = _toPointer(jsonString);
    try {
      return bindings._ffiRestore(playerId, createMessagePtr, resumeTime);
    } finally {
      _freePointer(createMessagePtr);
    }
  }

  int dispose(int playerId) {
    final bindings = VideoPlayerFFIBindings.instance;
    if (!bindings.isLoaded) {
      bindings.load();
    }
    return bindings._ffiDispose(playerId);
  }

  int play(int playerId) {
    final bindings = VideoPlayerFFIBindings.instance;
    if (!bindings.isLoaded) {
      bindings.load();
    }
    return bindings._ffiPlay(playerId);
  }

  int pause(int playerId) {
    final bindings = VideoPlayerFFIBindings.instance;
    if (!bindings.isLoaded) {
      bindings.load();
    }
    return bindings._ffiPause(playerId);
  }

  int seekTo(int playerId, int positionMs) {
    final bindings = VideoPlayerFFIBindings.instance;
    if (!bindings.isLoaded) {
      bindings.load();
    }
    return bindings._ffiSeekTo(playerId, positionMs);
  }

  int getPosition(int playerId) {
    final bindings = VideoPlayerFFIBindings.instance;
    if (!bindings.isLoaded) {
      bindings.load();
    }
    return bindings._ffiGetPosition(playerId);
  }

  DurationMessage duration(int playerId) {
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
      return DurationMessage.fromJson(jsonString);
    } finally {
      bindings._ffiFreeString(ptr);
    }
  }

  int setVolume(int playerId, double volume) {
    final bindings = VideoPlayerFFIBindings.instance;
    if (!bindings.isLoaded) {
      bindings.load();
    }
    return bindings._ffiSetVolume(playerId, volume);
  }

  int setPlaybackSpeed(int playerId, double speed) {
    final bindings = VideoPlayerFFIBindings.instance;
    if (!bindings.isLoaded) {
      bindings.load();
    }
    return bindings._ffiSetPlaybackSpeed(playerId, speed);
  }

  int setLooping(int playerId, bool isLooping) {
    final bindings = VideoPlayerFFIBindings.instance;
    if (!bindings.isLoaded) {
      bindings.load();
    }
    return bindings._ffiSetLooping(playerId, isLooping);
  }

  int setDisplayGeometry(int playerId, int x, int y, int width, int height) {
    final bindings = VideoPlayerFFIBindings.instance;
    if (!bindings.isLoaded) {
      bindings.load();
    }
    return bindings._ffiSetDisplayGeometry(playerId, x, y, width, height);
  }

  int setDisplayRotate(int playerId, int rotation) {
    final bindings = VideoPlayerFFIBindings.instance;
    if (!bindings.isLoaded) {
      bindings.load();
    }
    return bindings._ffiSetDisplayRotate(playerId, rotation);
  }

  int suspend(int playerId) {
    final bindings = VideoPlayerFFIBindings.instance;
    if (!bindings.isLoaded) {
      bindings.load();
    }
    return bindings._ffiSuspend(playerId);
  }

  int setActivate(int playerId) {
    final bindings = VideoPlayerFFIBindings.instance;
    if (!bindings.isLoaded) {
      bindings.load();
    }
    return bindings._ffiSetActivate(playerId);
  }

  int setDeactivate(int playerId) {
    final bindings = VideoPlayerFFIBindings.instance;
    if (!bindings.isLoaded) {
      bindings.load();
    }
    return bindings._ffiSetDeactivate(playerId);
  }

  TrackMessage getTrackInfo(int playerId, String trackType) {
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
      return TrackMessage.fromJson(jsonString);
    } finally {
      if (ptr != null) {
        bindings._ffiFreeString(ptr);
      }
      _freePointer(trackTypePtr);
    }
  }

  int setTrackSelection(int playerId, int trackId, String trackType) {
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

  int setMixWithOthers(bool mixWithOthers) {
    final bindings = VideoPlayerFFIBindings.instance;
    if (!bindings.isLoaded) {
      bindings.load();
    }
    return bindings._ffiSetMixWithOthers(mixWithOthers);
  }

  // P0-2 fix: Per-player event port registration
  void registerPlayerEventPort(int playerId, int port) {
    final bindings = VideoPlayerFFIBindings.instance;
    if (!bindings.isLoaded) {
      bindings.load();
    }
    bindings._ffiRegisterPlayerEventPort(playerId, port);
  }

  void unregisterPlayerEventPort(int playerId) {
    final bindings = VideoPlayerFFIBindings.instance;
    if (!bindings.isLoaded) {
      bindings.load();
    }
    bindings._ffiUnregisterPlayerEventPort(playerId);
  }

  // P1-2 fix: Unregister all player event ports (for hot restart cleanup)
  void unregisterAllPlayerEventPorts() {
    final bindings = VideoPlayerFFIBindings.instance;
    if (!bindings.isLoaded) {
      bindings.load();
    }
    bindings._ffiUnregisterAllPlayerEventPorts();
  }

  /// Check if video is live stream
  /// Returns true if live, false if VOD
  bool isLive(int playerId) {
    final bindings = VideoPlayerFFIBindings.instance;
    if (!bindings.isLoaded) {
      bindings.load();
    }
    final int result = bindings._ffiIsLive(playerId);
    return result != 0;
  }
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
