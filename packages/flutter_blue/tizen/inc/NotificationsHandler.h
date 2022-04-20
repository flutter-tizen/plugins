#ifndef FLUTTER_BLUE_TIZEN_NOTIFICATIONS_HANDLER_H
#define FLUTTER_BLUE_TIZEN_NOTIFICATIONS_HANDLER_H

#include <Utils.h>

#include <memory>

namespace flutter_blue_tizen {
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
}  // namespace flutter_blue_tizen
#endif  // FLUTTER_BLUE_TIZEN_NOTIFICATIONS_HANDLER_H
