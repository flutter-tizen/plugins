#ifndef FLUTTER_BLUE_TIZEN_NOTIFICATIONS_HANDLER_H
#define FLUTTER_BLUE_TIZEN_NOTIFICATIONS_HANDLER_H

#include <utils.h>

#include <memory>

namespace flutter_blue_tizen {

class NotificationsHandler {

 public:
  NotificationsHandler(std::shared_ptr<MethodChannel> method_channel);

  void NotifyUIThread(
      const std::string& method,
      const google::protobuf::MessageLite& encodable) const noexcept;

private:

  std::shared_ptr<MethodChannel> method_channel_;
};

}  // namespace flutter_blue_tizen
#endif  // FLUTTER_BLUE_TIZEN_NOTIFICATIONS_HANDLER_H
