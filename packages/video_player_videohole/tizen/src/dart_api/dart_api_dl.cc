/*
 * Copyright (c) 2020, the Dart project authors.  Please see the AUTHORS file
 * for details. All rights reserved. Use of this source code is governed by a
 * BSD-style license that can be found in the LICENSE file.
 */

// This file was copied from
// flutter-tizen/flutter/bin/cache/dart-sdk/include/dart_api_dl.c. We need to
// link to this file from video_player_tizen_plugin.cc. This file caused a build
// error when flutter-tizen build was used. The workaround was to copy this file
// here and make changes to make building possible. See FindFunctionPointer()
// function for details.

#include "dart_api_dl.h" /* NOLINT */

#include <string.h>

#include "dart_version.h"              /* NOLINT */
#include "internal/dart_api_dl_impl.h" /* NOLINT */

#define DART_API_DL_DEFINITIONS(name, R, A) name##_Type name##_DL = NULL;

DART_API_ALL_DL_SYMBOLS(DART_API_DL_DEFINITIONS)

#undef DART_API_DL_DEFINITIONS

typedef void* DartApiEntry_function;

DartApiEntry_function FindFunctionPointer(const DartApiEntry* entries,
                                          const char* name) {
  while (entries->name != NULL) {
    // This is the original line:
    // if (strcmp(entries->name, name) == 0) return entries->function;
    // The generated error is:
    // src/dart_api_dl.c:30:50: error: cannot initialize return object of type
    // 'DartApiEntry_function' (aka 'void *') with an lvalue of type 'void
    // (*const)()' if (strcmp(entries->name, name) == 0) return
    // entries->function;
    //                                              ^~~~~~~~~~~~~~~~~
    // Cast to (void *) was used to compile.
    if (strcmp(entries->name, name) == 0) return (void*)entries->function;
    entries++;
  }
  return NULL;
}

intptr_t Dart_InitializeApiDL(void* data) {
  DartApi* dart_api_data = (DartApi*)data;

  if (dart_api_data->major != DART_API_DL_MAJOR_VERSION) {
    // If the DartVM we're running on does not have the same version as this
    // file was compiled against, refuse to initialize. The symbols are not
    // compatible.
    return -1;
  }
  // Minor versions are allowed to be different.
  // If the DartVM has a higher minor version, it will provide more symbols
  // than we initialize here.
  // If the DartVM has a lower minor version, it will not provide all symbols.
  // In that case, we leave the missing symbols un-initialized. Those symbols
  // should not be used by the Dart and native code. The client is responsible
  // for checking the minor version number himself based on which symbols it
  // is using.
  // (If we would error out on this case, recompiling native code against a
  // newer SDK would break all uses on older SDKs, which is too strict.)

  const DartApiEntry* dart_api_function_pointers = dart_api_data->functions;

#define DART_API_DL_INIT(name, R, A) \
  name##_DL =                        \
      (name##_Type)(FindFunctionPointer(dart_api_function_pointers, #name));
  DART_API_ALL_DL_SYMBOLS(DART_API_DL_INIT)
#undef DART_API_DL_INIT

  return 0;
}
