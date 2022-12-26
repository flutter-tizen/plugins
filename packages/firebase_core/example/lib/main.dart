// Copyright (c) Invertase Limited <oss@invertase.io> & Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: public_member_api_docs

import 'dart:async';
import 'dart:developer';
import 'package:firebase_core/firebase_core.dart';
import 'package:flutter/material.dart';

void main() => runApp(const MyApp());

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  String get name => 'foo';

  FirebaseOptions get firebaseOptions => const FirebaseOptions(
        appId: '1:448618578101:ios:0b650370bb29e29cac3efc',
        apiKey: 'AIzaSyAgUhHU8wSJgO5MVNy95tMT07NEjzMOfz0',
        projectId: 'react-native-firebase-testing',
        messagingSenderId: '448618578101',
      );

  Future<void> initializeDefault() async {
    final FirebaseApp app =
        await Firebase.initializeApp(options: firebaseOptions);
    log('Initialized default app $app');
    print('Initialized default app $app');
  }

  Future<void> initializeSecondary() async {
    final FirebaseApp app = await Firebase.initializeApp(
      name: name,
      options: firebaseOptions,
    );

    log('Initialized $app');
    print('Initialized $app');
  }

  void apps() {
    log('Currently initialized apps: ${Firebase.apps}');
    print('Currently initialized apps: ${Firebase.apps}');
  }

  void options() {
    final FirebaseApp app = Firebase.app(name);
    final FirebaseOptions options = app.options;
    log('Current options for app $name: $options');
    print('Current options for app $name: $options');
  }

  Future<void> delete() async {
    final FirebaseApp app = Firebase.app(name);
    await app.delete();
    log('App $name deleted');
    print('App $name deleted');
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: const Text('Firebase Core example app'),
        ),
        body: Padding(
          padding: const EdgeInsets.all(20),
          child: Column(
            mainAxisAlignment: MainAxisAlignment.spaceAround,
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: <Widget>[
              ElevatedButton(
                onPressed: initializeDefault,
                child: const Text('Initialize default app'),
              ),
              ElevatedButton(
                onPressed: initializeSecondary,
                child: const Text('Initialize secondary app'),
              ),
              ElevatedButton(
                onPressed: apps,
                child: const Text('Get apps'),
              ),
              ElevatedButton(
                onPressed: options,
                child: const Text('List options'),
              ),
              ElevatedButton(
                onPressed: delete,
                child: const Text('Delete app'),
              ),
            ],
          ),
        ),
      ),
    );
  }
}
