// Copyright 2025 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter/foundation.dart' show immutable, objectRuntimeType;

/// Ad information class for DASH. This class contains all the necessary information about an ad.
@immutable
class AdInfoFromDash {
  /// Create a new instance of [AdInfoFromDash].
  const AdInfoFromDash({
    required this.adId,
    required this.cancelIndicator,
    required this.startTime,
    required this.endTime,
    required this.durationTime,
    required this.outOfNetworkIndicator,
  });

  /// The unique identifier for the ad event from DASH.
  final int adId;

  /// When set to "true", indicates that a previously sent segmentation event, has been cancelled.
  final bool cancelIndicator;

  /// The start time of the ad in milliseconds.
  final int startTime;

  /// The end time of the ad in milliseconds.
  final int endTime;

  /// The duration of the ad in milliseconds.
  final int durationTime;

  /// When set to "1", indicates that the splice event is AD insert start time point, AD start time, end time and Duration are valid.
  /// When set to "0",  indicates that the splice event is AD end time point, only end time is valid.
  final int outOfNetworkIndicator;

  /// Parse the ad information from a map. Returns null if the map is null or empty. Otherwise, returns a new instance of [AdInfoFromDash].
  static AdInfoFromDash? fromAdInfoMap(Map<Object?, Object?>? adInfo) {
    if (adInfo != null && adInfo.isNotEmpty) {
      final int adId = adInfo['id']! as int;
      final int startTime = adInfo['start_ms']! as int;
      final int endTime = adInfo['end_ms']! as int;
      final int durationTime = adInfo['duration_ms']! as int;
      final int outOfNetworkIndicator =
          adInfo['out_of_network_indicator']! as int;
      final bool cancelIndicator = adInfo['cancel_indicator']! as bool;

      return AdInfoFromDash(
        adId: adId,
        cancelIndicator: cancelIndicator,
        startTime: startTime,
        endTime: endTime,
        durationTime: durationTime,
        outOfNetworkIndicator: outOfNetworkIndicator,
      );
    }

    return null;
  }

  @override
  String toString() {
    return '${objectRuntimeType(this, 'AdInfoFromDash')}('
        'adId: $adId, '
        'cancelIndicator: $cancelIndicator, '
        'startTime: $startTime, '
        'endTime: $endTime, '
        'durationTime: $durationTime, '
        'outOfNetworkIndicator: $outOfNetworkIndicator)';
  }

  @override
  bool operator ==(Object other) =>
      identical(this, other) ||
      other is AdInfoFromDash &&
          runtimeType == other.runtimeType &&
          adId == other.adId &&
          cancelIndicator == other.cancelIndicator &&
          startTime == other.startTime &&
          endTime == other.endTime &&
          durationTime == other.durationTime &&
          outOfNetworkIndicator == other.outOfNetworkIndicator;

  @override
  int get hashCode => Object.hash(
        adId,
        cancelIndicator,
        startTime,
        endTime,
        durationTime,
        outOfNetworkIndicator,
      );
}
