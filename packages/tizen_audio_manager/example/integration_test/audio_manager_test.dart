// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';
import 'package:tizen_audio_manager/tizen_audio_manager.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  testWidgets('test alarm max level', (WidgetTester tester) async {
    final int result =
        await AudioManager.volumeController.getMaxLevel(AudioVolumeType.alarm);
    expect(result, isNonNegative);
  });

  testWidgets('test call max level', (WidgetTester tester) async {
    final int result =
        await AudioManager.volumeController.getMaxLevel(AudioVolumeType.call);
    expect(result, isNonNegative);
  });

  testWidgets('test media max level', (WidgetTester tester) async {
    final int result =
        await AudioManager.volumeController.getMaxLevel(AudioVolumeType.media);
    expect(result, isNonNegative);
  });

  testWidgets('test notification max level', (WidgetTester tester) async {
    final int result = await AudioManager.volumeController
        .getMaxLevel(AudioVolumeType.notification);
    expect(result, isNonNegative);
  });

  testWidgets('test ringtone max level', (WidgetTester tester) async {
    final int result = await AudioManager.volumeController
        .getMaxLevel(AudioVolumeType.ringtone);
    expect(result, isNonNegative);
  });

  testWidgets('test system max level', (WidgetTester tester) async {
    final int result =
        await AudioManager.volumeController.getMaxLevel(AudioVolumeType.system);
    expect(result, isNonNegative);
  });

  testWidgets('test voice max level', (WidgetTester tester) async {
    final int result =
        await AudioManager.volumeController.getMaxLevel(AudioVolumeType.voice);
    expect(result, isNonNegative);
  });

  testWidgets('test voip max level', (WidgetTester tester) async {
    final int result =
        await AudioManager.volumeController.getMaxLevel(AudioVolumeType.voip);
    expect(result, isNonNegative);
  });

  testWidgets('test alarm set level', (WidgetTester tester) async {
    final int max =
        await AudioManager.volumeController.getMaxLevel(AudioVolumeType.alarm);
    await AudioManager.volumeController.setLevel(AudioVolumeType.alarm, max);
    int level =
        await AudioManager.volumeController.getLevel(AudioVolumeType.alarm);
    expect(level, equals(max));

    await AudioManager.volumeController.setLevel(AudioVolumeType.alarm, 0);
    level = await AudioManager.volumeController.getLevel(AudioVolumeType.alarm);
    expect(level, equals(0));
  });

  testWidgets('test call set level', (WidgetTester tester) async {
    final int max =
        await AudioManager.volumeController.getMaxLevel(AudioVolumeType.call);
    await AudioManager.volumeController.setLevel(AudioVolumeType.call, max);
    int level =
        await AudioManager.volumeController.getLevel(AudioVolumeType.call);
    expect(level, equals(max));

    await AudioManager.volumeController.setLevel(AudioVolumeType.call, 0);
    level = await AudioManager.volumeController.getLevel(AudioVolumeType.call);
    expect(level, equals(0));
  });

  testWidgets('test media set level', (WidgetTester tester) async {
    final int max =
        await AudioManager.volumeController.getMaxLevel(AudioVolumeType.media);
    await AudioManager.volumeController.setLevel(AudioVolumeType.media, max);
    int level =
        await AudioManager.volumeController.getLevel(AudioVolumeType.media);
    expect(level, equals(max));

    await AudioManager.volumeController.setLevel(AudioVolumeType.media, 0);
    level = await AudioManager.volumeController.getLevel(AudioVolumeType.media);
    expect(level, equals(0));
  });

  testWidgets('test notification set level', (WidgetTester tester) async {
    final int max = await AudioManager.volumeController
        .getMaxLevel(AudioVolumeType.notification);
    await AudioManager.volumeController
        .setLevel(AudioVolumeType.notification, max);
    int level = await AudioManager.volumeController
        .getLevel(AudioVolumeType.notification);
    expect(level, equals(max));

    await AudioManager.volumeController
        .setLevel(AudioVolumeType.notification, 0);
    level = await AudioManager.volumeController
        .getLevel(AudioVolumeType.notification);
    expect(level, equals(0));
  });

  testWidgets('test ringtone set level', (WidgetTester tester) async {
    final int max = await AudioManager.volumeController
        .getMaxLevel(AudioVolumeType.ringtone);
    await AudioManager.volumeController.setLevel(AudioVolumeType.ringtone, max);
    int level =
        await AudioManager.volumeController.getLevel(AudioVolumeType.ringtone);
    expect(level, equals(max));

    await AudioManager.volumeController.setLevel(AudioVolumeType.ringtone, 0);
    level =
        await AudioManager.volumeController.getLevel(AudioVolumeType.ringtone);
    expect(level, equals(0));
  });

  testWidgets('test system set level', (WidgetTester tester) async {
    final int max =
        await AudioManager.volumeController.getMaxLevel(AudioVolumeType.system);
    await AudioManager.volumeController.setLevel(AudioVolumeType.system, max);
    int level =
        await AudioManager.volumeController.getLevel(AudioVolumeType.system);
    expect(level, equals(max));

    await AudioManager.volumeController.setLevel(AudioVolumeType.system, 0);
    level =
        await AudioManager.volumeController.getLevel(AudioVolumeType.system);
    expect(level, equals(0));
  });

  testWidgets('test voice set level', (WidgetTester tester) async {
    final int max =
        await AudioManager.volumeController.getMaxLevel(AudioVolumeType.voice);
    await AudioManager.volumeController.setLevel(AudioVolumeType.voice, max);
    int level =
        await AudioManager.volumeController.getLevel(AudioVolumeType.voice);
    expect(level, equals(max));

    await AudioManager.volumeController.setLevel(AudioVolumeType.voice, 0);
    level = await AudioManager.volumeController.getLevel(AudioVolumeType.voice);
    expect(level, equals(0));
  });

  testWidgets('test voip set level', (WidgetTester tester) async {
    final int max =
        await AudioManager.volumeController.getMaxLevel(AudioVolumeType.voip);
    await AudioManager.volumeController.setLevel(AudioVolumeType.voip, max);
    int level =
        await AudioManager.volumeController.getLevel(AudioVolumeType.voip);
    expect(level, equals(max));

    await AudioManager.volumeController.setLevel(AudioVolumeType.voip, 0);
    level = await AudioManager.volumeController.getLevel(AudioVolumeType.voip);
    expect(level, equals(0));
  });
}
