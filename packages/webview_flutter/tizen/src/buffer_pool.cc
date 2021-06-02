// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "buffer_pool.h"

#include "log.h"

#define BUFFER_POOL_SIZE 5

BufferUnit::BufferUnit(int index, int width, int height)
    : isAllocated_(false),
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
}

bool BufferUnit::Allocate() {
  if (!isAllocated_) {
    isAllocated_ = true;
    return true;
  }
  return false;
}

int BufferUnit::Index() { return index_; }
bool BufferUnit::isAllocate() { return isAllocated_ && tbm_surface_; }
tbm_surface_h BufferUnit::Surface() {
  if (isAllocate()) {
    return tbm_surface_;
  }
  return nullptr;
}

void BufferUnit::Release() { isAllocated_ = false; }

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
  tbm_surface_ = tbm_surface_create(width_, height_, TBM_FORMAT_ARGB8888);
}

BufferPool::BufferPool(int width, int height) {
  for (int idx = 0; idx < BUFFER_POOL_SIZE; idx++) {
    pool_.emplace_back(new BufferUnit(idx, width, height));
  }
  Prepare(width, height);
}

BufferPool::~BufferPool() {}

BufferUnit* BufferPool::Get(tbm_surface_h surface) {
  for (int idx = 0; idx < BUFFER_POOL_SIZE; idx++) {
    BufferUnit* buffer = pool_[idx].get();
    if (buffer->Surface() == surface) {
      return buffer;
    }
  }
  return nullptr;
}

BufferUnit* BufferPool::AllocateBuffer() {
  std::lock_guard<std::mutex> lock(mutex_);
  for (int idx = 0; idx < BUFFER_POOL_SIZE; idx++) {
    BufferUnit* buffer = pool_[idx].get();
    if (buffer->Allocate()) {
      return buffer;
    }
  }
  return nullptr;
}

void BufferPool::Release(BufferUnit* unit) {
  std::lock_guard<std::mutex> lock(mutex_);
  unit->Release();
}

void BufferPool::Prepare(int width, int height) {
  std::lock_guard<std::mutex> lock(mutex_);
  for (int idx = 0; idx < BUFFER_POOL_SIZE; idx++) {
    BufferUnit* buffer = pool_[idx].get();
    buffer->Reset(width, height);
  }
}
