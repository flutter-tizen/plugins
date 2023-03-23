// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: public_member_api_docs

import 'dart:async';
import 'dart:io';

import 'package:device_info_plus_tizen/device_info_plus_tizen.dart';
import 'package:flutter/material.dart';
import 'package:tizen_app_control/tizen_app_control.dart';
import 'package:tizen_app_manager/tizen_app_manager.dart';

/// The settings app ID.
const String settingsAppId = 'org.tizen.setting';

/// The wearable emulator settings app ID.
const String wearableSettingsAppId = 'org.tizen.watch-setting';

/// The tv emulator volume setting app ID.
const String tvVolumeSettingAppId = 'org.tizen.volume-setting';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'App Manager Demo',
      theme: ThemeData(primarySwatch: Colors.blue),
      home: const MyHomePage(),
    );
  }
}

class MyHomePage extends StatefulWidget {
  const MyHomePage({Key? key}) : super(key: key);

  @override
  State<MyHomePage> createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> {
  @override
  Widget build(BuildContext context) {
    return DefaultTabController(
      length: 3,
      child: Scaffold(
        appBar: AppBar(
          title: const Text('App Manager Demo'),
          bottom: const TabBar(
            isScrollable: true,
            tabs: <Tab>[
              Tab(text: 'This app'),
              Tab(text: 'App list'),
              Tab(text: 'App events'),
            ],
          ),
        ),
        body: const TabBarView(
          children: <Widget>[
            _CurrentAppScreen(),
            _AppListScreen(),
            _AppEventsScreen(),
          ],
        ),
      ),
    );
  }
}

class _CurrentAppScreen extends StatefulWidget {
  const _CurrentAppScreen({Key? key}) : super(key: key);

  @override
  State<_CurrentAppScreen> createState() => _CurrentAppScreenState();
}

class _CurrentAppScreenState extends State<_CurrentAppScreen> {
  final Future<AppInfo> _appInfo = () async {
    final String appId = await AppManager.currentAppId;
    return AppManager.getAppInfo(appId);
  }();

  Widget _infoTile(String title, String subtitle) {
    return ListTile(title: Text(title), subtitle: Text(subtitle));
  }

  @override
  Widget build(BuildContext context) {
    return FutureBuilder<AppInfo>(
      future: _appInfo,
      builder: (BuildContext context, AsyncSnapshot<AppInfo> snapshot) {
        if (snapshot.hasData) {
          final AppInfo appInfo = snapshot.data!;
          final AppRunningContext appContext =
              AppRunningContext(appId: appInfo.appId);
          return ListView(
            children: <Widget>[
              _infoTile('App ID', appInfo.appId),
              _infoTile('Package ID', appInfo.packageId),
              _infoTile('Label', appInfo.label),
              _infoTile('Type', appInfo.appType),
              _infoTile('Execuatable path', appInfo.executablePath),
              _infoTile('Shared resource path', appInfo.sharedResourcePath),
              _infoTile('Metadata', appInfo.metadata.toString()),
              _infoTile('Process ID', appContext.processId.toString()),
              _infoTile('State', appContext.appState.name),
            ],
          );
        } else if (snapshot.hasError) {
          return Center(child: Text(snapshot.error.toString()));
        } else {
          return const Center(child: CircularProgressIndicator());
        }
      },
    );
  }
}

class _AppListScreen extends StatefulWidget {
  const _AppListScreen({Key? key}) : super(key: key);

  @override
  State<_AppListScreen> createState() => _AppListScreenState();
}

class _AppListScreenState extends State<_AppListScreen>
    with AutomaticKeepAliveClientMixin {
  @override
  bool get wantKeepAlive => true;

  @override
  Widget build(BuildContext context) {
    super.build(context);

    return FutureBuilder<List<AppInfo>>(
      future: AppManager.getInstalledApps(),
      builder: (BuildContext context, AsyncSnapshot<List<AppInfo>> snapshot) {
        if (snapshot.hasData) {
          final List<AppInfo> apps = snapshot.data!;
          return ListView.builder(
            itemCount: apps.length,
            itemBuilder: (BuildContext context, int index) {
              final AppInfo appInfo = apps[index];
              Widget appIcon = const Icon(Icons.error_outline);
              if (appInfo.iconPath != null) {
                final File iconFile = File(appInfo.iconPath!);
                if (iconFile.existsSync()) {
                  appIcon = Image.file(iconFile);
                }
              }
              return ListTile(
                leading: SizedBox(width: 30, child: appIcon),
                title: Text(appInfo.label),
                subtitle: Text(appInfo.packageId),
                trailing: Text(
                  appInfo.appType,
                  style: Theme.of(context).textTheme.bodySmall,
                ),
              );
            },
          );
        } else if (snapshot.hasError) {
          return Center(child: Text(snapshot.error.toString()));
        } else {
          return const Center(child: CircularProgressIndicator());
        }
      },
    );
  }
}

class _AppEvent {
  _AppEvent({
    required this.event,
    required this.appId,
    required this.processId,
  });

  final String event;
  final String appId;
  final int processId;
}

class _AppEventsScreen extends StatefulWidget {
  const _AppEventsScreen({Key? key}) : super(key: key);

  @override
  State<_AppEventsScreen> createState() => _AppEventsScreenState();
}

class _AppEventsScreenState extends State<_AppEventsScreen>
    with AutomaticKeepAliveClientMixin {
  late final StreamSubscription<AppRunningContext>? _launchSubscription;
  late final StreamSubscription<AppRunningContext>? _terminateSubscription;
  final List<_AppEvent> _appEvents = <_AppEvent>[];
  String _settingsAppId = settingsAppId;

  @override
  bool get wantKeepAlive => true;

  @override
  void initState() {
    super.initState();

    _getDeviceInfo();

    _launchSubscription =
        AppManager.onAppLaunched.listen((AppRunningContext context) {
      setState(() {
        _appEvents.add(_AppEvent(
          event: 'Launched',
          appId: context.appId,
          processId: context.processId,
        ));
      });
    });
    _terminateSubscription =
        AppManager.onAppTerminated.listen((AppRunningContext context) {
      setState(() {
        _appEvents.add(_AppEvent(
          event: 'Terminated',
          appId: context.appId,
          processId: context.processId,
        ));
      });
    });
  }

  Future<void> _getDeviceInfo() async {
    final DeviceInfoPluginTizen deviceInfo = DeviceInfoPluginTizen();
    final TizenDeviceInfo tizenInfo = await deviceInfo.tizenInfo;
    setState(() {
      if (tizenInfo.profile == 'tv') {
        _settingsAppId = tvVolumeSettingAppId;
      } else if (tizenInfo.profile == 'wearable') {
        _settingsAppId = wearableSettingsAppId;
      }
    });
  }

  @override
  Widget build(BuildContext context) {
    super.build(context);

    return Column(
      mainAxisSize: MainAxisSize.min,
      children: <Widget>[
        Padding(
          padding: const EdgeInsets.all(10),
          child: ElevatedButton(
            onPressed: () {
              AppControl(appId: _settingsAppId).sendLaunchRequest();
            },
            child: Text('Launch $_settingsAppId'),
          ),
        ),
        Expanded(
          child: ListView.builder(
            itemCount: _appEvents.length,
            itemBuilder: (BuildContext context, int index) {
              final _AppEvent event = _appEvents.elementAt(index);
              return ListTile(
                title: Text(event.appId),
                subtitle: Text('Process ID: ${event.processId}'),
                trailing: Text(
                  event.event,
                  style: Theme.of(context).textTheme.bodySmall,
                ),
              );
            },
          ),
        ),
      ],
    );
  }

  @override
  void dispose() {
    super.dispose();
    _launchSubscription?.cancel();
    _terminateSubscription?.cancel();
  }
}
