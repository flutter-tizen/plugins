// Copyright 2020 Samsung Electronics Co., Ltd. All rights reserved.
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: public_member_api_docs

import 'dart:async';

import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:device_info_plus_tizen/device_info_plus_tizen.dart';

void main() {
  runZonedGuarded(() {
    runApp(MyApp());
  }, (dynamic error, dynamic stack) {
    print(error);
    print(stack);
  });
}

class MyApp extends StatefulWidget {
  @override
  _MyAppState createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  static final DeviceInfoPluginTizen deviceInfoPlugin = DeviceInfoPluginTizen();
  Map<String, dynamic> _deviceData = <String, dynamic>{};

  @override
  void initState() {
    super.initState();
    initPlatformState();
  }

  Future<void> initPlatformState() async {
    Map<String, dynamic> deviceData = <String, dynamic>{};

    try {
      deviceData = _readTizenDeviceInfo(await deviceInfoPlugin.tizenInfo);
    } on PlatformException {
      deviceData = <String, dynamic>{
        'Error:': 'Failed to get platform version.'
      };
    }

    if (!mounted) {
      return;
    }

    setState(() {
      _deviceData = deviceData;
    });
  }

  Map<String, dynamic> _readTizenDeviceInfo(TizenDeviceInfo data) {
    return <String, dynamic>{
      'modelName': data.modelName,
      'cpuArch': data.cpuArch,
      'nativeApiVersion': data.nativeApiVersion,
      'platformVersion': data.platformVersion,
      'webApiVersion': data.webApiVersion,
      'buildDate': data.buildDate,
      'buildId': data.buildId,
      'buildString': data.buildString,
      'buildTime': data.buildTime,
      'buildType': data.buildType,
      'buildVariant': data.buildVariant,
      'buildRelease': data.buildRelease,
      'deviceType': data.deviceType,
      'manufacturer': data.manufacturer,
    };
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(title: const Text('Tizen Device Info')),
        body: ListView(
          children: _deviceData.keys.map((String property) {
            return Row(
              children: <Widget>[
                Container(
                  padding: const EdgeInsets.all(10.0),
                  child: Text(
                    property,
                    style: const TextStyle(
                      fontWeight: FontWeight.bold,
                    ),
                  ),
                ),
                Expanded(
                    child: Container(
                  padding: const EdgeInsets.fromLTRB(0.0, 10.0, 0.0, 10.0),
                  child: Text(
                    '${_deviceData[property]}',
                    maxLines: 10,
                    overflow: TextOverflow.ellipsis,
                  ),
                )),
              ],
            );
          }).toList(),
        ),
      ),
    );
  }
}
