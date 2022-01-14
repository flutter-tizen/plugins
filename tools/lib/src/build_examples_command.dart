// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:file/file.dart';
import 'package:flutter_plugin_tools/src/common/package_looping_command.dart';
import 'package:flutter_plugin_tools/src/common/repository_package.dart';

/// A command to build the example applications for packages.
class BuildExamplesCommand extends PackageLoopingCommand {
  /// Creates an instance of the build command.
  BuildExamplesCommand(Directory packagesDir) : super(packagesDir);

  @override
  String get description => 'Builds all example apps.\n\n'
      'This command requires "flutter-tizen" to be in your path.\n\n';

  @override
  String get name => 'build-examples';

  @override
  Future<PackageResult> runForPackage(RepositoryPackage package) async {
    final List<String> errors = <String>[];

    if (await processRunner.runAndStream(
            'flutter-tizen', <String>['pub', 'get'],
            workingDir: package.directory) !=
        0) {
      errors.add('${package.displayName} (pub get failed)');
      return PackageResult.fail(errors);
    }

    bool builtSomething = false;
    for (final RepositoryPackage example in package.getExamples()) {
      final String packageName =
          getRelativePosixPath(example.directory, from: packagesDir);

      builtSomething = true;
      // TODO(HakkyuKim): Support different profiles.
          workingDir: example.directory);
      if (exitCode != 0) {
        errors.add(packageName);
      }
    }
    if (await processRunner.runAndStream('flutter-tizen', <String>['clean'],
            workingDir: package.directory) !=
        0) {
      logWarning('Failed to clean ${package.displayName} after build.');
    }

    if (!builtSomething) {
      errors.add('No examples found');
    }

    return errors.isEmpty
        ? PackageResult.success()
        : PackageResult.fail(errors);
  }
}
