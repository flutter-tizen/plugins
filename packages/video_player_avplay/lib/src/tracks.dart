// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter/foundation.dart';

/// Type of the track.
enum TrackType {
  /// The video track.
  video,

  /// The audio track.
  audio,

  /// The text track.
  text,
}

/// Type of the track subtitle type for [TrackType.text].
enum TextTrackSubtitleType {
  /// The text subtitle.
  text,

  /// The picture subtitle.
  picture,
}

/// A representation of a single track.
///
/// A typical video file will include several [Track]s.Such as [VideoTrack]s, [AudioTrack]s, [TextTrack]s.
class Track {
  /// Creates an instance of [Track].
  ///
  /// The [trackId] and [trackType] arguments are required.
  ///
  const Track({
    required this.trackId,
    required this.trackType,
    required this.mimetype,
  });

  /// The track id of track that uses to determine track.
  final int trackId;

  /// The type of the track.
  final TrackType trackType;

  /// The strack mime type.
  final String mimetype;
}

/// A representation of a video track.
class VideoTrack extends Track {
  /// Creates an instance of [VideoTrack].
  ///
  /// The [width], [height] and [bitrate] argument is required.
  ///
  /// [trackType] is [TrackType.video].
  VideoTrack({
    required super.trackId,
    super.trackType = TrackType.video,
    required super.mimetype,
    required this.width,
    required this.height,
    required this.bitrate,
  });

  /// The width of video track.
  final int width;

  /// The height of video track.
  final int height;

  /// The bitrate of video track.
  final int bitrate;

  @override
  String toString() {
    return '${objectRuntimeType(this, 'VideoTrack')}('
        'trackId: $trackId, '
        'trackType: $trackType, '
        'mimetype: $mimetype, '
        'width: $width, '
        'height: $height, '
        'bitrate: $bitrate),';
  }
}

/// A representation of a audio track.
class AudioTrack extends Track {
  /// Creates an instance of [AudioTrack].
  ///
  /// The [language], [channel] and [bitrate] arguments are required.
  ///
  /// [trackType] is [TrackType.audio].
  AudioTrack({
    required super.trackId,
    super.trackType = TrackType.audio,
    required super.mimetype,
    required this.language,
    required this.channel,
    required this.bitrate,
  });

  /// The language of audio track.
  final String language;

  /// The channel of audio track.
  final int channel;

  /// The bitrate of audio track.
  final int bitrate;

  @override
  String toString() {
    return '${objectRuntimeType(this, 'AudioTrack')}('
        'trackId: $trackId, '
        'trackType: $trackType, '
        'mimetype: $mimetype, '
        'language: $language, '
        'channel: $channel, '
        'bitrate: $bitrate),';
  }
}

/// A representation of a text track.
class TextTrack extends Track {
  /// Creates an instance of [TextTrack].
  ///
  /// The [language] arguments are required.
  ///
  /// [trackType] is [TrackType.text].
  TextTrack({
    required super.trackId,
    super.trackType = TrackType.text,
    required super.mimetype,
    required this.language,
  });

  /// The language of text track.
  final String language;

  @override
  String toString() {
    return '${objectRuntimeType(this, 'TextTrack')}('
        'trackId: $trackId, '
        'trackType: $trackType, '
        'mimetype: $mimetype, '
        'language: $language),';
  }
}
