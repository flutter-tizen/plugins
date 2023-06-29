// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_BILLING_MANAGER_H
#define FLUTTER_PLUGIN_BILLING_MANAGER_H

#include <flutter/encodable_value.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <tizen_error.h>

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "billing_service_proxy.h"

#define SSO_API_MAX_STRING_LEN 128

typedef struct sso_login_info {
  char login_id[SSO_API_MAX_STRING_LEN];
  char login_pwd[SSO_API_MAX_STRING_LEN];
  char login_guid[SSO_API_MAX_STRING_LEN];
  char uid[SSO_API_MAX_STRING_LEN];
  char user_icon[SSO_API_MAX_STRING_LEN * 8];
} sso_login_info_s;

typedef enum {
  PRD = 0,
  DEV,
} tv_server_type;

typedef enum {
  SYSTEM_INFO_ERROR_NONE = TIZEN_ERROR_NONE, /**< Successful */
  SYSTEM_INFO_ERROR_INVALID_PARAMETER =
      TIZEN_ERROR_INVALID_PARAMETER, /**< Invalid parameter */
  SYSTEM_INFO_ERROR_OUT_OF_MEMORY =
      TIZEN_ERROR_OUT_OF_MEMORY, /**< Out of memory */
  SYSTEM_INFO_ERROR_IO_ERROR =
      TIZEN_ERROR_IO_ERROR, /**< An input/output error occurred when reading
                               value from system */
  SYSTEM_INFO_ERROR_PERMISSION_DENIED =
      TIZEN_ERROR_PERMISSION_DENIED, /**< No permission to use the API */
  SYSTEM_INFO_ERROR_NOT_SUPPORTED =
      TIZEN_ERROR_NOT_SUPPORTED, /**< Not supported parameter (Since 3.0) */
} system_info_error_e;

typedef enum {
  SYSTEM_INFO_KEY_NUM_OF_TUNER = 18,
  SYSTEM_INFO_KEY_STAMRT_HUB_HBBTV_SUPPORTED = 41,
  SYSTEM_INFO_KEY_SMART_LED_SUPPORTED = 56,
  SYSTEM_INFO_KEY_KR_CABLE_QAM_SUPPORTED = 59,
  SYSTEM_INFO_KEY_SMART_LED_DEMO_POSITION = 63,
  SYSTEM_INFO_KEY_PANEL_SIZE = 69,
  SYSTEM_INFO_KEY_PANEL_TYPE = 73,
  SYSTEM_INFO_KEY_PANEL_TYPE_STRING = 74,
  SYSTEM_INFO_KEY_LOCAL_SET = 90,
  SYSTEM_INFO_KEY_WIFI_REGION = 109,
  SYSTEM_INFO_KEY_NUM_OF_DTV = 110,
  SYSTEM_INFO_KEY_NUM_OF_ATV = 112,
  SYSTEM_INFO_KEY_NUM_OF_RVU = 121,
  SYSTEM_INFO_KEY_NUM_OF_HDMI = 122,
  SYSTEM_INFO_KEY_INFO_LINK_SERVER_TYPE = 126,
  SYSTEM_INFO_KEY_REGION_KIND = 127,
  SYSTEM_INFO_KEY_SW_VERSION = 128,
  SYSTEM_INFO_KEY_TUNER_TYPE = 131,
  SYSTEM_INFO_KEY_VERSION_MICOM = 133,
  SYSTEM_INFO_KEY_VERSION_EMANUAL = 138,
  SYSTEM_INFO_KEY_AUTOSTORE = 174,
  SYSTEM_INFO_KEY_SERIAL_NUMBER = 187,
  SYSTEM_INFO_KEY_SW_VERSION_MODEL = 199,
  SYSTEM_INFO_KEY_TARGET_LOCATION = 201,
  SYSTEM_INFO_KEY_PANEL_VFREQ = 204,
  SYSTEM_INFO_KEY_PANEL_ASPECT_RATIO = 205,
  SYSTEM_INFO_KEY_VERSION_EPOP_APP = 209,
  SYSTEM_INFO_KEY_VERSION_WIFI = 216,
  SYSTEM_INFO_KEY_SATELLITE_MASK = 220,
  SYSTEM_INFO_KEY_LANGUAGE_LIST = 227,
  SYSTEM_INFO_KEY_USB_COPY_FORMAT_SUPPORTED = 236,
  SYSTEM_INFO_KEY_NUM_OF_PVR_RECORD = 238,
  SYSTEM_INFO_KEY_CERT_OPTION = 240,
  SYSTEM_INFO_KEY_GET_JP_MIGRATION_BACKUP_SOURCE_PATH = 247,
  SYSTEM_INFO_KEY_GET_JP_MIGRATION_RESTORE_SOURCE_PATH = 248,
  SYSTEM_INFO_KEY_AUTOMOTIONPLUS_CLEAR_BLUR_SUPPORTED = 258,
  SYSTEM_INFO_KEY_CHECK_SKIP_LOCALSET = 280,
  SYSTEM_INFO_KEY_CHECK_WIFI_VENDOR = 281,
  SYSTEM_INFO_KEY_LOCAL_SET_ENUM = 284,
  SYSTEM_INFO_KEY_REGION_KIND_ENUM = 285,
  SYSTEM_INFO_KEY_PRODUCT_CODE_SW = 291,
  SYSTEM_INFO_KEY_PRODUCT_CODE_BOM = 292,
  SYSTEM_INFO_KEY_ONTV_SUPPORTED = 304,
  SYSTEM_INFO_KEY_HOTEL_TV_SUPPORTED = 315,
  SYSTEM_INFO_KEY_ERROR_POPUP_ON_OFF = 320,
  SYSTEM_INFO_KEY_TUNER_SHAPE = 325,
  SYSTEM_INFO_KEY_PANEL_DEFAULT_VFREQ = 326,
  SYSTIM_INFO_KEY_DTV_TYPE = 333,
  SYSTEM_INFO_KEY_TUNER_SUP_EPOP = 342,
  SYSTEM_INFO_KEY_CES_OPTION = 343,
  SYSTEM_INFO_KEY_UPDATE_SUP_EMANUAL = 352,
  SYSTEM_INFO_KEY_ENERGY_STAR_LOGO_SUPPORTED = 360,
  SYSTEM_INFO_KEY_FHD_EVK_TV_YEAR = 362,
  SYSTEM_INFO_KEY_TV_CURRENT_YEAR = 367,
  SYSTEM_INFO_KEY_PC_DIMMING_SUPPORT = 370,
  SYSTEM_INFO_KEY_EVK_SW_VERSION = 378,
  SYSTEM_INFO_KEY_PNP_COUNTRY_LIST = 388,
  SYSTEM_INFO_KEY_DIGITAL_COUNTRY_LIST = 389,
  SYSTEM_INFO_KEY_ANALOG_COUNTRY_LIST = 390,
  SYSTEM_INFO_KEY_VERSION_CAMERA = 457,
  SYSTEM_INFO_KEY_VERSION_MIC = 458,
  SYSTEM_INFO_KEY_ALWAYS_INSTANT_ON_SUPPORT = 466,
  SYSTEM_INFO_KEY_APP_BOOTING_SUPPORT = 488,
  SYSTEM_INFO_KEY_PNP_LANGUAGE_LIST = 498,
  SYSTEM_INFO_KEY_MODEL_SERIES_INFO = 505,
  SYSTEM_INFO_KEY_IOT_HUB_SUPPORTED = 510,
  SYSTEM_INFO_KEY_DEFAULT_DIGITAL_COUNTRY = 511,
  SYSTEM_INFO_KEY_DEFAULT_ANALOG_COUNTRY = 512,
  SYSTEM_INFO_KEY_CH_MAP = 517,
  SYSTEM_INFO_KEY_A_PICTURE_DIRECT = 518,
  SYSTEM_INFO_KEY_MIN_BACKLIGHT = 532,
  SYSTEM_INFO_KEY_CLOUD_SCAN_UPLOAD = 537,
  SYSTEM_INFO_KEY_DEFAULT_HDMI_1_BOOTING = 538,
  SYSTEM_INFO_KEY_PLATFORM_TYPE = 564,
  SYSTEM_INFO_KEY_FRAME_TV = 583,
  SYSTEM_INFO_KEY_360VR_SUPPORT = 589,
  SYSTEM_INFO_KEY_DYNAMIC_CONTRAST = 590,
  SYSTEM_INFO_KEY_RUN_EW = 600,
  SYSTEM_INFO_KEY_EXHIBITION_MODE = 601,
  SYSTEM_INFO_KEY_ATSC3_SUPPORTED = 604,
  SYSTEM_INFO_KEY_PANEL_TIME = 605,
  SYSTEM_INFO_KEY_CN_WEB_MODEL = 620,
  SYSTEM_INFO_KEY_HOTEL_MIN_VOLUME = 645,
  SYSTEM_INFO_KEY_HOTEL_MAX_VOLUME = 646,
  SYSTEM_INFO_KEY_HOTEL_MODE = 647,
  SYSTEM_INFO_KEY_HOTEL_POWER_ON_VOLUME = 648,
  SYSTEM_INFO_KEY_NUM_OF_DISPLAY = 656,
  SYSTEM_INFO_KEY_STD_HDR_BL = 668,
  SYSTEM_INFO_KEY_HARDWARE_VERSION = 669,
  SYSTEM_INFO_KEY_PANEL_TYPE_82INCH_SDC = 670,
} system_info_key_e;

typedef bool (*FuncSsoGetLoginInfo)(sso_login_info_s *login_info);
typedef char *(*FuncVconfGetStr)(const char *in_key);
typedef int (*FuncSystemInfGetValueInt)(system_info_key_e key, int *value);

class BillingManager {
 public:
  explicit BillingManager(flutter::PluginRegistrar *plugin_registrar);
  ~BillingManager(){};

  bool Init();
  void Dispose();

 private:
  bool BillingIsAvailable();
  bool BuyItem(const flutter::EncodableMap *encodables);
  bool GetProductList(const flutter::EncodableMap *encodables);
  bool GetPurchaseList(const flutter::EncodableMap *encodables);
  bool VerifyInvoice(const flutter::EncodableMap *encodables);
  std::string GetCustomId();
  std::string GetCountryCode();

  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
  void SendResult(const flutter::EncodableValue &result);

  static void OnProducts(const char *detail_result, void *user_data);
  static void OnPurchase(const char *detail_result, void *user_data);
  static bool OnBuyItem(const char *pay_result, const char *detail_info,
                        void *user_data);
  static void OnAvailable(const char *detail_result, void *user_data);
  static void OnVerify(const char *detail_result, void *user_data);

  std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue>>
      method_channel_ = nullptr;
  std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>>
      method_result_ = nullptr;
  billing_server_type billing_server_type_;
};

#endif  // FLUTTER_PLUGIN_BILLING_MANAGER_H
