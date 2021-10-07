// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:io';
import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';
import 'package:path_provider/path_provider.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  testWidgets('getTemporaryDirectory', (WidgetTester tester) async {
    final Directory result = await getTemporaryDirectory();
    _verifySampleFile(result, 'temporaryDirectory');
  });

  testWidgets('getApplicationDocumentsDirectory', (WidgetTester tester) async {
    final Directory result = await getApplicationDocumentsDirectory();
    _verifySampleFile(result, 'applicationDocuments');
  });

  testWidgets('getApplicationSupportDirectory', (WidgetTester tester) async {
    final Directory result = await getApplicationSupportDirectory();
    _verifySampleFile(result, 'applicationSupport');
  });

  testWidgets('getLibraryDirectory', (WidgetTester tester) async {
    if (Platform.isIOS) {
      final Directory result = await getLibraryDirectory();
      _verifySampleFile(result, 'library');
    } else if (Platform.isAndroid) {
      final Future<Directory?> result = getLibraryDirectory();
      expect(result, throwsA(isInstanceOf<UnsupportedError>()));
    }
  });

  testWidgets('getExternalStorageDirectory', (WidgetTester tester) async {
    if (Platform.isIOS) {
      final Future<Directory?> result = getExternalStorageDirectory();
      expect(result, throwsA(isInstanceOf<UnsupportedError>()));
    } else if (Platform.isAndroid) {
      final Directory? result = await getExternalStorageDirectory();
      _verifySampleFile(result, 'externalStorage');
    }
  });

  testWidgets('getExternalCacheDirectories', (WidgetTester tester) async {
    if (Platform.isIOS) {
      final Future<List<Directory>?> result = getExternalCacheDirectories();
      expect(result, throwsA(isInstanceOf<UnsupportedError>()));
    } else if (Platform.isAndroid) {
      final List<Directory>? directories = await getExternalCacheDirectories();
      expect(directories, isNotNull);
      for (final Directory result in directories!) {
        _verifySampleFile(result, 'externalCache');
      }
    }
  });

  final List<StorageDirectory?> _allDirs = <StorageDirectory?>[
    null,
    StorageDirectory.music,
    StorageDirectory.podcasts,
    StorageDirectory.alarms,
    StorageDirectory.notifications,
    StorageDirectory.pictures,
    StorageDirectory.movies,
  ];

  for (final StorageDirectory? type in _allDirs) {
    test('getExternalStorageDirectories (type: $type)', () async {
      if (Platform.isIOS) {
        final Future<List<Directory>?> result =
            getExternalStorageDirectories(type: null);
        expect(result, throwsA(isInstanceOf<UnsupportedError>()));
      } else if (Platform.isAndroid) {
        final List<Directory>? directories =
            await getExternalStorageDirectories(type: type);
        expect(directories, isNotNull);
        for (final Directory result in directories!) {
          _verifySampleFile(result, '$type');
        }
      } else {
        // Tizen platform
        // Permissions for accessing media directories are granted
        // for TV by default, but not for wearables.
        // But the example app can't depend on permission_handler because
        // TV doesn't support permission_handler:
        //   https://github.com/flutter-tizen/plugins#device-limitations
        // Uncomment and remove the skip option when it's possible
        // to choose device types in testing.

        // if (!await Permission.mediaLibrary.isGranted) {
        //   await Permission.mediaLibrary.request();
        // }

        // final List<Directory>? directories =
        //     await getExternalStorageDirectories(type: type);
        // expect(directories, isNotNull);
        // for (final Directory result in directories!) {
        //   _verifySampleFile(result, '$type');
        // }
      }
    }, skip: true);
  }
}

/// Verify a file called [name] in [directory] by recreating it with test
/// contents when necessary.
void _verifySampleFile(Directory? directory, String name) {
  expect(directory, isNotNull);
  if (directory == null) {
    return;
  }
  final File file = File('${directory.path}/$name');

  if (file.existsSync()) {
    file.deleteSync();
    expect(file.existsSync(), isFalse);
  }

  file.writeAsStringSync('Hello world!');
  expect(file.readAsStringSync(), 'Hello world!');
  expect(directory.listSync(), isNotEmpty);
  file.deleteSync();
}
