// ignore_for_file: public_member_api_docs, implementation_imports

import 'package:file/file.dart';

import 'package:flutter_plugin_tools/src/common/core.dart';
import 'package:flutter_plugin_tools/src/common/plugin_command.dart';

class IntegrationTestCommand extends PluginCommand {
  IntegrationTestCommand(Directory packagesDir) : super(packagesDir) {
    argParser.addFlag(
      _generateEmulatorsArg,
      help: 'Create and destroy emulators during test.\n\n'
          'Must provide either $_platformsArg or $_recipeArg option to specify which '
          'platforms to create.',
    );
    argParser.addOption(
      _platformsArg,
      help: 'Run integration test on all connected targets that satisfy '
          '<device_profile>-<platform_version> (ex: wearable-5.5, tv-6.0).\n\n'
          'The selected targets will be used for all plugins, '
          'if you wish to run different targets for each plugin,\n'
          'use the $_recipeArg option instead.',
      valueHelp: '<device_profile>-<platform_version>',
    );
    argParser.addOption(_recipeArg,
        help:
            'The recipe file path. A recipe refers to a yaml file that defines '
            'a list of target platforms to test for each plugin.\n\n'
            'Pass this file if you want to select specific target platforms for different\n'
            'plugins. Every package listed in the recipe file will be recognized by the tool\n'
            '(same as $_packagesArg option) and those that specify an empty list will be\n'
            'explicitly excluded (same as $_excludeArg option). If $_recipeArg is used,\n'
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

  File get pythonTool => packagesDir.parent
      .childDirectory('tools')
      .childDirectory('tools')
      .childFile('run_command.py');

  @override
  Future<void> run() async {
    if (!pythonTool.existsSync()) {
      print('Error: Cannot find ${pythonTool.path}.');
      throw ToolExit(1);
    }

    // (TODO: HakkyuKim) Migrate python tool logic to dart.
    final int exitCode = await processRunner.runAndStream(
        pythonTool.path, <String>['test', ...argResults!.arguments]);
    if (exitCode != 0) {
      throw ToolExit(exitCode);
    }
  }
}
