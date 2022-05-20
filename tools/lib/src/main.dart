// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:io' as io;

import 'package:args/command_runner.dart';
import 'package:file/file.dart';
import 'package:file/local.dart';
import 'package:flutter_plugin_tools/src/common/core.dart';
import 'package:flutter_plugin_tools/src/format_command.dart';
import 'package:flutter_plugin_tools/src/list_command.dart';
import 'package:flutter_tizen_plugin_tools/src/publish_plugin_command.dart';

import 'build_examples_command.dart';
import 'integration_test_command.dart';

void main(List<String> args) {
  const FileSystem fileSystem = LocalFileSystem();

  Directory packagesDir =
      fileSystem.currentDirectory.childDirectory('packages');

  if (!packagesDir.existsSync()) {
    if (fileSystem.currentDirectory.basename == 'packages') {
      packagesDir = fileSystem.currentDirectory;
    } else {
      print('Error: Cannot find a "packages" sub-directory');
      io.exit(1);
    }
  }

  final CommandRunner<void> commandRunner = CommandRunner<void>(
      './tools/tools_runner.sh',
      'Productivity utils for hosting multiple plugins within one repository.')
    ..addCommand(BuildExamplesCommand(packagesDir))
    ..addCommand(FormatCommand(packagesDir))
    ..addCommand(IntegrationTestCommand(packagesDir))
    ..addCommand(ListCommand(packagesDir))
    ..addCommand(PublishPluginCommand(packagesDir));

  commandRunner.run(args).catchError((Object e) {
    final ToolExit toolExit = e as ToolExit;
    int exitCode = toolExit.exitCode;
    // This should never happen; this check is here to guarantee that a ToolExit
    // never accidentally has code 0 thus causing CI to pass.
    if (exitCode == 0) {
      assert(false);
      exitCode = 255;
    }
    io.exit(exitCode);
  }, test: (Object e) => e is ToolExit);
}
