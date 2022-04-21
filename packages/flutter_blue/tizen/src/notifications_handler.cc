#include <notifications_handler.h>

namespace flutter_blue_tizen {

NotificationsHandler::NotificationsHandler(
    std::shared_ptr<MethodChannel> methodChannel)
    : _methodChannel(methodChannel) {}

auto NotificationsHandler::notifyUIThread(
    std::string const& method,
    google::protobuf::MessageLite const& mess) const noexcept -> void {
  std::vector<uint8_t> encodable(mess.ByteSizeLong());
  mess.SerializeToArray(encodable.data(), mess.ByteSizeLong());
  _methodChannel->InvokeMethod(
      method, std::make_unique<flutter::EncodableValue>(encodable));
}
}  // namespace flutter_blue_tizen