// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:io';
import 'dart:ui';

import 'package:camera/camera.dart';
import 'package:flutter/painting.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';
import 'package:path_provider/path_provider.dart';
import 'package:video_player/video_player.dart';

void main() {
  late Directory testDir;

  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  setUpAll(() async {
    final Directory extDir = await getTemporaryDirectory();
    testDir = await Directory('${extDir.path}/test').create(recursive: true);
  });

  tearDownAll(() async {
    await testDir.delete(recursive: true);
  });

  final Map<ResolutionPreset, Size> presetExpectedSizes =
      <ResolutionPreset, Size>{
    ResolutionPreset.medium: const Size(480, 720),
    // Don't bother checking for max here since it could be anything.
  };

  /// Verify that [actual] has dimensions that are at least as large as
  /// [expectedSize]. Allows for a mismatch in portrait vs landscape. Returns
  /// whether the dimensions exactly match.
  bool assertExpectedDimensions(Size expectedSize, Size actual) {
    expect(actual.shortestSide, lessThanOrEqualTo(expectedSize.shortestSide));
    expect(actual.longestSide, lessThanOrEqualTo(expectedSize.longestSide));
    return actual.shortestSide == expectedSize.shortestSide &&
        actual.longestSide == expectedSize.longestSide;
  }

  // This tests that the capture is no bigger than the preset, since we have
  // automatic code to fall back to smaller sizes when we need to. Returns
  // whether the image is exactly the desired resolution.
  Future<bool> testCaptureImageResolution(
    CameraController controller,
    ResolutionPreset preset,
  ) async {
    final Size expectedSize = presetExpectedSizes[preset]!;

    // Take Picture
    final XFile file = await controller.takePicture();

    // Load picture
    final File fileImage = File(file.path);
    final Image image = await decodeImageFromList(fileImage.readAsBytesSync());

    // Verify image dimensions are as expected
    expect(image, isNotNull);
    return assertExpectedDimensions(
      expectedSize,
      Size(image.height.toDouble(), image.width.toDouble()),
    );
  }

  testWidgets('Capture specific image resolutions', (
    WidgetTester tester,
  ) async {
    final List<CameraDescription> cameras = await availableCameras();
    if (cameras.isEmpty) {
      return;
    }
    for (final CameraDescription cameraDescription in cameras) {
      bool previousPresetExactlySupported = true;
      for (final MapEntry<ResolutionPreset, Size> preset
          in presetExpectedSizes.entries) {
        final CameraController controller = CameraController(
          cameraDescription,
          preset.key,
        );
        await controller.initialize();
        final bool presetExactlySupported = await testCaptureImageResolution(
          controller,
          preset.key,
        );
        assert(
          !(!previousPresetExactlySupported && presetExactlySupported),
          'The camera took higher resolution pictures at a lower resolution.',
        );
        previousPresetExactlySupported = presetExactlySupported;
        await controller.dispose();
      }
    }
  });

  // This tests that the capture is no bigger than the preset, since we have
  // automatic code to fall back to smaller sizes when we need to. Returns
  // whether the image is exactly the desired resolution.
  Future<bool> testCaptureVideoResolution(
    CameraController controller,
    ResolutionPreset preset,
  ) async {
    final Size expectedSize = presetExpectedSizes[preset]!;

    // Take Video
    await controller.startVideoRecording();
    sleep(const Duration(milliseconds: 300));
    final XFile file = await controller.stopVideoRecording();

    // Load video metadata
    final File videoFile = File(file.path);
    final VideoPlayerController videoController = VideoPlayerController.file(
      videoFile,
    );
    await videoController.initialize();
    final Size video = videoController.value.size;

    // Verify image dimensions are as expected
    expect(video, isNotNull);
    return assertExpectedDimensions(
      expectedSize,
      Size(video.height, video.width),
    );
  }

  testWidgets('Capture specific video resolutions', (
    WidgetTester tester,
  ) async {
    final List<CameraDescription> cameras = await availableCameras();
    if (cameras.isEmpty) {
      return;
    }
    for (final CameraDescription cameraDescription in cameras) {
      bool previousPresetExactlySupported = true;
      for (final MapEntry<ResolutionPreset, Size> preset
          in presetExpectedSizes.entries) {
        final CameraController controller = CameraController(
          cameraDescription,
          preset.key,
        );
        await controller.initialize();
        // await controller.prepareForVideoRecording();
        final bool presetExactlySupported = await testCaptureVideoResolution(
          controller,
          preset.key,
        );
        assert(
          !(!previousPresetExactlySupported && presetExactlySupported),
          'The camera took higher resolution pictures at a lower resolution.',
        );
        previousPresetExactlySupported = presetExactlySupported;
        await controller.dispose();
      }
    }
  });

  testWidgets('Pause and resume video recording', (WidgetTester tester) async {
    final List<CameraDescription> cameras = await availableCameras();
    if (cameras.isEmpty) {
      return;
    }

    final CameraController controller = CameraController(
      cameras[0],
      ResolutionPreset.low,
      enableAudio: false,
    );

    await controller.initialize();
    // await controller.prepareForVideoRecording();

    int startPause;
    int timePaused = 0;

    await controller.startVideoRecording();
    final int recordingStart = DateTime.now().millisecondsSinceEpoch;
    sleep(const Duration(milliseconds: 500));

    await controller.pauseVideoRecording();
    startPause = DateTime.now().millisecondsSinceEpoch;
    sleep(const Duration(milliseconds: 500));
    await controller.resumeVideoRecording();
    timePaused += DateTime.now().millisecondsSinceEpoch - startPause;

    sleep(const Duration(milliseconds: 500));

    await controller.pauseVideoRecording();
    startPause = DateTime.now().millisecondsSinceEpoch;
    sleep(const Duration(milliseconds: 500));
    await controller.resumeVideoRecording();
    timePaused += DateTime.now().millisecondsSinceEpoch - startPause;

    sleep(const Duration(milliseconds: 500));

    final XFile file = await controller.stopVideoRecording();
    final int recordingTime =
        DateTime.now().millisecondsSinceEpoch - recordingStart;

    final File videoFile = File(file.path);
    final VideoPlayerController videoController = VideoPlayerController.file(
      videoFile,
    );
    await videoController.initialize();
    final int duration = videoController.value.duration.inMilliseconds;
    await videoController.dispose();

    expect(duration, lessThan(recordingTime - timePaused));
  });

  testWidgets('Android image streaming', (WidgetTester tester) async {
    final List<CameraDescription> cameras = await availableCameras();
    if (cameras.isEmpty) {
      return;
    }

    final CameraController controller = CameraController(
      cameras[0],
      ResolutionPreset.low,
      enableAudio: false,
    );

    await controller.initialize();
    bool isDetecting = false;

    await controller.startImageStream((CameraImage image) {
      if (isDetecting) {
        return;
      }

      isDetecting = true;

      expectLater(image, isNotNull).whenComplete(() => isDetecting = false);
    });

    expect(controller.value.isStreamingImages, true);

    sleep(const Duration(milliseconds: 500));

    await controller.stopImageStream();
    await controller.dispose();
  }, skip: !Platform.isAndroid);
}
