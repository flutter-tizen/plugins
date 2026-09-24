// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef FLUTTER_TIZEN_PLUGINS_WRAPPER_TIZEN_RPC_PORT_H_
#define FLUTTER_TIZEN_PLUGINS_WRAPPER_TIZEN_RPC_PORT_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int ftpw_tizen_rpc_port_rpc_port_parcel_get_reader(void *parcel,
                                                   uint32_t *value);
int ftpw_tizen_rpc_port_rpc_port_parcel_set_reader(void *parcel,
                                                   uint32_t value);
int ftpw_tizen_rpc_port_rpc_port_parcel_get_data_size(void *parcel,
                                                      uint32_t *value);
int ftpw_tizen_rpc_port_rpc_port_parcel_set_data_size(void *parcel,
                                                      uint32_t value);
int ftpw_tizen_rpc_port_rpc_port_parcel_reserve(void *parcel, uint32_t value);

#ifdef __cplusplus
}
#endif

#endif  // FLUTTER_TIZEN_PLUGINS_WRAPPER_TIZEN_RPC_PORT_H_
