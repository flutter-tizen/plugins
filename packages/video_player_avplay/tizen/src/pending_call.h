// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_PENDING_CALL_H_
#define FLUTTER_PLUGIN_PENDING_CALL_H_

#include <dart_api_dl.h>

#include <condition_variable>
#include <mutex>

#include "log.h"

class PendingCall {
 public:
  PendingCall(void **buffer, size_t *length)
      : response_buffer_(buffer), response_length_(length) {
    receive_port_ =
        Dart_NewNativePort_DL("cpp-response", &PendingCall::HandleResponse,
                              /*handle_concurrently=*/false);
  }
  ~PendingCall() { Dart_CloseNativePort_DL(receive_port_); }

  Dart_Port port() const { return receive_port_; }

  void PostAndWait(Dart_Port port, Dart_CObject *object) {
    std::unique_lock<std::mutex> lock(mutex);
    const bool success = Dart_PostCObject_DL(port, object);
    if (!success) {
      LOG_ERROR("[ffi] Failed to send message, invalid port or isolate died.");
      return;
    }

    LOG_INFO("[ffi] Waiting for result.");
    while (!notified) {
      cv.wait(lock);
    }
  }

  static void HandleResponse(Dart_Port port, Dart_CObject *message) {
    if (message->type != Dart_CObject_kArray) {
      LOG_ERROR("[ffi] Wrong Data: message->type != Dart_CObject_kArray");
    }
    Dart_CObject **c_response_args = message->value.as_array.values;
    Dart_CObject *c_pending_call = c_response_args[0];
    Dart_CObject *c_message = c_response_args[1];
    LOG_INFO("[ffi] HandleResponse (call: %d)",
             reinterpret_cast<intptr_t>(c_pending_call));

    auto *pending_call = reinterpret_cast<PendingCall *>(
        c_pending_call->type == Dart_CObject_kInt64
            ? c_pending_call->value.as_int64
            : c_pending_call->value.as_int32);

    pending_call->ResolveCall(c_message);
  }

 private:
  static bool NonEmptyBuffer(void **value) { return *value != nullptr; }

  void ResolveCall(Dart_CObject *bytes) {
    assert(bytes->type == Dart_CObject_kTypedData);
    if (bytes->type != Dart_CObject_kTypedData) {
      LOG_ERROR("[ffi] Wrong Data: bytes->type != Dart_CObject_kTypedData");
    }
    const intptr_t response_length = bytes->value.as_typed_data.length;
    const uint8_t *response_buffer = bytes->value.as_typed_data.values;

    void *buffer = malloc(response_length);
    memmove(buffer, response_buffer, response_length);

    *response_buffer_ = buffer;
    *response_length_ = response_length;

    LOG_INFO("[ffi] Notify result ready.");
    notified = true;
    cv.notify_one();
  }

  std::mutex mutex;
  std::condition_variable cv;
  bool notified = false;

  Dart_Port receive_port_;
  void **response_buffer_;
  size_t *response_length_;
};

#endif  // FLUTTER_PLUGIN_PENDING_CALL_H_
