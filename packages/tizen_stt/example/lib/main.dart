// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:tizen_stt/tizen_stt.dart';

void main() => runApp(const MyApp());

/// Minimal STT test application.
class MyApp extends StatelessWidget {
  /// Creates the example application.
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) => const MaterialApp(home: SttPage());
}

/// Exercises the native session lifecycle.
class SttPage extends StatefulWidget {
  /// Creates the test page.
  const SttPage({super.key});

  @override
  State<SttPage> createState() => _SttPageState();
}

class _SttPageState extends State<SttPage> with WidgetsBindingObserver {
  final TizenStt _stt = TizenStt();
  late final StreamSubscription<TizenSttEvent> _subscription;
  TizenSttState _state = TizenSttState.notCreated;
  bool _busy = false;
  bool _partial = false;
  List<String> _languages = <String>[];
  String? _language;
  String _text = '';
  String _error = '';

  @override
  void initState() {
    super.initState();
    WidgetsBinding.instance.addObserver(this);
    _subscription = _stt.events.listen((TizenSttEvent event) {
      if (!mounted) {
        return;
      }
      setState(() {
        if (event.isStateChange) {
          _state = event.state!;
        }
        if (event.isPartialResult || event.isFinalResult) {
          _text = event.text ?? '';
        }
        if (event.isError) {
          _error = event.errorName ?? event.message ?? 'Recognition failed';
        }
      });
    }, onError: (Object error) {
      if (mounted) {
        setState(() => _error = error.toString());
      }
    });
  }

  Future<void> _run(Future<void> Function() action) async {
    if (_busy) {
      return;
    }
    setState(() {
      _busy = true;
      _error = '';
    });
    try {
      await action();
    } on PlatformException catch (error) {
      if (mounted) {
        setState(() => _error = '${error.code}: ${error.message ?? ''}');
      }
    } finally {
      if (mounted) {
        setState(() => _busy = false);
      }
    }
  }

  Future<void> _refresh() async {
    final TizenSttState state = await _stt.getState();
    if (mounted) {
      setState(() => _state = state);
    }
  }

  Future<void> _initialize() async {
    await _stt.initialize();
    final List<String> languages = await _stt.getLanguages();
    if (mounted) {
      setState(() {
        _languages = languages;
        _language = null;
      });
    }
    await _refresh();
  }

  Future<void> _record() async {
    if (_state == TizenSttState.recording) {
      await _stt.stopListening();
    } else {
      setState(() => _text = '');
      await _stt.startListening(
          language: _language,
          recognitionType: _partial
              ? TizenSttRecognitionType.freePartial
              : TizenSttRecognitionType.free);
    }
    await _refresh();
  }

  @override
  void didChangeAppLifecycleState(AppLifecycleState state) {
    if (state == AppLifecycleState.paused) {
      // Release the microphone even if a command is in flight.
      unawaited(
          _stt.dispose().then((_) => _refresh()).catchError((Object error) {
        if (mounted) {
          setState(() => _error = error.toString());
        }
      }));
    }
  }

  @override
  void dispose() {
    WidgetsBinding.instance.removeObserver(this);
    unawaited(_subscription.cancel());
    unawaited(
        _stt.dispose().catchError((Object error) => debugPrint('$error')));
    super.dispose();
  }

  @override
  Widget build(BuildContext context) => Scaffold(
        appBar: AppBar(title: const Text('Tizen STT')),
        body: ListView(padding: const EdgeInsets.all(24), children: <Widget>[
          Text('State: ${_state.name}', key: const Key('state')),
          if (_busy) const Text('Working…'),
          if (_error.isNotEmpty) Text(_error, key: const Key('error')),
          Wrap(spacing: 12, children: <Widget>[
            ElevatedButton(
                autofocus: true,
                onPressed: !_busy &&
                        (_state == TizenSttState.notCreated ||
                            _state == TizenSttState.created)
                    ? () => _run(_initialize)
                    : null,
                child: const Text('Initialize')),
            ElevatedButton(
                key: const Key('record_button'),
                onPressed: !_busy &&
                        (_state == TizenSttState.ready ||
                            _state == TizenSttState.recording)
                    ? () => _run(_record)
                    : null,
                child: Text(
                    _state == TizenSttState.recording ? 'Stop' : 'Record')),
            ElevatedButton(
                onPressed: !_busy &&
                        (_state == TizenSttState.recording ||
                            _state == TizenSttState.processing)
                    ? () => _run(() async {
                          await _stt.cancel();
                          await _refresh();
                        })
                    : null,
                child: const Text('Cancel')),
            ElevatedButton(
                onPressed: !_busy && _state != TizenSttState.notCreated
                    ? () => _run(() async {
                          await _stt.dispose();
                          await _refresh();
                        })
                    : null,
                child: const Text('Dispose')),
          ]),
          DropdownButton<String>(
              value: _language,
              hint: const Text('Automatic language'),
              items: _languages
                  .map((String language) => DropdownMenuItem<String>(
                      value: language, child: Text(language)))
                  .toList(),
              onChanged: !_busy && _state == TizenSttState.ready
                  ? (String? value) => setState(() => _language = value)
                  : null),
          CheckboxListTile(
              title: const Text('Partial results (if supported)'),
              value: _partial,
              onChanged: !_busy && _state == TizenSttState.ready
                  ? (bool? value) => setState(() => _partial = value!)
                  : null),
          const SizedBox(height: 24),
          SelectableText(
              _text.isEmpty ? 'Recognized text appears here.' : _text),
        ]),
      );
}
