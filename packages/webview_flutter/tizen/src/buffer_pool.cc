// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "buffer_pool.h"

#include "log.h"

#define BUFFER_POOL_SIZE 5

BufferUnit::BufferUnit(int index, int width, int height)
    : isUsed_(false),
      index_(index),
      width_(0),
      height_(0),
      tbm_surface_(nullptr) {
  Reset(width, height);
}

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
  if (!isUsed_) {
    isUsed_ = true;
    return true;
  }
  return false;
}

FlutterDesktopGpuBuffer* BufferUnit::GpuBuffer() { return gpu_buffer_; }

int BufferUnit::Index() { return index_; }

bool BufferUnit::IsUsed() { return isUsed_ && tbm_surface_; }

tbm_surface_h BufferUnit::Surface() {
  if (IsUsed()) {
    return tbm_surface_;
  }
  return nullptr;
}

void BufferUnit::UnmarkInUse() { isUsed_ = false; }

void BufferUnit::Reset(int width, int height) {
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
  gpu_buffer_ = new FlutterDesktopGpuBuffer();
  gpu_buffer_->width = width_;
  gpu_buffer_->height = height_;
  gpu_buffer_->buffer = tbm_surface_;
}

BufferPool::BufferPool(int width, int height) : last_index_(0) {
  for (int idx = 0; idx < BUFFER_POOL_SIZE; idx++) {
    pool_.emplace_back(new BufferUnit(idx, width, height));
  }
  Prepare(width, height);
}

BufferPool::~BufferPool() {}

BufferUnit* BufferPool::Find(tbm_surface_h surface) {
  for (int idx = 0; idx < pool_.size(); idx++) {
    BufferUnit* buffer = pool_[idx].get();
    if (buffer->Surface() == surface) {
      return buffer;
    }
  }
  return nullptr;
}

BufferUnit* BufferPool::GetAvailableBuffer() {
  std::lock_guard<std::mutex> lock(mutex_);
  for (int idx = 0; idx < pool_.size(); idx++) {
    int current = (idx + last_index_) % pool_.size();
    BufferUnit* buffer = pool_[current].get();
    if (buffer->MarkInUse()) {
      last_index_ = current;
      return buffer;
    }
  }
  return nullptr;
}

void BufferPool::Release(BufferUnit* unit) {
  std::lock_guard<std::mutex> lock(mutex_);
  unit->UnmarkInUse();
}

void BufferPool::Prepare(int width, int height) {
  std::lock_guard<std::mutex> lock(mutex_);
  for (int idx = 0; idx < pool_.size(); idx++) {
    BufferUnit* buffer = pool_[idx].get();
    buffer->Reset(width, height);
  }
}

#include <cairo.h>
void BufferUnit::dumpToPng(int filename) {
  char filePath[256];
  sprintf(filePath, "/tmp/dump%d.png", filename);
  tbm_surface_info_s surfaceInfo;
  tbm_surface_map(tbm_surface_, TBM_SURF_OPTION_WRITE, &surfaceInfo);
  void* buffer = surfaceInfo.planes[0].ptr;

  cairo_surface_t* png_buffer;
  png_buffer = cairo_image_surface_create_for_data(
      (unsigned char*)buffer, CAIRO_FORMAT_ARGB32, width_, height_,
      surfaceInfo.planes[0].stride);

  cairo_surface_write_to_png(png_buffer, filePath);

  tbm_surface_unmap(tbm_surface_);
  cairo_surface_destroy(png_buffer);
}
