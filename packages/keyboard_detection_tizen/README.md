# keyboard_detection_tizen

A Tizen-specific Flutter plugin that detects software keyboard (input panel)
visibility and size on Tizen devices. The public API mirrors the
[`keyboard_detection`](https://pub.dev/packages/keyboard_detection) package.

The original `keyboard_detection` package detects keyboard visibility from
`MediaQuery.viewInsets.bottom`, which is not populated on Tizen. This plugin
listens to the `tizen/internal/inputpanel` event channel exposed by the
flutter-tizen embedder instead, and reads the keyboard geometry (height,
width, position) from the same channel.

> **Warning**
> Requires flutter-tizen `3.41.9-tizen.1.0.0` or later. Earlier versions
> of the embedder do not publish geometry on the
> `tizen/internal/inputpanel` channel, so the controller will stay in
> `KeyboardState.unknown`.

## Usage

```yaml
dependencies:
  keyboard_detection_tizen: ^0.1.0
```

```dart
import 'package:keyboard_detection_tizen/keyboard_detection_tizen.dart';

final controller = KeyboardDetectionController(
  onChanged: (state) => debugPrint('keyboard: $state'),
);

@override
Widget build(BuildContext context) {
  return Scaffold(
    body: Column(
      children: [
        const TextField(),
        StreamBuilder<KeyboardState>(
          stream: controller.stream,
          builder: (_, snapshot) => Text(
            'state: ${snapshot.data ?? KeyboardState.unknown} '
            'size: ${controller.size}',
          ),
        ),
      ],
    ),
  );
}
```

## Limitations

- The Tizen input panel channel emits `show` / `hide` / `will_show` only.
  `KeyboardState.hiding` is therefore never reached on Tizen.
- Geometry values are physical pixels reported by `ecore_imf`. Convert with
  `MediaQueryData.devicePixelRatio` if you need logical pixels.
- Floating / split keyboards are not supported.
