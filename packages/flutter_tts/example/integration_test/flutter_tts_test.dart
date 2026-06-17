import 'package:flutter_test/flutter_test.dart';
import 'package:flutter_tts/flutter_tts.dart';
import 'package:integration_test/integration_test.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  late FlutterTts flutterTts;

  setUpAll(() async {
    flutterTts = FlutterTts();
    // Give the native TTS engine time to initialize.
    await Future<void>.delayed(const Duration(seconds: 2));
  });

  tearDown(() async {
    await flutterTts.stop();
  });

  testWidgets('speak returns 1', (WidgetTester tester) async {
    expect(await flutterTts.speak('Hello, world!'), 1);
  });

  testWidgets('stop returns 1', (WidgetTester tester) async {
    await flutterTts.speak('Hello, world!');
    expect(await flutterTts.stop(), 1);
  });

  testWidgets('pause returns 1', (WidgetTester tester) async {
    await flutterTts.speak('Hello, world!');
    expect(await flutterTts.pause(), 1);
  });

  testWidgets('getLanguages returns a non-empty list', (
    WidgetTester tester,
  ) async {
    final languages = await flutterTts.getLanguages;
    expect(languages, isNotNull);
    expect(languages, isNotEmpty);
  });

  testWidgets('isLanguageAvailable is true for a supported language', (
    WidgetTester tester,
  ) async {
    final languages = (await flutterTts.getLanguages as List).cast<String>();
    expect(languages, isNotEmpty);
    expect(await flutterTts.isLanguageAvailable(languages.first), isTrue);
  });

  testWidgets('setLanguage returns 1 for a supported language', (
    WidgetTester tester,
  ) async {
    final languages = (await flutterTts.getLanguages as List).cast<String>();
    expect(languages, isNotEmpty);
    expect(await flutterTts.setLanguage(languages.first), 1);
  });

  testWidgets('getVoices returns a non-empty list', (
    WidgetTester tester,
  ) async {
    final voices = await flutterTts.getVoices;
    expect(voices, isNotNull);
    expect(voices, isNotEmpty);
  });

  testWidgets('getDefaultVoice returns a voice', (WidgetTester tester) async {
    expect(await flutterTts.getDefaultVoice, isNotNull);
  });

  testWidgets('setVoice returns 1', (WidgetTester tester) async {
    final voices = (await flutterTts.getVoices as List).cast<Map>();
    expect(voices, isNotEmpty);
    final voice = voices.first;
    expect(
      await flutterTts.setVoice(<String, String>{
        'name': voice['name'].toString(),
        'locale': voice['locale'].toString(),
      }),
      1,
    );
  });

  testWidgets('setSpeechRate returns 1', (WidgetTester tester) async {
    expect(await flutterTts.setSpeechRate(0.5), 1);
  });

  testWidgets('setVolume returns 1', (WidgetTester tester) async {
    expect(await flutterTts.setVolume(1), 1);
  });

  testWidgets('getMaxSpeechInputLength is positive', (
    WidgetTester tester,
  ) async {
    // tts_get_max_text_size requires the ready state; stop() ensures it.
    await flutterTts.stop();
    final maxLength = await flutterTts.getMaxSpeechInputLength;
    expect(maxLength, isNotNull);
    expect(maxLength, greaterThan(0));
  });
}
