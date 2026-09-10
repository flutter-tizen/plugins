// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'tizen_stt_state.dart';

/// Recognition type passed to `TizenStt.startListening`, matching Tizen's
/// `STT_RECOGNITION_TYPE_*` string constants.
///
/// Which types an engine supports varies; [TizenSttRecognitionType.free]
/// (free-form dictation) is the common default.
class TizenSttRecognitionType {
  const TizenSttRecognitionType._();

  /// Free-form dictation (`STT_RECOGNITION_TYPE_FREE`).
  static const String free = 'stt.recognition.type.FREE';

  /// Free-form dictation with partial results (`...FREE.PARTIAL`).
  static const String freePartial = 'stt.recognition.type.FREE.PARTIAL';

  /// Short search phrases (`STT_RECOGNITION_TYPE_SEARCH`).
  static const String search = 'stt.recognition.type.SEARCH';

  /// Web search phrases (`STT_RECOGNITION_TYPE_WEB_SEARCH`).
  static const String webSearch = 'stt.recognition.type.WEB_SEARCH';

  /// Map/navigation phrases (`STT_RECOGNITION_TYPE_MAP`).
  static const String map = 'stt.recognition.type.MAP';
}

/// An event emitted on `TizenStt.events`.
///
/// One of three [type]s: `state` (engine state changed), `result` (recognition
/// output — partial, final or error), or `error`. The original platform payload is always
/// available via [raw]; the typed getters below cover the common cases.
class TizenSttEvent {
  /// Creates an event with its original native payload.
  const TizenSttEvent({
    required this.type,
    required this.raw,
    this.text,
    this.message,
  });

  /// Builds an event from the raw platform map sent over the event channel.
  factory TizenSttEvent.fromMap(Map<Object?, Object?> map) {
    return TizenSttEvent(
      type: map['type'] as String? ?? 'unknown',
      raw: Map<Object?, Object?>.from(map),
      text: map['text'] as String?,
      message: map['message'] as String?,
    );
  }

  /// Event category: `state`, `result`, or `error`.
  final String type;

  /// Recognized text, for `result` events.
  final String? text;

  /// Human-readable recognition message, if provided by the engine.
  final String? message;

  /// The unmodified platform payload, for fields not exposed as getters.
  final Map<Object?, Object?> raw;

  /// Whether this is a `state` transition event.
  bool get isStateChange => type == 'state';

  /// The new engine state, for `state` events; otherwise `null`.
  TizenSttState? get state {
    if (!isStateChange) {
      return null;
    }
    final Object? current = raw['current'];
    if (current is Map) {
      return TizenSttState.fromName(current['name'] as String?);
    }
    return null;
  }

  /// Whether this is a final recognition result.
  bool get isFinalResult => type == 'result' && raw['event'] == 'final';

  /// Whether this is a partial (in-progress) recognition result.
  bool get isPartialResult => type == 'result' && raw['event'] == 'partial';

  /// Whether this is an error event.
  bool get isError =>
      type == 'error' || (type == 'result' && raw['event'] == 'error');

  /// The Tizen error name (e.g. `STT_ERROR_OPERATION_FAILED`), for `error`
  /// events; otherwise `null`.
  String? get errorName => isError ? raw['name'] as String? : null;

  /// Best text to show for this event: recognized [text], else [message],
  /// else the raw payload.
  String get displayText {
    if (text != null && text!.isNotEmpty) {
      return text!;
    }
    if (message != null && message!.isNotEmpty) {
      return message!;
    }
    return raw.toString();
  }
}
