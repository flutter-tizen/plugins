#include "flutter_secure_storage_tizen_plugin.h"

#include <app_common.h>
#include <ckmc/ckmc-manager.h>
#include <ckmc/ckmc-type.h>
#include <flutter/encodable_value.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>

#include <memory>
#include <string>

#include "log.h"

namespace {

template <typename T>
bool GetValueFromEncodableMap(const flutter::EncodableMap *map, const char *key,
                              T &out) {
  auto iter = map->find(flutter::EncodableValue(key));
  if (iter != map->end() && !iter->second.IsNull()) {
    if (auto *value = std::get_if<T>(&iter->second)) {
      out = *value;
      return true;
    }
  }
  return false;
}

class FlutterSecureStorageTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto channel =
        std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
            registrar->messenger(),
            "plugins.it_nomads.com/flutter_secure_storage",
            &flutter::StandardMethodCodec::GetInstance());

    auto plugin = std::make_unique<FlutterSecureStorageTizenPlugin>();

    channel->SetMethodCallHandler(
        [plugin_pointer = plugin.get()](const auto &call, auto result) {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });

    registrar->AddPlugin(std::move(plugin));
  }

  FlutterSecureStorageTizenPlugin() {}

  virtual ~FlutterSecureStorageTizenPlugin() {}

 private:
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    const auto &method_name = method_call.method_name();
    const auto *arguments =
        std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (method_name == "write") {
      if (!arguments) {
        result->Error("Invalid argument", "No arguments provided.");
        return;
      }

      std::string key;
      GetValueFromEncodableMap(arguments, "key", key);
      std::string value;
      GetValueFromEncodableMap(arguments, "value", value);

      std::optional<std::string> old_value = Read(key);
      if (old_value.has_value()) {
        Delete(key);
      }
      Write(key, value);

      result->Success();
    } else if (method_name == "read") {
      if (!arguments) {
        result->Error("Invalid argument", "No arguments provided.");
        return;
      }

      std::string key;
      GetValueFromEncodableMap(arguments, "key", key);

      std::optional<std::string> value = Read(key);
      if (value.has_value()) {
        result->Success(flutter::EncodableValue(value.value()));
      }
    } else if (method_name == "readAll") {
      result->Success(flutter::EncodableValue(ReadAll()));
    } else if (method_name == "delete") {
      if (!arguments) {
        result->Error("Invalid argument", "No arguments provided.");
        return;
      }
      std::string key;
      GetValueFromEncodableMap(arguments, "key", key);
      Delete(key);
      result->Success();
    } else if (method_name == "deleteAll") {
      DeleteAll();
      result->Success();
    } else if (method_name == "containsKey") {
      if (!arguments) {
        result->Error("Invalid argument", "No arguments provided.");
        return;
      }
      std::string key;
      GetValueFromEncodableMap(arguments, "key", key);
      bool ret = ContainsKey(key);
      result->Success(flutter::EncodableValue(ret));
    } else {
      result->NotImplemented();
    }
  }

  void Write(const std::string &key, const std::string &value) {
    ckmc_raw_buffer_s write_buffer;
    write_buffer.data = (unsigned char *)value.c_str();
    write_buffer.size = value.size();

    ckmc_policy_s policy = {
        .password = nullptr,
        .extractable = true,
    };

    int ret = ckmc_save_data(key.c_str(), write_buffer, policy);
    if (ret != CKMC_ERROR_NONE) {
      LOG_ERROR("Failed to write: key[%s] value[%s]", key.c_str(),
                value.c_str());
    }
  }

  std::optional<std::string> Read(const std::string &key) {
    ckmc_raw_buffer_s *read_buffer;
    int ret = ckmc_get_data(key.c_str(), nullptr, &read_buffer);
    if (ret != CKMC_ERROR_NONE) {
      LOG_ERROR("Failed to Read");
      return std::nullopt;
    }
    std::string read_string((char *)read_buffer->data, read_buffer->size);
    ckmc_buffer_free(read_buffer);
    return read_string;
  }

  flutter::EncodableMap ReadAll() {
    std::vector<std::string> keys = GetKeys();
    flutter::EncodableMap key_value_pairs;

    for (std::string key : keys) {
      std::optional<std::string> value = Read(key);
      if (value.has_value()) {
        key_value_pairs[flutter::EncodableValue(key)] =
            flutter::EncodableValue(value.value());
      }
    }

    return key_value_pairs;
  }

  void Delete(const std::string &key) { ckmc_remove_alias(key.c_str()); }

  void DeleteAll() {
    std::vector<std::string> keys = GetKeys();
    for (std::string key : keys) {
      Delete(key);
    }
  }

  bool ContainsKey(const std::string &key) {
    std::vector<std::string> keys = GetKeys();
    return std::find(keys.begin(), keys.end(), key) != keys.end();
  }

  std::vector<std::string> GetKeys() {
    ckmc_alias_list_s *ckmc_alias_list = nullptr;
    ckmc_get_data_alias_list(&ckmc_alias_list);

    char *app_id = nullptr;
    app_get_id(&app_id);
    std::string id = app_id;
    free(app_id);

    std::vector<std::string> names;
    ckmc_alias_list_s *current = ckmc_alias_list;

    while (current != nullptr) {
      std::string name = current->alias;
      names.push_back(name.substr(name.find(id) + id.length() + 1));
      current = current->next;
    }

    ckmc_alias_list_all_free(ckmc_alias_list);

    return names;
  }
};

}  // namespace

void FlutterSecureStorageTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  FlutterSecureStorageTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
