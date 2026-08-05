// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "video_player_tizen_plugin.h"

#include <flutter/plugin_registrar.h>
#include <flutter_tizen.h>

#include "ffi_messages.h"
#include "log.h"

namespace video_player_videohole_tizen {

extern "C" {

void VideoPlayerTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar_ref) {
  auto* plugin_registrar =
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar_ref);

  // Inject registrar reference into FFI layer
  ffi_set_plugin_registrar(registrar_ref, plugin_registrar);

  LOG_INFO("[VideoPlayerTizenPlugin] Registered with registrar");
}

}  // extern "C"

}  // namespace video_player_videohole_tizen
