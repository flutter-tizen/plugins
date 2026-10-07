# tizen_stt_example

[![pub package](https://img.shields.io/pub/v/tizen_stt.svg)](https://pub.dev/packages/tizen_stt)

Demonstrates how to use the tizen_stt plugin.

## Getting Started

To run this app on your Tizen device, use [flutter-tizen](https://github.com/flutter-tizen/flutter-tizen).

1. Select **Initialize** and wait for `ready`.
2. Move focus to **Hold Select to record** using the directional buttons.
3. Hold the remote's **Select** button while speaking (keyboard Enter also works).
4. Release Select to stop recording and request the final STT result. The
   recognized text appears below the controls.

Key repeats do not start additional recordings. Moving focus away stops the
recording; leaving the app releases the STT session. Silence detection is disabled
for each recording, so engines that do not support this setting report an error.
Engine recording time limits still apply.

The app uses the STT engine's microphone input. Select controls the STT session;
it does not activate a remote microphone or change TV audio routing. Verify that
your TV/engine provides microphone audio without pressing the remote's voice
button (which may launch Bixby). A supported, independently available microphone
is required. This cannot be verified with mocked widget tests.
