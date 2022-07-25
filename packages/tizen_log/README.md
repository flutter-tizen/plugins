# tizen_log

[![pub package](https://img.shields.io/pub/v/tizen_log.svg)](https://pub.dev/packages/tizen_log)

A Flutter plugin which provides the ability to use [Tizen dlog logging service](https://docs.tizen.org/application/native/guides/error/system-logs/).

## Getting Started

To use this package, add `tizen_log` as a dependency in your `pubspec.yaml` file.

```yaml
dependencies:
  tizen_log: ^0.1.1
```

### Simple logging

```dart
const logTag = 'TEST';
Log.verbose(logTag, 'verbose message');
Log.debug(logTag, 'debug message');
Log.info(logTag, 'info message');
Log.warn(logTag, 'warn message');
Log.error(logTag, 'error message');
Log.fatal(logTag, 'fatal message');
```

### Customizing logs

To add file name, function name and line number to logs, use `--dart-define=DEBUG_MODE=true` flag:

```sh
$ flutter-tizen run --dart-define=DEBUG_MODE=true
```

To override file name, function name or line number in logs, use additional parameters in function calls:

```dart
const logTag = 'TEST';
Log.warn(logTag, 'warn message', file: 'main');
Log.error(logTag, 'error message', func: 'constructor');
Log.fatal(logTag, 'fatal message', line: 1111);
Log.fatal(logTag, 'fatal message', file: 'main', line: 1234);
```

### Viewing logs

To view logs use the following command:

```sh
$ sdb dlog TEST  # Replace TEST with your log tag.
```

## Supported devices

- Galaxy Watch series (running Tizen 4.0 or later)
