import 'dart:async';
import 'dart:io';

import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart' show rootBundle;
import 'package:flutter/services.dart';
import 'package:path/path.dart';
import 'package:sqflite/sqflite.dart';
import 'package:sqflite/src/database_mixin.dart' // ignore: implementation_imports
    show
        SqfliteDatabaseMixin;
import 'package:sqflite/src/factory_mixin.dart' // ignore: implementation_imports
    show
        SqfliteDatabaseFactoryMixin;
import 'package:sqflite_tizen_example/src/dev_utils.dart';
import 'package:synchronized/synchronized.dart';

import 'test_page.dart';

// ignore_for_file: avoid_slow_async_io
// ignore_for_file: avoid_print

/// Open callbacks.
class OpenCallbacks {
  /// Open callbacks.
  OpenCallbacks() {
    onConfigure = (Database db) {
      //print('onConfigure');
      //verify(!onConfigureCalled, 'onConfigure must be called once');
      expect(onConfigureCalled, false,
          reason:
              'onConfigure already called'); // onConfigure must be called once
      onConfigureCalled = true;
    };

    onCreate = (Database db, int version) {
      //print('onCreate');
      expect(onConfigureCalled, true, reason: 'onConfigure not called');
      expect(onCreateCalled, false, reason: 'onCreate already called');
      onCreateCalled = true;
    };

    onOpen = (Database db) {
      //print('onOpen');
      verify(onConfigureCalled, 'onConfigure must be called before onOpen');
      verify(!onOpenCalled!, 'onOpen already called');
      onOpenCalled = true;
    };

    onUpgrade = (Database db, int oldVersion, int newVersion) {
      verify(onConfigureCalled, 'onConfigure not called in onUpgrade');
      verify(!onUpgradeCalled!, 'onUpgradeCalled already called');
      onUpgradeCalled = true;
    };

    onDowngrade = (Database db, int oldVersion, int newVersion) {
      verify(onConfigureCalled, 'onConfigure not called');
      verify(!onDowngradeCalled!, 'onDowngrade already called');
      onDowngradeCalled = true;
    };

    reset();
  }

  /// true when onConfigure is called.
  bool? onConfigureCalled;

  /// true when onOpen is called.
  bool? onOpenCalled;

  /// true when onCreate is called.
  bool? onCreateCalled;

  /// true when onDowngrade is called.
  bool? onDowngradeCalled;

  /// true when onUpgrade is called.
  bool? onUpgradeCalled;

  /// onCreate callback.
  late OnDatabaseCreateFn onCreate;

  /// onConfigure callback.
  OnDatabaseConfigureFn? onConfigure;

  /// onDowngrade callback.
  late OnDatabaseVersionChangeFn onDowngrade;

  /// onUpgrade callback.
  late OnDatabaseVersionChangeFn onUpgrade;

  /// onOpen callback.
  late OnDatabaseOpenFn onOpen;

  /// reset callbacks called information.
  void reset() {
    onConfigureCalled = false;
    onOpenCalled = false;
    onCreateCalled = false;
    onDowngradeCalled = false;
    onUpgradeCalled = false;
  }

  /// open the database.
  Future<Database> open(String path, {required int version}) async {
    reset();
    return await databaseFactory.openDatabase(path,
        options: OpenDatabaseOptions(
            version: version,
            onCreate: onCreate,
            onConfigure: onConfigure!,
            onDowngrade: onDowngrade,
            onUpgrade: onUpgrade,
            onOpen: onOpen));
  }
}

/// Check if a file is a valid database file
///
/// An empty file is a valid empty sqlite file
Future<bool> isDatabase(String path) async {
  Database? db;
  var isDatabase = false;
  try {
    db = await openReadOnlyDatabase(path);
    await db.getVersion();
    isDatabase = true;
  } catch (_) {
  } finally {
    await db?.close();
  }
  return isDatabase;
}

/// Open test page.
class OpenTestPage extends TestPage {
  /// Open test page.
  OpenTestPage({Key? key}) : super('Open tests', key: key) {
    final factory = databaseFactory;

    test('Databases path', () async {
      final databasesPath = await factory.getDatabasesPath();
      // On Android we know it is current a 'databases' folder in the package folder
      print('databasesPath: $databasesPath');
      if (Platform.isAndroid) {
        expect(basename(databasesPath), 'databases');
      } else if (Platform.isIOS) {
        expect(basename(databasesPath), 'Documents');
      }
      final path = join(databasesPath, 'in_default_directory.db');
      await factory.deleteDatabase(path);
      final db = await factory.openDatabase(path);
      await db.close();
    });
    test('Delete database', () async {
      final path = await initDeleteDb('delete_database.db');
      expect(await databaseExists(path), false);
      final db = await openDatabase(path);
      await db.close();
      expect((await File(path).exists()), true);
      expect(await databaseExists(path), true);
      print('Deleting database $path');
      await deleteDatabase(path);
      expect((await File(path).exists()), false);
      expect(await databaseExists(path), false);
    });

    test('Open no version', () async {
      final path = await initDeleteDb('open_no_version.db');
      expect((await File(path).exists()), false);
      final db = await openDatabase(path);
      verify(await File(path).exists());
      expect(await db.getVersion(), 0);
      await db.close();
    });

    test('isOpen', () async {
      final path = await initDeleteDb('is_open.db');
      expect((await File(path).exists()), false);
      final db = await openDatabase(path);
      expect(db.isOpen, true);
      verify(await File(path).exists());
      await db.close();
      expect(db.isOpen, false);
    });

    test('Open no version onCreate', () async {
      // should fail
      final path = await initDeleteDb('open_no_version_on_create.db');
      verify(!(await File(path).exists()));
      Database? db;
      try {
        db = await openDatabase(path, onCreate: (Database db, int version) {
          // never called
          verify(false);
        });
        verify(false);
      } on ArgumentError catch (_) {}
      verify(!await File(path).exists());
      expect(db, null);
    });

    test('Open onCreate', () async {
      final path = await initDeleteDb('open_test2.db');
      var onCreate = false;
      var onCreateTransaction = false;
      final db = await openDatabase(path, version: 1,
          onCreate: (Database db, int version) async {
        expect(version, 1);
        onCreate = true;

        await db.transaction((txn) async {
          await txn.execute('CREATE TABLE Test2 (id INTEGER PRIMARY KEY)');
          onCreateTransaction = true;
        });
      });
      verify(onCreate);
      expect(onCreateTransaction, true);
      expect(await db.getVersion(), 1);
      await db.close();
    });

    test('Simple onCreate', () async {
      final path = await initDeleteDb('open_simple_on_create.db');
      expect(await isDatabase(path), isFalse);

      final db =
          await openDatabase(path, version: 1, onCreate: (db, version) async {
        final batch = db.batch();

        batch.execute('CREATE TABLE Test (id INTEGER PRIMARY KEY, text NAME)');
        await batch.commit();
      });
      try {
        expect(
            await db.rawInsert('INSERT INTO Test (text) VALUES (?)', ['test']),
            1);
        final result = await db.query('Test');
        final expected = [
          {'id': 1, 'text': 'test'}
        ];
        expect(result, expected);

        expect(await isDatabase(path), isTrue);
      } finally {
        await db.close();
      }
      expect(await isDatabase(path), isTrue);
    });

    test('Open 2 databases', () async {
      final path1 = await initDeleteDb('open_db_1.db');
      final path2 = await initDeleteDb('open_db_2.db');
      final db1 = await openDatabase(path1, version: 1);
      final db2 = await openDatabase(path2, version: 1);
      await db1.close();
      await db2.close();
    });

    test('Open onUpgrade', () async {
      var onUpgrade = false;
      final path = await initDeleteDb('open_on_upgrade.db');
      var database = await openDatabase(path, version: 1,
          onCreate: (Database db, int version) async {
        await db.execute('CREATE TABLE Test(id INTEGER PRIMARY KEY)');
      });
      try {
        await database
            .insert('Test', <String, Object?>{'id': 1, 'name': 'test'});
        fail('should fail');
      } on DatabaseException catch (e) {
        print(e);
      }
      expect(await database.getVersion(), 1);
      await database.close();
      database = await openDatabase(path, version: 2,
          onUpgrade: (Database db, int oldVersion, int newVersion) async {
        expect(oldVersion, 1);
        expect(newVersion, 2);
        await db.execute('ALTER TABLE Test ADD name TEXT');
        onUpgrade = true;
      });
      verify(onUpgrade);
      expect(await database.getVersion(), 2);
      try {
        expect(
            await database
                .insert('Test', <String, Object?>{'id': 1, 'name': 'test'}),
            1);
      } finally {
        await database.close();
      }
    });

    test('Open onDowngrade', () async {
      final path = await initDeleteDb('open_on_downgrade.db');
      var database = await openDatabase(path, version: 2,
          onCreate: (Database db, int version) async {
        await db.execute('CREATE TABLE Test(id INTEGER PRIMARY KEY)');
      }, onDowngrade: (Database db, int oldVersion, int newVersion) async {
        verify(false, 'should not be called');
      });
      expect(await database.getVersion(), 2);
      await database.close();

      var onDowngrade = false;
      database = await openDatabase(path, version: 1,
          onDowngrade: (Database db, int oldVersion, int newVersion) async {
        expect(oldVersion, 2);
        expect(newVersion, 1);
        await db.execute('ALTER TABLE Test ADD name TEXT');
        onDowngrade = true;
      });
      verify(onDowngrade);
      expect(await database.getVersion(), 1);

      await database.close();
    });

    test('Open bad path', () async {
      try {
        await openDatabase('/invalid_path');
        fail();
      } on DatabaseException catch (e) {
        verify(e.isOpenFailedError());
      }
    });

    test('Open asset database', () async {
      final databasesPath = await getDatabasesPath();
      final path = join(databasesPath, 'asset_example.db');

      // delete existing if any
      await deleteDatabase(path);

      // Make sure the parent directory exists
      try {
        await Directory(dirname(path)).create(recursive: true);
      } catch (_) {}

      // Copy from asset
      final data = await rootBundle.load(join('assets', 'example.db'));
      final List<int> bytes =
          data.buffer.asUint8List(data.offsetInBytes, data.lengthInBytes);
      // Write and flush the bytes written
      await File(path).writeAsBytes(bytes, flush: true);

      // open the database
      final db = await openDatabase(path);

      // Our database as a single table with a single element
      final list = await db.rawQuery('SELECT * FROM Test');
      print('list $list');
      expect(list.first['name'], 'simple value');

      await db.close();
    });

    test('Open on configure', () async {
      final path = await initDeleteDb('open_on_configure.db');

      var onConfigured = false;
      var onConfiguredTransaction = false;
      Future _onConfigure(Database db) async {
        onConfigured = true;
        await db.execute('CREATE TABLE Test1 (id INTEGER PRIMARY KEY)');
        await db.transaction((txn) async {
          await txn.execute('CREATE TABLE Test2 (id INTEGER PRIMARY KEY)');
          onConfiguredTransaction = true;
        });
      }

      final db = await openDatabase(path, onConfigure: _onConfigure);
      expect(onConfigured, true);
      expect(onConfiguredTransaction, true);

      await db.close();
    });

    test('Open onDowngrade delete', () async {
      final path = await initDeleteDb('open_on_downgrade_delete.db');
      var database = await openDatabase(path, version: 3,
          onCreate: (Database db, int version) async {
        await db.execute('CREATE TABLE Test(id INTEGER PRIMARY KEY)');
      });
      await database.close();

      // should fail going back in versions
      var onCreated = false;
      var onOpened = false;
      var onConfiguredOnce = false; // onConfigure will be called twice here
      // since the database is re-opened
      var onConfigured = false;
      database =
          await openDatabase(path, version: 2, onConfigure: (Database db) {
        // Must not be configured nor created yet
        verify(!onConfigured);
        verify(!onCreated);
        if (!onConfiguredOnce) {
          // first time
          onConfiguredOnce = true;
        } else {
          onConfigured = true;
        }
      }, onCreate: (Database db, int version) {
        verify(onConfigured);
        verify(!onCreated);
        verify(!onOpened);
        onCreated = true;
        expect(version, 2);
      }, onOpen: (Database db) {
        verify(onCreated);
        onOpened = true;
      }, onDowngrade: onDatabaseDowngradeDelete);
      await database.close();

      expect(onCreated, true);
      expect(onOpened, true);
      expect(onConfigured, true);

      onCreated = false;
      onOpened = false;

      database = await openDatabase(path, version: 2,
          onCreate: (Database db, int version) {
        expect(false, 'should not be called');
      }, onOpen: (Database db) {
        onOpened = true;
      }, onDowngrade: onDatabaseDowngradeDelete);
      expect(onOpened, true);
      await database.close();
    });

    test('All open callback', () async {
      final path = await initDeleteDb('open_all_callbacks.db');

      var step = 1;
      final openCallbacks = OpenCallbacks();
      var db = await openCallbacks.open(path, version: 1);
      verify(openCallbacks.onConfigureCalled, 'onConfiguredCalled $step');
      verify(openCallbacks.onCreateCalled, 'onCreateCalled $step');
      verify(openCallbacks.onOpenCalled, 'onOpenCalled $step');
      verify(!openCallbacks.onUpgradeCalled!, 'onUpgradeCalled $step');
      verify(!openCallbacks.onDowngradeCalled!, 'onDowngradCalled $step');
      await db.close();

      ++step;
      db = await openCallbacks.open(path, version: 3);
      verify(openCallbacks.onConfigureCalled, 'onConfiguredCalled $step');
      verify(!openCallbacks.onCreateCalled!, 'onCreateCalled $step');
      verify(openCallbacks.onOpenCalled, 'onOpenCalled $step');
      verify(openCallbacks.onUpgradeCalled, 'onUpgradeCalled $step');
      verify(!openCallbacks.onDowngradeCalled!, 'onDowngradCalled $step');
      await db.close();

      ++step;
      db = await openCallbacks.open(path, version: 2);
      verify(openCallbacks.onConfigureCalled, 'onConfiguredCalled $step');
      verify(!openCallbacks.onCreateCalled!, 'onCreateCalled $step');
      verify(openCallbacks.onOpenCalled, 'onOpenCalled $step');
      verify(!openCallbacks.onUpgradeCalled!, 'onUpgradeCalled $step');
      verify(openCallbacks.onDowngradeCalled, 'onDowngradCalled $step');
      await db.close();

      openCallbacks.onDowngrade = onDatabaseDowngradeDelete;
      var configureCount = 0;
      final callback = openCallbacks.onConfigure;
      // allow being called twice
      openCallbacks.onConfigure = (Database db) {
        if (configureCount == 1) {
          openCallbacks.onConfigureCalled = false;
        }
        configureCount++;
        callback!(db);
      };
      ++step;
      db = await openCallbacks.open(path, version: 1);

      /*
      verify(openCallbacks.onConfigureCalled,'onConfiguredCalled $step');
      verify(configureCount == 2, 'onConfigure count');
      verify(openCallbacks.onCreateCalled, 'onCreateCalled $step');
      verify(openCallbacks.onOpenCalled, 'onOpenCalled $step');
      verify(!openCallbacks.onUpgradeCalled, 'onUpgradeCalled $step');
      verify(!openCallbacks.onDowngradeCalled, 'onDowngradCalled $step');
      */
      await db.close();
    });

    test('Open batch', () async {
      final path = await initDeleteDb('open_batch.db');

      Future _onConfigure(Database db) async {
        final batch = db.batch();
        batch.execute('CREATE TABLE Test (id INTEGER PRIMARY KEY, value TEXT)');
        await batch.commit();
      }

      Future _onCreate(Database db, int version) async {
        final batch = db.batch();
        batch.rawInsert('INSERT INTO Test(value) VALUES("value1")');
        await batch.commit();
      }

      Future _onOpen(Database db) async {
        final batch = db.batch();
        batch.rawInsert('INSERT INTO Test(value) VALUES("value2")');
        await batch.commit();
      }

      final db = await openDatabase(path,
          version: 1,
          onConfigure: _onConfigure,
          onCreate: _onCreate,
          onOpen: _onOpen);
      expect(
          Sqflite.firstIntValue(await db.rawQuery('SELECT COUNT(*) FROM Test')),
          2);

      await db.close();
    });

    test('Open read-only', () async {
      final path = await initDeleteDb('open_read_only.db');

      Future _onCreate(Database db, int version) async {
        final batch = db.batch();
        batch.execute('CREATE TABLE Test (id INTEGER PRIMARY KEY, value TEXT)');
        batch.rawInsert('INSERT INTO Test(value) VALUES("value1")');
        await batch.commit();
      }

      var db = await openDatabase(path, version: 1, onCreate: _onCreate);
      expect(
          Sqflite.firstIntValue(await db.rawQuery('SELECT COUNT(*) FROM Test')),
          1);

      await db.close();

      db = await openReadOnlyDatabase(path);
      expect(
          Sqflite.firstIntValue(await db.rawQuery('SELECT COUNT(*) FROM Test')),
          1);

      try {
        await db.rawInsert('INSERT INTO Test(value) VALUES("value1")');
        fail('should fail');
      } on DatabaseException catch (e) {
        // Error DatabaseException(attempt to write a readonly database (code 8)) running Open read-only
        expect(e.isReadOnlyError(), true);
      }

      final batch = db.batch();
      batch.rawQuery('SELECT COUNT(*) FROM Test');
      await batch.commit();

      await db.close();
    });

    test('Open demo (doc)', () async {
      final path = await initDeleteDb('open_read_only.db');

      {
        Future _onConfigure(Database db) async {
          // Add support for cascade delete
          await db.execute('PRAGMA foreign_keys = ON');
        }

        final db = await openDatabase(path, onConfigure: _onConfigure);
        await db.close();
      }

      {
        Future _onCreate(Database db, int version) async {
          // Database is created, delete the table
          await db.execute(
              'CREATE TABLE Test (id INTEGER PRIMARY KEY, value TEXT)');
        }

        Future _onUpgrade(Database db, int oldVersion, int newVersion) async {
          // Database version is updated, alter the table
          await db.execute('ALTER TABLE Test ADD name TEXT');
        }

        // Special callback used for onDowngrade here to recreate the database
        final db = await openDatabase(path,
            version: 1,
            onCreate: _onCreate,
            onUpgrade: _onUpgrade,
            onDowngrade: onDatabaseDowngradeDelete);
        await db.close();
      }

      {
        Future _onOpen(Database db) async {
          // Database is open, print its version
          print('db version ${await db.getVersion()}');
        }

        final db = await openDatabase(
          path,
          onOpen: _onOpen,
        );
        await db.close();
      }

      // asset (use existing copy if any
      {
        // Check if we have an existing copy first
        final databasesPath = await getDatabasesPath();
        final path = join(databasesPath, 'demo_asset_example.db');

        // try opening (will work if it exists)
        Database? db;
        try {
          db = await openDatabase(path, readOnly: true);
        } catch (e) {
          print('Error $e');
        }

        if (db == null) {
          // Should happen only the first time you launch your application
          print('Creating new copy from asset');

          // Copy from asset
          final data = await rootBundle.load(join('assets', 'example.db'));
          final bytes =
              data.buffer.asUint8List(data.offsetInBytes, data.lengthInBytes);
          await File(path).writeAsBytes(bytes);

          // open the database
          db = await openDatabase(path, readOnly: true);
        } else {
          print('Opening existing database');
        }

        await db.close();
      }
    });

    test('Database locked (doc)', () async {
      final path = await initDeleteDb('open_locked.db');
      final helper = Helper(path);

      // without the synchronized fix, this could faild
      for (var i = 0; i < 100; i++) {
        // ignore: unawaited_futures
        helper.getDb();
      }
      final db = await helper.getDb();
      await db.close();
    });

    test('single/multi instance (using factory)', () async {
      final path = await initDeleteDb('instances_test.db');
      Database? db1, db2, db3;
      try {
        db1 = await databaseFactory.openDatabase(path,
            options: OpenDatabaseOptions(singleInstance: false));
        db2 = await databaseFactory.openDatabase(path,
            options: OpenDatabaseOptions(singleInstance: true));
        db3 = await databaseFactory.openDatabase(path,
            options: OpenDatabaseOptions(singleInstance: true));
        expect(db1, isNot(db2));
        expect(db2, db3);
      } finally {
        await db1!.close();
        await db2!.close();
        await db3!.close(); // safe to close the same instance
      }
    });

    test('single/multi instance', () async {
      final path = await initDeleteDb('instances_test.db');
      final db1 = await openDatabase(path, singleInstance: false);
      final db2 = await openDatabase(path, singleInstance: true);
      final db3 = await openDatabase(path, singleInstance: true);
      verify(db1 != db2);
      verify(db2 == db3);
      await db1.close();
      await db2.close();
      await db3.close(); // safe to close the same instance
    });

    test('In memory database', () async {
      const inMemoryPath =
          inMemoryDatabasePath; // tried null without success, as it crashes on Android
      const path = inMemoryPath;

      var db = await openDatabase(path);
      await db
          .execute('CREATE TABLE IF NOT EXISTS Test(id INTEGER PRIMARY KEY)');
      await db.insert('Test', {'id': 1});
      expect(await db.query('Test'), [
        {'id': 1}
      ]);
      await db.close();

      // reopen, content should be gone
      db = await openDatabase(path);
      try {
        await db.query('Test');
        fail('fail');
      } on DatabaseException catch (e) {
        print(e);
      }
      await db.close();
    });

    test('Not in memory database', () async {
      final path = await initDeleteDb('not_in_memory.db');

      var db = await openDatabase(path);
      await db
          .execute('CREATE TABLE IF NOT EXISTS Test(id INTEGER PRIMARY KEY)');
      await db.insert('Test', {'id': 1});
      expect(await db.query('Test'), [
        {'id': 1}
      ]);
      await db.close();

      // reopen, content should be done
      db = await openDatabase(path);
      expect(await db.query('Test'), [
        {'id': 1}
      ]);
      await db.close();
    });

    test('open in sub directory fails', () async {
      final databasesPath = await factory.getDatabasesPath();
      final path = join(databasesPath, 'sub_that_should_not_exists');
      try {
        await Directory(path).delete(recursive: true);
      } catch (_) {}
      final dbPath = join(path, 'open.db');
      try {
        await factory.openDatabase(dbPath);
      } catch (e) {
        print("couldn't open database in sub directory");
      }
    });

    test('open in sub sub directory fails', () async {
      final databasesPath = await factory.getDatabasesPath();
      final path =
          join(databasesPath, 'sub2_that_should_not_exists', 'sub_sub');
      try {
        await Directory(path).delete(recursive: true);
      } catch (_) {}
      expect(await Directory(path).exists(), false);
      final dbPath = join(path, 'open.db');
      try {
        await factory.openDatabase(dbPath);
      } catch (e) {
        print("couldn't open database in sub sub directory");
      }
    });

    test('open_close_open_no_wait', () async {
      const path = 'open_close_open_no_wait.db';
      final factory = databaseFactory;
      await factory.deleteDatabase(path);
      final db = await factory.openDatabase(path,
          options: OpenDatabaseOptions(version: 1));
      try {
        expect(await db.getVersion(), 1);
        // close no wait
        unawaited(db.close());
        final db2 = await factory.openDatabase(path,
            options: OpenDatabaseOptions(version: 1));
        print('$db, $db2');
        verify(db != db2);
        verify((db as SqfliteDatabaseMixin).id !=
            (db2 as SqfliteDatabaseMixin).id);
        expect(await db2.getVersion(), 1);
      } finally {
        await db.close();
      }
    });
    test('close in transaction', () async {
      const path = 'test_close_in_transaction.db';
      final factory = databaseFactory;
      await factory.deleteDatabase(path);
      var db = await factory.openDatabase(path,
          options: OpenDatabaseOptions(version: 1));
      try {
        //await db.getVersion();
        await db.execute('BEGIN IMMEDIATE');
        await db.close();

        db = await factory.openDatabase(path,
            options: OpenDatabaseOptions(version: 1));
      } finally {
        await db.close();
      }
    });

    test('open in transaction', () async {
      const path = 'test_close_in_transaction.db';
      final factory = databaseFactory;
      await factory.deleteDatabase(path);
      var db = await factory.openDatabase(path,
          options: OpenDatabaseOptions(version: 1));
      try {
        //await db.getVersion();
        await db.execute('BEGIN IMMEDIATE');
        // Trick to make sure we don't reuse the same instance during open
        (factory as SqfliteDatabaseFactoryMixin)
            .databaseOpenHelpers
            .remove(db.path);

        final db2 = await factory.openDatabase(path,
            options: OpenDatabaseOptions(version: 1));
        print('after open');
        verify(db != db2);
        expect(
            (db as SqfliteDatabaseMixin).id, (db2 as SqfliteDatabaseMixin).id);
        //await db.getVersion();
        //await db.execute('ROLLBACK');

        db = await factory.openDatabase(path,
            options: OpenDatabaseOptions(version: 1));
        expect(db, db2);
      } finally {
        await db.close();
      }
    });

    test('Open non sqlite file', () async {
      // Kind of corruption simulation
      final factory = databaseFactory;
      final path =
          join(await factory.getDatabasesPath(), 'test_non_sqlite_file.db');

      await factory.deleteDatabase(path);
      // Write dummy content
      await File(path).writeAsString('dummy', flush: true);
      // check content
      expect(await File(path).readAsString(), 'dummy');

      // try read-only
      {
        late Database db;
        try {
          db = await factory.openDatabase(path,
              options: OpenDatabaseOptions(readOnly: true));
        } catch (e) {
          print('open error');
        }
        try {
          await db.getVersion();
        } catch (e) {
          print('getVersion error');
        }
        await db.close();

        // check content
        expect(await File(path).readAsString(), 'dummy');
      }

      expect(await isDatabase(path), isFalse);
      // try read-write
      const minExpectedSize = 1000;
      expect(
          (await File(path).readAsBytes()).length, lessThan(minExpectedSize));

      var db = await factory.openDatabase(path);
      try {
        await db.getVersion();
      } catch (e) {
        print('getVersion error');
      }
      await db.close();

      try {
        db = await factory.openDatabase(path,
            options: OpenDatabaseOptions(version: 1));
      } catch (e) {
        print('openDatabase error');
      }
      await db.close();
    });
  }
}

/// Open helper.
class Helper {
  /// Open helper.
  Helper(this.path);

  /// Datebase path.
  final String path;
  Database? _db;
  final _lock = Lock();

  /// Open the database if not done.
  Future<Database> getDb() async {
    if (_db == null) {
      await _lock.synchronized(() async {
        // Check again once entering the synchronized block
        _db ??= await openDatabase(path);
      });
    }
    return _db!;
  }
}
