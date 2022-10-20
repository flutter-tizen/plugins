// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_SCAN_FILTER_H_
#define FLUTTER_PLUGIN_SCAN_FILTER_H_

#include <bluetooth_type.h>

#include <string>

typedef std::string Uuid;

// A wrapper around the bt_scan_filter_h handle.
class ScanFilter {
 public:
  explicit ScanFilter(const Uuid &service_id);
  ~ScanFilter();

  // Prevent copying.
  ScanFilter(ScanFilter const &) = delete;
  ScanFilter &operator=(ScanFilter const &) = delete;

  ScanFilter(ScanFilter &&other) : handle_(other.handle_) {}
  ScanFilter &operator=(ScanFilter &&other) {
    handle_ = other.handle_;
    return *this;
  }

  bool Register();

  bool Unregister();

  static bool UnregisterAll();

  bt_scan_filter_h handle() const { return handle_; }

 private:
  bt_scan_filter_h handle_;
};

#endif  // FLUTTER_PLUGIN_SCAN_FILTER_H_
