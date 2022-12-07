// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:io' as io;

import 'package:args/args.dart';
import 'package:file/file.dart';
import 'package:file/local.dart';
import 'package:flutter_plugin_tools/src/common/core.dart';
import 'package:flutter_plugin_tools/src/common/package_looping_command.dart';
import 'package:flutter_plugin_tools/src/common/plugin_command.dart';
import 'package:flutter_plugin_tools/src/common/repository_package.dart';
import 'package:yaml/yaml.dart';

import 'device.dart';
import 'recipe.dart';
import 'tizen_sdk.dart';

/// A command that runs integration test of plugin examples.
class IntegrationTestCommand extends PackageLoopingCommand {
  /// Creates an instance of the integration-test command, which runs
  /// integration tests of each plugin example on physical or emulator devices.
  IntegrationTestCommand(
    super.packagesDir, {
    FileSystem fileSystem = const LocalFileSystem(),
  }) : _fileSystem = fileSystem {
    argParser.addFlag(
      _generateEmulatorsArg,
      help: 'Create and destroy emulators during test.\n'
          'Must provide either $_profilesArg or $_recipeArg option to specify '
          'which platforms to create.',
    );
    argParser.addMultiOption(
      _profilesArg,
      help: 'Profiles to run integration test on. (ex: wearable-5.5)\n'
          'The command will select all matching profile devices for each '
          'plugin. If you wish to set profiles individually for each plugin, '
          'use --$_recipeArg instead.',
      valueHelp: 'device_type-platform_version',
    );
    argParser.addOption(
      _recipeArg,
      help: 'The recipe file path. A recipe refers to a YAML file that defines '
          'a list of profiles to test for each plugin.\n'
          'Pass this file if you want to select specific profiles to test '
          'for different plugins. Every package listed in the recipe file '
          'will be recognized by the tool (same as $_packagesArg option) '
          'and those that specify an empty list will be explicitly excluded '
          '(same as $_excludeArg option). If --$_recipeArg is used, '
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
      defaultsTo: _timeout.inSeconds.toString(),
    );
  }

  /// Copied from [PluginCommand].
  static const String _excludeArg = 'exclude';

  /// Copied from [PluginCommand].
  static const String _packagesArg = 'packages';

  static const String _generateEmulatorsArg = 'generate-emulators';
  static const String _profilesArg = 'profiles';
  static const String _recipeArg = 'recipe';
  static const String _timeoutArg = 'timeout';

  final FileSystem _fileSystem;

  // Lazily initialize [_tizenSdk] so that other commands can run without having
  // Tizen SDK installed on the system.
  late final TizenSdk _tizenSdk = TizenSdk.locateTizenSdk();

  Duration _timeout = const Duration(seconds: 300);

  Recipe? _recipe;

  @override
  String get description =>
      'Runs integration tests for plugin example apps.\n\n'
      'This command requires "flutter-tizen" to be in your path.';

  @override
  String get name => 'integration-test';

  @override
  Future<void> initializeRun() async {
    final ArgResults args = argResults!;
    if (args.wasParsed(_profilesArg) && args.wasParsed(_recipeArg)) {
      print('Cannot specify both --$_profilesArg and --$_recipeArg.');
      throw ToolExit(exitInvalidArguments);
    }

    if (args.wasParsed(_generateEmulatorsArg) &&
        !args.wasParsed(_profilesArg) &&
        !args.wasParsed(_recipeArg)) {
      print('Either --$_profilesArg or --$_recipeArg must be '
          'provided with --$_generateEmulatorsArg.');
      throw ToolExit(exitInvalidArguments);
    }

    final int? timeoutInSeconds = int.tryParse(getStringArg(_timeoutArg));
    if (timeoutInSeconds == null) {
      print('Must specify an integer value for --$_timeoutArg.');
      throw ToolExit(exitCommandFoundErrors);
    }
    _timeout = Duration(seconds: timeoutInSeconds);

    if (args.wasParsed(_recipeArg)) {
      final File recipeFile = _fileSystem.file(getStringArg(_recipeArg));
      if (!recipeFile.existsSync()) {
        print("Recipe file doesn't exist: ${recipeFile.absolute.path}");
        throw ToolExit(exitCommandFoundErrors);
      }
      try {
        final YamlMap yamlMap =
            loadYaml(recipeFile.readAsStringSync()) as YamlMap;
        _recipe = Recipe.fromYaml(yamlMap);
      } on YamlException {
        print('Invalid YAML file.');
        throw ToolExit(exitCommandFoundErrors);
      }
    }

    final io.ProcessResult processResult = await processRunner.run(
      'flutter-tizen',
      <String>['precache', '--tizen'],
    );
    if (processResult.exitCode != 0) {
      print('Cannot cache tizen artifacts used for integration-test.');
      throw ToolExit(exitCommandFoundErrors);
    }
  }

  /// See: [PluginCommand.getTargetPackages].
  @override
  Stream<PackageEnumerationEntry> getTargetPackages({
    bool filterExcluded = true,
  }) async* {
    if (_recipe == null) {
      yield* super.getTargetPackages(filterExcluded: filterExcluded);
    } else {
      final Recipe recipe = _recipe!;
      final List<PackageEnumerationEntry> plugins = await super
          .getTargetPackages(filterExcluded: filterExcluded)
          .toList();

      for (final PackageEnumerationEntry plugin in plugins) {
        final String pluginName = plugin.package.displayName;
        if (!recipe.contains(pluginName)) {
          continue;
        }
        if (!(filterExcluded && plugin.excluded)) {
          yield recipe.isExcluded(pluginName)
              ? PackageEnumerationEntry(plugin.package, excluded: true)
              : plugin;
        }
      }
    }
  }

  @override
  Future<PackageResult> runForPackage(RepositoryPackage package) async {
    List<Profile> profiles = <Profile>[];
    if (argResults!.wasParsed(_profilesArg)) {
      profiles = getStringListArg(_profilesArg)
          .map((String profile) => Profile.fromString(profile))
          .toList();
    } else if (_recipe != null) {
      profiles = _recipe!.getProfiles(package.displayName);
    }

    final List<Device> devices = getBoolArg(_generateEmulatorsArg)
        ? _prepareNewEmulators(profiles)
        : _findConnectedDevices(profiles.isEmpty ? null : profiles);

    if (devices.isEmpty) {
      return PackageResult.fail(<String>['No devices to test.']);
    }

    final List<RepositoryPackage> examples = package.getExamples().toList();
    if (examples.isEmpty) {
      return PackageResult.fail(<String>[
        'Missing example directory (use --exclude if this is intentional).'
      ]);
    }

    io.ProcessResult processResult = await processRunner.run(
      'flutter-tizen',
      <String>['pub', 'get'],
      workingDir: package.directory,
    );
    if (processResult.exitCode != 0) {
      return PackageResult.fail(<String>[
        'Command pub get failed. Make sure the pubspec file in your project is valid.'
      ]);
    }

    // Number of test = examples * profiles
    final List<String> errors = <String>[];
    for (final RepositoryPackage example in examples) {
      if (!example.pubspecFile.existsSync()) {
        errors.add('Missing pubspec file in ${example.path}.');
        continue;
      }

      final Directory integrationTestDir =
          example.directory.childDirectory('integration_test');
      if (!integrationTestDir.existsSync() ||
          integrationTestDir.listSync().isEmpty) {
        errors.add('Missing integration tests in ${example.path} '
            '(use --exclude if this is intentional).');
        continue;
      }

      for (final Device device in devices) {
        final PackageResult packageResult = await device.runIntegrationTest(
          example.directory,
          _timeout,
        );
        if (packageResult.state == RunState.failed) {
          errors.addAll(packageResult.details);
        }
      }
    }

    processResult = await processRunner.run(
      'flutter-tizen',
      <String>['clean'],
      workingDir: package.directory,
    );
    if (processResult.exitCode != 0) {
      logWarning('Failed to clean ${package.displayName} after build.');
    }

    return errors.isEmpty
        ? PackageResult.success()
        : PackageResult.fail(errors);
  }

  List<EmulatorDevice> _prepareNewEmulators(List<Profile> profiles) {
    final List<EmulatorDevice> emulators = <EmulatorDevice>[];
    for (final Profile profile in profiles) {
      emulators.add(
        EmulatorDevice(
          '${profile.toString().replaceAll('.', '_')}-${io.pid}',
          profile,
          tizenSdk: _tizenSdk,
        ),
      );
    }
    return emulators;
  }

  /// Finds all devices connected to host PC.
  ///
  /// If [profiles] is passed, returns devices that match any of those profiles.
  ///
  /// If [profiles] is omitted or `null` is passed, returns all connected devices.
  List<Device> _findConnectedDevices([List<Profile>? profiles]) {
    final List<Device> devices = <Device>[];
    final List<SdbDeviceInfo> deviceInfos = _tizenSdk.sdbDevices();
    for (final SdbDeviceInfo deviceInfo in deviceInfos) {
      final Map<String, String> capability =
          _tizenSdk.sdbCapability(deviceInfo.serial);
      final String? deviceType = capability['profile_name'];
      final String? version = capability['platform_version'];
      final String? cpuArch = capability['cpu_arch'];
      if (deviceType == null || version == null || cpuArch == null) {
        throw Exception(
            'Cannot extract profile, Tizen version, or cpu arch from '
            'target ${deviceInfo.serial}.\n'
            'profile: $deviceType\n'
            'Tizen version: $version\n'
            'cpu arch: $cpuArch');
      }
      final Profile profile = Profile.fromString('$deviceType-$version');
      if (profiles != null && !profiles.contains(profile)) {
        continue;
      }
      final Device device = cpuArch == 'x86'
          ? Device.emulator(
              deviceInfo.name,
              profile,
              tizenSdk: _tizenSdk,
            )
          : Device.physical(
              deviceInfo.name,
              profile,
              tizenSdk: _tizenSdk,
            );
      devices.add(device);
    }
    return devices;
  }
}
