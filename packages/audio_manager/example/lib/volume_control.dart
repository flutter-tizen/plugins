// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'package:audio_manager_tizen/audio_manager_tizen.dart';
import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart';

/// A widget with audio controllers.
class VolumeControlScreen extends StatefulWidget {
  @override
  State<StatefulWidget> createState() => _VolumeControlScreenState();
}

class _VolumeControlScreenState extends State<VolumeControlScreen> {
  AudioVolumeType _currentPlaybackType = AudioVolumeType.none;
  AudioVolumeType _selectedType = AudioVolumeType.ringtone;
  int _currentVolume = 0;
  int _maxVolume = 0;
  Timer? _timer;
  VolumeChangedEvent? _volumeChangedEvent;
  StreamSubscription<VolumeChangedEvent>? _subscription;
  final List<DropdownMenuItem<AudioVolumeType>> _dropdownButtonItems =
      AudioVolumeType.values
          .where((AudioVolumeType e) => e != AudioVolumeType.none)
          .map((AudioVolumeType e) => DropdownMenuItem<AudioVolumeType>(
              value: e, child: Text(e.toString().split('.').last)))
          .toList();

  @override
  void initState() {
    super.initState();
    _onAudioTypeChanged(_selectedType);

    _timer = Timer.periodic(const Duration(seconds: 1), (Timer timer) async {
      final AudioVolumeType type =
          await AudioManager.volumeController.currentPlaybackType;

      if (type != _currentPlaybackType) {
        setState(() {
          _currentPlaybackType = type;
        });
      }
    });

    _subscription = AudioManager.volumeController.onChanged
        .listen((VolumeChangedEvent event) {
      setState(() {
        _volumeChangedEvent = event;

        if (event.type == _selectedType) {
          _currentVolume = event.level;
        }
      });
    });
  }

  @override
  void dispose() {
    _timer?.cancel();
    _subscription?.cancel();
    super.dispose();
  }

  Future<void> _onAudioTypeChanged(AudioVolumeType? type) async {
    type = type ?? AudioVolumeType.ringtone;
    final int currentVolume =
        await AudioManager.volumeController.getLevel(type);
    final int maxVolume = await AudioManager.volumeController.getMaxLevel(type);

    setState(() {
      _selectedType = type!;
      _currentVolume = currentVolume;
      _maxVolume = maxVolume;
    });
  }

  void _onVolumeSliderChanged(double value) {
    AudioManager.volumeController.setLevel(_selectedType, value.toInt());

    setState(() {
      _currentVolume = value.toInt();
    });
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text('Volume control')),
      body: Column(
        mainAxisAlignment: MainAxisAlignment.center,
        children: <Widget>[
          Text('Current playback type: ' +
              _currentPlaybackType.toString().split('.').last),
          Row(mainAxisAlignment: MainAxisAlignment.center, children: <Widget>[
            const Text('Audio type:'),
            const SizedBox(width: 10),
            DropdownButton<AudioVolumeType>(
                value: _selectedType,
                items: _dropdownButtonItems,
                onChanged: _onAudioTypeChanged)
          ]),
          Text('Volume: $_currentVolume/$_maxVolume'),
          SizedBox(
              width: 250,
              child: Slider(
                value: _currentVolume.toDouble(),
                min: 0,
                max: _maxVolume.toDouble(),
                onChanged: _onVolumeSliderChanged,
              )),
          Text(_volumeChangedEvent != null
              ? 'Volume for ' +
                  _volumeChangedEvent!.type.toString().split('.').last +
                  ' has changed to ' +
                  _volumeChangedEvent!.level.toString()
              : '')
        ],
      ),
    );
  }
}
