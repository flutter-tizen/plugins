@Timeout(Duration(minutes: 5))
library;

import 'dart:async';

import 'package:audioplayers/audioplayers.dart';
import 'package:audioplayers_platform_interface/audioplayers_platform_interface.dart';
import 'package:audioplayers_tizen_example/tabs/sources.dart';
import 'package:collection/collection.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';

import 'lib/lib_source_test_data.dart';
import 'lib/lib_test_utils.dart';
import 'platform_features.dart';
import 'test_utils.dart';

const _defaultTimeout = Duration(seconds: 30);

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();
  final features = PlatformFeatures.instance();
  final isAndroid = !kIsWeb && defaultTargetPlatform == TargetPlatform.android;
  // Only local asset sources are used; remote URL/stream/bytes playback does
  // not emit reliable events on Tizen. This list is built synchronously (no
  // await) so all tests are declared before the test runner starts. Awaiting
  // getAudioTestDataList() here races the runner on the TV emulator and fails
  // with "Can't call test() once tests have begun running".
  final assetTestDataList = <LibSourceTestData>[wavAsset2TestData];

  testWidgets('test asset source with special char',
      (WidgetTester tester) async {
    final player = AudioPlayer();

    await player.play(specialCharAssetTestData.source);
    await expectLater(player.onPlayerComplete.first, completes);
    await player.dispose();
  });

  testWidgets(
    'test device file source with special char',
    (WidgetTester tester) async {
      final player = AudioPlayer();

      final path = await player.audioCache.loadPath(specialCharAsset);
      expect(path, isNot(contains('%'))); // Ensure path is not URL encoded
      await player.play(DeviceFileSource(path));
      await expectLater(player.onPlayerComplete.first, completes);
      await player.dispose();
    },
    skip: kIsWeb,
  );

  testWidgets(
    'test url source with no extension',
    (WidgetTester tester) async {
      final player = AudioPlayer();

      await player.play(noExtensionAssetTestData.source);
      await expectLater(player.onPlayerComplete.first, completes);
      await player.dispose();
    },
  );

  testWidgets('data URI source', (WidgetTester tester) async {
    final player = AudioPlayer();

    await player.play(mp3DataUriTestData.source);
    await expectLater(player.onPlayerComplete.first, completes);
    await player.dispose();
  });

  group('AP events', () {
    late AudioPlayer player;

    setUp(() async {
      player = AudioPlayer(
        playerId: 'somePlayerId',
      );
    });

    void testPositionUpdater(
      LibSourceTestData td, {
      bool useTimerPositionUpdater = false,
    }) {
      final positionUpdaterName = useTimerPositionUpdater
          ? 'TimerPositionUpdater'
          : 'FramePositionUpdater';
      testWidgets(
        '#positionEvent with $positionUpdaterName: ${td.source}',
        (tester) async {
          if (useTimerPositionUpdater) {
            player.positionUpdater = TimerPositionUpdater(
              getPosition: player.getCurrentPosition,
              interval: const Duration(milliseconds: 100),
            );
          }
          final futurePositions = player.onPositionChanged.toList();

          await player.setReleaseMode(ReleaseMode.stop);
          await player.setSource(td.source);
          await player.resume();
          await tester.pumpGlobalFrames(const Duration(seconds: 5));

          if (!td.isLiveStream && td.duration! < const Duration(seconds: 2)) {
            expect(player.state, PlayerState.completed);
          } else {
            if (td.isLiveStream || td.duration! > const Duration(seconds: 10)) {
              expect(player.state, PlayerState.playing);
            } else {
              // Don't know for sure, if has yet completed or is still playing
            }
            await player.stop();
            expect(player.state, PlayerState.stopped);
          }
          await player.dispose();
          final positions = await futurePositions;
          printOnFailure('Positions: $positions');
          expect(positions, isNot(contains(null)));
          expect(positions, contains(greaterThan(Duration.zero)));
          if (td.isLiveStream) {
            // TODO(gustl22): Live streams may have zero or null as initial
            //  position. This should be consistent across all platforms.
          } else {
            expect(positions.first, Duration.zero);
            expect(positions.last, Duration.zero);
          }
        },
        skip:
            // FIXME(gustl22): [FLAKY] macos 13 fails on live streams.
            (isMacOS && td.isLiveStream) ||
                // FIXME(gustl22): Android provides no position for samples
                //  shorter than 0.5 seconds.
                (isAndroid &&
                    !td.isLiveStream &&
                    td.duration! < const Duration(seconds: 1)),
      );
    }

    /// Test at least one source with [TimerPositionUpdater].
    testPositionUpdater(wavAsset2TestData, useTimerPositionUpdater: true);

    for (final td in assetTestDataList) {
      testPositionUpdater(td);
    }
  });

  group('play multiple sources', () {
    testWidgets(
      'simultaneously',
      (WidgetTester tester) async {
        final players =
            List.generate(assetTestDataList.length, (_) => AudioPlayer());

        // Start all players simultaneously
        final iterator = List<int>.generate(assetTestDataList.length, (i) => i);
        await Future.wait(
          iterator.map(
            (i) async => players[i].play(assetTestDataList[i].source),
          ),
        );
        final playerStates = List<PlayerState?>.generate(
          assetTestDataList.length,
          (index) => null,
        );
        await tester.waitFor(
          () async {
            // TODO(gustl22): Improve detection of started players via player
            //  state.
            final unplayed = playerStates
                .mapIndexed(
                  (index, element) => element != null ? null : index,
                )
                .nonNulls;
            for (final i in unplayed) {
              final player = players[i];
              if (player.state == PlayerState.completed ||
                  player.state == PlayerState.disposed) {
                playerStates[i] = player.state;
              } else if (((await player.getCurrentPosition()) ??
                      Duration.zero) >
                  Duration.zero) {
                playerStates[i] = PlayerState.playing;
              }
            }
            expect(playerStates, everyElement(isNotNull));
          },
        );
        await Future.wait<void>(iterator.map((i) => players[i].stop()));
        await Future.wait(players.map((p) => p.dispose()));
      },
      // FIXME: Causes media error on Android (see #1333, #1353)
      // Unexpected platform error: MediaPlayer error with
      // what:MEDIA_ERROR_UNKNOWN {what:1} extra:MEDIA_ERROR_SYSTEM
      // FIXME: Cannot play multiple players simultaneously at exactly the same
      //  time on Android Exo Player
      skip: isAndroid,
    );

    testWidgets(
      'consecutively',
      (WidgetTester tester) async {
        final player = AudioPlayer();

        for (final td in assetTestDataList) {
          player.play(td.source);
          // TODO(gustl22): Improve detection of started players via player
          //  state.
          PlayerState? playerState;
          await tester.waitFor(
            () async {
              if (player.state == PlayerState.completed ||
                  player.state == PlayerState.disposed) {
                playerState = player.state;
              } else if (((await player.getCurrentPosition()) ??
                      Duration.zero) >
                  Duration.zero) {
                playerState = PlayerState.playing;
              }
              expect(playerState, isNotNull);
            },
          );
          await player.stop();
        }
        await player.dispose();
      },
    );
  });

  group('Audio Context', () {
    testWidgets(
      'Set global AudioContextConfig on unsupported platforms',
      (WidgetTester tester) async {
        final audioContext = AudioContextConfig().build();
        final globalLogFuture = AudioPlayer.global.onLog.first;
        await AudioPlayer.global.setAudioContext(audioContext);

        expect(
          await globalLogFuture,
          contains('Setting AudioContext is not supported'),
        );

        final player = AudioPlayer();
        final logFuture = player.onLog.first;
        await player.setAudioContext(audioContext);
        expect(
          await logFuture,
          contains('Setting AudioContext is not supported'),
        );

        await player.dispose();
      },
      skip: features.hasRespectSilence,
    );
  });

  testWidgets('Race condition on play and pause (#1687) with asset source',
      (WidgetTester tester) async {
    final player = AudioPlayer();

    final futurePlay = player.play(wavAsset2TestData.source);

    // Player is still in `stopped` state as it isn't playing yet.
    expect(player.state, PlayerState.stopped);
    expect(player.desiredState, PlayerState.playing);

    // Execute `pause` before `play` has finished.
    final futurePause = player.pause();
    expect(player.desiredState, PlayerState.paused);

    await futurePlay;
    await futurePause;

    expect(player.state, PlayerState.paused);

    await player.dispose();
  });

  // Ported from upstream platform_test.dart: low-level platform-interface
  // (channel contract) regression tests. Source-driven cases use the
  // asset-only list to avoid network playback, which is unreliable on Tizen.
  group('Platform method channel', () {
    late AudioplayersPlatformInterface platform;
    late String playerId;

    setUp(() async {
      platform = AudioplayersPlatformInterface.instance;
      playerId = 'somePlayerId';
      await platform.create(playerId);
    });

    tearDown(() async {
      await platform.dispose(playerId);
    });

    testWidgets('#create and #dispose', (tester) async {
      await platform.dispose(playerId);

      try {
        await platform.stop(playerId);
        fail('PlatformException not thrown');
      } on PlatformException catch (e) {
        // Tizen reports a plugin-specific message, unlike other platforms.
        expect(e.message, 'No AudioPlayer$playerId is exist.');
      }

      // Create player again, so it can be disposed in tearDown
      await platform.create(playerId);
    });

    if (features.hasVolume) {
      for (final td in assetTestDataList) {
        testWidgets('#volume ${td.source}', (tester) async {
          await tester.prepareSource(
            playerId: playerId,
            platform: platform,
            testData: td,
          );
          for (final volume in [0.0, 0.5, 1.0]) {
            await platform.setVolume(playerId, volume);
            await platform.resume(playerId);
            await tester.pump(const Duration(seconds: 1));
            await platform.stop(playerId);
          }
        });
      }
    }

    for (final td in assetTestDataList) {
      if (features.hasPlaybackRate && !td.isLiveStream) {
        testWidgets('#playbackRate ${td.source}', (tester) async {
          await tester.prepareSource(
            playerId: playerId,
            platform: platform,
            testData: td,
          );
          for (final playbackRate in [0.5, 1.0, 2.0]) {
            await platform.setPlaybackRate(playerId, playbackRate);
            await platform.resume(playerId);
            await tester.pump(const Duration(seconds: 1));
            await platform.stop(playerId);
          }
        });
      }
    }

    for (final td in assetTestDataList) {
      if (features.hasSeek && !td.isLiveStream) {
        testWidgets('#seek with millisecond precision ${td.source}',
            (tester) async {
          await tester.prepareSource(
            playerId: playerId,
            platform: platform,
            testData: td,
          );

          final eventStream = platform.getEventStream(playerId);
          final seekCompleter = Completer<void>();
          final onSeekSub = eventStream
              .where((event) => event.eventType == AudioEventType.seekComplete)
              .listen(
                (_) => seekCompleter.complete(),
                onError: seekCompleter.completeError,
              );
          await platform.seek(playerId, const Duration(milliseconds: 22));
          await seekCompleter.future.timeout(_defaultTimeout);
          await onSeekSub.cancel();
          final positionMs = await platform.getCurrentPosition(playerId);
          expect(
            positionMs != null ? Duration(milliseconds: positionMs) : null,
            (Duration? actual) => durationRangeMatcher(
              actual,
              const Duration(milliseconds: 22),
              deviation: const Duration(milliseconds: 1),
            ),
          );
        });
      }
    }

    for (final td in assetTestDataList) {
      if (features.hasReleaseModeLoop &&
          !td.isLiveStream &&
          td.duration! < const Duration(seconds: 2)) {
        testWidgets('#ReleaseMode.loop ${td.source}', (tester) async {
          await tester.prepareSource(
            playerId: playerId,
            platform: platform,
            testData: td,
          );
          await platform.setReleaseMode(playerId, ReleaseMode.loop);
          await platform.resume(playerId);
          await tester.pump(const Duration(seconds: 3));
          await platform.stop(playerId);
        });
      }
    }

    for (final td in assetTestDataList) {
      if (features.hasReleaseModeRelease && !td.isLiveStream) {
        testWidgets('#ReleaseMode.release ${td.source}', (tester) async {
          await tester.prepareSource(
            playerId: playerId,
            platform: platform,
            testData: td,
          );
          await platform.setReleaseMode(playerId, ReleaseMode.release);
          await platform.resume(playerId);
          if (td.duration! < const Duration(seconds: 2)) {
            await tester.pumpAndSettle(const Duration(seconds: 3));
          } else {
            await tester.pumpAndSettle(const Duration(seconds: 1));
            await platform.stop(playerId);
          }
          expect(await platform.getDuration(playerId), null);
          expect(await platform.getCurrentPosition(playerId), null);
        });
      }
    }

    for (final td in assetTestDataList) {
      testWidgets('#release ${td.source}', (tester) async {
        await tester.prepareSource(
          playerId: playerId,
          platform: platform,
          testData: td,
        );
        await tester.pump(const Duration(seconds: 1));
        await platform.release(playerId);
        expect(await platform.getDuration(playerId), null);
        expect(await platform.getCurrentPosition(playerId), null);
      });
    }
  });

  group('Platform event channel', () {
    late AudioplayersPlatformInterface platform;
    late String playerId;

    setUp(() async {
      platform = AudioplayersPlatformInterface.instance;
      playerId = 'somePlayerId';
      await platform.create(playerId);
    });

    tearDown(() async {
      await platform.dispose(playerId);
    });

    for (final td in assetTestDataList) {
      if (!td.isLiveStream && td.duration! < const Duration(seconds: 2)) {
        testWidgets('#completeEvent ${td.source}', (tester) async {
          await tester.prepareSource(
            playerId: playerId,
            platform: platform,
            testData: td,
          );

          expect(
            platform.getEventStream(playerId).map((event) => event.eventType),
            emitsThrough(AudioEventType.complete),
          );

          await platform.resume(playerId);
          await tester.pumpAndSettle(const Duration(seconds: 3));
        });
      }
    }

    testWidgets('Listen and cancel twice', (tester) async {
      final eventStream = platform.getEventStream(playerId);
      for (var i = 0; i < 2; i++) {
        final eventSub = eventStream.listen(null);
        await eventSub.cancel();
      }
    });

    testWidgets('Emit platform log', (tester) async {
      final eventStream = platform.getEventStream(playerId);
      expect(
        eventStream,
        emitsThrough(
          const AudioEvent(
            eventType: AudioEventType.log,
            logMessage: 'SomeLog',
          ),
        ),
      );
      await platform.emitLog(playerId, 'SomeLog');
    });

    testWidgets('Emit global platform log', (tester) async {
      final global = GlobalAudioplayersPlatformInterface.instance;

      final globalEventStream = global.getGlobalEventStream();
      expect(
        globalEventStream,
        emitsThrough(
          const GlobalAudioEvent(
            eventType: GlobalAudioEventType.log,
            logMessage: 'SomeGlobalLog',
          ),
        ),
      );

      await global.emitGlobalLog('SomeGlobalLog');
    });

    testWidgets('Emit platform error', (tester) async {
      final eventStream = platform.getEventStream(playerId);
      expect(
        eventStream,
        emitsThrough(
          emitsError(
            isA<PlatformException>()
                .having(
                  (PlatformException e) => e.code,
                  'code',
                  'SomeErrorCode',
                )
                .having(
                  (PlatformException e) => e.message,
                  'message',
                  'SomeErrorMessage',
                ),
          ),
        ),
      );

      await platform.emitError(
        playerId,
        'SomeErrorCode',
        'SomeErrorMessage',
      );
    });

    testWidgets('Emit global platform error', (tester) async {
      final global = GlobalAudioplayersPlatformInterface.instance;
      final globalEventStream = global.getGlobalEventStream();
      expect(
        globalEventStream,
        emitsThrough(
          emitsError(
            isA<PlatformException>()
                .having(
                  (PlatformException e) => e.code,
                  'code',
                  'SomeGlobalErrorCode',
                )
                .having(
                  (PlatformException e) => e.message,
                  'message',
                  'SomeGlobalErrorMessage',
                ),
          ),
        ),
      );

      await global.emitGlobalError(
        'SomeGlobalErrorCode',
        'SomeGlobalErrorMessage',
      );
    });
  });
}

extension on WidgetTester {
  Future<void> prepareSource({
    required String playerId,
    required AudioplayersPlatformInterface platform,
    required LibSourceTestData testData,
  }) async {
    final eventStream = platform.getEventStream(playerId);
    final preparedFuture = eventStream
        .firstWhere(
          (event) =>
              event.eventType == AudioEventType.prepared &&
              (event.isPrepared ?? false),
        )
        .timeout(_defaultTimeout);

    Future<void> setSource(Source source) async {
      if (source is UrlSource) {
        return platform.setSourceUrl(playerId, source.url);
      } else if (source is AssetSource) {
        final cachePath = await AudioCache.instance.loadPath(source.path);
        return platform.setSourceUrl(playerId, cachePath, isLocal: true);
      } else if (source is BytesSource) {
        return platform.setSourceBytes(playerId, source.bytes);
      } else {
        throw 'Unknown source type: ${source.runtimeType}';
      }
    }

    final setSourceFuture = setSource(testData.source);

    await Future.wait([setSourceFuture, preparedFuture]);
  }
}
