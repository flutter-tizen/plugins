// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:convert';
import 'dart:io' as io;

import 'package:file/file.dart';
import 'package:flutter_plugin_tools/src/common/core.dart';
import 'package:flutter_plugin_tools/src/common/file_utils.dart';
import 'package:flutter_plugin_tools/src/common/git_version_finder.dart';
import 'package:flutter_plugin_tools/src/common/package_looping_command.dart';
import 'package:flutter_plugin_tools/src/common/plugin_command.dart';
import 'package:flutter_plugin_tools/src/common/process_runner.dart';
import 'package:flutter_plugin_tools/src/common/pub_version_finder.dart';
import 'package:flutter_plugin_tools/src/common/repository_package.dart';
import 'package:git/git.dart';
import 'package:http/http.dart' as http;
import 'package:path/path.dart' as p;
import 'package:platform/platform.dart';
import 'package:pub_semver/pub_semver.dart';

/// Wraps pub publish with a few niceties used by the flutter-tizen team.
// The code is mostly copied from `PublishPluginCommand` in 
// `flutter_plugin_tools`, parts regarding git tagging are removed as we don't
// tag our releases.
class PublishPluginCommand extends PackageLoopingCommand {
  /// Creates an instance of the publish command.
  PublishPluginCommand(
    Directory packagesDir, {
    ProcessRunner processRunner = const ProcessRunner(),
    Platform platform = const LocalPlatform(),
    io.Stdin? stdinput,
    GitDir? gitDir,
    http.Client? httpClient,
  })  : _pubVersionFinder = PubVersionFinder(
          httpClient: httpClient ?? http.Client(),
        ),
        _stdin = stdinput ?? io.stdin,
        super(
          packagesDir,
          platform: platform,
          processRunner: processRunner,
          gitDir: gitDir,
        ) {
    argParser.addMultiOption(
      _pubFlagsOption,
      help:
          'A list of options that will be forwarded on to pub. Separate multiple flags with commas.',
    );
    argParser.addFlag(
      _allChangedFlag,
      help:
          'Release all packages that contains pubspec changes at the current commit compares to the base-sha.\n'
          'The --packages option is ignored if this is on.',
      defaultsTo: false,
    );
    argParser.addFlag(
      _dryRunFlag,
      help:
          'Skips the real `pub publish` and `git tag` commands and assumes both commands are successful.\n'
          'This does not run `pub publish --dry-run`.\n'
          'If you want to run the command with `pub publish --dry-run`, use `pub-publish-flags=--dry-run`',
      defaultsTo: false,
      negatable: true,
    );
    argParser.addFlag(
      _skipConfirmationFlag,
      help: 'Run the command without asking for Y/N inputs.\n'
          'This command will add a `--force` flag to the `pub publish` command if it is not added with $_pubFlagsOption\n',
      defaultsTo: false,
      negatable: true,
    );
  }

  static const String _pubFlagsOption = 'pub-publish-flags';
  static const String _allChangedFlag = 'all-changed';
  static const String _dryRunFlag = 'dry-run';
  static const String _skipConfirmationFlag = 'skip-confirmation';

  static const String _pubCredentialName = 'PUB_CREDENTIALS';

  @override
  String get name => 'publish-plugin';

  @override
  String get description => 'Attempts to publish the given packages to pub.\n'
      'If running this on CI, an environment variable named $_pubCredentialName must be set to a String that represents the pub credential JSON.\n'
      'WARNING: Do not check in the content of pub credential JSON, it should only come from secure sources.';

  final io.Stdin _stdin;
  StreamSubscription<String>? _stdinSubscription;
  final PubVersionFinder _pubVersionFinder;

  late List<String> _publishFlags;

  @override
  String get successSummaryMessage => 'published';

  @override
  String get failureListHeader =>
      'The following packages had failures during publishing:';

  @override
  Future<void> initializeRun() async {
    _publishFlags = <String>[
      ...getStringListArg(_pubFlagsOption),
      if (getBoolArg(_skipConfirmationFlag)) '--force',
    ];

    if (getBoolArg(_dryRunFlag)) {
      print('=============== DRY RUN ===============');
    }
  }

  @override
  Stream<PackageEnumerationEntry> getPackagesToProcess() async* {
    if (getBoolArg(_allChangedFlag)) {
      final GitVersionFinder gitVersionFinder = await retrieveVersionFinder();
      final String baseSha = await gitVersionFinder.getBaseSha();
      print(
          'Publishing all packages that have changed relative to "$baseSha"\n');
      final List<String> changedPubspecs =
          await gitVersionFinder.getChangedPubSpecs();

      for (final String pubspecPath in changedPubspecs) {
        // git outputs a relative, Posix-style path.
        final File pubspecFile = childFileWithSubcomponents(
            packagesDir.fileSystem.directory((await gitDir).path),
            p.posix.split(pubspecPath));
        yield PackageEnumerationEntry(RepositoryPackage(pubspecFile.parent),
            excluded: false);
      }
    } else {
      yield* getTargetPackages(filterExcluded: false);
    }
  }

  @override
  Future<PackageResult> runForPackage(RepositoryPackage package) async {
    final PackageResult? checkResult = await _checkNeedsRelease(package);
    if (checkResult != null) {
      return checkResult;
    }

    if (!await _checkGitStatus(package)) {
      return PackageResult.fail(<String>['uncommitted changes']);
    }

    if (!await _publish(package)) {
      return PackageResult.fail(<String>['publish failed']);
    }

    print('\nPublished ${package.directory.basename} successfully!');
    return PackageResult.success();
  }

  @override
  Future<void> completeRun() async {
    _pubVersionFinder.httpClient.close();
    await _stdinSubscription?.cancel();
    _stdinSubscription = null;
  }

  Future<PackageResult?> _checkNeedsRelease(RepositoryPackage package) async {
    if (!package.pubspecFile.existsSync()) {
      logWarning('''
The pubspec file for ${package.displayName} does not exist, so no publishing will happen.
Safe to ignore if the package is deleted in this commit.
''');
      return PackageResult.skip('package deleted');
    }

    final Pubspec pubspec = package.parsePubspec();

    if (pubspec.publishTo == 'none') {
      return PackageResult.skip('publish_to: none');
    }

    if (pubspec.version == null) {
      printError(
          'No version found. A package that intentionally has no version should be marked "publish_to: none"');
      return PackageResult.fail(<String>['no version']);
    }

    // Check if the package named `packageName` with `version` has already
    // been published.
    final Version version = pubspec.version!;
    final PubVersionFinderResponse pubVersionFinderResponse =
        await _pubVersionFinder.getPackageVersion(packageName: pubspec.name);
    if (pubVersionFinderResponse.versions.contains(version)) {
      print('${pubspec.name} $version has already been published.');
      return PackageResult.skip('already published');
    }
    return null;
  }

  Future<bool> _checkGitStatus(RepositoryPackage package) async {
    final io.ProcessResult statusResult = await (await gitDir).runCommand(
      <String>[
        'status',
        '--porcelain',
        '--ignored',
        package.directory.absolute.path
      ],
      throwOnError: false,
    );
    if (statusResult.exitCode != 0) {
      return false;
    }

    final String statusOutput = statusResult.stdout as String;
    if (statusOutput.isNotEmpty) {
      printError(
          "There are files in the package directory that haven't been saved in git. Refusing to publish these files:\n\n"
          '$statusOutput\n'
          'If the directory should be clean, you can run `git clean -xdf && git reset --hard HEAD` to wipe all local changes.');
    }
    return statusOutput.isEmpty;
  }

  Future<bool> _publish(RepositoryPackage package) async {
    print('Publishing...');
    print('Running `pub publish ${_publishFlags.join(' ')}` in '
        '${package.directory.absolute.path}...\n');
    if (getBoolArg(_dryRunFlag)) {
      return true;
    }

    if (_publishFlags.contains('--force')) {
      _ensureValidPubCredential();
    }

    final io.Process publish = await processRunner.start(
        flutterCommand, <String>['pub', 'publish', ..._publishFlags],
        workingDirectory: package.directory);
    publish.stdout.transform(utf8.decoder).listen((String data) => print(data));
    publish.stderr.transform(utf8.decoder).listen((String data) => print(data));
    _stdinSubscription ??= _stdin
        .transform(utf8.decoder)
        .listen((String data) => publish.stdin.writeln(data));
    final int result = await publish.exitCode;
    if (result != 0) {
      printError('Publishing ${package.directory.basename} failed.');
      return false;
    }

    print('Package published!');
    return true;
  }

  void _ensureValidPubCredential() {
    final String credentialsPath = _credentialsPath;
    final File credentialFile = packagesDir.fileSystem.file(credentialsPath);
    if (credentialFile.existsSync() &&
        credentialFile.readAsStringSync().isNotEmpty) {
      return;
    }
    final String? credential = io.Platform.environment[_pubCredentialName];
    if (credential == null) {
      printError('''
No pub credential available. Please check if `$credentialsPath` is valid.
If running this command on CI, you can set the pub credential content in the $_pubCredentialName environment variable.
''');
      throw ToolExit(1);
    }
    credentialFile.openSync(mode: FileMode.writeOnlyAppend)
      ..writeStringSync(credential)
      ..closeSync();
  }
}

final String _credentialsPath = () {
  String? cacheDir;
  final String? pubCache = io.Platform.environment['PUB_CACHE'];
  if (pubCache != null) {
    cacheDir = pubCache;
  } else if (io.Platform.isWindows) {
    final String? appData = io.Platform.environment['APPDATA'];
    if (appData == null) {
      printError('"APPDATA" environment variable is not set.');
    } else {
      cacheDir = p.join(appData, 'Pub', 'Cache');
    }
  } else {
    final String? home = io.Platform.environment['HOME'];
    if (home == null) {
      printError('"HOME" environment variable is not set.');
    } else {
      cacheDir = p.join(home, '.pub-cache');
    }
  }

  if (cacheDir == null) {
    printError('Unable to determine pub cache location');
    throw ToolExit(1);
  }

  return p.join(cacheDir, 'credentials.json');
}();
