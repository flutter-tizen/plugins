// ignore_for_file: public_member_api_docs

import 'package:flutter/material.dart';

import 'custom_page_view.dart';

void main() {
  runApp(MaterialApp(
    title: 'Rotary example app',
    home: MyApp(),
  ));
}

class MyApp extends StatelessWidget {
  final Widget _horizontalPageView = const CustomPageView(Axis.horizontal);
  final Widget _verticalPageView = const CustomPageView(Axis.vertical);

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text('Rotary example app')),
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: <Widget>[
            ElevatedButton(
              child: const Text('HorizontalPageView'),
              onPressed: () {
                Navigator.push<dynamic>(
                  context,
                  MaterialPageRoute<dynamic>(
                      builder: (BuildContext context) => _horizontalPageView),
                );
              },
            ),
            const SizedBox(height: 10),
            ElevatedButton(
              child: const Text('VerticalPageView'),
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
