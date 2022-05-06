#include <notifications_handler.h>

namespace flutter_blue_tizen {

NotificationsHandler::NotificationsHandler(
    std::shared_ptr<MethodChannel> methodChannel)
    : method_channel_(methodChannel) {}

auto NotificationsHandler::NotifyUIThread(
    const std::string& method,
    const google::protobuf::MessageLite& mess) const noexcept -> void {
  std::vector<uint8_t> encodable(mess.ByteSizeLong());
  mess.SerializeToArray(encodable.data(), mess.ByteSizeLong());
  method_channel_->InvokeMethod(
      method, std::make_unique<flutter::EncodableValue>(encodable));
}

}  // namespace flutter_blue_tizen