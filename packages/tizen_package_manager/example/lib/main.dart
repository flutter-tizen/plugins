// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: public_member_api_docs

import 'dart:async';

import 'package:flutter/material.dart';
import 'package:tizen_package_manager/tizen_package_manager.dart';

/// The example app package ID.
const String currentPackageId = 'org.tizen.tizen_package_manager_example';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Package Manager Demo',
      theme: ThemeData(primarySwatch: Colors.blue),
      home: const MyHomePage(),
    );
  }
}

class MyHomePage extends StatefulWidget {
  const MyHomePage({super.key});

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
          title: const Text('Package Manager Demo'),
          bottom: const TabBar(
            isScrollable: true,
            tabs: <Tab>[
              Tab(text: 'This package'),
              Tab(text: 'Package list'),
              Tab(text: 'Package events'),
            ],
          ),
        ),
        body: const TabBarView(
          children: <Widget>[
            _CurrentPackageScreen(),
            _PackageListScreen(),
            _PackageEventsScreen(),
          ],
        ),
      ),
    );
  }
}

class _CurrentPackageScreen extends StatefulWidget {
  const _CurrentPackageScreen();

  @override
  State<_CurrentPackageScreen> createState() => _CurrentPackageScreenState();
}

class _CurrentPackageScreenState extends State<_CurrentPackageScreen> {
  Widget _infoTile(String title, String subtitle) {
    return ListTile(title: Text(title), subtitle: Text(subtitle));
  }

  @override
  Widget build(BuildContext context) {
    return FutureBuilder<List<dynamic>>(
      future: Future.wait(<Future<dynamic>>[
        PackageManager.getPackageInfo(currentPackageId),
        PackageManager.getPackageSizeInfo(currentPackageId),
      ]),
      builder: (BuildContext context, AsyncSnapshot<List<dynamic>> snapshot) {
        if (snapshot.hasData) {
          final PackageInfo packageInfo = snapshot.data![0] as PackageInfo;
          final PackageSizeInfo sizeInfo = snapshot.data![1] as PackageSizeInfo;
          return ListView(
            children: <Widget>[
              _infoTile('Package ID', packageInfo.packageId),
              _infoTile('Label', packageInfo.label),
              _infoTile('Version', packageInfo.version),
              _infoTile('Package type', packageInfo.packageType.name),
              _infoTile('Icon path', packageInfo.iconPath ?? ''),
              _infoTile('System app', packageInfo.isSystem.toString()),
              _infoTile('Preloaded app', packageInfo.isPreloaded.toString()),
              _infoTile('Removable', packageInfo.isRemovable.toString()),
              const Divider(),
              const ListTile(
                title: Text('Package Size Information',
                    style: TextStyle(fontWeight: FontWeight.bold)),
              ),
              _infoTile('Data Size', '${sizeInfo.dataSize} bytes'),
              _infoTile('Cache Size', '${sizeInfo.cacheSize} bytes'),
              _infoTile('App Size', '${sizeInfo.appSize} bytes'),
              _infoTile(
                  'External Data Size', '${sizeInfo.externalDataSize} bytes'),
              _infoTile(
                  'External Cache Size', '${sizeInfo.externalCacheSize} bytes'),
              _infoTile(
                  'External App Size', '${sizeInfo.externalAppSize} bytes'),
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

class _PackageListScreen extends StatefulWidget {
  const _PackageListScreen();

  @override
  State<_PackageListScreen> createState() => _PackageListScreenState();
}

class _PackageListScreenState extends State<_PackageListScreen>
    with AutomaticKeepAliveClientMixin {
  @override
  bool get wantKeepAlive => true;

  Future<Map<String, PackageSizeInfo>> _getAllPackagesSizeInfo(
      List<PackageInfo> packages) async {
    final Map<String, PackageSizeInfo> sizeInfoMap =
        <String, PackageSizeInfo>{};

    for (final PackageInfo package in packages) {
      try {
        final PackageSizeInfo sizeInfo =
            await PackageManager.getPackageSizeInfo(package.packageId);
        sizeInfoMap[package.packageId] = sizeInfo;
      } catch (e) {
        debugPrint(
            'Failed to get size info for package ${package.packageId}: $e');
      }
    }

    return sizeInfoMap;
  }

  @override
  Widget build(BuildContext context) {
    super.build(context);

    return FutureBuilder<List<PackageInfo>>(
      future: PackageManager.getPackagesInfo(),
      builder: (
        BuildContext context,
        AsyncSnapshot<List<PackageInfo>> snapshot,
      ) {
        if (snapshot.hasData) {
          final List<PackageInfo> packages = snapshot.data!;
          return FutureBuilder<Map<String, PackageSizeInfo>>(
            future: _getAllPackagesSizeInfo(packages),
            builder: (
              BuildContext context,
              AsyncSnapshot<Map<String, PackageSizeInfo>> sizeSnapshot,
            ) {
              if (sizeSnapshot.hasData) {
                final Map<String, PackageSizeInfo> sizeInfoMap =
                    sizeSnapshot.data!;
                return ListView.builder(
                  itemCount: packages.length,
                  itemBuilder: (BuildContext context, int index) {
                    final PackageInfo package = packages[index];
                    final PackageSizeInfo? sizeInfo =
                        sizeInfoMap[package.packageId];

                    return ListTile(
                      title: Text(package.label),
                      subtitle: Padding(
                        padding: const EdgeInsets.symmetric(vertical: 5),
                        child: Text(
                          'Package ID: ${package.packageId}\n'
                          'Version: ${package.version}\n'
                          'Type: ${package.packageType.name}\n'
                          'System: ${package.isSystem}'
                          '${sizeInfo != null ? '\nApp Size: ${sizeInfo.appSize} bytes' : ''}',
                        ),
                      ),
                      isThreeLine: sizeInfo != null,
                    );
                  },
                );
              } else if (sizeSnapshot.hasError) {
                return Center(child: Text(sizeSnapshot.error.toString()));
              } else {
                return const Center(child: CircularProgressIndicator());
              }
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

class _PackageEventsScreen extends StatefulWidget {
  const _PackageEventsScreen();

  @override
  State<_PackageEventsScreen> createState() => _PackageEventsScreenState();
}

class _PackageEventsScreenState extends State<_PackageEventsScreen>
    with AutomaticKeepAliveClientMixin {
  late final StreamSubscription<PackageEvent>? _installSubscription;
  late final StreamSubscription<PackageEvent>? _uninstallSubscription;
  late final StreamSubscription<PackageEvent>? _updateSubscription;
  final List<PackageEvent> _packageEvents = <PackageEvent>[];

  @override
  void initState() {
    super.initState();

    _installSubscription = PackageManager.onInstallProgressChanged.listen((
      PackageEvent event,
    ) {
      setState(() {
        _packageEvents.add(event);
      });
    });
    _uninstallSubscription = PackageManager.onUninstallProgressChanged.listen((
      PackageEvent event,
    ) {
      setState(() {
        _packageEvents.add(event);
      });
    });
    _updateSubscription = PackageManager.onUpdateProgressChanged.listen((
      PackageEvent event,
    ) {
      setState(() {
        _packageEvents.add(event);
      });
    });
  }

  @override
  bool get wantKeepAlive => true;

  @override
  Widget build(BuildContext context) {
    super.build(context);

    if (_packageEvents.isEmpty) {
      return const Center(child: Text('No events'));
    } else {
      return ListView.builder(
        itemCount: _packageEvents.length,
        itemBuilder: (BuildContext context, int index) {
          final PackageEvent event = _packageEvents.elementAt(index);
          return ListTile(
            title: Text(event.packageId),
            subtitle: Text(
              'Type: ${event.eventType.name}\n'
              'State: ${event.eventState.name}',
            ),
          );
        },
      );
    }
  }

  @override
  void dispose() {
    super.dispose();
    _installSubscription?.cancel();
    _uninstallSubscription?.cancel();
    _updateSubscription?.cancel();
  }
}
