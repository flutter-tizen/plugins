import 'package:tizen_application_manager/tizen_application_manager.dart';
import 'package:flutter/material.dart';

class AppsListScreen extends StatefulWidget {
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
    return FutureBuilder<List<ApplicationInfo>>(
      future: TizenApplicationManager.getInstalledApplications(),
      builder: (BuildContext context,
          AsyncSnapshot<List<ApplicationInfo>> snapshot) {
        if (snapshot.data == null) {
          return const Center(child: CircularProgressIndicator());
        } else {
          final apps = snapshot.data!;

          return Scrollbar(
            child: ListView.builder(
                itemBuilder: (BuildContext context, int position) {
                  final app = apps[position];
                  return Column(
                    children: <Widget>[
                      ListTile(
                        title: Text(app.label),
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
