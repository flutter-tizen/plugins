// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_PERMISSIONS_H_
#define FLUTTER_PLUGIN_PERMISSIONS_H_

// Permissions that can be checked and requested.
//
// Originally defined in permissions.dart of the platform interface package.
enum class Permission {
  kCalendar = 0,
  kCamera = 1,
  kContacts = 2,
  kLocation = 3,
  kLocationAlways = 4,
  kLocationWhenInUse = 5,
  kMediaLibrary = 6,
  kMicrophone = 7,
  kPhone = 8,
  kPhotos = 9,
  kPhotosAddOnly = 10,
  kReminders = 11,
  kSensors = 12,
  kSMS = 13,
  kSpeech = 14,
  kStorage = 15,
  kIgnoreBatteryOptimizations = 16,
  kNotification = 17,
  kAccessMediaLocation = 18,
  kActivityRecognition = 19,
  kUnknown = 20,
  kBluetooth = 21,
  kManageExternalStorage = 22,
  kSystemAlertWindow = 23,
  kRequestInstallPackages = 24,
  kAppTrackingTransparency = 25,
  kCriticalAlerts = 26,
  kAccessNotificationPolicy = 27,
  kBluetoothScan = 28,
  kBluetoothAdvertise = 29,
  kBluetoothConnect = 30,
  kNearbyWifiDevices = 31,
  kVideos = 32,
  kAudio = 33,
  kScheduleExactAlarm = 34,
  kSensorsAlways = 35
};

#endif  // FLUTTER_PLUGIN_PERMISSIONS_H_
