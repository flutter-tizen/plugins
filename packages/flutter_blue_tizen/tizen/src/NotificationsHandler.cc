#include <NotificationsHandler.h>

namespace btu {
NotificationsHandler::NotificationsHandler(
    std::shared_ptr<MethodChannel> methodChannel)
    : _methodChannel(methodChannel) {}

auto NotificationsHandler::notifyUIThread(
    const std::string& method,
    const google::protobuf::MessageLite& mess) const noexcept -> void {
  std::vector<uint8_t> encodable(mess.ByteSizeLong());
  mess.SerializeToArray(encodable.data(), mess.ByteSizeLong());
  _methodChannel->InvokeMethod(
      method, std::make_unique<flutter::EncodableValue>(encodable));
}
}  // namespace btu
