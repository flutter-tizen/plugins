// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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

    PackageManager.getPackageInfo(currentPackageId).then(
      (PackageInfo packageInfo) {
        setState(() {
          _packageInfo = packageInfo;
        });
      },
    );
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
      appBar: AppBar(title: const Text(' Package list')),
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
