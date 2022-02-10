// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:flutter/material.dart';
import 'package:tizen_package_manager/package_manager.dart';

/// The example app package ID.
const String currentPackageId = 'com.example.tizen_package_manager_example';

/// The main entry point for the UI app.
void main() {
  runApp(const MyApp());
}

/// The main UI app widget.
class MyApp extends StatelessWidget {
  /// The constructor of the main UI app widget.
  const MyApp({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Package manager demo',
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
      appBar: AppBar(title: const Text('Package manager demo')),
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
                        _CurrentPackageInfoScreen(),
                  ),
                );
              },
              child: const Text('Current app package info'),
            ),
            TextButton(
              onPressed: () {
                Navigator.push(
                  context,
                  MaterialPageRoute<Object>(
                    builder: (BuildContext context) => _PackagesListScreen(),
                  ),
                );
              },
              child: const Text('Installed packages list'),
            ),
            TextButton(
              onPressed: () {
                Navigator.push(
                  context,
                  MaterialPageRoute<Object>(
                    builder: (BuildContext context) => _PackagesEventScreen(),
                  ),
                );
              },
              child: const Text('Package install/uninstall and listener'),
            ),
          ],
        ),
      ),
    );
  }
}

class _CurrentPackageInfoScreen extends StatefulWidget {
  @override
  _CurrentPackageInfoScreenState createState() =>
      _CurrentPackageInfoScreenState();
}

class _CurrentPackageInfoScreenState extends State<_CurrentPackageInfoScreen> {
  PackageInfo _packageInfo = PackageInfo(
    packageId: '',
    label: '',
    packageType: PackageType.unknown,
    iconPath: '',
    version: '',
    installedStorageType: '',
    isSystem: false,
    isPreloaded: false,
    isRemovable: false,
  );

  @override
  void initState() {
    super.initState();
    _initApplicationsInfo();
  }

  // Platform messages are asynchronous, so we initialize in an async method.
  Future<void> _initApplicationsInfo() async {
    PackageInfo? packageInfo;

    packageInfo = await PackageManager.getPackageInfo(currentPackageId);

    setState(() {
      if (packageInfo != null) {
        _packageInfo = packageInfo;
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
      appBar: AppBar(title: const Text('Current app package info')),
      body: ListView(
        children: <Widget>[
          _infoTile('Package ID', _packageInfo.packageId),
          _infoTile('Label', _packageInfo.label),
          _infoTile('Version', _packageInfo.version),
          _infoTile('Package type', _packageInfo.packageType.name),
          _infoTile('Icon path', _packageInfo.iconPath ?? ''),
          _infoTile('Is system app', _packageInfo.isSystem.toString()),
          _infoTile('Is preloaded app', _packageInfo.isPreloaded.toString()),
          _infoTile('Is removable', _packageInfo.isRemovable.toString()),
        ],
      ),
    );
  }
}

class _PackagesListScreen extends StatefulWidget {
  @override
  _PackagesListScreenState createState() => _PackagesListScreenState();
}

class _PackagesListScreenState extends State<_PackagesListScreen> {
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text(' Packages list')),
      body: _PackagesListScreenContent(key: GlobalKey()),
    );
  }
}

class _PackagesListScreenContent extends StatelessWidget {
  const _PackagesListScreenContent({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return FutureBuilder<List<PackageInfo>>(
      future: PackageManager.getPackagesInfo(),
      builder: (BuildContext context, AsyncSnapshot<List<PackageInfo>> data) {
        if (data.data == null) {
          return const Center(child: CircularProgressIndicator());
        } else {
          final List<PackageInfo> packages = data.data!;

          return Scrollbar(
            child: ListView.builder(
              itemBuilder: (BuildContext context, int position) {
                final PackageInfo package = packages[position];
                return Column(
                  children: <Widget>[
                    ListTile(
                      title: Text(package.label),
                      subtitle: Text('Package Id: ${package.packageId}\n'
                          'Version: ${package.version}\n'
                          'type: ${package.packageType}\n'
                          'isSystem: ${package.isSystem}\n'),
                    ),
                    const Divider(height: 1.0)
                  ],
                );
              },
              itemCount: packages.length,
            ),
          );
        }
      },
    );
  }
}

///  temp code, this code will be removed
const String _packageName = 'org.example.simplehome';
const String _packageFileName = 'org.example.simplehome.tpk';

class _PackagesEventScreen extends StatefulWidget {
  @override
  __PackagesEventScreenState createState() => __PackagesEventScreenState();
}

class __PackagesEventScreenState extends State<_PackagesEventScreen> {
  final List<PackageEvent> _events = <PackageEvent>[];
  final List<StreamSubscription<PackageEvent>> _subscriptions =
      <StreamSubscription<PackageEvent>>[];
  final String _sharedResPath =
      '/opt/usr/globalapps/com.example.tizen_package_manager_example/shared/res/';

  @override
  void initState() {
    super.initState();

    _subscriptions.add(
        PackageManager.onInstallProgressChanged.listen((PackageEvent event) {
      setState(() {
        _events.add(event);
      });
    }));
    _subscriptions.add(
        PackageManager.onUninstallProgressChanged.listen((PackageEvent event) {
      setState(() {
        _events.add(event);
      });
    }));
    _subscriptions.add(
        PackageManager.onUpdateProgressChanged.listen((PackageEvent event) {
      setState(() {
        _events.add(event);
      });
    }));
  }

  @override
  Widget build(BuildContext context) {
    final PageController controller = PageController(initialPage: 0);
    return Scaffold(
      appBar: AppBar(title: const Text('Package events')),
      body: PageView(
        scrollDirection: Axis.horizontal,
        controller: controller,
        children: <Widget>[
          Center(
            child: Column(
              mainAxisSize: MainAxisSize.min,
              children: <Widget>[
                ElevatedButton(
                  onPressed: () {
                    if (_sharedResPath.isNotEmpty) {
                      final String packagePath =
                          _sharedResPath + _packageFileName;
                      PackageManager.install(packagePath);
                    }
                  },
                  style: ElevatedButton.styleFrom(
                    shape: RoundedRectangleBorder(
                      borderRadius: BorderRadius.circular(10),
                    ),
                  ),
                  child: const Text('Install test app'),
                ),
                const SizedBox(height: 30),
                ElevatedButton(
                  onPressed: () {
                    PackageManager.uninstall(_packageName);
                  },
                  style: ElevatedButton.styleFrom(
                    shape: RoundedRectangleBorder(
                      borderRadius: BorderRadius.circular(10),
                    ),
                  ),
                  child: const Text('Uninstall test app'),
                )
              ],
            ),
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
    for (final StreamSubscription<PackageEvent> subscription
        in _subscriptions) {
      subscription.cancel();
    }
    _subscriptions.clear();
    super.dispose();
  }
}

class _EventsList extends StatelessWidget {
  const _EventsList({required List<PackageEvent> events}) : _events = events;

  final Iterable<PackageEvent> _events;

  @override
  Widget build(BuildContext context) {
    return Scrollbar(
      child: ListView.builder(
        itemBuilder: (BuildContext context, int position) {
          return KeyedSubtree(
            key: Key('$position'),
            child: _PackageEventItem(event: _events.elementAt(position)),
          );
        },
        itemCount: _events.length,
      ),
    );
  }
}

class _PackageEventItem extends StatelessWidget {
  const _PackageEventItem({required this.event});

  final PackageEvent event;

  @override
  Widget build(BuildContext context) {
    return Column(
      children: <Widget>[
        ListTile(
          title: Text(event.packageId),
          subtitle: Text('${event.eventType.name} : ${event.eventState.name}'),
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
