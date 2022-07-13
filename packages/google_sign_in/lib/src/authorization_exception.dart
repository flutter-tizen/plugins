// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/// A class that represents authorization error response object descriped in OAuth 2.0
/// [spec](https://datatracker.ietf.org/doc/html/draft-ietf-oauth-v2-31#section-5.2).
class AuthorizationException implements Exception {
  /// Creates an [AuthorizationException] instance.
  AuthorizationException(this.error, this.description, this.uri);

  /// A name of the error.
  ///
  /// Possible names are enumerated in the [spec](https://datatracker.ietf.org/doc/html/draft-ietf-oauth-v2-31#section-5.2).
  final String error;

  /// A human-readable text providing additional information of the error.
  final String? description;

  /// A Uri identifying a human-readable web page with information about the error.
  final Uri? uri;

  @override
  String toString() {
    String errorString = 'Authorization error ($error)';
    if (description != null) {
      errorString = '$errorString: $description';
    }
    if (uri != null) {
      errorString = '$errorString: $uri';
    }
    return '$errorString.';
  }
}
