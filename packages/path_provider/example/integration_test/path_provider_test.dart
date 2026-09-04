// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:io';
import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';
import 'package:path_provider/path_provider.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  test('getTemporaryDirectory', () async {
    final Directory result = await getTemporaryDirectory();
    _verifySampleFile(result, 'temporaryDirectory');
  });

  test('getApplicationDocumentsDirectory', () async {
    final Directory result = await getApplicationDocumentsDirectory();
    if (Platform.isMacOS) {
      // _verifySampleFile causes hangs in driver when sandboxing is disabled
      // because the path changes from an app specific directory to
      // ~/Documents, which requires additional permissions to access on macOS.
      // Instead, validate that a non-empty path was returned.
      expect(result.path, isNotEmpty);
    } else {
      _verifySampleFile(result, 'applicationDocuments');
    }
  });

  test('getApplicationSupportDirectory', () async {
    final Directory result = await getApplicationSupportDirectory();
    _verifySampleFile(result, 'applicationSupport');
  });

  test('getApplicationCacheDirectory', () async {
    final Directory result = await getApplicationCacheDirectory();
    _verifySampleFile(result, 'applicationCache');
  });

  test('getLibraryDirectory', () async {
    if (Platform.isIOS) {
      final Directory result = await getLibraryDirectory();
      _verifySampleFile(result, 'library');
    } else if (Platform.isAndroid) {
      final Future<Directory?> result = getLibraryDirectory();
      await expectLater(result, throwsA(isInstanceOf<UnsupportedError>()));
    }
  });

  test('getExternalStorageDirectory', () async {
    if (Platform.isIOS) {
      final Future<Directory?> result = getExternalStorageDirectory();
      await expectLater(result, throwsA(isInstanceOf<UnsupportedError>()));
    } else if (Platform.isAndroid) {
      final Directory? result = await getExternalStorageDirectory();
      _verifySampleFile(result, 'externalStorage');
    }
  });

  test('getExternalCacheDirectories', () async {
    if (Platform.isIOS) {
      final Future<List<Directory>?> result = getExternalCacheDirectories();
      await expectLater(result, throwsA(isInstanceOf<UnsupportedError>()));
    } else if (Platform.isAndroid) {
      final List<Directory>? directories = await getExternalCacheDirectories();
      expect(directories, isNotNull);
      for (final Directory result in directories!) {
        _verifySampleFile(result, 'externalCache');
      }
    }
  });

  test('getDownloadsDirectory', () async {
    final Directory? result = await getDownloadsDirectory();
    if (result != null) {
      _verifySampleFile(result, 'downloads');
    }
  });

  final List<StorageDirectory?> allDirs = <StorageDirectory?>[
    null,
    StorageDirectory.music,
    StorageDirectory.podcasts,
    StorageDirectory.ringtones,
    StorageDirectory.alarms,
    StorageDirectory.notifications,
    StorageDirectory.pictures,
    StorageDirectory.movies,
  ];

  for (final StorageDirectory? type in allDirs) {
    test('getExternalStorageDirectories (type: $type)', () async {
      if (Platform.isIOS) {
        final Future<List<Directory>?> result = getExternalStorageDirectories();
        await expectLater(result, throwsA(isInstanceOf<UnsupportedError>()));
      } else if (Platform.isAndroid) {
        final List<Directory>? directories =
            await getExternalStorageDirectories(type: type);
        expect(directories, isNotNull);
        for (final Directory result in directories!) {
          _verifySampleFile(result, '$type');
        }
      }
    });
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
  // This check intentionally avoids using Directory.listSync on Android due to
  // https://github.com/dart-lang/sdk/issues/54287.
  if (Platform.isAndroid) {
    expect(
      Process.runSync('ls', <String>[directory.path]).stdout,
      contains(name),
    );
  } else {
    expect(directory.listSync(), isNotEmpty);
  }
  file.deleteSync();
}
