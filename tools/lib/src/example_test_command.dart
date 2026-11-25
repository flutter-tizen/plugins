// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:convert';
import 'dart:io' as io;

import 'package:file/file.dart';
import 'package:flutter_plugin_tools/src/common/core.dart';
import 'package:flutter_plugin_tools/src/common/package_command.dart';
import 'package:flutter_plugin_tools/src/common/repository_package.dart';

import 'example_test_config.dart';
import 'example_test_device.dart';
import 'tizen_sdk.dart';

/// Builds, launches, and captures the example apps selected in a recipe.
class ExampleTestCommand extends PackageCommand {
  /// Creates the example test command for the repository's [packagesDir].
  ExampleTestCommand(super.packagesDir, {super.processRunner}) {
    argParser
      ..addOption('recipe', defaultsTo: '.github/recipe_example.yaml', help: 'Example target YAML.')
      ..addOption('events', help: 'Optional, separate scenario YAML with a scenarios map.')
      ..addOption('device-id', help: 'Serial of the connected SDB device to use.')
      ..addOption('output',
          defaultsTo: 'build/example_test', help: 'Parent of the run output directory.');
  }

  @override
  final String name = 'example-test';

  @override
  final String description = 'Launches example apps and saves screenshots, DLog, and JSON results.';

  @override
  Future<void> run() async {
    final FileSystem fs = packagesDir.fileSystem;
    final ExampleTestRecipe recipe;
    final ExampleScenarios scenarios;
    try {
      recipe = ExampleTestRecipe.parse(fs.file(getStringArg('recipe')).readAsStringSync());
      scenarios = argResults!.wasParsed('events')
          ? parseExampleScenarios(fs.file(getStringArg('events')).readAsStringSync())
          : <String, Map<String, List<ExampleStep>>>{};
    } on Object catch (error) {
      print(error);
      throw ToolExit(exitInvalidArguments);
    }

    final packages = <String, RepositoryPackage>{
      await for (final PackageEnumerationEntry entry in getTargetPackages())
        entry.package.parsePubspec().name: entry.package,
    };
    final TizenSdk sdk = TizenSdk.locateTizenSdk(fileSystem: fs, processRunner: processRunner);
    final serial = argResults!['device-id'] as String?;
    final devices = <String, Map<String, String>>{
      for (final SdbDeviceInfo info in sdk.sdbDevices())
        if (info.status == 'device' && (serial == null || serial == info.serial))
          info.serial: sdk.sdbCapability(info.serial),
    };
    final Directory output = fs.directory(getStringArg('output')).absolute
      ..createSync(recursive: true);
    final Directory runDir = output.createTempSync('run-');
    final cases = <Map<String, Object?>>[];
    for (final MapEntry<String, List<ExampleTarget>> entry in recipe.plugins.entries) {
      if (entry.value.isEmpty) {
        continue;
      }
      final String plugin = entry.key;
      final RepositoryPackage? package = packages[plugin];
      final List<RepositoryPackage>? examples = package?.getExamples().toList();
      if (examples == null || examples.isEmpty) {
        cases.add(<String, Object?>{
          'plugin': plugin,
          'status': 'failed',
          'error': examples == null ? 'Package not found or excluded.' : 'No example apps found.',
        });
        continue;
      }
      final Map<String, List<ExampleStep>> configured =
          scenarios[plugin] ?? <String, List<ExampleStep>>{};
      final Set<String> paths =
          examples.map((RepositoryPackage example) => _examplePath(package!, example)).toSet();
      final Set<String> unknown = configured.keys.toSet().difference(paths);
      if (unknown.isNotEmpty) {
        cases.add(<String, Object?>{
          'plugin': plugin,
          'status': 'failed',
          'error': 'Unknown example paths in event file: ${unknown.join(', ')}',
        });
        continue;
      }
      for (final RepositoryPackage example in examples) {
        final String path = _examplePath(package!, example);
        for (final ExampleTarget target in entry.value) {
          final targetName = '${target.profile}-${target.apiVersion}';
          final Directory caseDir = fs
              .directory(fs.path.join(runDir.path, plugin, path, targetName))
            ..createSync(recursive: true);
          final Map<String, Object?> result = await _runCase(
            sdk.sdb.path,
            devices,
            recipe.platformVersion,
            target,
            example.directory,
            configured[path] ?? const <ExampleStep>[],
            caseDir,
          );
          cases.add(<String, Object?>{
            'plugin': plugin,
            'example': path,
            'target': targetName,
            'status': result['status'],
            'result': fs.path.relative(caseDir.childFile('result.json').path, from: runDir.path),
          });
          print(
              '$plugin/$path [$targetName]: ${result['status']}${result['error'] == null ? '' : ' - ${result['error']}'}');
        }
      }
    }
    final bool failed = cases.any((Map<String, Object?> result) => result['status'] == 'failed');
    final File summary = runDir.childFile('summary.json');
    _writeJson(summary, <String, Object?>{
      'status': failed ? 'failed' : 'passed',
      'platform_version': recipe.platformVersion,
      'cases': cases,
    });
    print('Example test results: ${summary.path}');
    if (failed) {
      throw ToolExit(exitCommandFoundErrors);
    }
  }

  String _examplePath(RepositoryPackage package, RepositoryPackage example) =>
      example.directory.fileSystem.path
          .split(example.directory.fileSystem.path.relative(example.path, from: package.path))
          .join('/');

  Future<Map<String, Object?>> _runCase(
    String sdb,
    Map<String, Map<String, String>> devices,
    String platformVersion,
    ExampleTarget target,
    Directory example,
    List<ExampleStep> steps,
    Directory output,
  ) async {
    final checkpoints = <Map<String, Object?>>[];
    final result = <String, Object?>{
      'status': 'failed',
      'stage': 'device',
      'platform_version': platformVersion,
      'profile': target.profile,
      'api_version': target.apiVersion,
      'launch': 'not_attempted',
      'checkpoints': checkpoints,
    };
    try {
      final List<MapEntry<String, Map<String, String>>> matches = devices.entries
          .where((MapEntry<String, Map<String, String>> device) =>
              device.value['profile_name'] == target.profile &&
              device.value['platform_version'] == platformVersion)
          .toList();
      if (matches.length != 1) {
        throw StateError('Expected one connected ${target.profile}-$platformVersion device. '
            'Found ${matches.length}. Use --device-id to select a device.');
      }
      final MapEntry<String, Map<String, String>> selected = matches.single;
      final device = ExampleTestDevice(sdb, selected.key, processRunner);
      final String arch = switch (selected.value['cpu_arch']) {
        'x86_64' => 'x64',
        'x86' => 'x86',
        'aarch64' || 'arm64' => 'arm64',
        'armv7' || 'armv7l' => 'arm',
        final String? value => throw StateError('Unsupported device architecture: $value'),
      };
      result['device_id'] = device.serial;
      final File lockFile = example.fileSystem.systemTempDirectory.childFile(
        'flutter-tizen-example-test-${Uri.encodeComponent(device.serial)}.lock',
      );
      final io.RandomAccessFile deviceLock = await lockFile.open(mode: io.FileMode.append);
      try {
        await deviceLock.lock();
        final Directory staging =
            example.fileSystem.systemTempDirectory.createTempSync('example-test-');
        try {
          result['stage'] = 'build';
          final ({File tpk, ExampleManifest manifest}) built =
              await _build(example, target, arch, staging, output.childFile('build.log'));
          result['app_id'] = built.manifest.appId;
          result['stage'] = 'install';
          await _runApp(device, built.tpk, built.manifest, steps, output, result, checkpoints);
          result['status'] = 'passed';
          result.remove('stage');
        } finally {
          staging.deleteSync(recursive: true);
        }
      } finally {
        await deviceLock.close();
      }
    } on Object catch (error) {
      result['status'] = 'failed';
      result['error'] = error.toString();
    }
    _writeJson(output.childFile('result.json'), result);
    return result;
  }

  Future<({File tpk, ExampleManifest manifest})> _build(
    Directory example,
    ExampleTarget target,
    String arch,
    Directory staging,
    File log,
  ) async {
    final Directory tizen = example.childDirectory('tizen');
    final Directory ui = tizen.childDirectory('ui');
    final Directory service = tizen.childDirectory('service');
    final bool multiApp = ui.existsSync() && service.existsSync();
    final manifests = <File>[
      (multiApp ? ui : tizen).childFile('tizen-manifest.xml'),
      if (multiApp) service.childFile('tizen-manifest.xml'),
    ];
    final File lockFile = example.childDirectory('.dart_tool').childFile('example_test.lock')
      ..createSync(recursive: true);
    final io.RandomAccessFile lock = await lockFile.open(mode: io.FileMode.append);
    try {
      await lock.lock();
      final originals = <File, List<int>>{};
      try {
        final app = ExampleManifest.parse(manifests.first.readAsStringSync());
        final String tpkName = app.tpkName;
        for (final file in manifests) {
          final List<int> original = file.readAsBytesSync();
          final manifest = ExampleManifest.parse(utf8.decode(original));
          originals[file] = original;
          manifest.apiVersion = target.apiVersion;
          file.writeAsStringSync(manifest.toString(), flush: true);
        }
        final io.ProcessResult build = await runExampleProcess(
          processRunner,
          'flutter-tizen',
          <String>[
            'build',
            'tpk',
            '--debug',
            // Send Flutter framework exceptions to the app console and DLog.
            '--dart-define=flutter.inspector.structuredErrors=false',
            '--target-arch',
            arch,
            '--device-profile',
            target.profile,
          ],
          workingDir: example,
          timeout: const Duration(minutes: 10),
        );
        log.writeAsStringSync('${build.stdout}\n${build.stderr}');
        if (build.exitCode != 0) {
          throw StateError('flutter-tizen build tpk failed (${build.exitCode}). See build.log.');
        }
        final Directory buildDir =
            example.childDirectory('build').childDirectory('tizen').childDirectory('tpk');
        final File packaged = buildDir.childDirectory('tpkroot').childFile('tizen-manifest.xml');
        final builtManifest = ExampleManifest.parse(packaged.readAsStringSync());
        if (builtManifest.apiVersion != target.apiVersion ||
            builtManifest.appId != app.appId ||
            builtManifest.packageId != app.packageId) {
          throw StateError(
              'Built manifest does not match the example and API ${target.apiVersion}.');
        }
        return (
          tpk: buildDir.childFile(tpkName).copySync(staging.childFile(tpkName).path),
          manifest: builtManifest,
        );
      } finally {
        for (final MapEntry<File, List<int>> entry in originals.entries) {
          entry.key.writeAsBytesSync(entry.value, flush: true);
        }
      }
    } finally {
      await lock.close();
    }
  }

  Future<void> _runApp(
    ExampleTestDevice device,
    File tpk,
    ExampleManifest manifest,
    List<ExampleStep> steps,
    Directory output,
    Map<String, Object?> result,
    List<Map<String, Object?>> checkpoints,
  ) async {
    final String appId = manifest.appId;
    final String installedApps = await device.shell(<String>['app_launcher', '-l']);
    final bool wasInstalled = installedApps.contains("'$appId'");
    ExampleDeviceLog? logs;
    int? pid;
    final cleanupErrors = <String>[];
    try {
      final String install = await device.command(
        <String>['install', tpk.path],
        timeout: const Duration(minutes: 2),
      );
      output.childFile('install.log').writeAsStringSync(install);
      if (!(await device.shell(<String>['app_launcher', '-l'])).contains("'$appId'") ||
          !RegExp(r'(?:key\[end\]\s+val\[ok\]|install completed|successfully installed)',
                  caseSensitive: false)
              .hasMatch(install)) {
        throw StateError('TPK installation failed. See install.log.');
      }
      if (await device.runningPid(appId) != null) {
        await device.shell(<String>['app_launcher', '-k', appId]);
      }
      result['stage'] = 'launch';
      logs = await ExampleDeviceLog.start(device, output.childFile('dlog.jsonl'));
      result['log'] = 'dlog.jsonl';
      await logs.synchronize();
      result['launch'] = 'failed';
      pid = await device.launch(appId);
      result['launch'] = 'passed';
      result['pid'] = pid;

      result['stage'] = 'runtime';
      await _checkpoint(device, logs, appId, pid, output, checkpoints, 0,
          finishLogging: steps.isEmpty);
      for (var index = 0; index < steps.length; index++) {
        await _checkpoint(device, logs, appId, pid, output, checkpoints, index + 1,
            step: steps[index], finishLogging: index == steps.length - 1);
      }
    } finally {
      if (logs != null) {
        try {
          await logs.close();
        } on Object catch (error) {
          cleanupErrors.add('DLog: $error');
        }
      }
      try {
        if (result['launch'] != 'not_attempted' && await device.runningPid(appId) != null) {
          await device.shell(<String>['app_launcher', '-k', appId]);
        }
        if (!wasInstalled &&
            (await device.shell(<String>['app_launcher', '-l'])).contains("'$appId'")) {
          await device.command(<String>['uninstall', manifest.packageId]);
          if ((await device.shell(<String>['app_launcher', '-l'])).contains("'$appId'")) {
            throw StateError('$appId remains installed.');
          }
        }
      } on Object catch (error) {
        cleanupErrors.add('App cleanup: $error');
      }
      if (cleanupErrors.isNotEmpty) {
        result['cleanup_errors'] = cleanupErrors;
      }
    }
    if (cleanupErrors.isNotEmpty) {
      result['stage'] = 'cleanup';
      throw StateError(cleanupErrors.join('\n'));
    }
  }

  Future<void> _checkpoint(
    ExampleTestDevice device,
    ExampleDeviceLog logs,
    String appId,
    int pid,
    Directory output,
    List<Map<String, Object?>> checkpoints,
    int index, {
    ExampleStep? step,
    required bool finishLogging,
  }) async {
    final String id = step?.id ?? 'startup';
    final checkpoint = <String, Object?>{
      'id': id,
      'status': 'failed',
      'exception': 'unknown',
      'crash': 'unknown',
    };
    checkpoints.add(checkpoint);
    final File screenshot = output.childFile('${index.toString().padLeft(2, '0')}-$id.png');
    var failure = step == null ? 'startup_failed' : 'event_failed';
    try {
      if (step == null) {
        final stopwatch = Stopwatch()..start();
        while (!await device.isForeground(pid)) {
          await _checkHealth(device, logs, appId, pid, checkpoint);
          if (stopwatch.elapsed >= const Duration(seconds: 30)) {
            throw StateError('Timed out waiting for the example in the foreground.');
          }
          await Future<void>.delayed(const Duration(milliseconds: 250));
        }
      } else {
        await _checkHealth(device, logs, appId, pid, checkpoint);
        await device.sendEvents(step.events);
      }
      final renderDelay = Duration(seconds: step == null ? 2 : 1);
      await Future<void>.delayed(renderDelay);
      failure = 'capture_failed';
      await _checkHealth(device, logs, appId, pid, checkpoint);
      await device.capture(screenshot);
      failure = 'health_check_failed';
      await _checkHealth(device, logs, appId, pid, checkpoint, finishLogging: finishLogging);
      checkpoint['screenshot'] = screenshot.basename;
      checkpoint['status'] = 'passed';
    } on Object catch (error) {
      if (!checkpoint.containsKey('failure')) {
        try {
          await _checkHealth(device, logs, appId, pid, checkpoint);
        } on StateError {
          // The checkpoint already contains the observation and its evidence.
        }
      }
      checkpoint['failure'] ??= failure;
      checkpoint['error'] = error.toString();
      if (screenshot.existsSync()) {
        screenshot.deleteSync();
      }
      rethrow;
    }
  }

  Future<void> _checkHealth(
    ExampleTestDevice device,
    ExampleDeviceLog logs,
    String appId,
    int pid,
    Map<String, Object?> checkpoint, {
    bool finishLogging = false,
  }) async {
    if (finishLogging) {
      await logs.synchronize();
    }
    bool? alive;
    try {
      alive = await device.runningPid(appId) == pid;
    } on Object catch (error) {
      checkpoint['observation_error'] = error.toString();
    }
    if (alive == false && logs.active) {
      // The process can disappear before the system crash reporter writes its log.
      await Future<void>.delayed(const Duration(milliseconds: 750));
    }
    final bool loggingAvailable = logs.active;
    if (finishLogging) {
      await logs.close();
    }
    // shortcut: track the launched UI process only, extend if service checks are needed.
    final List<ExampleLogIssue> issues =
        logs.issues.where((ExampleLogIssue issue) => issue.concerns(appId, pid)).toList();
    final bool crashed = issues.any((ExampleLogIssue issue) => issue.crash);
    final bool exception = issues.any((ExampleLogIssue issue) => issue.exception);
    final absence = loggingAvailable && alive != null ? 'not_observed' : 'unknown';
    checkpoint['app_alive'] = alive;
    checkpoint['exception'] = exception ? 'observed' : absence;
    checkpoint['crash'] = crashed ? 'observed' : absence;
    if (issues.isNotEmpty) {
      checkpoint['evidence'] = issues.map((ExampleLogIssue issue) => issue.message).toList();
    }
    final String? failure = switch ((crashed, exception, alive, loggingAvailable)) {
      (true, _, _, _) => 'crash',
      (_, true, _, _) => 'exception',
      (_, _, false, _) => 'unexpected_exit',
      (_, _, true, true) => null,
      _ => 'unknown',
    };
    if (failure != null) {
      checkpoint['failure'] = failure;
      throw StateError('${checkpoint['id']}: $failure');
    }
  }

  void _writeJson(File file, Map<String, Object?> json) {
    file.writeAsStringSync('${const JsonEncoder.withIndent('  ').convert(json)}\n');
  }
}
