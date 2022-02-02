# tizen_log

[![pub package](https://img.shields.io/pub/v/tizen_log.svg)](https://pub.dev/packages/tizen_log)

A flutter plugin which provides the ability to use [Tizen dlog logging service](https://docs.tizen.org/application/native/guides/error/system-logs/).

## Getting Started

To use this package, add `tizen_log` as a dependency in your `pubspec.yaml` file.

```yaml
dependencies:
  tizen_log: ^0.1.0
```

### Logging

Simple logs:

```dart
const LOG_TAG = 'TEST';
Log.verbose(LOG_TAG, 'verbose message');
Log.debug(LOG_TAG, 'debug message');
Log.info(LOG_TAG, 'info message');
Log.warn(LOG_TAG, 'warn message');
Log.error(LOG_TAG, 'error message');
Log.fatal(LOG_TAG, 'fatal message');
```

### Customizing the logs

To add file name, function name and line number to logs, use `--dart-define=DEBUG_MODE=debug` flag:

```console
$ flutter-tizen run --dart-define=DEBUG_MODE=debug
```

To override file name, function name or line number in logs, use additional parameters in function calls:

```dart
const LOG_TAG = 'TEST';
Log.warn(LOG_TAG, 'warn message', file: 'main');
Log.error(LOG_TAG, 'error message', func: 'constructor');
Log.fatal(LOG_TAG, 'fatal message', line: 1111);
Log.fatal(LOG_TAG, 'fatal message', file: 'main', line: 1234);
```

### Viewing logs

To view logs use the following command:

```console
$ sdb dlog LOG_TAG
```
