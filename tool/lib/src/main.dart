// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: implementation_imports

import 'dart:io' as io;

import 'package:file/file.dart';
import 'package:file/local.dart';

import 'package:flutter_plugin_tools/src/common/core.dart';
import 'package:flutter_plugin_tools/src/common/process_runner.dart';

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

  const ProcessRunner processRunner = ProcessRunner();
  final Directory sourceTreeRoot = packagesDir.parent;
  final File pythonTool =
      sourceTreeRoot.childDirectory('tools').childFile('run_command.py');

  if (!pythonTool.existsSync()) {
    print('Error: Cannot find ${pythonTool.path}.');
    io.exit(1);
  }

  processRunner.runAndStream(pythonTool.path, args).catchError((Object e) {
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
