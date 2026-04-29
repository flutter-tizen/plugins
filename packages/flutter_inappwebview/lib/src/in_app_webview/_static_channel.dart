// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter/services.dart';

/// Process-wide method channel used by static InAppWebView APIs.
const MethodChannel IN_APP_WEBVIEW_STATIC_CHANNEL = MethodChannel(
  'com.pichillilorenzo/flutter_inappwebview_manager',
);
