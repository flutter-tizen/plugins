// Copyright 2025 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter/foundation.dart';
import 'package:flutter/foundation.dart' show immutable, objectRuntimeType;

/// The different types of subtitle attributes that can be set on the player.
enum SubtitleAttrType {
  /// Subtitle attribute type region x position
  subAttrRegionXPos,

  /// Subtitle attribute type region y position
  subAttrRegionYPos,

  /// Subtitle attribute type region width
  subAttrRegionWidth,

  /// Subtitle attribute type region height
  subAttrRegionHeight,

  /// Subtitle attribute type window x padding
  subAttrWindowXPadding,

  /// Subtitle attribute type window y padding
  subAttrWindowYPadding,

  /// Subtitle attribute type window left margin
  subAttrWindowLeftMargin,

  /// Subtitle attribute type window right margin
  subAttrWindowRightMargin,

  /// Subtitle attribute type window top margin
  subAttrWindowTopMargin,

  /// Subtitle attribute type window bottom margin
  subAttrWindowBottomMargin,

  /// Subtitle attribute type window opacity
  subAttrWindowBgColor,

  /// Subtitle attribute type window opacity
  subAttrWindowOpacity,

  /// Subtitle attribute type window show background
  subAttrWindowShowBg,

  /// Subtitle attribute type font family
  subAttrFontFamily,

  /// Subtitle attribute type font size
  subAttrFontSize,

  /// Subtitle attribute type font color
  subAttrFontWeight,

  /// Subtitle attribute type font style
  subAttrFontStyle,

  /// Subtitle attribute type font color
  subAttrFontColor,

  /// Subtitle attribute type font bg color
  subAttrFontBgColor,

  /// Subtitle attribute type font opacity
  subAttrFontOpacity,

  /// Subtitle attribute type font bg opacity
  subAttrFontBgOpacity,

  /// Subtitle attribute type font text outline color
  subAttrFontTextOutlineColor,

  /// Subtitle attribute type font text outline thickness
  subAttrFontTextOutlineThickness,

  /// Subtitle attribute type font text outline blur radius
  subAttrFontTextOutlineBlurRadius,

  /// Subtitle attribute type font vertical align
  subAttrFontVerticalAlign,

  /// Subtitle attribute type font horizontal align
  subAttrFontHorizontalAlign,

  /// Subtitle attribute type raw subtitle
  subAttrRawSubtitle,

  /// Subtitle attribute type webvtt cue line num type
  subAttrWebvttCueLine,

  /// Subtitle attribute type webvtt cue line align
  subAttrWebvttCueLineNum,

  /// Subtitle attribute type webvtt cue line align
  subAttrWebvttCueLineAlign,

  /// Subtitle attribute type webvtt cue line align
  subAttrWebvttCueAlign,

  /// Subtitle attribute type webvtt cue size
  subAttrWebvttCueSize,

  /// Subtitle attribute type webvtt cue position
  subAttrWebvttCuePosition,

  /// Subtitle attribute type webvtt cue position align
  subAttrWebvttCuePositionAlign,

  /// Subtitle attribute type webvtt cue vertical
  subAttrWebvttCueVertical,

  /// Subtitle attribute type timestamp
  subAttrTimestamp,

  /// File index of external subtitle
  subAttrExtsubIndex,

  /// Default type
  subAttrTypeNone,
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
    attrType: SubtitleAttrType.subAttrTypeNone,
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
