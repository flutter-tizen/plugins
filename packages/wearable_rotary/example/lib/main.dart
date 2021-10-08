// ignore_for_file: public_member_api_docs

import 'package:flutter/material.dart';

import './custom_page_view.dart';

void main() => runApp(
      MaterialApp(
        title: 'Rotary example app',
        home: MyApp(),
      ),
    );

class MyApp extends StatelessWidget {
  final CustomPageView _horizontalPageView =
      const CustomPageView(Axis.horizontal);
  final CustomPageView _verticalPageView = const CustomPageView(Axis.vertical);
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Rotary example app'),
      ),
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: <Widget>[
            ElevatedButton(
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
            ElevatedButton(
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
