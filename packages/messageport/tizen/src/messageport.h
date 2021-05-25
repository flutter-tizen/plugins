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

  operator bool() const { return MESSAGE_PORT_ERROR_NONE == error_code; }

  std::string message() {
    switch (error_code) {
      case MESSAGE_PORT_ERROR_NONE:
        return "No error";
        break;
      case MESSAGE_PORT_ERROR_IO_ERROR:
        return "Internal I/O error";
        break;
      case MESSAGE_PORT_ERROR_OUT_OF_MEMORY:
        return "Out of memory";
        break;
      case MESSAGE_PORT_ERROR_INVALID_PARAMETER:
        return "Invalid parameter";
        break;
      case MESSAGE_PORT_ERROR_PORT_NOT_FOUND:
        return "The message port of the remote application is not found";
        break;
      case MESSAGE_PORT_ERROR_CERTIFICATE_NOT_MATCH:
        return "The remote application is not signed with the same certificate";
        break;
      case MESSAGE_PORT_ERROR_MAX_EXCEEDED:
        return "The size of the message has exceeded the maximum limit";
        break;
      case MESSAGE_PORT_ERROR_RESOURCE_UNAVAILABLE:
        return "Resource is temporarily unavailable";
        break;
      default:
        return "Unknown error";
        break;
    }
  }

  int error_code;
};

class MessagePortManager {
 public:
  MessagePortManager();
  ~MessagePortManager();

  MessagePortResult CheckRemotePort(std::string& remote_app_id,
                                    std::string& port_name, bool is_trusted,
                                    bool* result);
  MessagePortResult RegisterLocalPort(
      const std::string& port_name,
      std::unique_ptr<flutter::EventSink<flutter::EncodableValue>> sink,
      bool is_trusted, int* local_port);
  MessagePortResult UnregisterLocalPort(int local_port_id);
  MessagePortResult Send(std::string& remote_app_id, std::string& port_name,
                         flutter::EncodableValue& message);
  MessagePortResult Send(std::string& remote_app_id, std::string& port_name,
                         flutter::EncodableValue& message, int local_port);

 private:
  static void OnMessageReceived(int local_port_id, const char* remote_app_id,
                                const char* remote_port,
                                bool trusted_remote_port, bundle* message,
                                void* user_data);

  MessagePortResult CreateResult(int return_code);
  std::map<int, EventSink> sinks_;
  std::set<int> trusted_ports_;
};

#endif  // MESSAGEPORT_H
