// ignore_for_file: public_member_api_docs, avoid_classes_with_only_static_members

import 'dart:async';

import 'package:flutter/services.dart';

class Foo {
  static const MethodChannel _channel = MethodChannel('foo');

  static Future<String?> get platformVersion async {
    final String? version = await _channel.invokeMethod('getPlatformVersion');
    return version;
  }
}
