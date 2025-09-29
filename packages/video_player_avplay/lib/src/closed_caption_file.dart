// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:typed_data' show Uint8List;
import 'package:flutter/foundation.dart'
    show immutable, listEquals, objectRuntimeType;
import 'package:flutter/material.dart';

import 'sub_rip.dart';
import 'web_vtt.dart';

export 'sub_rip.dart' show SubRipCaptionFile;
export 'web_vtt.dart' show WebVTTCaptionFile;

/// A structured representation of a parsed closed caption file.
///
/// A closed caption file includes a list of captions, each with a start and end
/// time for when the given closed caption should be displayed.
///
/// The [captions] are a list of all captions in a file, in the order that they
/// appeared in the file.
///
/// See:
/// * [SubRipCaptionFile].
/// * [WebVTTCaptionFile].
abstract class ClosedCaptionFile {
  /// The full list of captions from a given file.
  ///
  /// The [captions] will be in the order that they appear in the given file.
  List<TextCaption> get textCaptions;
}

/// A representation of a single caption.
///
/// A typical closed captioning file will include several [Caption]s, each
/// linked to a start and end time.
@immutable
sealed class Caption {
  /// Creates a new [Caption] object.
  ///
  /// This constructor is for internal use by subclasses.
  const Caption({
    required this.number,
    required this.start,
    required this.end,
  });

  /// The number that this caption was assigned.
  final int number;

  /// When in the given video should this [Caption] begin displaying.
  final Duration start;

  /// When in the given video should this [Caption] be dismissed.
  final Duration end;
}

/// A representation of a text-based caption.
@immutable
class TextCaption extends Caption {
  /// Creates a new [TextCaption] object.
  const TextCaption({
    required super.number,
    required super.start,
    required super.end,
    required this.text,
    this.subtitleAttributes,
    this.textStyle,
  });

  /// The actual text that should appear on screen to be read between [start]
  /// and [end].
  final String text;

  /// The subtitile attributes associated with this caption.
  final List<SubtitleAttribute?>? subtitleAttributes;

  /// Specifies how the text in the closed caption should look.
  final TextStyle? textStyle;

  /// A no text caption object. This is a caption with [start] and [end] durations of zero,
  /// and an empty [text] string.
  static const TextCaption none = TextCaption(
    number: 0,
    start: Duration.zero,
    end: Duration.zero,
    text: '',
    subtitleAttributes: <SubtitleAttribute?>[],
  );

  static Color? _toColor(int colorValue) {
    String hexValue = colorValue.toRadixString(16);
    if (hexValue.length < 6) {
      hexValue = hexValue.padLeft(6, '0');
    }
    if (hexValue.length == 6) {
      hexValue = 'FF$hexValue';
    }
    if (hexValue.length == 8) {
      return Color(int.parse('0x$hexValue'));
    }
    return null;
  }

  static FontWeight _toWeight(int weightValue) {
    return weightValue == 0 ? FontWeight.normal : FontWeight.bold;
  }

  static FontStyle _toStyle(int styleValue) {
    return styleValue == 0 ? FontStyle.normal : FontStyle.italic;
  }

  /// Specifies the text style according to subtitle attributes.
  static TextStyle? specifyTextStyle(
      List<SubtitleAttribute> subtitleAttributes) {
    TextStyle actualTextStyle = const TextStyle();
    for (final SubtitleAttribute attr in subtitleAttributes) {
      switch (attr.attrType) {
        case SubtitleAttrType.subAttrFontFamily:
          actualTextStyle =
              actualTextStyle.copyWith(fontFamily: attr.attrValue as String);
        case SubtitleAttrType.subAttrFontSize:
          final double fontSize = attr.attrValue as double;
          if (fontSize < 1.0) {
            const double getSysDefaultFontSize = 36.0;
            actualTextStyle = actualTextStyle.copyWith(
                fontSize: getSysDefaultFontSize * fontSize);
          } else {
            actualTextStyle = actualTextStyle.copyWith(fontSize: fontSize);
          }
        case SubtitleAttrType.subAttrFontWeight:
          actualTextStyle = actualTextStyle.copyWith(
              fontWeight: _toWeight(attr.attrValue as int));
        case SubtitleAttrType.subAttrFontStyle:
          actualTextStyle = actualTextStyle.copyWith(
              fontStyle: _toStyle(attr.attrValue as int));
        case SubtitleAttrType.subAttrFontColor:
          if (actualTextStyle.foreground == null) {
            actualTextStyle = actualTextStyle.copyWith(
                color: _toColor(attr.attrValue as int));
          }
        case SubtitleAttrType.subAttrFontBgColor:
          if (actualTextStyle.background == null) {
            actualTextStyle = actualTextStyle.copyWith(
                backgroundColor: _toColor(attr.attrValue as int));
          }
        case SubtitleAttrType.subAttrFontOpacity:
          actualTextStyle = actualTextStyle.copyWith(
              color:
                  actualTextStyle.color!.withOpacity(attr.attrValue as double));
        case SubtitleAttrType.subAttrFontBgOpacity:
          actualTextStyle = actualTextStyle.copyWith(
              backgroundColor: actualTextStyle.backgroundColor!
                  .withOpacity(attr.attrValue as double));
        case SubtitleAttrType.subAttrFontTextOutlineColor:
        case SubtitleAttrType.subAttrFontTextOutlineThickness:
        case SubtitleAttrType.subAttrFontTextOutlineBlurRadius:
          // actualTextStyle = actualTextStyle.copyWith(shadows: <Shadow>[
          //   const Shadow(color: Colors.green, offset: Offset(1, 1))
          // ]);
          break;
        case SubtitleAttrType.subAttrRegionXPos:
        case SubtitleAttrType.subAttrRegionYPos:
        case SubtitleAttrType.subAttrRegionWidth:
        case SubtitleAttrType.subAttrRegionHeight:
        case SubtitleAttrType.subAttrWindowXPadding:
        case SubtitleAttrType.subAttrWindowYPadding:
        case SubtitleAttrType.subAttrWindowLeftMargin:
        case SubtitleAttrType.subAttrWindowRightMargin:
        case SubtitleAttrType.subAttrWindowTopMargin:
        case SubtitleAttrType.subAttrWindowBottomMargin:
        case SubtitleAttrType.subAttrWindowBgColor:
        case SubtitleAttrType.subAttrWindowOpacity:
        case SubtitleAttrType.subAttrWindowShowBg:
        case SubtitleAttrType.subAttrFontVerticalAlign:
        case SubtitleAttrType.subAttrFontHorizontalAlign:
        case SubtitleAttrType.subAttrRawSubtitle:
        case SubtitleAttrType.subAttrWebvttCueLine:
        case SubtitleAttrType.subAttrWebvttCueLineNum:
        case SubtitleAttrType.subAttrWebvttCueLineAlign:
        case SubtitleAttrType.subAttrWebvttCueAlign:
        case SubtitleAttrType.subAttrWebvttCueSize:
        case SubtitleAttrType.subAttrWebvttCuePosition:
        case SubtitleAttrType.subAttrWebvttCuePositionAlign:
        case SubtitleAttrType.subAttrWebvttCueVertical:
        case SubtitleAttrType.subAttrTimestamp:
        case SubtitleAttrType.subAttrExtsubInde:
        case SubtitleAttrType.subAttrTypeNone:
          break;
      }
    }
    return actualTextStyle == const TextStyle() ? null : actualTextStyle;
  }

  @override
  String toString() {
    return '${objectRuntimeType(this, 'TextCaption')}('
        'number: $number, '
        'start: $start, '
        'end: $end, '
        'text: $text, '
        'subtitleAttributes: $subtitleAttributes, '
        'textStyle: $textStyle)';
  }

  @override
  bool operator ==(Object other) =>
      identical(this, other) ||
      other is TextCaption &&
          runtimeType == other.runtimeType &&
          number == other.number &&
          start == other.start &&
          end == other.end &&
          text == other.text &&
          listEquals(subtitleAttributes, other.subtitleAttributes) &&
          textStyle == other.textStyle;

  @override
  int get hashCode =>
      Object.hash(number, start, end, text, subtitleAttributes, textStyle);
}

/// A representation of a picture-based caption.
@immutable
class PictureCaption extends Caption {
  /// Creates a new [PictureCaption] object.
  const PictureCaption({
    required super.number,
    required super.start,
    required super.end,
    this.picture,
    this.pictureWidth,
    this.pictureHeight,
  });

  /// The image data for the caption, typically in a format like PNG or JPEG.
  final Uint8List? picture;

  /// Specifies the picture's width.
  final double? pictureWidth;

  /// Specifies the picture's height.
  final double? pictureHeight;

  /// A no picture caption object. This is a caption with [start] and [end] durations of zero,
  /// and an empty [picture].
  static const PictureCaption none = PictureCaption(
    number: 0,
    start: Duration.zero,
    end: Duration.zero,
    pictureWidth: 0.0,
    pictureHeight: 0.0,
  );

  @override
  String toString() {
    return '${objectRuntimeType(this, 'PictureCaption')}('
        'number: $number, '
        'start: $start, '
        'end: $end, '
        'picture: ${picture == null ? 'null' : '${picture?.length} bytes'}, '
        'pictureWidth: $pictureWidth, '
        'pictureHeight: $pictureHeight)';
  }

  @override
  bool operator ==(Object other) =>
      identical(this, other) ||
      other is PictureCaption &&
          runtimeType == other.runtimeType &&
          number == other.number &&
          start == other.start &&
          end == other.end &&
          picture == other.picture &&
          pictureWidth == other.pictureWidth &&
          pictureHeight == other.pictureHeight;

  @override
  int get hashCode =>
      Object.hash(number, start, end, picture, pictureWidth, pictureHeight);
}

/// The different types of subtitle attributes that can be set on the player.
enum SubtitleAttrType {
  /// Subtitle attribute type region x position
  subAttrRegionXPos(SubtitleAttrValueType.double),

  /// Subtitle attribute type region y position
  subAttrRegionYPos(SubtitleAttrValueType.double),

  /// Subtitle attribute type region width
  subAttrRegionWidth(SubtitleAttrValueType.double),

  /// Subtitle attribute type region height
  subAttrRegionHeight(SubtitleAttrValueType.double),

  /// Subtitle attribute type window x padding
  subAttrWindowXPadding(SubtitleAttrValueType.double),

  /// Subtitle attribute type window y padding
  subAttrWindowYPadding(SubtitleAttrValueType.double),

  /// Subtitle attribute type window left margin
  subAttrWindowLeftMargin(SubtitleAttrValueType.int),

  /// Subtitle attribute type window right margin
  subAttrWindowRightMargin(SubtitleAttrValueType.int),

  /// Subtitle attribute type window top margin
  subAttrWindowTopMargin(SubtitleAttrValueType.int),

  /// Subtitle attribute type window bottom margin
  subAttrWindowBottomMargin(SubtitleAttrValueType.int),

  /// Subtitle attribute type window opacity
  subAttrWindowBgColor(SubtitleAttrValueType.int),

  /// Subtitle attribute type window opacity
  subAttrWindowOpacity(SubtitleAttrValueType.double),

  /// Subtitle attribute type window show background
  subAttrWindowShowBg(SubtitleAttrValueType.int),

  /// Subtitle attribute type font family
  subAttrFontFamily(SubtitleAttrValueType.string),

  /// Subtitle attribute type font size
  subAttrFontSize(SubtitleAttrValueType.double),

  /// Subtitle attribute type font color
  subAttrFontWeight(SubtitleAttrValueType.int),

  /// Subtitle attribute type font style
  subAttrFontStyle(SubtitleAttrValueType.int),

  /// Subtitle attribute type font color
  subAttrFontColor(SubtitleAttrValueType.int),

  /// Subtitle attribute type font bg color
  subAttrFontBgColor(SubtitleAttrValueType.int),

  /// Subtitle attribute type font opacity
  subAttrFontOpacity(SubtitleAttrValueType.double),

  /// Subtitle attribute type font bg opacity
  subAttrFontBgOpacity(SubtitleAttrValueType.double),

  /// Subtitle attribute type font text outline color
  subAttrFontTextOutlineColor(SubtitleAttrValueType.int),

  /// Subtitle attribute type font text outline thickness
  subAttrFontTextOutlineThickness(SubtitleAttrValueType.int),

  /// Subtitle attribute type font text outline blur radius
  subAttrFontTextOutlineBlurRadius(SubtitleAttrValueType.int),

  /// Subtitle attribute type font vertical align
  subAttrFontVerticalAlign(SubtitleAttrValueType.int),

  /// Subtitle attribute type font horizontal align
  subAttrFontHorizontalAlign(SubtitleAttrValueType.int),

  /// Subtitle attribute type raw subtitle
  subAttrRawSubtitle(SubtitleAttrValueType.string),

  /// Subtitle attribute type webvtt cue line num type
  subAttrWebvttCueLine(SubtitleAttrValueType.double),

  /// Subtitle attribute type webvtt cue line align
  subAttrWebvttCueLineNum(SubtitleAttrValueType.int),

  /// Subtitle attribute type webvtt cue line align
  subAttrWebvttCueLineAlign(SubtitleAttrValueType.int),

  /// Subtitle attribute type webvtt cue line align
  subAttrWebvttCueAlign(SubtitleAttrValueType.int),

  /// Subtitle attribute type webvtt cue size
  subAttrWebvttCueSize(SubtitleAttrValueType.double),

  /// Subtitle attribute type webvtt cue position
  subAttrWebvttCuePosition(SubtitleAttrValueType.double),

  /// Subtitle attribute type webvtt cue position align
  subAttrWebvttCuePositionAlign(SubtitleAttrValueType.int),

  /// Subtitle attribute type webvtt cue vertical
  subAttrWebvttCueVertical(SubtitleAttrValueType.int),

  /// Subtitle attribute type timestamp
  subAttrTimestamp(SubtitleAttrValueType.int),

  /// File index of external subtitle
  subAttrExtsubInde(SubtitleAttrValueType.none),

  /// Default type
  subAttrTypeNone(SubtitleAttrValueType.none);

  const SubtitleAttrType(this.valueType);

  /// The type of the subtitle attribute value. Do not modify.
  final SubtitleAttrValueType valueType;

  /// Returns the subtitle attribute value type based on the index.
  static SubtitleAttrValueType getValueType(int idx) {
    return SubtitleAttrType.values[idx].valueType;
  }
}

/// The different types of subtitle attribute values that can be set on the player.
enum SubtitleAttrValueType {
  /// Subtitle attribute value type integer or uint32_t
  int,

  /// Subtitle attribute value type float or double
  double,

  /// Subtitle attribute value type string or char*
  string,

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

  /// Parse a subtitle attribute list from the subtitle attribute list which given by eventListener.
  static List<SubtitleAttribute> fromEventSubtitleAttrList(
    List<dynamic>? eventSubtitleAttrList,
  ) {
    final List<SubtitleAttribute> subtitleAttributes = <SubtitleAttribute>[];
    final List<Map<Object?, Object?>?> subtitleAttrList =
        eventSubtitleAttrList!.cast<Map<Object?, Object?>?>();

    for (final Map<Object?, Object?>? attr in subtitleAttrList) {
      final int attrTypeNum = attr!['attrType']! as int;
      final int startTime = attr['startTime']! as int;
      final int stopTime = attr['stopTime']! as int;

      Object attrValue;
      if (SubtitleAttrType.getValueType(attrTypeNum) ==
          SubtitleAttrValueType.double) {
        attrValue = attr['attrValue']! as double;
      } else if (SubtitleAttrType.getValueType(attrTypeNum) ==
          SubtitleAttrValueType.int) {
        attrValue = attr['attrValue']! as int;
      } else if (SubtitleAttrType.getValueType(attrTypeNum) ==
          SubtitleAttrValueType.string) {
        attrValue = attr['attrValue']! as String;
      } else {
        attrValue = 'failed';
      }

      subtitleAttributes.add(
        SubtitleAttribute(
          attrType: SubtitleAttrType.values[attrTypeNum],
          startTime: startTime,
          stopTime: stopTime,
          attrValue: attrValue,
        ),
      );
    }
    return subtitleAttributes;
  }

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
