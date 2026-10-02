// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef VIDEO_PLAYER_TIZEN_ENCODABLE_VALUE_COMPAT_H_
#define VIDEO_PLAYER_TIZEN_ENCODABLE_VALUE_COMPAT_H_

#include <flutter/encodable_value.h>

namespace video_player_tizen {

// std::visit does not accept classes derived from std::variant (such as
// flutter::EncodableValue) with libstdc++ older than GCC 11, which the Tizen
// SDK toolchain ships (LLVM-10 clang++ with GCC 9.2 headers). Cast to the
// base variant type before visiting.
//
// This header is force-included into every translation unit via
// USER_CPP_OPTS in project_def.prop, so Pigeon-generated code (messages.cc)
// can call AsVariant() without including this header explicitly. Keep the
// generated-file patch minimal: only the two std::visit call sites in
// messages.cc need to wrap their argument with AsVariant().
inline const ::flutter::internal::EncodableValueVariant& AsVariant(
    const ::flutter::EncodableValue& value) {
  return value;
}

}  // namespace video_player_tizen

#endif  // VIDEO_PLAYER_TIZEN_ENCODABLE_VALUE_COMPAT_H_
