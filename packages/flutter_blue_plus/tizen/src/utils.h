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
  T var;

  std::mutex mutex_;

  SafeType(const T& t) : var(t) {}

  SafeType(T&& t) : var(std::move(t)) {}

  SafeType() : var(T()) {}
};

class BtException : public std::exception {
 public:
  explicit BtException(std::string message) : message_(std::move(message)){};
  BtException(const int tizen_error, std::string message)
      : tizen_error_(tizen_error), message_(std::move(message)) {
    message_ += ": " + std::string(get_error_message(tizen_error));
  };

  explicit BtException(const int tizen_error)
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
  bool allow_duplicates;
  bool clear_devices;
  std::vector<std::string> device_ids_filters;
  std::vector<std::string> service_uuids_filters;
};

struct AdvertisementData {
  bool connectable;
  std::string local_name;
};

AdvertisementData DecodeAdvertisementData(const char* packets_data,
                                          int data_len) noexcept;

}  // namespace flutter_blue_plus_tizen

#endif  // FLUTTER_BLUE_PLUS_TIZEN_UTILS_H
