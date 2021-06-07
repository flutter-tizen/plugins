// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_WEBVIEW_FLUTTER_TIZEN_WEBVIEW_BUFFER_POOL_H_
#define FLUTTER_PLUGIN_WEBVIEW_FLUTTER_TIZEN_WEBVIEW_BUFFER_POOL_H_

#include <tbm_surface.h>

#include <memory>
#include <mutex>
#include <string>
#include <vector>

class BufferUnit {
 public:
  explicit BufferUnit(int index, int width, int height);
  ~BufferUnit();
  void Reset(int width, int height);
  bool MarkInUse();
  void UnmarkInUse();
  int Index();
  bool IsUsed();
  tbm_surface_h Surface();

 private:
  bool isUsed_;
  int index_;
  int width_;
  int height_;
  tbm_surface_h tbm_surface_;
};

class BufferPool {
 public:
  explicit BufferPool(int width, int height);
  ~BufferPool();

  BufferUnit* GetAvailableBuffer();
  BufferUnit* Find(tbm_surface_h surface);
  void Release(BufferUnit* unit);
  void Prepare(int with, int height);

 private:
  std::mutex mutex_;
  std::vector<std::unique_ptr<BufferUnit>> pool_;
};

#endif
