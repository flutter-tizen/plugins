import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';
import 'package:permission_handler/permission_handler.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  testWidgets('get permission status', (tester) async {
    expect(await Permission.camera.status.isDenied, true);
  });

  testWidgets('get location service status', (tester) async {
    var status = await Permission.location.serviceStatus;
    expect(status.isEnabled || status.isDisabled, true);
  });

  testWidgets('open app settings', (tester) async {
    expect(await openAppSettings(), true);
  }, skip: true);
}
