import 'dart:core';

import 'package:flutter/material.dart';

class CallSettings extends StatefulWidget {
  static String tag = 'call_settings';

  @override
  _CallSettingsState createState() => _CallSettingsState();
}

class _CallSettingsState extends State<CallSettings> {
  @override
  void initState() {
    super.initState();
  }

  @override
  void deactivate() {
    super.deactivate();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: Text('Settings')),
      body: OrientationBuilder(
        builder: (context, orientation) {
          return Center(child: Text('settings'));
        },
      ),
    );
  }
}
