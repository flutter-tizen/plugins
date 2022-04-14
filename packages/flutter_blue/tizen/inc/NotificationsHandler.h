#ifndef NOTIFICATIONS_HANDLER_H
#define NOTIFICATIONS_HANDLER_H
#include <Utils.h>

#include <memory>

namespace btu {
class NotificationsHandler {
  std::shared_ptr<MethodChannel> _methodChannel;

 public:
  NotificationsHandler(std::shared_ptr<MethodChannel> methodChannel);
  auto notifyUIThread(
      const std::string& method,
      const google::protobuf::MessageLite& encodable) const noexcept -> void;
};
};  // namespace btu

#endif  // NOTIFICATIONS_HANDLER_H
