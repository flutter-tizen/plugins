// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:device_info_plus_tizen/device_info_plus_tizen.dart';
import 'package:flutter/material.dart';
import 'package:tizen_app_control/app_control.dart';
import 'package:tizen_app_manager/app_manager.dart';

/// The settings app ID
const String settingAppId = 'org.tizen.setting';

/// The wearable emulator settings app ID
const String wearbleSettingAppId = 'org.tizen.watch-setting';

/// The tv emulator volume setting app ID
const String tvVolumeSettingAppId = 'org.tizen.volume-setting';

/// Represents application event string and AppRunningContext instance.
class AppEventContext {
  /// Creates an instance of [AppEventContext] with event string and AppRunningContext instance.
  AppEventContext({required this.event, required this.context});

  /// The event string.
  final String event;

  /// The AppRunningContext instance.
  final AppRunningContext context;
}

/// The application event page widget.
class AppsEventScreen extends StatefulWidget {
  /// The constructor of the application event page widget.
  const AppsEventScreen({Key? key}) : super(key: key);

  @override
  _AppsEventScreenState createState() => _AppsEventScreenState();
}

class _AppsEventScreenState extends State<AppsEventScreen> {
  final List<AppEventContext> _events = <AppEventContext>[];
  final List<StreamSubscription<dynamic>> _subscriptions =
      <StreamSubscription<dynamic>>[];
  String _appId = settingAppId;

  @override
  void initState() {
    super.initState();

    _getDeviceInfos();

    _subscriptions
        .add(AppManager.onAppLaunched.listen((AppRunningContext event) {
      setState(() {
        _events.add(AppEventContext(event: 'launched', context: event));
      });
    }));
    _subscriptions
        .add(AppManager.onAppTerminated.listen((AppRunningContext event) {
      setState(() {
        _events.add(AppEventContext(event: 'terminated', context: event));
      });
    }));
  }

  @override
  Widget build(BuildContext context) {
    final PageController controller = PageController(initialPage: 0);
    return Scaffold(
      appBar: AppBar(
        title: const Text('application context events'),
      ),
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
                            borderRadius: BorderRadius.circular(10))),
                    child: Text('launch $_appId')),
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
          ]),
    );
  }

  @override
  void dispose() {
    // ignore: always_specify_types
    for (final StreamSubscription s in _subscriptions) {
      s.cancel();
    }
    _subscriptions.clear();
    super.dispose();
  }

  Future<void> _getDeviceInfos() async {
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
  const _EventsList({required List<AppEventContext> events}) : _events = events;

  final Iterable<AppEventContext> _events;

  @override
  Widget build(BuildContext context) {
    return Scrollbar(
      child: ListView.builder(
        itemBuilder: (BuildContext context, int position) {
          return KeyedSubtree(
              key: Key('$position'),
              child: _AppContext(
                appEventContext: _events.elementAt(position),
              ));
        },
        itemCount: _events.length,
      ),
    );
  }
}

class _AppContext extends StatelessWidget {
  const _AppContext({required this.appEventContext});

  final AppEventContext appEventContext;

  @override
  Widget build(BuildContext context) {
    return Column(
      children: <Widget>[
        ListTile(
          title: Text(appEventContext.context.appId),
          subtitle: Text(
              'event: ${appEventContext.event}, pid: ${appEventContext.context.processId}'),
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
