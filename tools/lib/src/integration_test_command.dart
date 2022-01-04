// ignore_for_file: public_member_api_docs, implementation_imports

import 'package:file/file.dart';

import 'package:flutter_plugin_tools/src/common/core.dart';
import 'package:flutter_plugin_tools/src/common/plugin_command.dart';

class IntegrationTestCommand extends PluginCommand {
  IntegrationTestCommand(Directory packagesDir) : super(packagesDir);

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
