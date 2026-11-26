// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:convert';
import 'dart:io' as io;

import 'package:file/file.dart';
import 'package:flutter_plugin_tools/src/common/process_runner.dart';

import 'device.dart';
import 'example_test_config.dart';
import 'tizen_sdk.dart';

/// Collects process output and ends the process if it exceeds [timeout].
Future<io.ProcessResult> runExampleProcess(
  ProcessRunner runner,
  String executable,
  List<String> arguments, {
  Directory? workingDir,
  Duration timeout = const Duration(seconds: 20),
}) async {
  final io.Process process =
      await runner.start(executable, arguments, workingDirectory: workingDir);
  const decoder = Utf8Decoder(allowMalformed: true);
  try {
    final List<Object> results = await Future.wait<Object>(<Future<Object>>[
      process.exitCode,
      process.stdout.transform(decoder).join(),
      process.stderr.transform(decoder).join(),
    ]).timeout(timeout);
    return io.ProcessResult(process.pid, results[0] as int, results[1], results[2]);
  } on TimeoutException {
    process.kill(io.ProcessSignal.sigkill);
    throw TimeoutException('$executable ${arguments.first} timed out', timeout);
  }
}

/// A device and the Flutter build architecture for its example apps.
typedef ExampleTestConnection = ({ExampleTestDevice device, String arch});

/// Selects a device per recipe target and owns any emulators created for the run.
class ExampleTestDevices {
  /// Uses running emulators by default, with an optional explicit SDB serial.
  ExampleTestDevices(this._sdk, this._runner, {required bool createEmulator, String? serial})
      : _createEmulator = createEmulator,
        _serial = serial;

  final TizenSdk _sdk;
  final ProcessRunner _runner;
  final bool _createEmulator;
  final String? _serial;
  final String _runId = '${io.pid}-${DateTime.now().microsecondsSinceEpoch.toRadixString(36)}';
  final _devices = <ExampleTarget, Future<ExampleTestConnection>>{};
  final _emulators = <EmulatorDevice>[];

  /// Reuses the same selection, including a failed creation, for a target.
  Future<ExampleTestConnection> forTarget(ExampleTarget target) =>
      _devices.putIfAbsent(target, () => _select(target));

  Future<ExampleTestConnection> _select(ExampleTarget target) async {
    final targetName = '${target.profile}-${target.apiVersion}';
    final String? serial = _createEmulator ? await _launchEmulator(targetName) : _serial;
    final matches = <String, Map<String, String>>{};
    for (final SdbDeviceInfo info in _sdk.sdbDevices()) {
      if (info.status != 'device' || (serial == null ? !info.isEmulator : info.serial != serial)) {
        continue;
      }
      final Map<String, String> capability = _sdk.sdbCapability(info.serial);
      if (capability['profile_name'] == target.profile &&
          capability['platform_version'] == target.apiVersion) {
        matches[info.serial] = capability;
      }
    }
    if (matches.length != 1) {
      if (serial != null) {
        throw StateError('Device $serial is not connected as $targetName.');
      }
      final help = matches.isEmpty
          ? 'Start one or use --create-emulator.'
          : 'Use --device-id to select an emulator.';
      throw StateError('Expected one running $targetName emulator. Found ${matches.length}. $help');
    }
    final MapEntry<String, Map<String, String>> selected = matches.entries.single;
    final String arch = switch (selected.value['cpu_arch']) {
      'x86_64' => 'x64',
      'x86' => 'x86',
      'aarch64' || 'arm64' => 'arm64',
      'armv7' || 'armv7l' => 'arm',
      final String? value => throw StateError('Unsupported device architecture: $value'),
    };
    return (device: ExampleTestDevice(_sdk.sdb.path, selected.key, _runner), arch: arch);
  }

  Future<String> _launchEmulator(String target) async {
    final emulator = EmulatorDevice(
      'ft-ex-$_runId-${_emulators.length}',
      Profile.fromString(target),
      tizenSdk: _sdk,
      processRunner: _runner,
    );
    if (emulator.exists) {
      throw StateError('Emulator ${emulator.name} already exists.');
    }
    _emulators.add(emulator);
    await emulator.create();
    await emulator.launch();
    // Fresh images can restrict compositor inspection to the root SDB shell.
    if (_sdk.sdbCapability(emulator.serial!)['rootonoff_support'] == 'enabled') {
      await emulator.enableSdbRoot();
    }
    return emulator.serial!;
  }

  /// Closes and deletes the emulators created by this run.
  Future<List<String>> close() async {
    final errors = <String>[];
    for (final EmulatorDevice emulator in _emulators.reversed) {
      try {
        if (emulator.exists) {
          try {
            await emulator.close();
          } finally {
            await emulator.delete();
          }
          if (emulator.exists) {
            throw StateError('The emulator still exists.');
          }
        }
      } on Object catch (error) {
        errors.add('Could not clean up emulator ${emulator.name}: $error');
      }
    }
    return errors;
  }
}

/// SDB operations for a connected Tizen test device.
class ExampleTestDevice {
  /// Uses the SDK's SDB executable to address a single [serial].
  const ExampleTestDevice(this.sdb, this.serial, this.runner);

  /// Path to the SDK's SDB executable.
  final String sdb;

  /// The target's SDB serial.
  final String serial;

  /// Runner used to start host processes.
  final ProcessRunner runner;

  /// Runs an SDB subcommand and returns its output.
  Future<String> command(
    List<String> arguments, {
    Duration timeout = const Duration(seconds: 20),
  }) async {
    final io.ProcessResult result = await runExampleProcess(
      runner,
      sdb,
      <String>['-s', serial, ...arguments],
      timeout: timeout,
    );
    if (result.exitCode != 0) {
      throw StateError('sdb ${arguments.first} failed: ${result.stdout}${result.stderr}');
    }
    return (result.stdout as String).replaceAll('\r', '').trim();
  }

  /// Runs a shell command and checks the remote exit status.
  Future<String> shell(List<String> arguments) async {
    // SDB can report a successful transport even when the remote command failed.
    final String remote =
        arguments.map((String arg) => "'${arg.replaceAll("'", "'\"'\"'")}'").join(' ');
    final String output = await command(<String>[
      'shell',
      '$remote; printf "\\n__example_status=%s\\n" "\$?"',
    ]);
    final RegExpMatch? result = RegExp(r'(?:^|\n)__example_status=(\d+)$').firstMatch(output);
    if (result == null || result[1] != '0') {
      throw StateError('sdb shell ${arguments.first} failed: $output');
    }
    return output.substring(0, result.start).trimRight();
  }

  /// Gets the current PID for [appId], or null when it has stopped.
  Future<int?> runningPid(String appId) async {
    final String output = await shell(<String>['app_launcher', '-S']);
    if (!output.contains('appId (PID)')) {
      throw StateError('Unable to read running applications: $output');
    }
    final RegExpMatch? match =
        RegExp('^\\s*${RegExp.escape(appId)}\\s+\\((\\d+)\\)\\s*\$', multiLine: true)
            .firstMatch(output);
    return match == null ? null : int.parse(match[1]!);
  }

  /// Launches the installed app and returns the PID from the launcher.
  Future<int> launch(String appId) async {
    final String output = await shell(<String>['app_launcher', '-e', appId]);
    final RegExpMatch? match = RegExp(r'successfully launched pid\s*=\s*(\d+)').firstMatch(output);
    if (match == null) {
      throw StateError('Unable to launch $appId: $output');
    }
    return int.parse(match[1]!);
  }

  /// Checks that a window owned by [pid] is visible and focused.
  Future<bool> isForeground(int pid) async {
    final Object? result =
        jsonDecode(await shell(<String>['enlightenment_info', '--json', '-topvwins']));
    if (result is! List<dynamic>) {
      throw const FormatException('Unable to read compositor windows.');
    }
    return result.whereType<Map<String, dynamic>>().any(
          (Map<String, dynamic> window) =>
              window['pid'] == pid && window['visible'] == true && window['focus'] == true,
        );
  }

  /// Pulls a compositor PNG to [destination] and validates its framing.
  Future<void> capture(File destination) async {
    final name = 'example-test-${io.pid}-${DateTime.now().microsecondsSinceEpoch}.png';
    try {
      await shell(<String>['enlightenment_info', '-dump_screen', '-p', '/tmp/', '-n', name]);
      await command(<String>['pull', '/tmp/$name', destination.absolute.path]);
      final List<int> bytes = destination.readAsBytesSync();
      if (bytes.length < 45 ||
          latin1.decode(bytes.sublist(0, 8)) != '\x89PNG\r\n\x1A\n' ||
          latin1.decode(bytes.sublist(bytes.length - 8, bytes.length - 4)) != 'IEND') {
        throw StateError('The compositor returned an incomplete PNG.');
      }
    } finally {
      await shell(<String>['rm', '-f', '/tmp/$name']);
    }
  }

  /// Sends the events in order and releases the input device afterwards.
  Future<void> sendEvents(List<ExampleEvent> events) async {
    String? activeInput;
    try {
      for (final event in events) {
        final input = event is ExampleTap ? 'touch' : 'keyboard';
        if (input != activeInput) {
          if (activeInput != null) {
            await shell(<String>['enlightenment_info', '-deinit_device']);
          }
          await shell(<String>['enlightenment_info', '-init_device', '--type=$input']);
          activeInput = input;
        }
        final List<String> trigger;
        switch (event) {
          case ExampleKey():
            trigger = <String>['-keygen', '--name=${event.name}'];
          case ExampleTap():
            final String axis = await _tapAxis(event);
            trigger = <String>['-touchgen', '--idx=0', '--axis=$axis'];
        }
        for (final state in <String>['press', 'release']) {
          await shell(<String>['enlightenment_info', ...trigger, '--state=$state']);
          await Future<void>.delayed(const Duration(milliseconds: 100));
        }
      }
    } finally {
      if (activeInput != null) {
        await shell(<String>['enlightenment_info', '-deinit_device']);
      }
    }
  }

  Future<String> _tapAxis(ExampleTap tap) async {
    final Object? data =
        jsonDecode(await shell(<String>['enlightenment_info', '--json', '-screen_info']));
    if (data
        case {
          'outputs': [
            {
              'geometry': {
                'x': final int x,
                'y': final int y,
                'width': final int w,
                'height': final int h
              }
            }
          ]
        }) {
      return '${x + (tap.x * (w - 1)).round()},${y + (tap.y * (h - 1)).round()}';
    }
    throw const FormatException('Tap requires one compositor display with known geometry.');
  }
}

final RegExp _exception = RegExp(
  r'\b(?:unhandled exceptions?|uncaught exceptions?|exceptions? caught by)\b',
  caseSensitive: false,
);
final RegExp _crash = RegExp(
  r'\b(?:SIGSEGV|SIGABRT|SIGBUS|SIGILL|SIGFPE|segmentation fault|core dumped|fatal signal'
  r'|received signal (?:4|6|7|8|11))\b',
  caseSensitive: false,
);
final RegExp _reportedPid = RegExp(r'\bpid[\s:=(]+(\d+)\b', caseSensitive: false);

/// An error from DLog and the identity of the affected process, when available.
class ExampleLogIssue {
  const ExampleLogIssue._(this._pid, this.message, {required this.exception, required this.crash});

  /// Recognizes unhandled exceptions and fatal signals in a DLog record.
  static ExampleLogIssue? parse(Map<String, dynamic> log) {
    final Object? message = log['msg'];
    final Object? pid = log['pid'];
    if (message is! String || pid is! int) {
      return null;
    }
    final bool exception = _exception.hasMatch(message);
    final bool crash = _crash.hasMatch(message) ||
        (log['tag'] == 'CRASH_MANAGER' && message.contains('Creating report for pid ')) ||
        (log['tag'] == 'AMD' && message.contains('is crashed by signal('));
    return exception || crash
        ? ExampleLogIssue._(pid, message.trim(), exception: exception, crash: crash)
        : null;
  }

  final int _pid;

  /// The DLog message recorded as evidence.
  final String message;

  /// Whether the message indicates an unhandled exception.
  final bool exception;

  /// Whether the message indicates a crash.
  final bool crash;

  /// Matches direct app logs and crash reports for the observed process.
  bool concerns(String appId, int? appPid) {
    if (appPid == _pid) {
      return true;
    }
    if (!crash) {
      return false;
    }
    final String? reportedPid = _reportedPid.firstMatch(message)?[1];
    if (appPid != null && reportedPid != null) {
      return reportedPid == '$appPid';
    }
    return RegExp(
      '(?<![\\w.])${RegExp.escape(appId)}(?![\\w.])',
    ).hasMatch(message);
  }
}

/// Monitors fresh device logs from before launch through the last screenshot.
class ExampleDeviceLog {
  ExampleDeviceLog._(this._device, this._process, this._output) {
    const decoder = Utf8Decoder(allowMalformed: true);
    _stdout = _process.stdout.transform(decoder).transform(const LineSplitter()).listen(
      _record,
      onError: _streamFailed,
      onDone: () {
        active = false;
        _stdoutDone.complete();
      },
    );
    _stderr = _process.stderr.transform(decoder).listen(
          _output.write,
          onError: _streamFailed,
          onDone: _stderrDone.complete,
        );
  }

  static const _buffers = <String>['main', 'system', 'apps'];
  static const _markerTag = 'FLUTTER_EXAMPLE_TEST';

  /// Starts a fresh DLog stream and copies its output to [logFile].
  static Future<ExampleDeviceLog> start(ExampleTestDevice device, File logFile) async {
    final io.Process process = await device.runner.start(device.sdb, <String>[
      '-s',
      device.serial,
      'shell',
      'dlogutil',
      for (final buffer in _buffers) ...<String>['-b', buffer],
      '--monitor',
      '-u',
      '0',
      '-e',
      '0',
      '-v',
      'json',
    ]);
    return ExampleDeviceLog._(device, process, logFile.openWrite());
  }

  final ExampleTestDevice _device;
  final io.Process _process;
  final io.IOSink _output;
  late final StreamSubscription<String> _stdout;
  late final StreamSubscription<String> _stderr;
  final _stdoutDone = Completer<void>();
  final _stderrDone = Completer<void>();
  final _pendingMarkers = <String>{};
  Completer<void>? _markersReceived;
  Object? _streamError;
  Future<void>? _closing;

  /// Issues found in the current log stream.
  final List<ExampleLogIssue> issues = <ExampleLogIssue>[];

  /// Whether the log stream is open.
  bool active = true;

  /// Confirms that every monitored buffer has delivered its synchronization marker.
  Future<void> synchronize() async {
    const timeout = Duration(seconds: 10);
    final timer = Stopwatch()..start();
    final prefix = 'example-test-${io.pid}-${DateTime.now().microsecondsSinceEpoch}';
    final receipt = Completer<void>();
    _markersReceived = receipt;
    _pendingMarkers.addAll(_buffers.map((buffer) => '$prefix-$buffer'));
    try {
      while (active && timer.elapsed < timeout) {
        await Future.wait(<Future<String>>[
          for (final buffer in _buffers)
            if (_pendingMarkers.contains('$prefix-$buffer'))
              _device
                  .shell(<String>['dlogsend', '-b', buffer, '-t', _markerTag, '$prefix-$buffer']),
        ]);
        // Retry when dlogsend reached the device before the monitor attached.
        await receipt.future.timeout(const Duration(milliseconds: 250), onTimeout: () {});
        if (receipt.isCompleted) {
          return;
        }
      }
      if (!active) {
        throw StateError('DLog monitor stopped before all synchronization markers arrived.');
      }
      throw TimeoutException('DLog synchronization markers did not arrive', timeout);
    } finally {
      _pendingMarkers.clear();
      _markersReceived = null;
    }
  }

  void _record(String line) {
    _output.writeln(line);
    if (!line.startsWith('{')) {
      return;
    }
    try {
      final Object? entry = jsonDecode(line);
      if (entry is Map<String, dynamic> && entry['pid'] is int) {
        if (entry['tag'] == _markerTag &&
            _pendingMarkers.remove(entry['msg']) &&
            _pendingMarkers.isEmpty) {
          _markersReceived!.complete();
        }
        final ExampleLogIssue? issue = ExampleLogIssue.parse(entry);
        if (issue != null) {
          issues.add(issue);
        }
      }
    } on FormatException {
      // Keep malformed lines in the raw log for diagnosis.
    }
  }

  void _streamFailed(Object error) {
    active = false;
    _streamError ??= error;
  }

  /// Stops the monitor and drains its output before closing the raw log.
  Future<void> close() => _closing ??= _close();

  Future<void> _close() async {
    _process.kill();
    try {
      await Future.wait(<Future<void>>[
        _stdoutDone.future,
        _stderrDone.future,
        _process.exitCode,
      ]).timeout(const Duration(seconds: 5));
      if (_streamError case final Object error) {
        throw StateError('DLog stream failed: $error');
      }
    } on TimeoutException {
      _process.kill(io.ProcessSignal.sigkill);
      rethrow;
    } finally {
      try {
        await Future.wait(<Future<void>>[_stdout.cancel(), _stderr.cancel()]);
      } finally {
        await _output.close();
      }
    }
  }
}
