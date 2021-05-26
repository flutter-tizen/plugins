import 'dart:async';
import 'package:flutter/material.dart';

import './custom_page_view.dart';

void main() => runApp(
      MaterialApp(
        title: 'Rotary example app',
        home: MyApp(),
      ),
    );

class MyApp extends StatelessWidget {
  final CustomPageView _horizontalPageView = CustomPageView(Axis.horizontal);
  final CustomPageView _verticalPageView = CustomPageView(Axis.vertical);
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Rotary example app'),
      ),
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            RaisedButton(
              padding: const EdgeInsets.all(20.0),
              child: const Text(
                'HorizontalPageView',
                style: TextStyle(fontSize: 15),
              ),
              onPressed: () {
                Navigator.push<dynamic>(
                  context,
                  MaterialPageRoute<dynamic>(
                      builder: (BuildContext context) => _horizontalPageView),
                );
              },
            ),
            RaisedButton(
              padding: const EdgeInsets.all(20.0),
              child: const Text(
                'VerticalPageView',
                style: TextStyle(fontSize: 15),
              ),
              onPressed: () {
                Navigator.push<dynamic>(
                  context,
                  MaterialPageRoute<dynamic>(
                      builder: (BuildContext context) => _verticalPageView),
                );
              },
            ),
          ],
        ),
      ),
    );
  }
}
