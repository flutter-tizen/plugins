// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/// The DRM scheme for the video.
enum DrmType {
  /// None.
  none,

  /// PlayReady.
  playready,

  /// Widevine CDM.
  widevine,
}

/// Configurations for playing DRM content.
class DrmConfigs {
  /// Creates a new [DrmConfigs].
  const DrmConfigs({
    this.type = DrmType.none,
    this.licenseServerUrl,
  });

  /// The DRM type.
  final DrmType type;

  /// The URL of the server to retrieve the DRM license (optional).
  final String? licenseServerUrl;

  /// Converts to a map.
  Map<String, Object?> toMap() {
    return <String, Object?>{
      'drmType': type.index,
      'licenseServerUrl': licenseServerUrl,
    };
  }
}
