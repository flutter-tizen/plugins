#ifndef NOTIFICATIONS_HANDLER_H
#define NOTIFICATIONS_HANDLER_H
#include <Utils.h>

#include <memory>

namespace btu {
class NotificationsHandler {
  std::shared_ptr<MethodChannel> _methodChannel;

 public:
  NotificationsHandler(std::shared_ptr<MethodChannel> methodChannel);
  void notifyUIThread(
      std::string const& method,
      google::protobuf::MessageLite const& encodable) const noexcept;
};
};  // namespace btu

#endif  // NOTIFICATIONS_HANDLER_H
