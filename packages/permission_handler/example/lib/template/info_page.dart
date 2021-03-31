import 'package:flutter/material.dart';
import 'package:flutter/rendering.dart';
import 'package:permission_handler/permission_handler.dart';

import 'globals.dart';

/// [StatelessWidget] displaying information about Baseflow
class InfoPage extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return SizedBox.expand(
      child: Align(
        alignment: Alignment.center,
        child: SingleChildScrollView(
          child: Padding(
            padding: defaultHorizontalPadding + defaultVerticalPadding,
            child: Column(
              mainAxisSize: MainAxisSize.max,
              mainAxisAlignment: MainAxisAlignment.end,
              children: <Widget>[
                Text(
                  'The permission can be changed in the app settings.'
                ),
                const Padding(
                  padding: EdgeInsets.symmetric(vertical: 8),
                ),
                _launcherRaisedButton(
                  'Open App Settings',
                  context,
                ),
              ],
            ),
          ),
        ),
      ),
    );
  }

  Widget _launcherRaisedButton(String text, BuildContext context) {
    return Container(
      width: MediaQuery.of(context).size.width,
      height: 50,
      margin: const EdgeInsets.only(top: 24.0),
      alignment: Alignment.center,
      child: SizedBox.expand(
        child: RaisedButton(
          textTheme: Theme.of(context).buttonTheme.textTheme,
          shape:
              RoundedRectangleBorder(borderRadius: BorderRadius.circular(30.0)),
          padding: const EdgeInsets.all(8),
          child: Text(text),
          onPressed: () => openAppSettings(),
        ),
      ),
    );
  }
}
