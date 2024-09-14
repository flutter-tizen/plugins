// Copyright (c) 2020-2023, creativecreatorormaybenot
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: public_member_api_docs

import 'package:flutter/material.dart';
import 'package:wakelock_plus/wakelock_plus.dart';

void main() {
  runApp(const WakelockPlusExampleApp());
}

/// Example app widget demonstrating how to use the wakelock plugin.
///
/// The example implementation is located inside [OutlinedButton.onPressed]
/// callback functions and a [FutureBuilder].
class WakelockPlusExampleApp extends StatefulWidget {
  const WakelockPlusExampleApp({super.key});

  /// Creates the [WakelockPlusExampleApp] widget.

  @override
  State<WakelockPlusExampleApp> createState() => _WakelockPlusExampleAppState();
}

class _WakelockPlusExampleAppState extends State<WakelockPlusExampleApp> {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: const Text('Wakelock example app'),
        ),
        body: Center(
          child: Column(
            mainAxisAlignment: MainAxisAlignment.spaceEvenly,
            children: <Widget>[
              const Spacer(
                flex: 3,
              ),
              OutlinedButton(
                onPressed: () {
                  // The following code will enable the wakelock on the device
                  // using the wakelock plugin.
                  setState(() {
                    WakelockPlus.enable();
                    // You could also use Wakelock.toggle(on: true);
                  });
                },
                child: const Text('enable wakelock'),
              ),
              const Spacer(),
              OutlinedButton(
                onPressed: () {
                  // The following code will disable the wakelock on the device
                  // using the wakelock plugin.
                  setState(() {
                    WakelockPlus.disable();
                    // You could also use Wakelock.toggle(on: false);
                  });
                },
                child: const Text('disable wakelock'),
              ),
              const Spacer(
                flex: 2,
              ),
              FutureBuilder(
                future: WakelockPlus.enabled,
                builder: (context, AsyncSnapshot<bool> snapshot) {
                  final data = snapshot.data;
                  // The use of FutureBuilder is necessary here to await the
                  // bool value from the `enabled` getter.
                  if (data == null) {
                    // The Future is retrieved so fast that you will not be able
                    // to see any loading indicator.
                    return Container();
                  }

                  return Text('The wakelock is currently '
                      '${data ? 'enabled' : 'disabled'}.');
                },
              ),
              const Spacer(
                flex: 3,
              ),
            ],
          ),
        ),
      ),
    );
  }
}
