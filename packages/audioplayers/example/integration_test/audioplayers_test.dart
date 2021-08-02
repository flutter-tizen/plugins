import 'dart:async';

import 'package:integration_test/integration_test.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:audioplayers/audio_cache.dart';
import 'package:audioplayers/audioplayers.dart';

const Duration _playDuration = const Duration(seconds: 1);

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  group('asset audio', () {
    testWidgets('can be initialized', (WidgetTester tester) async {
      AudioPlayer audioPlayer = AudioPlayer();
      final Completer<void> initialized = Completer();
      audioPlayer.onDurationChanged
          .listen((duration) => initialized.complete());

      AudioCache audioCache = AudioCache();
      await audioPlayer.setUrl(await audioCache.getAbsoluteUrl('audio2.mp3'));
      await initialized.future;
      expect(audioPlayer.state, AudioPlayerState.STOPPED);

      int duration = await audioPlayer.getDuration();
      expect(duration, greaterThan(0));

      int position = await audioPlayer.getCurrentPosition();
      expect(position, 0);

      audioPlayer.dispose();
    });

    testWidgets('can be played', (WidgetTester tester) async {
      AudioPlayer audioPlayer = AudioPlayer();
      final Completer<void> started = Completer();
      audioPlayer.onAudioPositionChanged.listen((position) {
        if (!started.isCompleted) {
          started.complete();
        }
      });

      AudioCache audioCache = AudioCache();
      await audioPlayer.play(await audioCache.getAbsoluteUrl('audio2.mp3'));
      await started.future;
      expect(audioPlayer.state, AudioPlayerState.PLAYING);

      int position = await audioPlayer.getCurrentPosition();
      await tester.pumpAndSettle(_playDuration);
      int currentPosition = await audioPlayer.getCurrentPosition();
      expect(currentPosition, greaterThan(position));

      audioPlayer.dispose();
    });

    testWidgets('can seek', (WidgetTester tester) async {
      AudioPlayer audioPlayer = AudioPlayer();
      final Completer<void> seek = Completer();
      audioPlayer.onSeekComplete.listen((event) => seek.complete());

      AudioCache audioCache = AudioCache();
      await audioPlayer.setUrl(await audioCache.getAbsoluteUrl('audio2.mp3'));
      Duration seekToPosition = const Duration(seconds: 1);
      await audioPlayer.seek(seekToPosition);
      await seek.future;
      expect(audioPlayer.state, AudioPlayerState.STOPPED);

      int position = await audioPlayer.getCurrentPosition();
      expect(position, seekToPosition.inMilliseconds);

      audioPlayer.dispose();
    });

    testWidgets('can be paused', (WidgetTester tester) async {
      AudioPlayer audioPlayer = AudioPlayer();
      final Completer<void> started = Completer();
      audioPlayer.onAudioPositionChanged.listen((position) {
        if (!started.isCompleted) {
          started.complete();
        }
      });

      AudioCache audioCache = AudioCache();
      await audioPlayer.play(await audioCache.getAbsoluteUrl('audio2.mp3'));
      await started.future;
      expect(audioPlayer.state, AudioPlayerState.PLAYING);

      await tester.pumpAndSettle(_playDuration);
      await audioPlayer.pause();
      int pausedPosition = await audioPlayer.getCurrentPosition();
      await tester.pumpAndSettle(_playDuration);
      int currentPosition = await audioPlayer.getCurrentPosition();

      expect(audioPlayer.state, AudioPlayerState.PAUSED);
      expect(currentPosition, pausedPosition);

      audioPlayer.dispose();
    });

    testWidgets('do not exceed duration after audio completed',
        (WidgetTester tester) async {
      AudioPlayer audioPlayer = AudioPlayer();
      final Completer<void> initialized = Completer();
      final Completer<void> seek = Completer();
      audioPlayer.onDurationChanged.listen((duration) {
        if (!initialized.isCompleted) {
          initialized.complete();
        }
      });
      audioPlayer.onSeekComplete.listen((event) => seek.complete());

      AudioCache audioCache = AudioCache();
      await audioPlayer.setUrl(await audioCache.getAbsoluteUrl('audio2.mp3'));
      await initialized.future;
      int duration = await audioPlayer.getDuration();
      await audioPlayer
          .seek(Duration(milliseconds: duration) - Duration(milliseconds: 500));
      await seek.future;

      await audioPlayer.resume();
      await tester.pumpAndSettle(_playDuration);
      expect(audioPlayer.state, AudioPlayerState.COMPLETED);

      audioPlayer.dispose();
    });
  });
}
