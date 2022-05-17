// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tizen_notification_plugin.h"

#include <app_common.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>
#include <notification.h>
#include <notification_error.h>

#include <memory>
#include <string>

#include "log.h"

class TizenNotificationPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto channel =
        std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
            registrar->messenger(), "tizen/notification",
            &flutter::StandardMethodCodec::GetInstance());

    auto plugin = std::make_unique<TizenNotificationPlugin>();

    channel->SetMethodCallHandler(
        [plugin_pointer = plugin.get()](const auto &call, auto result) {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });

    registrar->AddPlugin(std::move(plugin));
  }

  TizenNotificationPlugin() {}

  virtual ~TizenNotificationPlugin() {}

 private:
  template <typename T>
  bool GetValueFromArgs(const flutter::EncodableValue *args, const char *key,
                        T &out) {
    if (std::holds_alternative<flutter::EncodableMap>(*args)) {
      flutter::EncodableMap map = std::get<flutter::EncodableMap>(*args);
      if (map.find(flutter::EncodableValue(key)) != map.end()) {
        flutter::EncodableValue value = map[flutter::EncodableValue(key)];
        if (std::holds_alternative<T>(value)) {
          out = std::get<T>(value);
          return true;
        }
      }
    }
    return false;
  }

  bool GetEncodableValueFromArgs(const flutter::EncodableValue *args,
                                 const char *key,
                                 flutter::EncodableValue &out) {
    if (std::holds_alternative<flutter::EncodableMap>(*args)) {
      flutter::EncodableMap map = std::get<flutter::EncodableMap>(*args);
      if (map.find(flutter::EncodableValue(key)) != map.end()) {
        out = map[flutter::EncodableValue(key)];
        return true;
      }
    }
    return false;
  }

  void FreeNotification(notification_h &noti) {
    int ret = notification_free(noti);
    if (ret != NOTIFICATION_ERROR_NONE) {
      LOG_ERROR("notification_free failed : %s", get_error_message(ret));
    }
  }

  void DestroyAppControl(app_control_h &app_control) {
    int ret = app_control_destroy(app_control);
    if (ret != APP_CONTROL_ERROR_NONE) {
      LOG_ERROR("app_control_destroy failed : %s", get_error_message(ret));
    }
  }

  notification_sound_type_e StringToSoundType(std::string sound_str) {
    notification_sound_type_e ret = NOTIFICATION_SOUND_TYPE_NONE;
    if (sound_str.compare("none") == 0) {
      ret = NOTIFICATION_SOUND_TYPE_NONE;
    } else if (sound_str.compare("builtIn") == 0) {
      ret = NOTIFICATION_SOUND_TYPE_DEFAULT;
    } else if (sound_str.compare("userData") == 0) {
      ret = NOTIFICATION_SOUND_TYPE_USER_DATA;
    }
    return ret;
  }

  notification_vibration_type_e StringToVibrationType(
      std::string vibration_str) {
    notification_vibration_type_e ret = NOTIFICATION_VIBRATION_TYPE_NONE;
    if (vibration_str.compare("none") == 0) {
      ret = NOTIFICATION_VIBRATION_TYPE_NONE;
    } else if (vibration_str.compare("builtIn") == 0) {
      ret = NOTIFICATION_VIBRATION_TYPE_DEFAULT;
    } else if (vibration_str.compare("userData") == 0) {
      ret = NOTIFICATION_VIBRATION_TYPE_USER_DATA;
    }
    return ret;
  }

  notification_image_type_e StringToImageType(std::string image_type) {
    notification_image_type_e ret = NOTIFICATION_IMAGE_TYPE_ICON;
    if (image_type.compare("icon") == 0) {
      ret = NOTIFICATION_IMAGE_TYPE_ICON;
    } else if (image_type.compare("iconForIndicator") == 0) {
      ret = NOTIFICATION_IMAGE_TYPE_ICON_FOR_INDICATOR;
    } else if (image_type.compare("iconForLock") == 0) {
      ret = NOTIFICATION_IMAGE_TYPE_ICON_FOR_LOCK;
    }
    return ret;
  }

  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    if (method_call.method_name().compare("show") == 0) {
      if (method_call.arguments()) {
        const flutter::EncodableValue *arguments = method_call.arguments();
        notification_h noti_handle = nullptr;
        app_control_h app_control = nullptr;

        int ret = NOTIFICATION_ERROR_NONE;
        std::string id;
        std::string title;
        std::string body;
        flutter::EncodableValue app_control_data;

        if (GetValueFromArgs(arguments, "id", id)) {
          noti_handle = notification_load_by_tag(id.c_str());
          if (noti_handle) {
            ret = notification_delete(noti_handle);
            if (ret != NOTIFICATION_ERROR_NONE) {
              LOG_ERROR("notification_delete failed : %s",
                        get_error_message(ret));
              result->Error("notification_delete failed",
                            std::string(get_error_message(ret)));
              return;
            }
          }

          bool ongoing;
          if (GetValueFromArgs(arguments, "ongoing", ongoing)) {
            if (ongoing) {
              noti_handle = notification_create(NOTIFICATION_TYPE_ONGOING);
              if (!noti_handle) {
                LOG_ERROR(
                    "notification_create failed : Fail to create notification");
                result->Error("notification_create failed",
                              "Fail to create notification");
                return;
              }
              ret = notification_set_layout(noti_handle,
                                            NOTIFICATION_LY_ONGOING_EVENT);
              if (ret != NOTIFICATION_ERROR_NONE) {
                FreeNotification(noti_handle);
                LOG_ERROR("notification_set_layout failed : %s",
                          get_error_message(ret));
                result->Error("notification_set_layout failed",
                              std::string(get_error_message(ret)));
                return;
              }
            } else {
              noti_handle = notification_create(NOTIFICATION_TYPE_NOTI);
              if (!noti_handle) {
                LOG_ERROR(
                    "notification_create failed : Fail to create notification");
                result->Error("notification_create failed",
                              "Fail to create notification");
                return;
              }
            }
          }
          ret = notification_set_tag(noti_handle, id.c_str());
          if (ret != NOTIFICATION_ERROR_NONE) {
            FreeNotification(noti_handle);
            LOG_ERROR("notification_set_tag failed : %s",
                      get_error_message(ret));
            result->Error("notification_set_tag failed",
                          std::string(get_error_message(ret)));
            return;
          }
        }

        if (GetValueFromArgs(arguments, "title", title)) {
          ret = notification_set_text(noti_handle, NOTIFICATION_TEXT_TYPE_TITLE,
                                      title.c_str(), nullptr,
                                      NOTIFICATION_VARIABLE_TYPE_NONE);
          if (ret != NOTIFICATION_ERROR_NONE) {
            FreeNotification(noti_handle);
            LOG_ERROR("notification_set_text failed : %s",
                      get_error_message(ret));
            result->Error("notification_set_text failed",
                          std::string(get_error_message(ret)));
            return;
          }
        }

        if (GetValueFromArgs(arguments, "body", body)) {
          ret = notification_set_text(
              noti_handle, NOTIFICATION_TEXT_TYPE_CONTENT, body.c_str(),
              nullptr, NOTIFICATION_VARIABLE_TYPE_NONE);
          if (ret != NOTIFICATION_ERROR_NONE) {
            FreeNotification(noti_handle);
            LOG_ERROR("notification_set_text failed : %s",
                      get_error_message(ret));
            result->Error("notification_set_text failed",
                          std::string(get_error_message(ret)));
            return;
          }
        }

        flutter::EncodableMap images;
        if (GetValueFromArgs(arguments, "image", images)) {
          for (const auto &image : images) {
            std::string type;
            std::string path;
            char *resource_path = app_get_shared_resource_path();
            if (std::holds_alternative<std::string>(image.first)) {
              type = std::get<std::string>(image.first);
            }
            if (std::holds_alternative<std::string>(image.second)) {
              path = std::get<std::string>(image.second);
              path = std::string(resource_path) + path;
            }
            ret = notification_set_image(noti_handle, StringToImageType(type),
                                         path.c_str());
            free(resource_path);
            if (ret != NOTIFICATION_ERROR_NONE) {
              FreeNotification(noti_handle);
              LOG_ERROR("notification_set_image failed : %s",
                        get_error_message(ret));
              result->Error("notification_set_image failed",
                            std::string(get_error_message(ret)));
              return;
            }
          }
        }

        int32_t display_applist;
        if (GetValueFromArgs(arguments, "displayApplist", display_applist)) {
          ret = notification_set_display_applist(noti_handle, display_applist);
          if (ret != NOTIFICATION_ERROR_NONE) {
            FreeNotification(noti_handle);
            LOG_ERROR("notification_set_display_applist failed : %s",
                      get_error_message(ret));
            result->Error("notification_set_display_applist failed",
                          std::string(get_error_message(ret)));
            return;
          }
        }

        int32_t properties;
        if (GetValueFromArgs(arguments, "properties", properties)) {
          ret = notification_set_property(noti_handle, properties);
          if (ret != NOTIFICATION_ERROR_NONE) {
            FreeNotification(noti_handle);
            LOG_ERROR("notification_set_property failed : %s",
                      get_error_message(ret));
            result->Error("notification_set_property failed",
                          std::string(get_error_message(ret)));
            return;
          }
        }

        flutter::EncodableValue sound;
        if (GetEncodableValueFromArgs(arguments, "sound", sound)) {
          std::string type;
          if (GetValueFromArgs(&sound, "type", type)) {
            std::string path;
            char *resource_path = app_get_shared_resource_path();
            if (GetValueFromArgs(&sound, "path", path)) {
              path = std::string(resource_path) + path;
            }
            ret = notification_set_sound(noti_handle, StringToSoundType(type),
                                         path.c_str());
            free(resource_path);
            if (ret != NOTIFICATION_ERROR_NONE) {
              FreeNotification(noti_handle);
              LOG_ERROR("notification_set_sound failed : %s",
                        get_error_message(ret));
              result->Error("notification_set_sound failed",
                            std::string(get_error_message(ret)));
              return;
            }
          }
        }

        flutter::EncodableValue vibration;
        if (GetEncodableValueFromArgs(arguments, "vibration", vibration)) {
          std::string type;
          if (GetValueFromArgs(&vibration, "type", type)) {
            std::string path;
            char *resource_path = app_get_shared_resource_path();
            if (GetValueFromArgs(&vibration, "path", path)) {
              path = std::string(resource_path) + path;
            }
            ret = notification_set_vibration(
                noti_handle, StringToVibrationType(type), path.c_str());
            free(resource_path);
            if (ret != NOTIFICATION_ERROR_NONE) {
              FreeNotification(noti_handle);
              LOG_ERROR("notification_set_vibration failed : %s",
                        get_error_message(ret));
              result->Error("notification_set_vibration failed",
                            std::string(get_error_message(ret)));
              return;
            }
          }
        }

        if (GetEncodableValueFromArgs(arguments, "appControl",
                                      app_control_data)) {
          std::string app_id;
          if (GetValueFromArgs(&app_control_data, "appId", app_id)) {
            std::string operation;
            std::string uri;
            std::string category;
            std::string mime;
            flutter::EncodableMap extras;
            flutter::EncodableValue extra_value;
            std::string key;
            ret = app_control_create(&app_control);
            if (ret != APP_CONTROL_ERROR_NONE) {
              FreeNotification(noti_handle);
              LOG_ERROR("app_control_create failed : %s",
                        get_error_message(ret));
              result->Error("app_control_create failed",
                            std::string(get_error_message(ret)));
              return;
            }

            ret = app_control_set_app_id(app_control, app_id.c_str());
            if (ret != APP_CONTROL_ERROR_NONE) {
              DestroyAppControl(app_control);
              FreeNotification(noti_handle);
              LOG_ERROR("app_control_set_app_id failed : %s",
                        get_error_message(ret));
              result->Error("app_control_set_app_id failed",
                            std::string(get_error_message(ret)));
              return;
            }

            if (GetValueFromArgs(&app_control_data, "operation", operation)) {
              ret = app_control_set_operation(app_control, operation.c_str());
              if (ret != APP_CONTROL_ERROR_NONE) {
                DestroyAppControl(app_control);
                FreeNotification(noti_handle);
                LOG_ERROR("app_control_set_operation failed : %s",
                          get_error_message(ret));
                result->Error("app_control_set_operation failed",
                              std::string(get_error_message(ret)));
                return;
              }
            }

            if (GetValueFromArgs(&app_control_data, "uri", uri)) {
              ret = app_control_set_uri(app_control, uri.c_str());
              if (ret != APP_CONTROL_ERROR_NONE) {
                DestroyAppControl(app_control);
                FreeNotification(noti_handle);
                LOG_ERROR("app_control_set_uri failed : %s",
                          get_error_message(ret));
                result->Error("app_control_set_uri failed",
                              std::string(get_error_message(ret)));
                return;
              }
            }

            if (GetValueFromArgs(&app_control_data, "category", category)) {
              ret = app_control_set_category(app_control, category.c_str());
              if (ret != APP_CONTROL_ERROR_NONE) {
                DestroyAppControl(app_control);
                FreeNotification(noti_handle);
                LOG_ERROR("app_control_set_category failed : %s",
                          get_error_message(ret));
                result->Error("app_control_set_category failed",
                              std::string(get_error_message(ret)));
                return;
              }
            }

            if (GetValueFromArgs(&app_control_data, "mime", mime)) {
              ret = app_control_set_mime(app_control, mime.c_str());
              if (ret != APP_CONTROL_ERROR_NONE) {
                DestroyAppControl(app_control);
                FreeNotification(noti_handle);
                LOG_ERROR("app_control_set_mime failed : %s",
                          get_error_message(ret));
                result->Error("app_control_set_mime failed",
                              std::string(get_error_message(ret)));
                return;
              }
            }

            if (GetValueFromArgs(&app_control_data, "extraData", extras)) {
              extra_value = extras;
              for (const auto &extra : extras) {
                std::string map_key;
                if (std::holds_alternative<std::string>(extra.first)) {
                  map_key = std::get<std::string>(extra.first);
                  flutter::EncodableList value_list;
                  std::vector<const char *> values;
                  std::vector<std::string> dummy;
                  std::string value;
                  if (GetValueFromArgs(&extra_value, map_key.c_str(),
                                       value_list)) {
                    for (size_t i = 0; i < value_list.size(); i++) {
                      dummy.push_back(std::get<std::string>(value_list[i]));
                    }
                    for (size_t i = 0; i < value_list.size(); i++) {
                      values.push_back(dummy[i].c_str());
                    }
                    app_control_add_extra_data_array(
                        app_control, map_key.c_str(), values.data(),
                        values.size());
                  } else if (GetValueFromArgs(&extra_value, map_key.c_str(),
                                              value)) {
                    app_control_add_extra_data(app_control, map_key.c_str(),
                                               value.c_str());
                  }
                }
              }
              ret = notification_set_launch_option(
                  noti_handle, NOTIFICATION_LAUNCH_OPTION_APP_CONTROL,
                  (void *)app_control);
              if (ret != NOTIFICATION_ERROR_NONE) {
                FreeNotification(noti_handle);
                DestroyAppControl(app_control);
                LOG_ERROR("notification_set_launch_option failed : %s",
                          get_error_message(ret));
                result->Error("notification_set_launch_option failed",
                              std::string(get_error_message(ret)));
                return;
              }
            }
          }
        }

        if (app_control) {
          ret = app_control_destroy(app_control);
          if (ret != APP_CONTROL_ERROR_NONE) {
            FreeNotification(noti_handle);
            LOG_ERROR("app_control_destroy failed : %s",
                      get_error_message(ret));
            result->Error("app_control_destroy failed",
                          std::string(get_error_message(ret)));
            return;
          }
        }
        ret = notification_post(noti_handle);
        if (ret != NOTIFICATION_ERROR_NONE) {
          FreeNotification(noti_handle);
          LOG_ERROR("notification_post failed : %s", get_error_message(ret));
          result->Error("notification_post failed",
                        std::string(get_error_message(ret)));
          return;
        }
        ret = notification_free(noti_handle);
        if (ret != NOTIFICATION_ERROR_NONE) {
          LOG_ERROR("notification_free failed : %s", get_error_message(ret));
          result->Error("notification_free failed",
                        std::string(get_error_message(ret)));
          return;
        }
        result->Success();
        return;
      }
      result->Error("InvalidArguments", "Please check 'show'");
    } else if (method_call.method_name().compare("cancel") == 0) {
      const flutter::EncodableValue *arguments = method_call.arguments();
      if (arguments != nullptr &&
          std::holds_alternative<std::string>(*arguments)) {
        std::string id = std::get<std::string>(*arguments);
        notification_h noti_handle = nullptr;
        noti_handle = notification_load_by_tag(id.c_str());
        if (noti_handle != nullptr) {
          int ret = notification_delete(noti_handle);
          if (ret != NOTIFICATION_ERROR_NONE) {
            LOG_ERROR("notification_delete failed : %s",
                      get_error_message(ret));
            result->Error("notification_delete failed",
                          std::string(get_error_message(ret)));
            return;
          }
        }
        result->Success();
        return;
      }
      result->Error("InvalidArguments", "Please check 'cancel'");
    } else if (method_call.method_name().compare("cancelAll") == 0) {
      int ret = NOTIFICATION_ERROR_NONE;
      ret = notification_delete_all(NOTIFICATION_TYPE_NOTI);
      if (ret != NOTIFICATION_ERROR_NONE) {
        LOG_ERROR("notification_delete_all failed : %s",
                  get_error_message(ret));
        result->Error("notification_delete_all failed",
                      std::string(get_error_message(ret)));
        return;
      }
      ret = notification_delete_all(NOTIFICATION_TYPE_ONGOING);
      if (ret != NOTIFICATION_ERROR_NONE) {
        LOG_ERROR("notification_delete_all failed : %s",
                  get_error_message(ret));
        result->Error("notification_delete_all failed",
                      std::string(get_error_message(ret)));
        return;
      }
      result->Success();
    } else {
      result->NotImplemented();
    }
  }
};

void TizenNotificationPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  TizenNotificationPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
