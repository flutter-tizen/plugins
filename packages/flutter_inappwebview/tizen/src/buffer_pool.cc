// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "buffer_pool.h"

#include <atomic>

struct BufferReleaseState {
  std::atomic_bool is_used = false;
};

namespace {

struct GpuSurfaceDescriptorContext {
  std::shared_ptr<BufferReleaseState> release_state;
  FlutterDesktopGpuSurfaceDescriptor descriptor = {};
};

void ReleaseGpuSurfaceDescriptor(void* release_context) {
  auto context =
      reinterpret_cast<GpuSurfaceDescriptorContext*>(release_context);
  context->release_state->is_used.store(false);
  delete context;
}

}  // namespace

BufferUnit::BufferUnit(int32_t width, int32_t height)
    : release_state_(std::make_shared<BufferReleaseState>()) {
  Reset(width, height);
}

BufferUnit::~BufferUnit() {
  if (tbm_surface_ && !use_external_buffer_) {
    tbm_surface_destroy(tbm_surface_);
    tbm_surface_ = nullptr;
  }
}

void BufferUnit::UseExternalBuffer() {
  if (!use_external_buffer_) {
    use_external_buffer_ = true;
    if (tbm_surface_) {
      tbm_surface_destroy(tbm_surface_);
      tbm_surface_ = nullptr;
    }
  }
}

void BufferUnit::SetExternalBuffer(tbm_surface_h tbm_surface) {
  if (use_external_buffer_) {
    tbm_surface_ = tbm_surface;
  }
}

bool BufferUnit::MarkInUse() {
  bool expected = false;
  return release_state_->is_used.compare_exchange_strong(expected, true);
}

void BufferUnit::UnmarkInUse() { release_state_->is_used.store(false); }

bool BufferUnit::IsUsed() {
  return release_state_->is_used.load() && tbm_surface_;
}

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

  if (tbm_surface_ && !use_external_buffer_) {
    tbm_surface_destroy(tbm_surface_);
  }
  tbm_surface_ = nullptr;

  if (!use_external_buffer_) {
    tbm_surface_ = tbm_surface_create(width_, height_, TBM_FORMAT_ARGB8888);
  }
}

FlutterDesktopGpuSurfaceDescriptor* BufferUnit::GpuSurface() {
  if (!tbm_surface_) {
    UnmarkInUse();
    return nullptr;
  }

  auto context = new GpuSurfaceDescriptorContext();
  context->release_state = release_state_;
  context->descriptor.width = width_;
  context->descriptor.height = height_;
  context->descriptor.handle = tbm_surface_;
  context->descriptor.release_callback = ReleaseGpuSurfaceDescriptor;
  context->descriptor.release_context = context;
  return &context->descriptor;
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
  std::lock_guard<std::mutex> lock(mutex_);
  BufferUnit* buffer = pool_[0].get();
  if (buffer->MarkInUse()) {
    return buffer;
  }
  return nullptr;
}

void SingleBufferPool::Release(BufferUnit* buffer) {
  BufferPool::Release(buffer);
}

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
