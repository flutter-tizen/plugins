// @dart = 2.9

import 'dart:async';

import 'package:flutter_test/flutter_test.dart';
import 'package:permission_handler/permission_handler.dart';
import 'package:integration_test/integration_test.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  testWidgets('get permission status', (WidgetTester tester) async {
    expect(await Permission.camera.status.isDenied, true);
  });

  testWidgets('get location service status', (WidgetTester tester) async {
    var status = await Permission.location.serviceStatus;
    expect(status.isEnabled || status.isDisabled, true);
  });

  testWidgets('open app settings', (WidgetTester tester) async {
    expect(await openAppSettings(), true);
  }, skip: true);
}
