// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:args/command_runner.dart';
import 'package:file/file.dart';
import 'package:file/memory.dart';
import 'package:flutter_plugin_tools/src/common/core.dart';
import 'package:flutter_tizen_plugin_tools/src/integration_test_command.dart';
import 'package:test/test.dart';

import 'utils.dart';

void main() {
  late FileSystem fileSystem;
  late Directory packagesDir;
  late IntegrationTestCommand command;
  late CommandRunner<void> commandRunner;

  setUp(() {
    fileSystem = MemoryFileSystem();
    packagesDir = fileSystem.currentDirectory.childDirectory(
      'packages',
    )..createSync(recursive: true);
    command = IntegrationTestCommand(
      packagesDir,
      fileSystem: fileSystem,
    );
    commandRunner = CommandRunner<void>(
      'integration-test runner',
      'Test for integration-test',
    )..addCommand(command);

    // TODO(HakkyuKim): Relocate this step to setup functions for relevant groups
    // after subclassing [PackageLoopingCommand].
    packagesDir
        .childDirectory('a')
        .childFile('pubspec.yaml')
        .createSync(recursive: true);
  });

  group('conflicting options: ', () {
    test('does not allow --profiles with --recipe', () async {
      Error? commandError;
      final List<String> output = await runCapturingPrint(
        commandRunner,
        <String>[
          command.name,
          '--profiles',
          'tv-6.0',
          '--recipe',
          'recipe.yaml',
        ],
        errorHandler: (Error e) {
          commandError = e;
        },
      );

      expect(commandError, isA<ToolExit>());
      expect(
        output,
        containsAllInOrder(
          <Matcher>[contains('Cannot specify both --profiles and --recipe.')],
        ),
      );
    });

    test('--generate-emulators requires either --profiles or --recipe',
        () async {
      Error? commandError;
      final List<String> output = await runCapturingPrint(
        commandRunner,
        <String>[
          command.name,
          '--generate-emulators',
        ],
        errorHandler: (Error e) {
          commandError = e;
        },
      );

      expect(commandError, isA<ToolExit>());
      expect(
        output,
        containsAllInOrder(<Matcher>[
          contains('Either --profiles or --recipe must be '
              'provided with --generate-emulators.')
        ]),
      );
    });
  });

  group('parsing recipe file: ', () {
    test('handle when recipe file doesn\'t exist', () async {
      Error? commandError;
      final List<String> output = await runCapturingPrint(
        commandRunner,
        <String>[
          command.name,
          '--recipe',
          'recipe.yaml',
        ],
        errorHandler: (Error e) {
          commandError = e;
        },
      );

      expect(commandError, isA<ToolExit>());
      expect(
        output,
        containsAllInOrder(
          <Matcher>[contains('Recipe file doesn\'t exist')],
        ),
      );
    });

    test('handle invalid recipe files', () async {
      final File recipeFile = packagesDir.childFile('recipe.yaml')
        ..createSync(recursive: true);
      recipeFile.writeAsStringSync('''
plugins: 
  a: [
''');

      Error? commandError;
      final List<String> output = await runCapturingPrint(
        commandRunner,
        <String>[
          command.name,
          '--recipe',
          recipeFile.path,
        ],
        errorHandler: (Error e) {
          commandError = e;
        },
      );

      expect(commandError, isA<ToolExit>());
      expect(
        output,
        containsAllInOrder(
          <Matcher>[contains('Invalid YAML file.')],
        ),
      );
    });

    test('handle empty list packages as excluded', () async {
      final File recipeFile = packagesDir.childFile('recipe.yaml')
        ..createSync(recursive: true);
      recipeFile.writeAsStringSync('''
plugins: 
  a: []
''');

      final List<String> output = await runCapturingPrint(
        commandRunner,
        <String>[
          command.name,
          '--recipe',
          recipeFile.path,
        ],
      );

      expect(
        output,
        containsAllInOrder(
          <Matcher>[contains('Excluded by recipe: a')],
        ),
      );
    });
  });
}
