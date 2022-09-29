// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter_plugin_tools/src/common/package_looping_command.dart';
import 'package:flutter_plugin_tools/src/common/repository_package.dart';

/// A command to build the example applications for packages.
class BuildExamplesCommand extends PackageLoopingCommand {
  /// Creates an instance of the build command.
  BuildExamplesCommand(super.packagesDir);

  @override
  String get description => 'Builds all example apps.\n\n'
      'This command requires "flutter-tizen" to be in your path.';

  @override
  String get name => 'build-examples';

  @override
  Future<PackageResult> runForPackage(RepositoryPackage package) async {
    final List<String> errors = <String>[];
    bool builtSomething = false;
    for (final RepositoryPackage example in package.getExamples()) {
      int exitCode = await processRunner.runAndStream(
        'flutter-tizen',
        <String>['pub', 'get'],
        workingDir: example.directory,
      );
      if (exitCode != 0) {
        errors.add('${example.displayName} (pub get failed)');
        continue;
      }

      builtSomething = true;
      exitCode = await processRunner.runAndStream(
        'flutter-tizen',
        <String>[
          'build',
          'tpk',
          '--device-profile',
          'wearable',
          '-v',
        ],
        workingDir: example.directory,
      );
      if (exitCode != 0) {
        final String packageName = getRelativePosixPath(
          example.directory,
          from: packagesDir,
        );
        errors.add(packageName);
      }
    }

    if (!builtSomething) {
      errors.add('No examples found');
    }

    return errors.isEmpty
        ? PackageResult.success()
        : PackageResult.fail(errors);
  }
}
