// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter_test/flutter_test.dart';
import 'package:google_sign_in/google_sign_in.dart';
import 'package:google_sign_in_tizen/google_sign_in_tizen.dart';
import 'package:google_sign_in_tizen_example/credentials.dart' as credentials;
import 'package:integration_test/integration_test.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  testWidgets('Can initialize the plugin', (WidgetTester tester) async {
    GoogleSignInTizen.setCredentials(
      clientId: credentials.clientId,
      clientSecret: credentials.clientSecret,
    );

    final GoogleSignIn signIn = GoogleSignIn.instance;
    expect(signIn, isNotNull);

    await signIn.initialize(clientId: credentials.clientId);
  });
}
