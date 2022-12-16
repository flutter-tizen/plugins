// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_TIZEN_PACKAGE_MANAGER_H_
#define FLUTTER_PLUGIN_TIZEN_PACKAGE_MANAGER_H_

#include <package_manager.h>
#include <tizen_error.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

struct PackageInfo {
  std::string package_id;
  std::string label;
  std::string type;
  std::optional<std::string> icon_path;
  std::string version;
  std::string installed_storage_type;
  bool is_system;
  bool is_preloaded;
  bool is_removable;
};

enum class PacakgeEventState { kStarted, kProcessing, kFailed, kCompleted };

using OnPackageEvent =
    std::function<void(std::string package_id, std::string package_type,
                       PacakgeEventState state, int32_t progress)>;

class TizenPackageManager {
 public:
  ~TizenPackageManager();

  // Returns a unique instance of TizenPackageManager.
  static TizenPackageManager& GetInstance() {
    static TizenPackageManager instance;
    return instance;
  }

  std::optional<PackageInfo> GetPackageInfo(const std::string& package_id);

  std::optional<std::vector<PackageInfo>> GetAllPackagesInfo();

  bool Install(const std::string& package_path);

  bool Uninstall(const std::string& package_id);

  void SetInstallHandler(OnPackageEvent on_install) {
    install_callback_ = on_install;
  }

  void SetUninstallHandler(OnPackageEvent on_uninstall) {
    uninstall_callback_ = on_uninstall;
  }

  void SetUpdateHandler(OnPackageEvent on_update) {
    update_callback_ = on_update;
  }

  int GetLastError() { return last_error_; }

  std::string GetLastErrorString() { return get_error_message(last_error_); }

 private:
  explicit TizenPackageManager();

  std::optional<PackageInfo> GetPackageData(package_info_h handle);

  package_manager_h package_manager_ = nullptr;
  OnPackageEvent install_callback_;
  OnPackageEvent uninstall_callback_;
  OnPackageEvent update_callback_;
  std::vector<PackageInfo> packages_;

  int last_error_ = PACKAGE_MANAGER_ERROR_NONE;
};

#endif  // FLUTTER_PLUGIN_TIZEN_PACKAGE_MANAGER_H_
