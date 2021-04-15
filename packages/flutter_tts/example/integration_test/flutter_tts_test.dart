// @dart = 2.9

import 'dart:async';
import 'package:flutter_test/flutter_test.dart';
import 'package:flutter_tts/flutter_tts.dart';
import 'package:integration_test/integration_test.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  testWidgets('Can speak', (WidgetTester tester) async {
    final FlutterTts flutterTts = FlutterTts();
    var result = await flutterTts.speak("Hello World");
    expect(result, 1);
  });
}
