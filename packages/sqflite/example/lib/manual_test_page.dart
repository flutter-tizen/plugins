import 'dart:async';

import 'package:flutter/material.dart';
import 'package:sqflite/sqflite.dart';
// ignore: implementation_imports
import 'package:sqflite/src/factory_mixin.dart' as impl;
import 'package:sqflite/utils/utils.dart';
import 'package:sqflite_tizen_example/model/item.dart';
import 'package:sqflite_tizen_example/src/item_widget.dart';
import 'package:sqflite_tizen_example/utils.dart';

// ignore_for_file: avoid_print

/// Manual test page.
class ManualTestPage extends StatefulWidget {
  /// Test page.
  const ManualTestPage({Key? key}) : super(key: key);

  @override
  _ManualTestPageState createState() => _ManualTestPageState();
}

class _ManualTestPageState extends State<ManualTestPage> {
  Database? database;
  static const String dbName = 'manual_test.db';

  Future<Database> _openDatabase() async {
    return database ??= await databaseFactory.openDatabase(dbName);
  }

  Future _closeDatabase() async {
    await database?.close();
    database = null;
  }

  Future _deleteDatabase() async {
    await databaseFactory.deleteDatabase(dbName);
  }

  Future _incrementVersion() async {
    final version = await database?.getVersion() ?? 0;
    print('version $version');
    await database?.setVersion(version + 1);
  }

  late List<MenuItem> items;
  late List<ItemWidget> itemWidgets;

  Future<bool> pop() async {
    return true;
  }

  @override
  void didChangeDependencies() {
    super.didChangeDependencies();
    items = <MenuItem>[
      MenuItem('openDatabase', () async {
        await _openDatabase();
      }, summary: 'Open the database'),
      MenuItem('BEGIN EXCLUSIVE', () async {
        final db = await _openDatabase();
        await db.execute('BEGIN EXCLUSIVE');
      },
          summary:
              'Execute than exit or hot-restart the application. Open the database if needed'),
      MenuItem('close', () async {
        await _closeDatabase();
      },
          summary:
              'Execute after starting then exit the app using the back button on Android and restart from the launcher.'),
      MenuItem('delete', () async {
        await _deleteDatabase();
      },
          summary:
              'Try open (then optionally) delete, exit or hot-restart then delete then open'),
      MenuItem('log level: none', () async {
        // ignore: deprecated_member_use
        await Sqflite.devSetOptions(
            // ignore: deprecated_member_use
            SqfliteOptions(logLevel: sqfliteLogLevelNone));
      }, summary: 'No logs'),
      MenuItem('log level: sql', () async {
        // ignore: deprecated_member_use
        await Sqflite.devSetOptions(
            // ignore: deprecated_member_use
            SqfliteOptions(logLevel: sqfliteLogLevelSql));
      }, summary: 'Log sql command and basic database operation'),
      MenuItem('log level: verbose', () async {
        // ignore: deprecated_member_use
        await Sqflite.devSetOptions(
            // ignore: deprecated_member_use
            SqfliteOptions(logLevel: sqfliteLogLevelVerbose));
      }, summary: 'Verbose logs, for debugging'),
      MenuItem('Get info', () async {
        final factory = databaseFactory as impl.SqfliteDatabaseFactoryMixin;
        final info = await factory.getDebugInfo();
        print(info.toString());
      }, summary: 'Implementation info (dev only)'),
      MenuItem('Increment version', () async {
        await _incrementVersion();
      }, summary: 'Implementation info (dev only)'),
      MenuItem('Multiple db', () async {
        await Navigator.of(context).push(MaterialPageRoute(builder: (_) {
          return const MultipleDbTestPage();
        }));
      }, summary: 'Open multiple databases')
    ];
  }

  @override
  Widget build(BuildContext context) {
    itemWidgets = items
        .map((item) => ItemWidget(
              item,
              (item) async {
                final stopwatch = Stopwatch()..start();
                final future = (item as MenuItem).run();
                setState(() {});
                await future;
                // always add a small delay
                final elapsed = stopwatch.elapsedMilliseconds;
                if (elapsed < 300) {
                  await sleep(300 - elapsed);
                }
                setState(() {});
              },
              summary: item.summary,
            ))
        .toList(growable: false);
    return Scaffold(
      appBar: AppBar(
        title: const Text('Manual tests'),
      ),
      body: WillPopScope(
        onWillPop: pop,
        child: ListView(
          children: itemWidgets,
        ),
      ),
    );
  }
}

/// Multiple db test page.
class MultipleDbTestPage extends StatelessWidget {
  /// Test page.
  const MultipleDbTestPage({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    Widget dbTile(String name) {
      return ListTile(
        title: Text(name),
        onTap: () {
          Navigator.of(context).push(MaterialPageRoute(builder: (_) {
            return SimpleDbTestPage(
              dbName: name,
            );
          }));
        },
      );
    }

    return Scaffold(
      appBar: AppBar(
        title: const Text('Multiple databases'),
      ),
      body: ListView(
        children: <Widget>[
          dbTile('data1.db'),
          dbTile('data2.db'),
          dbTile('data3.db')
        ],
      ),
    );
  }
}

/// Simple db test page.
class SimpleDbTestPage extends StatefulWidget {
  /// Simple db test page.
  const SimpleDbTestPage({Key? key, required this.dbName}) : super(key: key);

  /// db name.
  final String dbName;

  @override
  _SimpleDbTestPageState createState() => _SimpleDbTestPageState();
}

class _SimpleDbTestPageState extends State<SimpleDbTestPage> {
  Database? database;

  Future<Database> _openDatabase() async {
    // await Sqflite.devSetOptions(SqfliteOptions(logLevel: sqfliteLogLevelVerbose));
    return database ??= await databaseFactory.openDatabase(widget.dbName,
        options: OpenDatabaseOptions(
            version: 1,
            onCreate: (db, version) async {
              await db.execute('CREATE TABLE Test (value TEXT)');
            }));
  }

  Future _closeDatabase() async {
    await database?.close();
    database = null;
  }

  @override
  void initState() {
    super.initState();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
        appBar: AppBar(
          title: Text('Simple db ${widget.dbName}'),
        ),
        body: Builder(
          builder: (context) {
            Widget menuItem(String title, void Function() onTap,
                {String? summary}) {
              return ListTile(
                title: Text(title),
                subtitle: summary == null ? null : Text(summary),
                onTap: onTap,
              );
            }

            Future _countRecord() async {
              final db = await _openDatabase();
              final result =
                  firstIntValue(await db.query('test', columns: ['COUNT(*)']));
              // Temp for nnbd successfull lint
              // ignore: deprecated_member_use
              Scaffold.of(context).showSnackBar(SnackBar(
                content: Text('$result records'),
                duration: const Duration(milliseconds: 700),
              ));
            }

            final items = <Widget>[
              menuItem('open Database', () async {
                await _openDatabase();
              }, summary: 'Open the database'),
              menuItem('Add record', () async {
                final db = await _openDatabase();
                await db.insert('test', {'value': 'some_value'});
                await _countRecord();
              }, summary: 'Add one record. Open the database if needed'),
              menuItem('Count record', () async {
                await _countRecord();
              }, summary: 'Count records. Open the database if needed'),
              menuItem(
                'Close Database',
                () async {
                  await _closeDatabase();
                },
              ),
              menuItem(
                'Delete database',
                () async {
                  await databaseFactory.deleteDatabase(widget.dbName);
                },
              ),
            ];
            return ListView(
              children: items,
            );
          },
        ));
  }
}
