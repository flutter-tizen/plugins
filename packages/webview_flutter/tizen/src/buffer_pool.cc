// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "buffer_pool.h"

#include "log.h"

BufferUnit::BufferUnit(int32_t width, int32_t height) { Reset(width, height); }

BufferUnit::~BufferUnit() {
  if (tbm_surface_) {
    tbm_surface_destroy(tbm_surface_);
    tbm_surface_ = nullptr;
  }
  if (gpu_buffer_) {
    delete gpu_buffer_;
    gpu_buffer_ = nullptr;
  }
}

bool BufferUnit::MarkInUse() {
  if (!is_used_) {
    is_used_ = true;
    return true;
  }
  return false;
}

void BufferUnit::UnmarkInUse() { is_used_ = false; }

tbm_surface_h BufferUnit::Surface() {
  if (IsUsed()) {
    return tbm_surface_;
  }
  return nullptr;
}

void BufferUnit::Reset(int32_t width, int32_t height) {
  if (width_ == width && height_ == height) {
    return;
  }
  width_ = width;
  height_ = height;

  if (tbm_surface_) {
    tbm_surface_destroy(tbm_surface_);
    tbm_surface_ = nullptr;
  }
  if (gpu_buffer_) {
    delete gpu_buffer_;
    gpu_buffer_ = nullptr;
  }

  tbm_surface_ = tbm_surface_create(width_, height_, TBM_FORMAT_ARGB8888);
  gpu_buffer_ = new FlutterDesktopGpuSurfaceDescriptor();
  gpu_buffer_->width = width_;
  gpu_buffer_->visible_width = width_;
  gpu_buffer_->height = height_;
  gpu_buffer_->visible_height = height_;
  gpu_buffer_->handle = tbm_surface_;
  gpu_buffer_->release_callback = [](void* release_context) {
    BufferUnit* buffer = reinterpret_cast<BufferUnit*>(release_context);
    buffer->UnmarkInUse();
  };
  gpu_buffer_->release_context = this;
}

BufferPool::BufferPool(int32_t width, int32_t height, size_t pool_size) {
  for (size_t index = 0; index < pool_size; index++) {
    pool_.emplace_back(std::make_unique<BufferUnit>(width, height));
  }
  Prepare(width, height);
}

BufferPool::~BufferPool() {}

BufferUnit* BufferPool::GetAvailableBuffer() {
  std::lock_guard<std::mutex> lock(mutex_);
  for (size_t index = 0; index < pool_.size(); index++) {
    size_t current = (index + last_index_) % pool_.size();
    BufferUnit* buffer = pool_[current].get();
    if (buffer->MarkInUse()) {
      last_index_ = current;
      return buffer;
    }
  }
  return nullptr;
}

void BufferPool::Release(BufferUnit* buffer) {
  std::lock_guard<std::mutex> lock(mutex_);
  buffer->UnmarkInUse();
}

void BufferPool::Prepare(int32_t width, int32_t height) {
  std::lock_guard<std::mutex> lock(mutex_);
  for (size_t index = 0; index < pool_.size(); index++) {
    BufferUnit* buffer = pool_[index].get();
    buffer->Reset(width, height);
  }
}

SingleBufferPool::SingleBufferPool(int32_t width, int32_t height)
    : BufferPool(width, height, 1) {}

SingleBufferPool::~SingleBufferPool() {}

BufferUnit* SingleBufferPool::GetAvailableBuffer() {
  BufferUnit* buffer = pool_[0].get();
  buffer->MarkInUse();
  return buffer;
}

void SingleBufferPool::Release(BufferUnit* buffer) {}

#ifndef NDEBUG
#include <cairo.h>
void BufferUnit::DumpToPng(int file_name) {
  char file_path[256];
  sprintf(file_path, "/tmp/dump%d.png", file_name);

  tbm_surface_info_s surface_info;
  tbm_surface_map(tbm_surface_, TBM_SURF_OPTION_WRITE, &surface_info);

  unsigned char* buffer = surface_info.planes[0].ptr;
  cairo_surface_t* png_buffer = cairo_image_surface_create_for_data(
      buffer, CAIRO_FORMAT_ARGB32, width_, height_,
      surface_info.planes[0].stride);

  cairo_surface_write_to_png(png_buffer, file_path);

  tbm_surface_unmap(tbm_surface_);
  cairo_surface_destroy(png_buffer);
}
#endif
