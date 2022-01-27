import 'package:flutter/material.dart';

import './current_app_info.dart';
import './apps_list.dart';
import './apps_event.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Application manager demo',
      theme: ThemeData(primarySwatch: Colors.blue),
      home: const MyHomePage(title: 'Application manager demo'),
    );
  }
}

class MyHomePage extends StatefulWidget {
  const MyHomePage({Key? key, required this.title}) : super(key: key);

  final String title;

  @override
  _MyHomePageState createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> {
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text('application common demo')),
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
                            const CurrentAppScreen()),
                  );
                },
                child: const Text('Current application info')),
            TextButton(
                onPressed: () {
                  Navigator.push(
                    context,
                    MaterialPageRoute<Object>(
                        builder: (BuildContext context) =>
                            const AppsListScreen()),
                  );
                },
                child: const Text('Installed applications list')),
            TextButton(
                onPressed: () {
                  Navigator.push(
                    context,
                    MaterialPageRoute<Object>(
                        builder: (BuildContext context) =>
                            const AppsEventScreen()),
                  );
                },
                child: const Text('Application launch/terminate and listener')),
          ],
        ),
      ),
    );
  }
}
