// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:convert' as convert;

/// Decodes JWT payload as a json object.
Map<String, dynamic> decodeJWT(String token) {
  final List<String> splitTokens = token.split('.');
  if (splitTokens.length != 3) {
    throw const FormatException('Invalid token.');
  }
  final String normalizedPayload = convert.base64.normalize(splitTokens[1]);
  final String payloadString =
      convert.utf8.decode(convert.base64.decode(normalizedPayload));
  return convert.jsonDecode(payloadString) as Map<String, dynamic>;
}

/// Verifies that if keys in [names] exist in [parameters], their associated
/// values are of type [T].
void checkFormat<T>(List<String> names, Map<String, dynamic> parameters) {
  for (final String name in names) {
    final dynamic value = parameters[name];
    if (value != null && value is! T) {
      throw FormatException('parameter "$name" is not a $T, is "$value".');
    }
  }
}
