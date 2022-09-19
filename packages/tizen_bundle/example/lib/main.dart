import 'package:flutter/material.dart';
import 'package:tizen_bundle/tizen_bundle.dart';

/// The main entry point.
void main() {
  runApp(const MyApp());
}

/// The UI app widget.
class MyApp extends StatefulWidget {
  /// The UI app widget.
  const MyApp({super.key});

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  final Bundle _bundle = Bundle();
  int _count = 0;
  String _msg = '';

  @override
  void initState() {
    super.initState();
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Tizen Bundle example',
      theme: ThemeData(primarySwatch: Colors.blue),
      home: Scaffold(
        appBar: AppBar(title: const Text('Tizen Bundle example')),
        body: Center(
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children: <Widget>[
              const Text(
                'Bundle example',
                style: TextStyle(fontWeight: FontWeight.bold),
              ),
              Padding(
                padding: const EdgeInsets.fromLTRB(0.0, 50.0, 0.0, 50.0),
                child: Text('Message: $_msg\n'),
              ),
              const SizedBox(height: 20),
              TextButton(
                onPressed: () {
                  try {
                    _bundle['stringKey_${_count++}'] = 'stringValue_$_count';
                    setState(() {
                      _msg = 'addString done.';
                    });
                  } on Exception catch (_, e) {
                    setState(() {
                      _msg = 'Exception($e) occurs';
                    });
                  }
                },
                child: const Text('Add String'),
              ),
              TextButton(
                onPressed: () {
                  if (_count > 0) {
                    final Object? value = _bundle['stringKey_${_count - 1}'];
                    setState(() {
                      _msg = 'getString done. value: $value';
                    });
                  } else {
                    setState(() {
                      _msg = 'Bundle is empty';
                    });
                  }
                },
                child: const Text('Get String'),
              ),
              TextButton(
                onPressed: () {
                  setState(() {
                    _msg = 'length: ${_bundle.length}';
                  });
                },
                child: const Text('Get length'),
              ),
              TextButton(
                onPressed: () {
                  if (_count > 0) {
                    _bundle.remove('stringKey_${--_count}');
                    setState(() {
                      _msg = 'remove done.';
                    });
                  } else {
                    setState(() {
                      _msg = 'Bundle is empty';
                    });
                  }
                },
                child: const Text('Remove String'),
              ),
              TextButton(
                onPressed: () {
                  _bundle.clear();
                  setState(() {
                    _msg = 'clear done.';
                  });
                },
                child: const Text('Clear'),
              ),
            ],
          ),
        ),
      ),
    );
  }
}
