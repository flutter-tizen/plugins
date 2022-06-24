import 'dart:async';

import 'package:audioplayers/audioplayers.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';

const String _kAssetAudio = 'nasa_on_a_mission.mp3';
const Duration _kPlayDuration = Duration(seconds: 1);

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  group('asset audio', () {
    testWidgets('can be initialized', (WidgetTester tester) async {
      final player = AudioPlayer();
      final initialized = Completer<void>();
      player.onDurationChanged
          .listen((Duration duration) => initialized.complete());

      await player.setSourceAsset(_kAssetAudio);
      await initialized.future;
      expect(player.state, PlayerState.stopped);

      final duration = await player.getDuration();
      expect(duration, isNotNull);
      expect(duration!.inMilliseconds, greaterThan(0));

      final position = await player.getCurrentPosition();
      expect(duration, isNotNull);
      expect(position!.inMilliseconds, 0);

      await player.dispose();
    });

    testWidgets('can be played', (WidgetTester tester) async {
      final player = AudioPlayer();
      final started = Completer<void>();
      player.onPositionChanged.listen((position) {
        if (!started.isCompleted) {
          started.complete();
        }
      });

      await player.play(AssetSource(_kAssetAudio));
      await started.future;
      expect(player.state, PlayerState.playing);

      final position = await player.getCurrentPosition();
      await Future<void>.delayed(_kPlayDuration);
      final currentPosition = await player.getCurrentPosition();
      expect(position, isNotNull);
      expect(currentPosition, isNotNull);
      expect(position! < currentPosition!, true);

      await player.dispose();
    });

    testWidgets('can seek', (WidgetTester tester) async {
      final player = AudioPlayer();
      final seek = Completer<void>();
      player.onSeekComplete.listen((event) => seek.complete());

      await player.setSourceAsset(_kAssetAudio);
      const seekToPosition = Duration(seconds: 1);
      await player.seek(seekToPosition);
      await seek.future;
      expect(player.state, PlayerState.stopped);

      final position = await player.getCurrentPosition();
      expect(position, isNotNull);
      expect(position!.inMilliseconds, seekToPosition.inMilliseconds);

      await player.dispose();
    });

    testWidgets('can seek with different playrate',
        (WidgetTester tester) async {
      final player = AudioPlayer();
      final started = Completer<void>();
      player.onPositionChanged.listen((position) {
        if (!started.isCompleted) {
          started.complete();
        }
      });

      final seek = Completer<void>();
      player.onSeekComplete.listen((event) => seek.complete());

      await player.play(AssetSource(_kAssetAudio));
      await player.setPlaybackRate(2.0);
      await started.future;

      const seekToPosition = Duration(seconds: 10);
      await player.seek(seekToPosition);
      await seek.future;
      await player.pause();

      final position = await player.getCurrentPosition();
      expect(position, isNotNull);
      expect(position, greaterThanOrEqualTo(seekToPosition));

      await player.dispose();
    });

    testWidgets('can be paused', (WidgetTester tester) async {
      final player = AudioPlayer();
      final started = Completer<void>();
      player.onPositionChanged.listen((position) {
        if (!started.isCompleted) {
          started.complete();
        }
      });

      await player.play(AssetSource(_kAssetAudio));
      await started.future;
      expect(player.state, PlayerState.playing);

      await Future<void>.delayed(_kPlayDuration);
      await player.pause();
      final pausedPosition = await player.getCurrentPosition();
      await Future<void>.delayed(_kPlayDuration);
      final currentPosition = await player.getCurrentPosition();

      expect(player.state, PlayerState.paused);
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

      await player.setSourceAsset(_kAssetAudio);
      await initialized.future;
      final duration = await player.getDuration();
      expect(duration, isNotNull);
      await player.seek(duration! - const Duration(milliseconds: 500));
      await seek.future;

      var isComplete = false;
      player.onPlayerComplete.listen((event) {
        isComplete = true;
      });
      await player.resume();
      await Future<void>.delayed(_kPlayDuration);
      expect(isComplete, true);

      await player.dispose();
    });

    testWidgets('receives position updates regularly',
        (WidgetTester tester) async {
      final player = AudioPlayer();
      final started = Completer<void>();
      var count = 0;
      player.onPositionChanged.listen((position) {
        if (!started.isCompleted) {
          started.complete();
        }
        count += 1;
      });

      await player.play(AssetSource(_kAssetAudio));
      await started.future;
      await Future<void>.delayed(_kPlayDuration);
      expect(count, greaterThanOrEqualTo(5));

      await player.dispose();
    });
  });
}
