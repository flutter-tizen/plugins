// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:flutter/services.dart';

import 'tizen_stt_event.dart';
import 'tizen_stt_state.dart';

export 'tizen_stt_event.dart';
export 'tizen_stt_state.dart';

/// Speech recognition through the Tizen STT service.
///
/// All instances share one native session. Subscribe to [events] before use,
/// and call [dispose] when finished. This plugin uses the engine's recorder;
/// microphone routing and recognition support depend on the installed engine.
class TizenStt {
  static const MethodChannel _channel = MethodChannel('tizen_stt');
  static const EventChannel _eventChannel = EventChannel('tizen_stt/events');
  static Stream<TizenSttEvent>? _events;

  /// Broadcast stream of state changes, partial/final results and errors.
  Stream<TizenSttEvent> get events =>
      _events ??= _eventChannel.receiveBroadcastStream().map((dynamic event) =>
          TizenSttEvent.fromMap(event as Map<Object?, Object?>));

  /// Creates the session and waits for READY, or throws [PlatformException].
  ///
  /// Preparation times out after 10 seconds. Call once before starting;
  /// concurrent initialization requests are rejected. If already prepared,
  /// returns the current state without interrupting recognition.
  Future<TizenSttState> initialize() => _invokeState('initialize');

  /// Starts recording after [initialize] has completed.
  ///
  /// A null [language] uses the engine's automatic language selection.
  /// Otherwise use a value returned by [getLanguages], such as `en_US`.
  /// Recognition types and partial results depend on the installed engine.
  /// The returned state is a snapshot; use [events] for later transitions.
  Future<TizenSttState> startListening({
    String? language,
    String recognitionType = TizenSttRecognitionType.free,
  }) =>
      _invokeState('start', <String, Object?>{
        'language': language,
        'type': recognitionType,
      });

  /// Stops recording and requests a final result (or error) through [events].
  Future<TizenSttState> stopListening() => _invokeState('stop');

  /// Discards recognition. Cancelling preparation releases the session.
  /// Does nothing when idle or already disposed.
  Future<TizenSttState> cancel() => _invokeState('cancel');

  /// Returns the current session state without creating a session.
  Future<TizenSttState> getState() => _invokeState('getState');

  /// Lists languages supported by the engine after [initialize] completes.
  Future<List<String>> getLanguages() async =>
      (await _channel.invokeListMethod<String>('getLanguages'))!;

  /// Cancels pending work and releases the shared native session.
  /// Safe to call repeatedly. Call [initialize] to use the session again.
  Future<void> dispose() => _channel.invokeMethod<void>('dispose');

  Future<TizenSttState> _invokeState(
    String method, [
    Map<String, Object?>? arguments,
  ]) async {
    final Map<Object?, Object?>? result =
        await _channel.invokeMapMethod<Object?, Object?>(method, arguments);
    return TizenSttState.fromStatus(result!);
  }
}
