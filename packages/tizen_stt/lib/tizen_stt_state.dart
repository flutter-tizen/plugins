// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/// Lifecycle state of the STT engine, mirroring Tizen's `stt_state_e`.
///
/// Transitions are driven by the engine and surfaced both as the return value
/// of the control methods on `TizenStt` and as `state` events on
/// `TizenStt.events`.
enum TizenSttState {
  /// No engine handle exists yet, or it was released by `dispose()`.
  notCreated,

  /// Handle created but not prepared yet (`STT_STATE_CREATED`).
  created,

  /// Prepared and idle — ready to start listening (`STT_STATE_READY`).
  ready,

  /// Capturing audio (`STT_STATE_RECORDING`).
  recording,

  /// Recognizing the captured audio (`STT_STATE_PROCESSING`).
  processing,

  /// The state could not be determined.
  unknown;

  /// Maps a state name (as emitted by the platform) to a [TizenSttState].
  static TizenSttState fromName(String? name) {
    switch (name) {
      case 'notCreated':
        return TizenSttState.notCreated;
      case 'created':
        return TizenSttState.created;
      case 'ready':
        return TizenSttState.ready;
      case 'recording':
        return TizenSttState.recording;
      case 'processing':
        return TizenSttState.processing;
      default:
        return TizenSttState.unknown;
    }
  }

  /// Extracts the state from a platform status map shaped like
  /// `{ "state": { "code": int, "name": String } }`. Also accepts a bare
  /// `{ "name": String }` map (as carried by `state` events).
  static TizenSttState fromStatus(Map<Object?, Object?> status) {
    final Object? state = status['state'];
    if (state is Map) {
      return fromName(state['name'] as String?);
    }
    return fromName(status['name'] as String?);
  }
}
