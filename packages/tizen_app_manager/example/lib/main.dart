// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: public_member_api_docs

import 'dart:async';

import 'package:device_info_plus_tizen/device_info_plus_tizen.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:tizen_app_control/tizen_app_control.dart';
import 'package:tizen_app_manager/tizen_app_manager.dart';

/// The settings app ID.
const String settingAppId = 'org.tizen.setting';

/// The wearable emulator settings app ID.
const String wearbleSettingAppId = 'org.tizen.watch-setting';

/// The tv emulator volume setting app ID.
const String tvVolumeSettingAppId = 'org.tizen.volume-setting';

void main() {
  runApp(const MyApp());
}

/// The main app widget.
class MyApp extends StatelessWidget {
  const MyApp({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Application manager demo',
      theme: ThemeData(primarySwatch: Colors.blue),
      home: const _MyHomePage(),
    );
  }
}

class _MyHomePage extends StatefulWidget {
  const _MyHomePage({Key? key}) : super(key: key);

  @override
  _MyHomePageState createState() => _MyHomePageState();
}

class _MyHomePageState extends State<_MyHomePage> {
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text('Application manager demo')),
      body: Center(
        child: Column(
          mainAxisSize: MainAxisSize.min,
          children: <Widget>[
            TextButton(
              onPressed: () {
                Navigator.push(
                  context,
                  MaterialPageRoute<Object>(
                      builder: (BuildContext context) =>
                          const _CurrentAppScreen()),
                );
              },
              child: const Text('Current application info'),
            ),
            TextButton(
              onPressed: () {
                Navigator.push(
                  context,
                  MaterialPageRoute<Object>(
                      builder: (BuildContext context) =>
                          const _AppsListScreen()),
                );
              },
              child: const Text('Installed applications list'),
            ),
            TextButton(
              onPressed: () {
                Navigator.push(
                  context,
                  MaterialPageRoute<Object>(
                      builder: (BuildContext context) =>
                          const _AppsEventScreen()),
                );
              },
              child: const Text('Application event listener'),
            ),
          ],
        ),
      ),
    );
  }
}

/// The current application info page widget.
class _CurrentAppScreen extends StatefulWidget {
  const _CurrentAppScreen({Key? key}) : super(key: key);

  @override
  _CurrentAppScreenState createState() => _CurrentAppScreenState();
}

class _CurrentAppScreenState extends State<_CurrentAppScreen> {
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
      AppRunningContext(appId: 'org.tizen.tizen_app_manager_example');

  @override
  void initState() {
    super.initState();

    _initApplicationsInfo();
  }

  Future<void> _initApplicationsInfo() async {
    String appId;
    AppInfo? appInfo;
    AppRunningContext? currentAppContext;

    // Platform messages may fail, so we use a try/catch.
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
      appBar: AppBar(title: const Text('Current application info')),
      body: ListView(
        children: <Widget>[
          _infoTile('App ID', _appId),
          _infoTile('Package ID', _appInfo.packageId),
          _infoTile('Label', _appInfo.label),
          _infoTile('Application type', _appInfo.appType),
          _infoTile('Execuatable path', _appInfo.executablePath),
          _infoTile('Shared res path', _appInfo.sharedResourcePath),
          _infoTile('Metadata', _appInfo.metadata.toString()),
          _infoTile('Terminated', _currentAppContext.isTerminated.toString()),
          _infoTile('Process ID', _currentAppContext.processId.toString()),
          _infoTile('State', _currentAppContext.appState.toString()),
        ],
      ),
    );
  }
}

/// The installed applications' information page widget.
class _AppsListScreen extends StatefulWidget {
  const _AppsListScreen({Key? key}) : super(key: key);

  @override
  _AppsListScreenState createState() => _AppsListScreenState();
}

class _AppsListScreenState extends State<_AppsListScreen> {
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text('Applications list')),
      body: _AppsListScreenContent(key: GlobalKey()),
    );
  }
}

class _AppsListScreenContent extends StatelessWidget {
  const _AppsListScreenContent({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return FutureBuilder<List<AppInfo>>(
      future: AppManager.getInstalledApps(),
      builder: (BuildContext context, AsyncSnapshot<List<AppInfo>> snapshot) {
        if (snapshot.data == null) {
          return const Center(child: CircularProgressIndicator());
        } else {
          final List<AppInfo> apps = snapshot.data!;

          return Scrollbar(
            child: ListView.builder(
              itemBuilder: (BuildContext context, int position) {
                final AppInfo app = apps[position];
                return Column(
                  children: <Widget>[
                    ListTile(
                      title: Text(app.label),
                      subtitle: Text('Package Id: ${app.packageId}\n'
                          'App type: ${app.appType}\n'),
                    ),
                    const Divider(height: 1.0)
                  ],
                );
              },
              itemCount: apps.length,
            ),
          );
        }
      },
    );
  }
}

/// Represents application event, app ID, and process ID.
class _AppEventContext {
  _AppEventContext({
    required this.event,
    required this.appId,
    required this.processId,
  });
  final String event;
  final String appId;
  final int processId;
}

/// The application event page widget.
class _AppsEventScreen extends StatefulWidget {
  const _AppsEventScreen({Key? key}) : super(key: key);

  @override
  _AppsEventScreenState createState() => _AppsEventScreenState();
}

class _AppsEventScreenState extends State<_AppsEventScreen> {
  final List<_AppEventContext> _events = <_AppEventContext>[];
  final List<StreamSubscription<AppRunningContext>> _subscriptions =
      <StreamSubscription<AppRunningContext>>[];
  String _appId = settingAppId;

  @override
  void initState() {
    super.initState();

    _getDeviceInfo();

    _subscriptions
        .add(AppManager.onAppLaunched.listen((AppRunningContext event) {
      setState(() {
        _events.add(_AppEventContext(
          event: 'launched',
          appId: event.appId,
          processId: event.processId,
        ));
      });
    }));
    _subscriptions
        .add(AppManager.onAppTerminated.listen((AppRunningContext event) {
      setState(() {
        _events.add(_AppEventContext(
          event: 'terminated',
          appId: event.appId,
          processId: event.processId,
        ));
      });
    }));
  }

  @override
  Widget build(BuildContext context) {
    final PageController controller = PageController(initialPage: 0);
    return Scaffold(
      appBar: AppBar(title: const Text('application context events')),
      body: PageView(
        scrollDirection: Axis.horizontal,
        controller: controller,
        children: <Widget>[
          Center(
            child: Column(mainAxisSize: MainAxisSize.min, children: <Widget>[
              ElevatedButton(
                onPressed: () async {
                  try {
                    final AppControl appControl = AppControl(appId: _appId);
                    await appControl.sendLaunchRequest();
                  } catch (e) {
                    // ignore: avoid_print
                    print('$_appId launch request failed : $e');
                  }
                },
                style: ElevatedButton.styleFrom(
                  shape: RoundedRectangleBorder(
                      borderRadius: BorderRadius.circular(10)),
                ),
                child: Text('launch $_appId'),
              ),
            ]),
          ),
          Stack(
            alignment: Alignment.center,
            children: <Widget>[
              Visibility(
                visible: _events.isNotEmpty,
                child: _EventsList(events: _events),
              ),
              Visibility(
                visible: _events.isEmpty,
                child: const _EmptyList(),
              )
            ],
          ),
        ],
      ),
    );
  }

  @override
  void dispose() {
    for (final StreamSubscription<AppRunningContext> subscription
        in _subscriptions) {
      subscription.cancel();
    }
    _subscriptions.clear();
    super.dispose();
  }

  Future<void> _getDeviceInfo() async {
    final DeviceInfoPluginTizen deviceInfo = DeviceInfoPluginTizen();
    final TizenDeviceInfo tizenInfo = await deviceInfo.tizenInfo;
    setState(() {
      if (tizenInfo.profile == 'tv') {
        _appId = tvVolumeSettingAppId;
      } else if (tizenInfo.profile == 'wearable') {
        _appId = wearbleSettingAppId;
      }
    });
  }
}

class _EventsList extends StatelessWidget {
  const _EventsList({required List<_AppEventContext> events})
      : _events = events;

  final Iterable<_AppEventContext> _events;

  @override
  Widget build(BuildContext context) {
    return Scrollbar(
      child: ListView.builder(
        itemBuilder: (BuildContext context, int position) {
          return KeyedSubtree(
              key: Key('$position'),
              child: _AppContext(appEventContext: _events.elementAt(position)));
        },
        itemCount: _events.length,
      ),
    );
  }
}

class _AppContext extends StatelessWidget {
  const _AppContext({required this.appEventContext});

  final _AppEventContext appEventContext;

  @override
  Widget build(BuildContext context) {
    return Column(
      children: <Widget>[
        ListTile(
          title: Text(appEventContext.appId),
          subtitle: Text(
              'event: ${appEventContext.event}, pid: ${appEventContext.processId}'),
        ),
        const Divider()
      ],
    );
  }
}

class _EmptyList extends StatelessWidget {
  const _EmptyList();

  @override
  Widget build(BuildContext context) {
    return const Center(child: Text('No event yet!'));
  }
}
