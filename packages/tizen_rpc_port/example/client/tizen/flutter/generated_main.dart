//
// Generated file. Do not edit.
//
// @dart = 2.17

// ignore_for_file: avoid_classes_with_only_static_members
// ignore_for_file: avoid_private_typedef_functions
// ignore_for_file: depend_on_referenced_packages
// ignore_for_file: directives_ordering
// ignore_for_file: lines_longer_than_80_chars
// ignore_for_file: unnecessary_cast

import 'package:example_proxy/main.dart' as entrypoint;

@pragma('vm:entry-point')
class _PluginRegistrant {
  @pragma('vm:entry-point')
  static void register() {
  }
}

typedef _UnaryFunction = dynamic Function(List<String> args);
typedef _NullaryFunction = dynamic Function();

@pragma('vm:entry-point')
void main(List<String> args) {
  if (entrypoint.main is _UnaryFunction) {
    (entrypoint.main as _UnaryFunction)(args);
  } else {
    (entrypoint.main as _NullaryFunction)();
  }
}
