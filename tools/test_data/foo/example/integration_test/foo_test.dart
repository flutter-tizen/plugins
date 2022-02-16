import 'package:flutter_test/flutter_test.dart';
import 'package:foo/foo.dart';
import 'package:integration_test/integration_test.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  testWidgets('Can get platform version', (WidgetTester tester) async {
    String platformVersion = await Foo.platformVersion ?? '';
    expect(platformVersion, 'Tizen');
  });
}
