// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_WEBVIEW_FLUTTER_TIZEN_WEBVIEW_BUFFER_POOL_H_
#define FLUTTER_PLUGIN_WEBVIEW_FLUTTER_TIZEN_WEBVIEW_BUFFER_POOL_H_

#include <flutter_texture_registrar.h>
#include <tbm_surface.h>

#include <memory>
#include <mutex>
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
  FlutterDesktopGpuBuffer* GpuBuffer();
#ifndef NDEBUG
  void DumpToPng(int file_name);
#endif

 private:
  bool isUsed_;
  int index_;
  int width_;
  int height_;
  tbm_surface_h tbm_surface_;
  FlutterDesktopGpuBuffer* gpu_buffer_;
};

class BufferPool {
 public:
  explicit BufferPool(int width, int height, int pool_size);
  ~BufferPool();

  virtual BufferUnit* GetAvailableBuffer();
  virtual BufferUnit* Find(tbm_surface_h surface);
  void Release(BufferUnit* unit);
  void Prepare(int with, int height);

 protected:
  std::vector<std::unique_ptr<BufferUnit>> pool_;

 private:
  int last_index_;
  std::mutex mutex_;
};

class SingleBufferPool : public BufferPool {
 public:
  explicit SingleBufferPool(int width, int height);

  virtual BufferUnit* GetAvailableBuffer();
  virtual BufferUnit* Find(tbm_surface_h surface);
};

#endif
