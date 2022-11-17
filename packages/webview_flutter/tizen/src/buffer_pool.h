// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_BUFFER_POOL_H_
#define FLUTTER_PLUGIN_BUFFER_POOL_H_

#include <flutter_texture_registrar.h>
#include <tbm_surface.h>

#include <memory>
#include <mutex>
#include <vector>

class BufferUnit {
 public:
  explicit BufferUnit(int32_t width, int32_t height);
  ~BufferUnit();

  void Reset(int32_t width, int32_t height);

  bool MarkInUse();
  void UnmarkInUse();

  bool IsUsed() { return is_used_ && tbm_surface_; }

  void UseExternalBuffer();
  void SetExternalBuffer(tbm_surface_h tbm_surface);

  tbm_surface_h Surface();

  FlutterDesktopGpuSurfaceDescriptor* GpuSurface() { return gpu_surface_; }

#ifndef NDEBUG
  // TODO: Unused code.
  void DumpToPng(int file_name);
#endif

 private:
  bool is_used_ = false;
  bool use_external_buffer_ = false;
  int32_t width_ = 0;
  int32_t height_ = 0;
  tbm_surface_h tbm_surface_ = nullptr;
  FlutterDesktopGpuSurfaceDescriptor* gpu_surface_ = nullptr;
};

class BufferPool {
 public:
  explicit BufferPool(int32_t width, int32_t height, size_t pool_size);
  virtual ~BufferPool();

  virtual BufferUnit* GetAvailableBuffer();
  virtual void Release(BufferUnit* buffer);

  void Prepare(int32_t with, int32_t height);

 protected:
  std::vector<std::unique_ptr<BufferUnit>> pool_;

 private:
  size_t last_index_ = 0;
  std::mutex mutex_;
};

class SingleBufferPool : public BufferPool {
 public:
  explicit SingleBufferPool(int32_t width, int32_t height);
  ~SingleBufferPool();

  virtual BufferUnit* GetAvailableBuffer() override;
  virtual void Release(BufferUnit* buffer) override;
};

#endif  // FLUTTER_PLUGIN_BUFFER_POOL_H_
