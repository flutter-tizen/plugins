import 'dart:async';

import 'package:audioplayers/audioplayers.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';

const Duration _kPlayDuration = Duration(seconds: 1);

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  group('asset audio', () {
    testWidgets('can be initialized', (WidgetTester tester) async {
      final player = AudioPlayer();
      final initialized = Completer<void>();
      player.onDurationChanged
          .listen((Duration duration) => initialized.complete());

      final audioCache = AudioCache();
      final uri = await audioCache.load('audio2.mp3');
      await player.setUrl(uri.toString());
      await initialized.future;
      expect(player.state, PlayerState.STOPPED);

      final duration = await player.getDuration();
      expect(duration, greaterThan(0));

      final position = await player.getCurrentPosition();
      expect(position, 0);

      await player.dispose();
    });

    testWidgets('can be played', (WidgetTester tester) async {
      final player = AudioPlayer();
      final started = Completer<void>();
      player.onAudioPositionChanged.listen((position) {
        if (!started.isCompleted) {
          started.complete();
        }
      });

      final audioCache = AudioCache();
      final uri = await audioCache.load('audio2.mp3');
      await player.play(uri.toString());
      await started.future;
      expect(player.state, PlayerState.PLAYING);

      final position = await player.getCurrentPosition();
      await Future<void>.delayed(_kPlayDuration);
      final currentPosition = await player.getCurrentPosition();
      expect(currentPosition, greaterThan(position));

      await player.dispose();
    });

    testWidgets('can seek', (WidgetTester tester) async {
      final player = AudioPlayer();
      final seek = Completer<void>();
      player.onSeekComplete.listen((event) => seek.complete());

      final audioCache = AudioCache();
      final uri = await audioCache.load('audio2.mp3');
      await player.setUrl(uri.toString());
      const seekToPosition = Duration(seconds: 1);
      await player.seek(seekToPosition);
      await seek.future;
      expect(player.state, PlayerState.STOPPED);

      final position = await player.getCurrentPosition();
      expect(position, seekToPosition.inMilliseconds);

      await player.dispose();
    });

    testWidgets('can be paused', (WidgetTester tester) async {
      final player = AudioPlayer();
      final started = Completer<void>();
      player.onAudioPositionChanged.listen((position) {
        if (!started.isCompleted) {
          started.complete();
        }
      });

      final audioCache = AudioCache();
      final uri = await audioCache.load('audio2.mp3');
      await player.play(uri.toString());
      await started.future;
      expect(player.state, PlayerState.PLAYING);

      await Future<void>.delayed(_kPlayDuration);
      await player.pause();
      final pausedPosition = await player.getCurrentPosition();
      await Future<void>.delayed(_kPlayDuration);
      final currentPosition = await player.getCurrentPosition();

      expect(player.state, PlayerState.PAUSED);
      expect(currentPosition, pausedPosition);

      await player.dispose();
    });

    testWidgets('do not exceed duration after audio completed',
        (WidgetTester tester) async {
      final player = AudioPlayer();
      final initialized = Completer<void>();
      final seek = Completer<void>();
      player.onDurationChanged.listen((duration) {
        if (!initialized.isCompleted) {
          initialized.complete();
        }
      });
      player.onSeekComplete.listen((event) => seek.complete());

      final audioCache = AudioCache();
      final uri = await audioCache.load('audio2.mp3');
      await player.setUrl(uri.toString());
      await initialized.future;
      final duration = await player.getDuration();
      await player.seek(
          Duration(milliseconds: duration) - const Duration(milliseconds: 500));
      await seek.future;

      await player.resume();
      await Future<void>.delayed(_kPlayDuration);
      expect(player.state, PlayerState.COMPLETED);

      await player.dispose();
    });
  });
}
