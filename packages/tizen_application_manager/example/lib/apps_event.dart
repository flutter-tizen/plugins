import 'dart:async';
import 'package:flutter/material.dart';
import 'package:device_info_plus_tizen/device_info_plus_tizen.dart';
import 'package:tizen_app_control/app_control.dart';
import 'package:tizen_application_manager/tizen_application_manager.dart';
import 'package:tizen_application_manager/application_running_context.dart';

const String settingAppId = 'org.tizen.setting';
const String wearbleSettingAppId = 'org.tizen.watch-setting';
const String tvVolumeSettingAppId = 'org.tizen.volume-setting';

class AppEventContext {
  final String event;

  final ApplicationRunningContext context;

  AppEventContext({required this.event, required this.context});
}

class AppsEventScreen extends StatefulWidget {
  @override
  _AppsEventScreenState createState() => _AppsEventScreenState();
}

class _AppsEventScreenState extends State<AppsEventScreen> {
  final List<AppEventContext> _events = <AppEventContext>[];
  final List<StreamSubscription<dynamic>> _subscriptions = [];
  String _appId = settingAppId;

  @override
  void initState() {
    super.initState();

    getDeviceInfos();

    _subscriptions.add(TizenApplicationManager.onApplicationLaunched
        .listen((ApplicationRunningContext event) {
      setState(() {
        _events.add(AppEventContext(event: 'launched', context: event));
      });
    }));
    _subscriptions.add(TizenApplicationManager.onApplicationTerminated
        .listen((ApplicationRunningContext event) {
      setState(() {
        _events.add(AppEventContext(event: 'terminated', context: event));
      });
    }));
  }

  @override
  Widget build(BuildContext context) {
    var controller = PageController(initialPage: 0);
    return Scaffold(
      appBar: AppBar(
        title: Text('application context events'),
      ),
      body: PageView(
          scrollDirection: Axis.horizontal,
          controller: controller,
          children: <Widget>[
            Center(
              child: Column(mainAxisSize: MainAxisSize.min, children: <Widget>[
                ElevatedButton(
                    onPressed: () async {
                      print('launch button pressed');
                      try {
                        var appControl = AppControl(appId: _appId);
                        await appControl.sendLaunchRequest();
                      } catch (e) {
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
    _subscriptions.forEach((StreamSubscription s) => s.cancel());
    _subscriptions.clear();
    super.dispose();
  }

  void getDeviceInfos() async {
    DeviceInfoPluginTizen deviceInfo = DeviceInfoPluginTizen();
    TizenDeviceInfo tizenInfo = await deviceInfo.tizenInfo;
    setState(() {
      print('device info profile : ${tizenInfo.profile}');
      if (tizenInfo.profile == 'tv') {
        _appId = tvVolumeSettingAppId;
      } else if (tizenInfo.profile == 'wearable') {
        _appId = wearbleSettingAppId;
      }
    });
  }
}

class _EventsList extends StatelessWidget {
  final Iterable<AppEventContext> _events;

  _EventsList({required List<AppEventContext> events}) : _events = events;

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
  final AppEventContext appEventContext;

  _AppContext({required this.appEventContext});

  @override
  Widget build(BuildContext context) {
    return Column(
      children: <Widget>[
        ListTile(
          title: Text(appEventContext.context.applicationId),
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
    return Center(child: Text('No event yet!'));
  }
}
