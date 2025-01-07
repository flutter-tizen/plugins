// Copyright 2025 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_MESSAGE_DISPATCHER_H_
#define FLUTTER_PLUGIN_MESSAGE_DISPATCHER_H_

#include <functional>

class MessageDispatcher {
 public:
  MessageDispatcher();
  ~MessageDispatcher();

  void dispatchTaskOnMainThread(std::function<void()>&& fn);
};

#endif  // FLUTTER_PLUGIN_MESSAGE_DISPATCHER_H_
