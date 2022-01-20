// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:file/file.dart';
import 'package:file/local.dart';
import 'package:flutter_plugin_tools/src/common/core.dart';
import 'package:flutter_plugin_tools/src/common/package_looping_command.dart';
import 'package:flutter_plugin_tools/src/common/plugin_command.dart';
import 'package:flutter_plugin_tools/src/common/repository_package.dart';
import 'package:yaml/yaml.dart';

/// A command that runs integration test of plugin examples.
class IntegrationTestCommand extends PackageLoopingCommand {
  /// Creates an instance of the integration-test command, which runs
  /// integration tests of each plugin example on physical or emulator devices.
  IntegrationTestCommand(
    Directory packagesDir, {
    FileSystem fileSystem = const LocalFileSystem(),
  })  : _fileSystem = fileSystem,
        super(packagesDir) {
    argParser.addFlag(
      _generateEmulatorsArg,
      help: 'Create and destroy emulators during test.\n'
          'Must provide either $_platformsArg or $_recipeArg option to specify '
          'which platforms to create.',
    );
    argParser.addMultiOption(
      _platformsArg,
      help: 'Run integration test on all connected devices that satisfy '
          'profile-version (ex: wearable-5.5, tv-6.0).\n'
          'Selected devices will be used to test all plugins. If you wish to '
          'run different devices for each plugin, use $_recipeArg instead.',
      valueHelp: 'profile-version',
    );
    argParser.addOption(
      _recipeArg,
      help: 'The recipe file path. A recipe refers to a yaml file that defines '
          'a list of target platforms to test for each plugin.\n'
          'Pass this file if you want to select specific target platforms '
          'for different plugins. Every package listed in the recipe file '
          'will be recognized by the tool(same as $_packagesArg option) '
          'and those that specify an empty list will be explicitly excluded'
          '(same as $_excludeArg option). If $_recipeArg is used, '
          '$_packagesArg and $_excludeArg options will be ignored.\n\n'
          'plugins:\n'
          '  a: [wearable-5.5, tv-6.0]\n'
          '  b: [mobile-6.0]\n'
          '  c: [wearable-4.0]\n'
          '  d: [] # explicitly excluded\n',
      valueHelp: 'recipe.yaml',
    );
    argParser.addOption(
      _timeoutArg,
      help: 'Timeout limit of each integration test in seconds.',
      valueHelp: 'seconds',
      defaultsTo: '120',
    );
  }

  /// Copied from [PluginCommand].
  static const String _excludeArg = 'exclude';

  /// Copied from [PluginCommand].
  static const String _packagesArg = 'packages';

  static const String _generateEmulatorsArg = 'generate-emulators';
  static const String _platformsArg = 'platforms';
  static const String _recipeArg = 'recipe';
  static const String _timeoutArg = 'timeout';

  final FileSystem _fileSystem;

  @override
  String get description =>
      'Runs integration tests for plugin example apps.\n\n'
      'This command requires "flutter-tizen" to be in your path.';

  @override
  String get name => 'integration-test';

  // TODO(HakkyuKim): Consider validating command-line arguments in the upper
  // a Command object by subclassing [PackageLoopingCommand].
  @override
  Future<PackageResult> runForPackage(RepositoryPackage package) async {
    if (argResults!.wasParsed(_platformsArg) &&
        argResults!.wasParsed(_recipeArg)) {
      print('Cannot specify both --$_platformsArg and --$_recipeArg.');
      throw ToolExit(exitInvalidArguments);
    }

    if (argResults!.wasParsed(_generateEmulatorsArg) &&
        !argResults!.wasParsed(_platformsArg) &&
        !argResults!.wasParsed(_recipeArg)) {
      print('Either --$_platformsArg or --$_recipeArg must be '
          'provided with --$_generateEmulatorsArg.');
      throw ToolExit(exitInvalidArguments);
    }

    late List<String> platforms;
    if (argResults!.wasParsed(_platformsArg)) {
      platforms = getStringListArg(_platformsArg);
    } else if (argResults!.wasParsed(_recipeArg)) {
      final File recipeFile = _fileSystem.file(getStringArg(_recipeArg));
      if (!recipeFile.existsSync()) {
        print('Recipe file doesn\'t exist: ${recipeFile.absolute.path}');
        throw ToolExit(exitCommandFoundErrors);
      }
      try {
        final YamlMap recipe =
            loadYaml(recipeFile.readAsStringSync()) as YamlMap;
        platforms =
            (recipe['plugins'][package.displayName] as YamlList).cast<String>();
        if (platforms.isEmpty) {
          // TODO(HakkyuKim): Return [PackageResult.exclude()] after subclassing
          // [PackageLoopingCommand].
          return PackageResult.skip(
              'Skipped by recipe: ${package.displayName}.');
        }
      } on YamlException {
        print('Invalid yaml file.');
        throw ToolExit(exitCommandFoundErrors);
      }
    }

    if (getBoolArg(_generateEmulatorsArg)) {
      // TODO(HakkyuKim): Return emulator objects matching [platforms].
    } else {
      // TODO(HakkyuKim): Return all connected targets matching [platforms].
    }

    return PackageResult.success();
  }
}
