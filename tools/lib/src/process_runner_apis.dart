import 'dart:convert';
import 'dart:io' as io;

import 'package:file/file.dart';
import 'package:flutter_plugin_tools/src/common/core.dart';

import 'package:flutter_plugin_tools/src/common/process_runner.dart';

/// An extension method of [ProcessRunner] to supports synchronous operations.
extension Synchronizable on ProcessRunner {
  /// Runs the [executable] with [args].
  ///
  /// This is synchronous version of [run].
  ///
  /// The current working directory of [executable] can be overridden by
  /// passing [workingDir].
  ///
  /// If [exitOnError] is set to `true`, then this will throw an error if
  /// the [executable] terminates with a non-zero exit code.
  /// Defaults to `false`.
  ///
  /// If [logOnError] is set to `true`, it will print a formatted message about the error.
  /// Defaults to `false`
  ///
  /// Returns the [io.ProcessResult] of the [executable].
  io.ProcessResult runSync(
    String executable,
    List<String> args, {
    Directory? workingDir,
    bool exitOnError = false,
    bool logOnError = false,
    Encoding stdoutEncoding = io.systemEncoding,
    Encoding stderrEncoding = io.systemEncoding,
  }) {
    final io.ProcessResult result = io.Process.runSync(executable, args,
        workingDirectory: workingDir?.path,
        stdoutEncoding: stdoutEncoding,
        stderrEncoding: stderrEncoding);
    if (result.exitCode != 0) {
      if (logOnError) {
        final String error =
            _getErrorString(executable, args, workingDir: workingDir);
        print('$error Stderr:\n${result.stdout}');
      }
      if (exitOnError) {
        throw ToolExit(result.exitCode);
      }
    }
    return result;
  }

  /// Copied from [ProcessRunner._getErrorString].
  String _getErrorString(String executable, List<String> args,
      {Directory? workingDir}) {
    final String workdir = workingDir == null ? '' : ' in ${workingDir.path}';
    return 'ERROR: Unable to execute "$executable ${args.join(' ')}"$workdir.';
  }
}
