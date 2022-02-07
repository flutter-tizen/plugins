// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter/material.dart';
import 'package:tizen_app_manager/app_manager.dart';

/// The installed applications's information page widget.
class AppsListScreen extends StatefulWidget {
  /// The constructor of the installed applications's information page widget.
  const AppsListScreen({Key? key}) : super(key: key);

  @override
  _AppsListScreenState createState() => _AppsListScreenState();
}

class _AppsListScreenState extends State<AppsListScreen> {
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Applications list'),
      ),
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
                      const Divider(
                        height: 1.0,
                      )
                    ],
                  );
                },
                itemCount: apps.length),
          );
        }
      },
    );
  }
}
