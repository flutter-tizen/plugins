// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:ffi';

import 'package:ffi/ffi.dart';
import 'package:flutter/foundation.dart';

typedef _DlogNative = Void Function(
    Int32, Pointer<Utf8>, Pointer<Utf8>, Pointer<Utf8>);
typedef _Dlog = void Function(int, Pointer<Utf8>, Pointer<Utf8>, Pointer<Utf8>);

/// Provides the ability to use Tizen dlog logging service.
/// More info: https://developer.tizen.org/dev-guide/training/native-app/en/wearable/lesson_13/index.html
class Log {
  Log._();

  static final DynamicLibrary _library = DynamicLibrary.open('liblog.so');
  static final _Dlog _dlogPrint =
      _library.lookup<NativeFunction<_DlogNative>>('dlog_print').asFunction();
  static final RegExp _stackTraceRegExp =
      RegExp(r'^#(\d+)\s+(.+)\((.+):(\d+):(\d+)\)$', multiLine: true);

  /// Indicates whether debug mode is enabled. If so, logs contain
  /// a file name, function name and line number. To enable debug mode use
  /// --dart-define=DEBUG_MODE=debug flag in build time:
  ///
  /// flutter-tizen run --dart-define=DEBUG_MODE=debug
  static bool get isDebugEnabled =>
      const String.fromEnvironment('DEBUG_MODE').toUpperCase() == 'DEBUG';

  /// Sends log with VERBOSE priority and tag. Verbose log messages are
  /// supposed to provide detailed information for development.
  ///
  /// [file], [func] and [line] parameters are set automatically if debug mode
  /// is enabled, but they can be explicitly overridden.
  static void verbose(String tag, String message,
      {String? file, String? func, int? line}) {
    _log(_LogPriority.verbose, tag, message,
        file: file, func: func, line: line);
  }

  /// Sends log with DEBUG priority and tag. Debug log messages are supposed
  /// to provide useful information for development.
  ///
  /// [file], [func] and [line] parameters are set automatically if debug mode
  /// is enabled, but they can be explicitly overridden.
  static void debug(String tag, String message,
      {String? file, String? func, int? line}) {
    _log(_LogPriority.debug, tag, message, file: file, func: func, line: line);
  }

  /// Sends log with INFO priority and tag. Info log messages are
  /// for administration, typically used to report progress of the application.
  ///
  /// [file], [func] and [line] parameters are set automatically if debug mode
  /// is enabled, but they can be explicitly overridden.
  static void info(String tag, String message,
      {String? file, String? func, int? line}) {
    _log(_LogPriority.info, tag, message, file: file, func: func, line: line);
  }

  /// Sends log with WARN priority and tag. Warn log messages are supposed to
  /// indicate problems that the program can tolerate, but should be resolved
  /// whenever possible.
  ///
  /// [file], [func] and [line] parameters are set automatically if debug mode
  /// is enabled, but they can be explicitly overridden.
  static void warn(String tag, String message,
      {String? file, String? func, int? line}) {
    _log(_LogPriority.warn, tag, message, file: file, func: func, line: line);
  }

  /// Sends log with ERROR priority and tag. Error log messages are supposed
  /// to indicate problems that disturb the normal workflow of the application,
  /// such as functional or performance limitations.
  ///
  /// [file], [func] and [line] parameters are set automatically if debug mode
  /// is enabled, but they can be explicitly overridden.
  static void error(String tag, String message,
      {String? file, String? func, int? line}) {
    _log(_LogPriority.error, tag, message, file: file, func: func, line: line);
  }

  /// Sends log with FATAL priority and tag. Fatal log messages are supposed
  /// to indicate problems that entirely block the normal workflow
  /// of the application.
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
        final _StackFrame? frame = _getStackFrameAt(3);
        if (frame != null) {
          file ??= frame.file;
          func ??= frame.function;
          line ??= frame.line;
        }
      }
      message = '${file ?? "-"}: ${func ?? "-"}(${line ?? "-"}) > $message';
    }
    final Pointer<Utf8> tagPtr = tag.toNativeUtf8();
    final Pointer<Utf8> formatPtr = '%s'.toNativeUtf8();
    final Pointer<Utf8> messagePtr = message.toNativeUtf8();
    _dlogPrint(priority.value, tagPtr, formatPtr, messagePtr);
    malloc.free(tagPtr);
    malloc.free(formatPtr);
    malloc.free(messagePtr);
  }

  static _StackFrame? _getStackFrameAt(int index) {
    try {
      final Iterable<RegExpMatch> matches =
          _stackTraceRegExp.allMatches(StackTrace.current.toString());
      for (final RegExpMatch match in matches) {
        final List<String?> groups = match.groups(<int>[1, 2, 3, 4, 5]);
        if (groups.any((String? group) => group == null) == false) {
          final int frameIndex = int.parse(groups[0]!);
          if (frameIndex == index) {
            return _StackFrame(
              frameIndex,
              groups[1]!.trim(),
              groups[2]!.trim(),
              int.parse(groups[3]!),
              int.parse(groups[4]!),
            );
          }
        }
      }
    } catch (error) {
      debugPrint('[Log] Error parsing stack trace: $error');
    }
    return null;
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
