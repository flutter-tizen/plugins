import 'dart:async';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

import 'package:tizen_application_manager/tizen_application_manager.dart';
import 'package:tizen_application_manager/application_running_context.dart';

class CurrentAppScreen extends StatefulWidget {
  const CurrentAppScreen({Key? key}) : super(key: key);

  @override
  _CurrentAppScreenState createState() => _CurrentAppScreenState();
}

class _CurrentAppScreenState extends State<CurrentAppScreen> {
  String _appId = 'Unknown';

  ApplicationInfo _appInfo = ApplicationInfo(
    appId: '',
    packageId: '',
    label: '',
    applicationType: '',
    iconPath: '',
    executablePath: '',
    sharedResourcePath: '',
    isNoDisplay: false,
    metaData: <String, String>{},
  );

  late ApplicationRunningContext _currentAppContext = ApplicationRunningContext(
      applicationId: 'com.example.tizen_application_manager_example');

  @override
  void initState() {
    super.initState();
    _initApplicationsInfo();
  }

  // Platform messages are asynchronous, so we initialize in an async method.
  Future<void> _initApplicationsInfo() async {
    String appId;
    ApplicationInfo? appInfo;
    ApplicationRunningContext? currentAppContext;

    // Platform messages may fail, so we use a try/catch PlatformException.
    try {
      appId = await TizenApplicationManager.currentAppId;
      appInfo = await TizenApplicationManager.getApplicationInfo(appId);
      currentAppContext = ApplicationRunningContext(applicationId: appId);
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
          _infoTile('Application type', _appInfo.applicationType),
          _infoTile('Execuatable path', _appInfo.executablePath),
          _infoTile('Shared res path', _appInfo.sharedResourcePath),
          _infoTile('App meta data', _appInfo.metaData.toString()),
          _infoTile(
              'App is terminated', _currentAppContext.isTerminated.toString()),
          _infoTile('process id', _currentAppContext.processId.toString()),
          _infoTile('state', _currentAppContext.appState.toString()),
        ],
      ),
    );
  }
}
