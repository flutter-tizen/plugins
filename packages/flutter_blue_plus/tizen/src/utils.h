// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_BLUE_PLUS_TIZEN_UTILS_H
#define FLUTTER_BLUE_PLUS_TIZEN_UTILS_H

#include <bluetooth.h>

#include <algorithm>
#include <exception>
#include <mutex>
#include <optional>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

#include "log.h"

namespace flutter_blue_plus_tizen {

template <typename T>
struct SafeType {
  T var_;

  std::mutex mutex_;

  SafeType(const T& t) : var_(t) {}

  SafeType(T&& t) : var_(std::move(t)) {}

  SafeType() : var_(T()) {}
};

class BtException : public std::exception {
 public:
  BtException(const std::string& message) : message_(message){};
  BtException(const int tizen_error, const std::string& message)
      : tizen_error_(tizen_error),
        message_(std::string(get_error_message(tizen_error)) + ": " +
                 message){};

  BtException(const int tizen_error)
      : tizen_error_(tizen_error), message_(get_error_message(tizen_error)){};

  const char* what() const noexcept override { return message_.c_str(); }

  int GetTizenError() const noexcept { return tizen_error_; }

 private:
  int tizen_error_ = -1;
  std::string message_;
};

std::string GetGattValue(bt_gatt_h handle);

std::string GetGattUuid(bt_gatt_h handle);

struct BleScanSettings {
  bool allow_duplicates_;
  bool clear_devices_;
  std::vector<std::string> device_ids_filters_;
  std::vector<std::string> service_uuids_filters_;
};

struct AdvertisementData {
  bool connectable_;
  std::string local_name_;
};

AdvertisementData DecodeAdvertisementData(const char* packets_data,
                                          int data_len) noexcept;

}  // namespace flutter_blue_plus_tizen

#endif  // FLUTTER_BLUE_PLUS_TIZEN_UTILS_H
