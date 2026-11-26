// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:convert';
import 'dart:io' show Process, ProcessResult;

import 'package:args/command_runner.dart';
import 'package:file/file.dart';
import 'package:file/memory.dart';
import 'package:flutter_plugin_tools/src/common/core.dart';
import 'package:flutter_plugin_tools/src/common/process_runner.dart';
import 'package:flutter_tizen_plugin_tools/src/device.dart';
import 'package:flutter_tizen_plugin_tools/src/example_test_command.dart';
import 'package:flutter_tizen_plugin_tools/src/example_test_config.dart';
import 'package:flutter_tizen_plugin_tools/src/example_test_device.dart';
import 'package:flutter_tizen_plugin_tools/src/tizen_sdk.dart';
import 'package:mocktail/mocktail.dart';
import 'package:test/test.dart';

import 'utils.dart';

class _MockProcessRunner extends Mock implements ProcessRunner {}

class _MockProcess extends Mock implements Process {}

class _MockExampleTestDevice extends Mock implements ExampleTestDevice {}

class _MockTizenSdk extends Mock implements TizenSdk {}

void main() {
  test('reads each target version without a global platform version', () {
    final recipe = ExampleTestRecipe.parse('''
plugins:
  webview_flutter_tizen: ["common-11.0", "common-10.1"]
  skipped: []
''');

    expect(recipe.plugins['webview_flutter_tizen'], <ExampleTarget>[
      (profile: 'common', apiVersion: '11.0'),
      (profile: 'common', apiVersion: '10.1'),
    ]);
    expect(recipe.plugins['skipped'], isEmpty);
  });

  test('accepts recipes with a legacy global version', () {
    final recipe = ExampleTestRecipe.parse('''
platform_version: "11.0"
plugins:
  webview_flutter_tizen: ["common-10.1"]
''');
    expect(recipe.plugins['webview_flutter_tizen'], [(profile: 'common', apiVersion: '10.1')]);
  });

  test('rejects an invalid API version or scenarios embedded in the recipe', () {
    expect(
      () => ExampleTestRecipe.parse('plugins: {webview: [common-11.]}'),
      throwsFormatException,
    );
    expect(
      () => ExampleTestRecipe.parse(
        'plugins: {webview: [common-11.0]}\nscenarios: {}',
      ),
      throwsFormatException,
    );
  });

  test('rejects event packages without targets in the recipe', () async {
    final FileSystem fs = MemoryFileSystem();
    final Directory packages = fs.directory('packages')..createSync();
    fs.file('recipe.yaml').writeAsStringSync('''
plugins:
  webview_flutter_tizen: ["common-11.0"]
  skipped: []
''');
    fs.file('events.yaml').writeAsStringSync('''
scenarios:
  webview_flutter:
    example:
      - id: tap_menu
        events: [{tap: {x: 0.5, y: 0.5}}]
  skipped:
    example: []
''');
    final runner = CommandRunner<void>('tools', 'Example test command')
      ..addCommand(ExampleTestCommand(packages));

    Error? error;
    final List<String> output = await runCapturingPrint(
      runner,
      <String>['example-test', '--recipe', 'recipe.yaml', '--events', 'events.yaml'],
      errorHandler: (Error caught) => error = caught,
    );

    expect(
        error, isA<ToolExit>().having((exit) => exit.exitCode, 'exitCode', exitInvalidArguments));
    expect(output.single, contains('No recipe targets for event scenario packages'));
    expect(output.single, contains('webview_flutter, skipped'));
  });

  test('selects running emulators by each target profile and version', () async {
    final TizenSdk sdk = _MockTizenSdk();
    final FileSystem fs = MemoryFileSystem();
    when(() => sdk.sdb).thenReturn(fs.file('/tools/sdb'));
    when(() => sdk.sdbDevices()).thenReturn(const <SdbDeviceInfo>[
      SdbDeviceInfo(serial: 'emulator-1', status: 'device', name: 'common11'),
      SdbDeviceInfo(serial: 'emulator-2', status: 'device', name: 'common10'),
      SdbDeviceInfo(serial: 'emulator-3', status: 'offline', name: 'offline'),
      SdbDeviceInfo(serial: 'physical-1', status: 'device', name: 'physical'),
    ]);
    when(() => sdk.sdbCapability('emulator-1'))
        .thenReturn({'profile_name': 'common', 'platform_version': '11.0', 'cpu_arch': 'x86_64'});
    when(() => sdk.sdbCapability('emulator-2'))
        .thenReturn({'profile_name': 'common', 'platform_version': '10.1', 'cpu_arch': 'x86'});
    when(() => sdk.sdbCapability('physical-1'))
        .thenReturn({'profile_name': 'common', 'platform_version': '11.0', 'cpu_arch': 'aarch64'});

    final devices = ExampleTestDevices(sdk, const ProcessRunner(), createEmulator: false);
    const ExampleTarget target = (profile: 'common', apiVersion: '11.0');
    final ExampleTestConnection match = await devices.forTarget(target);
    expect(match.device.serial, 'emulator-1');
    expect(match.arch, 'x64');
    expect((await devices.forTarget(target)).device, same(match.device));
    expect((await devices.forTarget((profile: 'common', apiVersion: '10.1'))).device.serial,
        'emulator-2');
    verifyNever(() => sdk.sdbCapability('emulator-3'));
    verifyNever(() => sdk.sdbCapability('physical-1'));

    final explicit =
        ExampleTestDevices(sdk, const ProcessRunner(), createEmulator: false, serial: 'physical-1');
    expect((await explicit.forTarget(target)).arch, 'arm64');
  });

  test('creates a common emulator using the exact installed platform version', () async {
    final TizenSdk sdk = _MockTizenSdk();
    final ProcessRunner runner = _MockProcessRunner();
    const emCli = '/tools/em-cli';
    const create = ['create', '-n', 'example-test', '-p', 'tizen-11.0-x86_64'];
    when(() => sdk.emCli).thenReturn(MemoryFileSystem().file(emCli));
    when(() => sdk.sdbDevices()).thenReturn(const <SdbDeviceInfo>[]);
    when(() => runner.run(emCli, <String>['list-platform'], exitOnError: true))
        .thenAnswer((_) async => ProcessResult(0, 0, 'tizen-10.0-x86_64\ntizen-11.0-x86_64\n', ''));
    when(() => runner.runAndStream(emCli, create, exitOnError: true)).thenAnswer((_) async => 0);

    final Profile profile = Profile.fromString('common-11.0');
    expect(profile.toString(), 'common-11.0');
    final emulator = EmulatorDevice('example-test', profile, tizenSdk: sdk, processRunner: runner);
    await emulator.create();

    verify(() => runner.runAndStream(emCli, create, exitOnError: true)).called(1);
  });

  test('loads inputs in file order for a nested example', () {
    final ExampleScenarios examples = parseExampleScenarios('''
scenarios:
  webview_flutter_tizen:
    example/client:
      - id: open_menu
        events:
          - tap: {x: 0.98, y: 0.04}
          - key: Return
''');

    final ExampleStep step = examples['webview_flutter_tizen']!['example/client']!.single;
    expect(step.id, 'open_menu');
    expect(step.events, <Matcher>[
      isA<ExampleTap>().having((ExampleTap tap) => tap.x, 'x', 0.98),
      isA<ExampleKey>().having((ExampleKey key) => key.name, 'name', 'Return'),
    ]);
  });

  test('rejects out-of-bounds tap and conflicting checkpoint IDs', () {
    expect(
      () => parseExampleScenarios('''
scenarios:
  webview_flutter_tizen:
    example:
      - id: tap
        events: [{tap: {x: 1.01, y: 0.5}}]
'''),
      throwsFormatException,
    );
    expect(
      () => parseExampleScenarios('''
scenarios:
  webview_flutter_tizen:
    example:
      - id: startup
        events: [{key: Return}]
'''),
      throwsFormatException,
    );
  });

  test('updates the root manifest API independently of application IDs', () {
    final manifest = ExampleManifest.parse('''
<?xml version="1.0" encoding="utf-8"?>
<manifest xmlns="http://tizen.org/ns/packages" package="org.tizen.myapp" version="1.0.0" api-version="6.0">
  <profile name="common"/>
  <ui-application appid="org.tizen.myapp.ui"/>
</manifest>
''');
    manifest.apiVersion = '11.0';
    final rebuilt = ExampleManifest.parse(manifest.toString());

    expect(rebuilt.apiVersion, '11.0');
    expect(rebuilt.appId, 'org.tizen.myapp.ui');
    expect(rebuilt.tpkName, 'org.tizen.myapp-1.0.0.tpk');
    expect(rebuilt.toString(), contains('<profile name="common"/>'));
  });

  test('reports Dart exceptions only for the example process', () {
    final ExampleLogIssue issue = ExampleLogIssue.parse(<String, dynamic>{
      'pid': 42,
      'msg': '══╡ EXCEPTION CAUGHT BY GESTURE ╞══',
    })!;
    expect(issue.exception, isTrue);
    expect(issue.crash, isFalse);
    expect(issue.concerns('org.tizen.myapp', 42), isTrue);
    expect(issue.concerns('org.tizen.myapp', 43), isFalse);
  });

  test('correlates fatal logs from the crash reporter with the example', () {
    final ExampleLogIssue issue = ExampleLogIssue.parse(<String, dynamic>{
      'pid': 11,
      'tag': 'CRASH_MANAGER',
      'msg': 'crash-manager.c: run(1279) > Creating report for pid 42, pkgid org.tizen.myapp',
    })!;
    expect(issue.exception, isFalse);
    expect(issue.crash, isTrue);
    expect(issue.concerns('unrelated.app', 42), isTrue);
    expect(issue.concerns('org.tizen.myapp', 43), isFalse);
    expect(issue.concerns('org.tizen.myapp', null), isTrue);
    expect(issue.concerns('org.tizen.myapp.other', 142), isFalse);
    expect(
        ExampleLogIssue.parse(<String, dynamic>{'pid': 42, 'msg': 'Recovered normally'}), isNull);
  });

  test('correlates app manager crash reports by the reported PID', () {
    final ExampleLogIssue issue = ExampleLogIssue.parse(<String, dynamic>{
      'pid': 11,
      'tag': 'AMD',
      'msg': 'app_status_manager.cc: OnSigchldEvent(1353) > pid(42) is crashed by signal(6)',
    })!;
    expect(issue.concerns('org.tizen.myapp', 42), isTrue);
    expect(issue.concerns('org.tizen.myapp', 43), isFalse);
  });

  test('matches crash reports by app ID when the target PID is unavailable', () {
    final ExampleLogIssue issue = ExampleLogIssue.parse(<String, dynamic>{
      'pid': 11,
      'msg': 'org.tizen.myapp: fatal signal 11',
    })!;
    expect(issue.concerns('org.tizen.myapp', 42), isTrue);
    expect(issue.concerns('org.tizen.myapp.other', 42), isFalse);
  });

  test('recognizes the WebView signal and ignores normal termination', () {
    expect(
      ExampleLogIssue.parse(<String, dynamic>{'pid': 42, 'msg': 'Received signal 11 SEGV_MAPERR'})
          ?.crash,
      isTrue,
    );
    expect(
      ExampleLogIssue.parse(<String, dynamic>{'pid': 42, 'msg': 'Received signal 15'}),
      isNull,
    );
  });

  group('DLog monitor', () {
    late ExampleTestDevice device;
    late ExampleDeviceLog log;
    late Process process;
    late File file;
    late StreamController<List<int>> stdout;
    late StreamController<List<int>> stderr;

    setUp(() async {
      final ProcessRunner runner = _MockProcessRunner();
      device = _MockExampleTestDevice();
      process = _MockProcess();
      file = MemoryFileSystem().file('dlog.jsonl');
      stdout = StreamController<List<int>>();
      stderr = StreamController<List<int>>();

      when(() => device.runner).thenReturn(runner);
      when(() => device.sdb).thenReturn('sdb');
      when(() => device.serial).thenReturn('device');
      when(() => runner.start('sdb', any())).thenAnswer((_) async => process);
      when(() => process.stdout).thenAnswer((_) => stdout.stream);
      when(() => process.stderr).thenAnswer((_) => stderr.stream);
      when(() => process.exitCode).thenAnswer((_) async => 0);
      when(() => process.kill()).thenAnswer((_) {
        unawaited(stdout.close());
        unawaited(stderr.close());
        return true;
      });

      log = await ExampleDeviceLog.start(device, file);
      addTearDown(log.close);
    });

    test('retries a missing probe and requires a tagged marker from each buffer', () async {
      final attempts = <String, int>{};
      when(() => device.shell(any())).thenAnswer((invocation) async {
        final args = invocation.positionalArguments.single as List<String>;
        expect(args.first, 'dlogsend');
        final String buffer = args[2];
        final int attempt = attempts.update(buffer, (count) => count + 1, ifAbsent: () => 1);
        final String tag = buffer == 'main' && attempt == 1 ? 'UNRELATED' : args[4];
        stdout.add(utf8.encode('${jsonEncode({'pid': 10, 'tag': tag, 'msg': args.last})}\n'));
        return '';
      });

      await log.synchronize();

      expect(attempts, {'main': 2, 'system': 1, 'apps': 1});
    });

    test('drains the final exception and stderr before closing the log', () async {
      const message = 'Unhandled exception: final record';
      stdout.add(utf8.encode(jsonEncode({'pid': 42, 'msg': message})));
      stderr.add(utf8.encode('monitor diagnostic\n'));

      await log.close();
      await log.close();

      final ExampleLogIssue issue = log.issues.single;
      expect(issue.exception, isTrue);
      expect(issue.concerns('org.tizen.example', 42), isTrue);
      expect(file.readAsStringSync(), allOf(contains(message), contains('monitor diagnostic')));
      verify(() => process.kill()).called(1);
    });
  });
}
