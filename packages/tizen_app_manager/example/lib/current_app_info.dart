// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

import 'package:tizen_app_manager/app_manager.dart';

/// The current app's information page widget.
class CurrentAppScreen extends StatefulWidget {
  /// The constructor of the current app's information page widget.
  const CurrentAppScreen({Key? key}) : super(key: key);

  @override
  _CurrentAppScreenState createState() => _CurrentAppScreenState();
}

class _CurrentAppScreenState extends State<CurrentAppScreen> {
  String _appId = 'Unknown';

  AppInfo _appInfo = AppInfo(
    appId: '',
    packageId: '',
    label: '',
    appType: '',
    iconPath: '',
    executablePath: '',
    sharedResourcePath: '',
    isNoDisplay: false,
    metadata: <String, String>{},
  );

  late AppRunningContext _currentAppContext =
      AppRunningContext(appId: 'com.example.tizen_app_manager_example');

  @override
  void initState() {
    super.initState();
    _initApplicationsInfo();
  }

  // Platform messages are asynchronous, so we initialize in an async method.
  Future<void> _initApplicationsInfo() async {
    String appId;
    AppInfo? appInfo;
    AppRunningContext? currentAppContext;

    // Platform messages may fail, so we use a try/catch PlatformException.
    try {
      appId = await AppManager.currentAppId;
      appInfo = await AppManager.getAppInfo(appId);
      currentAppContext = AppRunningContext(appId: appId);
    } on PlatformException {
      appId = 'Fail to get current app ID';
    }

    setState(() {
      _appId = appId;
      if (appInfo != null) {
        _appInfo = appInfo;
      }

      if (currentAppContext != null) {
        _currentAppContext = currentAppContext;
      }
    });
  }

  Widget _infoTile(String title, String subtitle) {
    return ListTile(
      title: Text(title),
      subtitle: Text(subtitle.isNotEmpty ? subtitle : 'Not set'),
    );
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Current application info'),
      ),
      body: ListView(
        children: <Widget>[
          _infoTile('App ID', _appId),
          _infoTile('Package ID', _appInfo.packageId),
          _infoTile('Label', _appInfo.label),
          _infoTile('Application type', _appInfo.appType),
          _infoTile('Execuatable path', _appInfo.executablePath),
          _infoTile('Shared res path', _appInfo.sharedResourcePath ?? ''),
          _infoTile('App meta data', _appInfo.metadata.toString()),
          _infoTile(
              'App is terminated', _currentAppContext.isTerminated.toString()),
          _infoTile('process id', _currentAppContext.processId.toString()),
          _infoTile('state', _currentAppContext.appState.toString()),
        ],
      ),
    );
  }
}
