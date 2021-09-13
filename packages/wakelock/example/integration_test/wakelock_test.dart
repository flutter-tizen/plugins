// @dart = 2.9

import 'package:flutter_test/flutter_test.dart';
import 'package:wakelock/wakelock.dart';
import 'package:integration_test/integration_test.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  testWidgets('initial wakelock status', (WidgetTester tester) async {
    expect(await Wakelock.enabled, false);
  });

  testWidgets('enable wakelock', (WidgetTester tester) async {
    await Wakelock.enable();
    expect(await Wakelock.enabled, true);
  });

  testWidgets('disable wakelock', (WidgetTester tester) async {
    await Wakelock.disable();
    expect(await Wakelock.enabled, false);
  });
}
