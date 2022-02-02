// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:ffi';
import 'package:ffi/ffi.dart';
import 'package:flutter/foundation.dart';

typedef _DLogNative = Void Function(
    Int32, Pointer<Utf8>, Pointer<Utf8>, Pointer<Utf8>);
typedef _DLog = void Function(int, Pointer<Utf8>, Pointer<Utf8>, Pointer<Utf8>);

/// Provides the ability to use Tizen dlog logging service.
/// More info: https://docs.tizen.org/application/native/guides/error/system-logs/
class Log {
  Log._();

  static final DynamicLibrary _library = DynamicLibrary.open('liblog.so');
  static final _DLog _dlogPrint =
      _library.lookup<NativeFunction<_DLogNative>>('dlog_print').asFunction();
  static final RegExp _stackTraceRegExp =
      RegExp(r'^#(\d+)\s+(.+)\((.+):(\d+):(\d+)\)$', multiLine: true);

  /// For displaying a file name, function name and line number in the logs,
  /// use --dart-define=DEBUG_MODE=debug flag in build time.
  /// flutter-tizen run --dart-define=DEBUG_MODE=debug
  static const String debugMode =
      String.fromEnvironment('DEBUG_MODE', defaultValue: 'no');

  /// Indicates whether debug mode is enabled. If so, logs contain
  /// a file name, function name and line number.
  static bool get isDebugEnabled => debugMode.toUpperCase() == 'DEBUG';

  /// Sends log with VERBOSE priority and tag.
  ///
  /// [file], [func] and [line] parameters are set automatically if debug mode
  /// is enabled, but they can be explicitly overridden.
  static void verbose(String tag, String message,
      {String? file, String? func, int? line}) {
    _log(_LogPriority.verbose, tag, message,
        file: file, func: func, line: line);
  }

  /// Sends log with DEBUG priority and tag.
  ///
  /// [file], [func] and [line] parameters are set automatically if debug mode
  /// is enabled, but they can be explicitly overridden.
  static void debug(String tag, String message,
      {String? file, String? func, int? line}) {
    _log(_LogPriority.debug, tag, message, file: file, func: func, line: line);
  }

  /// Sends log with INFO priority and tag.
  ///
  /// [file], [func] and [line] parameters are set automatically if debug mode
  /// is enabled, but they can be explicitly overridden.
  static void info(String tag, String message,
      {String? file, String? func, int? line}) {
    _log(_LogPriority.info, tag, message, file: file, func: func, line: line);
  }

  /// Sends log with WARN priority and tag.
  ///
  /// [file], [func] and [line] parameters are set automatically if debug mode
  /// is enabled, but they can be explicitly overridden.
  static void warn(String tag, String message,
      {String? file, String? func, int? line}) {
    _log(_LogPriority.warn, tag, message, file: file, func: func, line: line);
  }

  /// Sends log with ERROR priority and tag.
  ///
  /// [file], [func] and [line] parameters are set automatically if debug mode
  /// is enabled, but they can be explicitly overridden.
  static void error(String tag, String message,
      {String? file, String? func, int? line}) {
    _log(_LogPriority.error, tag, message, file: file, func: func, line: line);
  }

  /// Sends log with FATAL priority and tag.
  ///
  /// [file], [func] and [line] parameters are set automatically if debug mode
  /// is enabled, but they can be explicitly overridden.
  static void fatal(String tag, String message,
      {String? file, String? func, int? line}) {
    _log(_LogPriority.fatal, tag, message, file: file, func: func, line: line);
  }

  static void _log(_LogPriority priority, String tag, String message,
      {String? file, String? func, int? line}) {
    if (isDebugEnabled) {
      if (file == null || func == null || line == null) {
        const int frameIndex = 3;
        final _StackFrame frame = _stackTrace().firstWhere(
            (_StackFrame e) => e.index == frameIndex,
            orElse: () => _StackFrame(0, '', '', 0, 0));
        if (frame.index == frameIndex) {
          file ??= frame.file;
          func ??= frame.function;
          line ??= frame.line;
        }
      }
      message = '${file ?? "-"}: ${func ?? "-"}(${line ?? "-"}) > ' + message;
    }
    final Pointer<Utf8> tagPtr = tag.toNativeUtf8();
    final Pointer<Utf8> formatPtr = '%s'.toNativeUtf8();
    final Pointer<Utf8> messagePtr = message.toNativeUtf8();
    _dlogPrint(priority.value, tagPtr, formatPtr, messagePtr);
    malloc.free(tagPtr);
    malloc.free(formatPtr);
    malloc.free(messagePtr);
  }

  static List<_StackFrame> _stackTrace() {
    final List<_StackFrame> frames = <_StackFrame>[];
    try {
      final Iterable<RegExpMatch> matches =
          _stackTraceRegExp.allMatches(StackTrace.current.toString());
      for (final RegExpMatch m in matches) {
        final List<String?> g = m.groups(<int>[1, 2, 3, 4, 5]);
        if (g.any((String? e) => e == null) == false) {
          frames.add(_StackFrame(
            int.parse(g[0]!),
            g[1]!.trim(),
            g[2]!.trim(),
            int.parse(g[3]!),
            int.parse(g[4]!),
          ));
        }
      }
    } catch (error) {
      debugPrint('[Log] Error parsing stack trace: $error');
      return <_StackFrame>[];
    }
    return frames;
  }
}

class _LogPriority {
  const _LogPriority._(this.value);

  final int value;

  static const _LogPriority verbose = _LogPriority._(2);
  static const _LogPriority debug = _LogPriority._(3);
  static const _LogPriority info = _LogPriority._(4);
  static const _LogPriority warn = _LogPriority._(5);
  static const _LogPriority error = _LogPriority._(6);
  static const _LogPriority fatal = _LogPriority._(7);
}

class _StackFrame {
  _StackFrame(this.index, this.function, this.file, this.line, this.column);

  final int index;
  final String function;
  final String file;
  final int line;
  final int column;
}
