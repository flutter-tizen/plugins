import 'package:tizen_application_manager/tizen_application_manager.dart';
import 'package:flutter/material.dart';

class AppsListScreen extends StatefulWidget {
  @override
  _AppsListScreenState createState() => _AppsListScreenState();
}

class _AppsListScreenState extends State<AppsListScreen> {
  bool _onlyDotnetApps = false;

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text('Applications list'),
        actions: <Widget>[
          PopupMenuButton<String>(
            itemBuilder: (BuildContext context) {
              return <PopupMenuItem<String>>[
                PopupMenuItem<String>(
                  value: 'dotnet_apps',
                  child: Text('Toggle dotnet apps only'),
                )
              ];
            },
            onSelected: (String key) {
              if (key == 'dotnet_apps') {
                setState(() {
                  _onlyDotnetApps = !_onlyDotnetApps;
                });
              }
            },
          )
        ],
      ),
      body: _AppsListScreenContent(
          onlyDotnetApps: _onlyDotnetApps, key: GlobalKey()),
    );
  }
}

class _AppsListScreenContent extends StatelessWidget {
  final bool onlyDotnetApps;

  const _AppsListScreenContent({Key? key, this.onlyDotnetApps = false})
      : super(key: key);

  @override
  Widget build(BuildContext context) {
    return FutureBuilder<List<ApplicationInfo>>(
      future: TizenApplicationManager.getInstalledApplications(),
      builder: (BuildContext context,
          AsyncSnapshot<List<ApplicationInfo>> snapshot) {
        if (snapshot.data == null) {
          return const Center(child: CircularProgressIndicator());
        } else {
          var apps = snapshot.data!;

          return Scrollbar(
            child: ListView.builder(
                itemBuilder: (BuildContext context, int position) {
                  var app = apps[position];
                  return Column(
                    children: <Widget>[
                      ListTile(
                        title: Text('${app.label}'),
                        subtitle: Text('Package Id: ${app.packageId}\n'
                            'App type: ${app.applicationType}\n'),
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
