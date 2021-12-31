#ifndef MESSAGEPORT_H
#define MESSAGEPORT_H

#include <flutter/event_channel.h>
#include <flutter/standard_method_codec.h>
#include <message_port.h>

#include <map>
#include <set>

typedef std::unique_ptr<flutter::EventSink<flutter::EncodableValue>> EventSink;

struct MessagePortResult {
  MessagePortResult() : error_code(MESSAGE_PORT_ERROR_NONE){};
  MessagePortResult(int code) : error_code(code) {}

  // Returns false on error
  operator bool() const { return MESSAGE_PORT_ERROR_NONE == error_code; }

  std::string message() { return get_error_message(error_code); }

  int error_code;
};

class MessagePortManager {
 public:
  MessagePortManager();
  ~MessagePortManager();

  MessagePortResult CheckRemotePort(std::string& remote_app_id,
                                    std::string& port_name, bool is_trusted,
                                    bool* result);
  MessagePortResult RegisterLocalPort(const std::string& port_name,
                                      EventSink sink, bool is_trusted,
                                      int* local_port);
  MessagePortResult UnregisterLocalPort(int local_port_id);
  MessagePortResult Send(std::string& remote_app_id, std::string& port_name,
                         flutter::EncodableValue& message, bool is_trusted);
  MessagePortResult Send(std::string& remote_app_id, std::string& port_name,
                         flutter::EncodableValue& message, bool is_trusted,
                         int local_port);

 private:
  static void OnMessageReceived(int local_port_id, const char* remote_app_id,
                                const char* remote_port,
                                bool trusted_remote_port, bundle* message,
                                void* user_data);

  MessagePortResult CreateResult(int return_code);
  MessagePortResult PrepareBundle(flutter::EncodableValue& message, bundle*& b);

  std::map<int, EventSink> sinks_;
  std::set<int> trusted_ports_;
};

#endif  // MESSAGEPORT_H
