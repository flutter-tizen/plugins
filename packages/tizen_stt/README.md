# tizen_stt

Speech-to-text for Samsung TVs using the Tizen STT engine.

## Requirements

Requires a supported microphone.

Add the following to `tizen/tizen-manifest.xml`:

```xml
<privileges>
    <privilege>http://tizen.org/privilege/recorder</privilege>
    <privilege>http://tizen.org/privilege/internet</privilege>
</privileges>
<feature name="http://tizen.org/feature/microphone"/>
<feature name="http://tizen.org/feature/speech.recognition"/>
```

## Usage

```dart
import 'package:tizen_stt/tizen_stt.dart';

final stt = TizenStt();
final subscription = stt.events.listen((event) {
  if (event.isPartialResult || event.isFinalResult) print(event.text);
  if (event.isError) print(event.errorName ?? event.message);
});

await stt.initialize();
await stt.startListening();
// After the user speaks:
await stt.stopListening();
// After receiving the result, when finished:
await stt.dispose();
await subscription.cancel();
```

Use `getLanguages()` to select a language and `cancel()` to discard recognition.
All instances share one session. Handle `PlatformException` from control calls.
See the [example](example) for a complete app.
