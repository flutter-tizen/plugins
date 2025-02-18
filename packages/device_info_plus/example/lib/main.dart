// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: public_member_api_docs

import 'dart:async';
import 'dart:developer' as developer;

import 'package:device_info_plus_tizen/device_info_plus_tizen.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

void main() {
  runZonedGuarded(
    () {
      runApp(const MyApp());
    },
    (dynamic error, dynamic stack) {
      developer.log("Something went wrong!", error: error, stackTrace: stack);
    },
  );
}

class MyApp extends StatefulWidget {
  const MyApp({Key? key}) : super(key: key);

  @override
  State<MyApp> createState() => _MyAppState();
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
    var deviceData = <String, dynamic>{};

    try {
      deviceData = _readTizenDeviceInfo(await deviceInfoPlugin.tizenInfo);
    } on PlatformException {
      deviceData = <String, dynamic>{
        'Error:': 'Failed to get platform version.',
      };
    }

    if (!mounted) return;

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
      'profile': data.profile,
      'buildDate': data.buildDate,
      'buildId': data.buildId,
      'buildString': data.buildString,
      'buildTime': data.buildTime,
      'buildType': data.buildType,
      'buildVariant': data.buildVariant,
      'buildRelease': data.buildRelease,
      'deviceType': data.deviceType,
      'manufacturer': data.manufacturer,
      'platformName': data.platformName,
      'platformProcessor': data.platformProcessor,
      'tizenId': data.tizenId,
    };
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      theme: ThemeData(
        useMaterial3: true,
        colorSchemeSeed: const Color(0x9f4376f8),
      ),
      home: Scaffold(
        appBar: AppBar(title: const Text('Tizen Device Info'), elevation: 4),
        body: ListView(
          children:
              _deviceData.keys.map((String property) {
                return Row(
                  children: <Widget>[
                    Container(
                      padding: const EdgeInsets.all(10),
                      child: Text(
                        property,
                        style: const TextStyle(fontWeight: FontWeight.bold),
                      ),
                    ),
                    Expanded(
                      child: Container(
                        padding: const EdgeInsets.symmetric(vertical: 10),
                        child: Text(
                          '${_deviceData[property]}',
                          maxLines: 10,
                          overflow: TextOverflow.ellipsis,
                        ),
                      ),
                    ),
                  ],
                );
              }).toList(),
        ),
      ),
    );
  }
}
