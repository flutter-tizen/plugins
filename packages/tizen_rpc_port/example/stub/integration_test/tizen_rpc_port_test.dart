// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';
import 'package:tizen_rpc_port/src/parcel.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  testWidgets('Parcel test', (WidgetTester tester) async {
    final Parcel parcel = Parcel();
    parcel.writeBool(false);
    parcel.writeInt32(123);
    parcel.writeString('Hello');
    parcel.writeByte(0x3f);
    parcel.writeDouble(123.4);

    expect(parcel.readBool(), false);
    expect(parcel.readInt32(), 123);
    expect(parcel.readString(), 'Hello');
    expect(parcel.readByte(), 0x3f);
    expect(parcel.readDouble(), 123.4);
  });
}
