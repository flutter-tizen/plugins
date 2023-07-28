// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'drm_configs.dart';

/// Register DRM callback function (No-op implementation).
///
/// This function is a no-op (no-operation) implementation for registering the DRM callback
/// and communication channel to handle license challenges for video playback. It does not
/// perform any actual DRM operations but serves as a placeholder to maintain consistency
/// in the codebase when DRM functionality is not required.
///
/// This function does not perform any communication with the native side or any DRM-related
/// operations. It can be used when DRM functionality is not required, or when a specific DRM
/// plugin or platform support is not available.
///
/// Example usage:
/// ```dart
/// registerDrmCallback(drmConfigs, _playerId);
/// ```
///
/// Note: This function does not have any DRM functionality and should only be used in cases
/// where DRM support is not necessary or when using a DRM plugin that does not require native
/// communication for license handling.
void registerDrmCallback(LicenseCallback licenseCallback, int playerId) {}
