// ignore_for_file: public_member_api_docs

import 'package:flutter/material.dart';
import 'client_server.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Cion Sample App',
      theme: ThemeData(
        primarySwatch: Colors.blue,
      ),
      home: const ClientServerApp(),
    );
  }
}
