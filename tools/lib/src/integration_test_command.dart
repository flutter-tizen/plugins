// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: implementation_imports

import 'package:file/file.dart';
import 'package:flutter_plugin_tools/src/common/core.dart';
import 'package:flutter_plugin_tools/src/common/plugin_command.dart';

/// A command that runs integration test of plugin examples.
class IntegrationTestCommand extends PluginCommand {
  /// Creates an instance of the integration-test command, which runs the
  /// integration test of each plugin example on physical targets or emulators.
  IntegrationTestCommand(Directory packagesDir) : super(packagesDir) {
    argParser.addFlag(
      _generateEmulatorsArg,
      help: 'Create and destroy emulators during test.\n\n'
          'Must provide either $_platformsArg or $_recipeArg option to specify\n'
          'which platforms to create.',
    );
    argParser.addOption(
      _platformsArg,
      help: 'Run integration test on all connected targets that satisfy\n'
          '<device_profile>-<platform_version> (ex: wearable-5.5, tv-6.0).\n\n'
          'The selected targets will be used for all plugins,\n'
          'if you wish to run different targets for each plugin,\n'
          'use the $_recipeArg option instead.',
      valueHelp: '<device_profile>-<platform_version>',
    );
    argParser.addOption(_recipeArg,
        help:
            'The recipe file path. A recipe refers to a yaml file that defines\n'
            'a list of target platforms to test for each plugin.\n\n'
            'Pass this file if you want to select specific target platforms\n'
            'for different plugins. Every package listed in the recipe file\n'
            'will be recognized by the tool (same as $_packagesArg option)\n'
            'and those that specify an empty list will be explicitly excluded\n'
            '(same as $_excludeArg option). If $_recipeArg is used,\n'
            '$_packagesArg and $_excludeArg options will be ignored.\n\n'
            'plugins:\n'
            '  a: [wearable-5.5, tv-6.0]\n'
            '  b: [mobile-6.0]\n'
            '  c: [wearable-4.0]\n'
            '  d: [] # explicitly excluded\n',
        valueHelp: 'recipe.yaml');
    argParser.addOption(
      _timeoutArg,
      help: 'Timeout limit of each integration test in seconds.',
      valueHelp: 'seconds',
      defaultsTo: '120',
    );
  }

  // Copied from PluginCommand.
  static const String _excludeArg = 'exclude';
  static const String _packagesArg = 'packages';

  static const String _generateEmulatorsArg = 'generate-emulators';
  static const String _platformsArg = 'platforms';
  static const String _recipeArg = 'recipe';
  static const String _timeoutArg = 'timeout';

  @override
  String get description =>
      'Runs integration tests for plugin example apps.\n\n'
      'This command requires "flutter-tizen" to be in your path.';

  @override
  String get name => 'integration-test';

  File get _pythonTool => packagesDir.parent
      .childDirectory('tools')
      .childDirectory('tools')
      .childFile('run_command.py');

  @override
  Future<void> run() async {
    if (!_pythonTool.existsSync()) {
      print('Error: Cannot find ${_pythonTool.path}.');
      throw ToolExit(1);
    }

    // (TODO: HakkyuKim) Migrate python tool logic to dart.
    final int exitCode = await processRunner.runAndStream(
        _pythonTool.path, <String>['test', ...argResults!.arguments]);
    if (exitCode != 0) {
      throw ToolExit(exitCode);
    }
  }
}
