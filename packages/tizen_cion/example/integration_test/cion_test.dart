// Copyright 2022 - 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';
import 'package:tizen_cion/tizen_cion.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  testWidgets('DataPayload test', (WidgetTester _) async {
    final Uint8List data = Uint8List.fromList(List<int>.filled(100, 0x3f));
    final DataPayload payload = DataPayload(data);
    expect(payload.handle.address != 0, true);
    expect(payload.id.isEmpty, false);
    expect(payload.type, PayloadType.data);
    expect(payload.data, data);
  });

  testWidgets('SecurityInfo test', (WidgetTester _) async {
    final SecurityInfo security = SecurityInfo();
    security.caPath = 'test_caPath';
    security.certPath = 'test_certPath';
    security.privateKeyPath = 'test_privateKey';
    expect(security.caPath, 'test_caPath');
    expect(security.certPath, 'test_certPath');
    expect(security.privateKeyPath, 'test_privateKey');
  });
}
