// Copyright 2025 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter/foundation.dart';
import 'package:flutter/foundation.dart' show immutable, objectRuntimeType;

/// The different types of subtitle attributes that can be set on the player.
enum SubtitleAttrType {
  /// Subtitle attribute type region x position
  kSubAttrRegionXPos,

  /// Subtitle attribute type region y position
  kSubAttrRegionYPos,

  /// Subtitle attribute type region width
  kSubAttrRegionWidth,

  /// Subtitle attribute type region height
  kSubAttrRegionHeight,

  /// Subtitle attribute type window x padding
  kSubAttrWindowXPadding,

  /// Subtitle attribute type window y padding
  kSubAttrWindowYPadding,

  /// Subtitle attribute type window left margin
  kSubAttrWindowLeftMargin,

  /// Subtitle attribute type window right margin
  kSubAttrWindowRightMargin,

  /// Subtitle attribute type window top margin
  kSubAttrWindowTopMargin,

  /// Subtitle attribute type window bottom margin
  kSubAttrWindowBottomMargin,

  /// Subtitle attribute type window opacity
  kSubAttrWindowBgColor,

  /// Subtitle attribute type window opacity
  kSubAttrWindowOpacity,

  /// Subtitle attribute type window show background
  kSubAttrWindowShowBg,

  /// Subtitle attribute type font family
  kSubAttrFontFamily,

  /// Subtitle attribute type font size
  kSubAttrFontSize,

  /// Subtitle attribute type font color
  kSubAttrFontWeight,

  /// Subtitle attribute type font style
  kSubAttrFontStyle,

  /// Subtitle attribute type font color
  kSubAttrFontColor,

  /// Subtitle attribute type font bg color
  kSubAttrFontBgColor,

  /// Subtitle attribute type font opacity
  kSubAttrFontOpacity,

  /// Subtitle attribute type font bg opacity
  kSubAttrFontBgOpacity,

  /// Subtitle attribute type font text outline color
  kSubAttrFontTextOutlineColor,

  /// Subtitle attribute type font text outline thickness
  kSubAttrFontTextOutlineThickness,

  /// Subtitle attribute type font text outline blur radius
  kSubAttrFontTextOutlineBlurRadius,

  /// Subtitle attribute type font vertical align
  kSubAttrFontVerticalAlign,

  /// Subtitle attribute type font horizontal align
  kSubAttrFontHorizontalAlign,

  /// Subtitle attribute type raw subtitle
  kSubAttrRawSubtitle,

  /// Subtitle attribute type webvtt cue line num type
  kSubAttrWebvttCueLine,

  /// Subtitle attribute type webvtt cue line align
  kSubAttrWebvttCueLineNum,

  /// Subtitle attribute type webvtt cue line align
  kSubAttrWebvttCueLineAlign,

  /// Subtitle attribute type webvtt cue line align
  kSubAttrWebvttCueAlign,

  /// Subtitle attribute type webvtt cue size
  kSubAttrWebvttCueSize,

  /// Subtitle attribute type webvtt cue position
  kSubAttrWebvttCuePosition,

  /// Subtitle attribute type webvtt cue position align
  kSubAttrWebvttCuePositionAlign,

  /// Subtitle attribute type webvtt cue vertical
  kSubAttrWebvttCueVertical,

  /// Subtitle attribute type timestamp
  kSubAttrTimestamp,

  /// File index of external subtitle
  kSubAttrExtsubIndex,

  /// Default type
  kSubAttrTypeNone,
}

/// The different types of subtitle attribute values that can be set on the player.
enum SubtitleAttrValueType {
  /// Subtitle attribute value type integer or uint32_t
  int,

  /// Subtitle attribute value type float or double
  double,

  /// Subtitle attribute value type string or char*
  String,

  /// Subtitle attribute value type none
  none,
}

/// A representation of a single attribute.
///
/// The subtitle attributes of the video.
@immutable
class SubtitleAttribute {
  /// Creates a new instance of [SubtitleAttribute].
  const SubtitleAttribute({
    required this.attrType,
    required this.startTime,
    required this.stopTime,
    required this.attrValue,
  });

  /// Subtitle attribute type of the video.
  ///
  /// Only used if [eventType] is [VideoEventType.subtitleAttrUpdate].
  final SubtitleAttrType attrType;

  /// Subtitle start time of the video.
  ///
  /// Only used if [eventType] is [VideoEventType.subtitleAttrUpdate].
  final int startTime;

  /// Subtitle stop time of the video.
  ///
  /// Only used if [eventType] is [VideoEventType.subtitleAttrUpdate].
  final int stopTime;

  /// Subtitle attribute value of the video.
  ///
  /// Only used if [eventType] is [VideoEventType.subtitleAttrUpdate].
  final Object attrValue;

  /// A no subtitle attribute object.
  static const SubtitleAttribute none = SubtitleAttribute(
    attrType: SubtitleAttrType.kSubAttrTypeNone,
    startTime: 0,
    stopTime: 0,
    attrValue: '',
  );

  @override
  String toString() {
    return '${objectRuntimeType(this, 'SubtitleAttribute')}('
        'attrType: $attrType, '
        'startTime: $startTime, '
        'stopTime: $stopTime, '
        'attrValue: $attrValue)';
  }

  @override
  bool operator ==(Object other) =>
      identical(this, other) ||
      other is SubtitleAttribute &&
          runtimeType == other.runtimeType &&
          attrType == other.attrType &&
          startTime == other.startTime &&
          stopTime == other.stopTime &&
          attrValue == other.attrValue;

  @override
  int get hashCode => Object.hash(attrType, startTime, stopTime, attrValue);
}
