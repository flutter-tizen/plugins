// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: public_member_api_docs

import 'package:ffi/ffi.dart';
import 'package:flutter/services.dart';

import 'ffi.dart';

String enumToString<E>(E enumValue) {
  return enumValue.toString().split('.').last;
}

E enumFromString<E>(
  List<E> enumValues,
  String stringValue, [
  E? defaultValue,
]) {
  return enumValues.firstWhere(
    (E e) => e.toString().split('.').last == stringValue,
    orElse: () => defaultValue ?? enumValues.first,
  );
}

void throwOnError(int ret) {
  if (ret != 0) {
    throw PlatformException(
      code: ret.toString(),
      message: getErrorMessage(ret).toDartString(),
    );
  }
}
