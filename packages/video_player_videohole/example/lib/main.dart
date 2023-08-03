// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: public_member_api_docs, avoid_print

/// An example of using the plugin, controlling lifecycle and playback of the
/// video.

import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:http/http.dart' as http;
import 'package:video_player_videohole/video_player.dart';
import 'package:video_player_videohole/video_player_platform_interface.dart';

void main() {
  runApp(
    MaterialApp(
      home: _App(),
    ),
  );
}

class _App extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return DefaultTabController(
      length: 5,
      child: Scaffold(
        key: const ValueKey<String>('home_page'),
        appBar: AppBar(
          title: const Text('Video player example'),
          bottom: const TabBar(
            isScrollable: true,
            tabs: <Widget>[
              Tab(icon: Icon(Icons.cloud), text: 'MP4'),
              Tab(icon: Icon(Icons.cloud), text: 'HLS'),
              Tab(icon: Icon(Icons.cloud), text: 'Dash'),
              Tab(icon: Icon(Icons.cloud), text: 'DRM Widevine'),
              Tab(icon: Icon(Icons.cloud), text: 'DRM PlayReady'),
              Tab(icon: Icon(Icons.cloud), text: 'Track Selections'),
            ],
          ),
        ),
        body: TabBarView(
          children: <Widget>[
            _Mp4RemoteVideo(),
            _HlsRomoteVideo(),
            _DashRomoteVideo(),
            _DrmRemoteVideo(),
            _DrmRemoteVideo2(),
            _TrackSelectionTest(),
          ],
        ),
      ),
    );
  }
}

class _HlsRomoteVideo extends StatefulWidget {
  @override
  State<_HlsRomoteVideo> createState() => _HlsRomoteVideoState();
}

class _HlsRomoteVideoState extends State<_HlsRomoteVideo> {
  late VideoPlayerController _controller;

  @override
  void initState() {
    super.initState();
    _controller = VideoPlayerController.network(
        'https://bitdash-a.akamaihd.net/content/sintel/hls/playlist.m3u8');

    _controller.addListener(() {
      if (_controller.value.hasError) {
        print(_controller.value.errorDescription);
      }
      setState(() {});
    });
    _controller.setLooping(true);
    _controller.initialize().then((_) => setState(() {}));
    _controller.play();
  }

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return SingleChildScrollView(
      child: Column(
        children: <Widget>[
          Container(
            padding: const EdgeInsets.only(top: 20.0),
          ),
          const Text('With Hls'),
          Container(
            padding: const EdgeInsets.all(20),
            child: AspectRatio(
              aspectRatio: _controller.value.aspectRatio,
              child: Stack(
                alignment: Alignment.bottomCenter,
                children: <Widget>[
                  VideoPlayer(_controller),
                  ClosedCaption(text: _controller.value.caption.text),
                  _ControlsOverlay(controller: _controller),
                  VideoProgressIndicator(_controller, allowScrubbing: true),
                ],
              ),
            ),
          ),
        ],
      ),
    );
  }
}

class _DashRomoteVideo extends StatefulWidget {
  @override
  State<_DashRomoteVideo> createState() => _DashRomoteVideoState();
}

class _DashRomoteVideoState extends State<_DashRomoteVideo> {
  late VideoPlayerController _controller;

  @override
  void initState() {
    super.initState();
    _controller = VideoPlayerController.network(
        'https://dash.akamaized.net/dash264/TestCasesUHD/2b/11/MultiRate.mpd');

    _controller.addListener(() {
      if (_controller.value.hasError) {
        print(_controller.value.errorDescription);
      }
      setState(() {});
    });
    _controller.setLooping(true);
    _controller.initialize().then((_) => setState(() {}));
    _controller.play();
  }

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return SingleChildScrollView(
      child: Column(
        children: <Widget>[
          Container(padding: const EdgeInsets.only(top: 20.0)),
          const Text('With Dash'),
          Container(
            padding: const EdgeInsets.all(20),
            child: AspectRatio(
              aspectRatio: _controller.value.aspectRatio,
              child: Stack(
                alignment: Alignment.bottomCenter,
                children: <Widget>[
                  VideoPlayer(_controller),
                  ClosedCaption(text: _controller.value.caption.text),
                  _ControlsOverlay(controller: _controller),
                  VideoProgressIndicator(_controller, allowScrubbing: true),
                ],
              ),
            ),
          ),
        ],
      ),
    );
  }
}

class _Mp4RemoteVideo extends StatefulWidget {
  @override
  State<_Mp4RemoteVideo> createState() => _Mp4RemoteVideoState();
}

class _Mp4RemoteVideoState extends State<_Mp4RemoteVideo> {
  late VideoPlayerController _controller;

  @override
  void initState() {
    super.initState();
    _controller = VideoPlayerController.network(
        'https://media.w3.org/2010/05/bunny/trailer.mp4');

    _controller.addListener(() {
      if (_controller.value.hasError) {
        print(_controller.value.errorDescription);
      }
      setState(() {});
    });
    _controller.setLooping(true);
    _controller.initialize().then((_) => setState(() {}));
    _controller.play();
  }

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return SingleChildScrollView(
      child: Column(
        children: <Widget>[
          Container(padding: const EdgeInsets.only(top: 20.0)),
          const Text('With remote mp4'),
          Container(
            padding: const EdgeInsets.all(20),
            child: AspectRatio(
              aspectRatio: _controller.value.aspectRatio,
              child: Stack(
                alignment: Alignment.bottomCenter,
                children: <Widget>[
                  VideoPlayer(_controller),
                  ClosedCaption(text: _controller.value.caption.text),
                  _ControlsOverlay(controller: _controller),
                  VideoProgressIndicator(_controller, allowScrubbing: true),
                ],
              ),
            ),
          ),
        ],
      ),
    );
  }
}

class _DrmRemoteVideo extends StatefulWidget {
  @override
  State<_DrmRemoteVideo> createState() => _DrmRemoteVideoState();
}

class _DrmRemoteVideoState extends State<_DrmRemoteVideo> {
  late VideoPlayerController _controller;

  @override
  void initState() {
    super.initState();

    _controller = VideoPlayerController.network(
      'https://storage.googleapis.com/wvmedia/cenc/hevc/tears/tears.mpd',
      drmConfigs: DrmConfigs(
        type: DrmType.widevine,
        licenseCallback: (Uint8List challenge) async {
          final http.Response response = await http.post(
            Uri.parse('https://proxy.uat.widevine.com/proxy'),
            body: challenge,
          );
          return response.bodyBytes;
        },
      ),
    );

    _controller.addListener(() {
      if (_controller.value.hasError) {
        print(_controller.value.errorDescription);
      }
      setState(() {});
    });
    _controller.setLooping(true);
    _controller.initialize().then((_) => setState(() {}));
    _controller.play();
  }

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return SingleChildScrollView(
      child: Column(
        children: <Widget>[
          Container(padding: const EdgeInsets.only(top: 20.0)),
          const Text('Play DRM Widevine'),
          Container(
            padding: const EdgeInsets.all(20),
            child: AspectRatio(
              aspectRatio: _controller.value.aspectRatio,
              child: Stack(
                alignment: Alignment.bottomCenter,
                children: <Widget>[
                  VideoPlayer(_controller),
                  ClosedCaption(text: _controller.value.caption.text),
                  _ControlsOverlay(controller: _controller),
                  VideoProgressIndicator(_controller, allowScrubbing: true),
                ],
              ),
            ),
          ),
        ],
      ),
    );
  }
}

class _DrmRemoteVideo2 extends StatefulWidget {
  @override
  State<_DrmRemoteVideo2> createState() => _DrmRemoteVideoState2();
}

class _DrmRemoteVideoState2 extends State<_DrmRemoteVideo2> {
  late VideoPlayerController _controller;

  @override
  void initState() {
    super.initState();

    _controller = VideoPlayerController.network(
      'https://test.playready.microsoft.com/smoothstreaming/SSWSS720H264PR/SuperSpeedway_720.ism/Manifest',
      drmConfigs: const DrmConfigs(
        type: DrmType.playready,
        licenseServerUrl:
            'http://test.playready.microsoft.com/service/rightsmanager.asmx',
      ),
    );

    _controller.addListener(() {
      if (_controller.value.hasError) {
        print(_controller.value.errorDescription);
      }
      setState(() {});
    });
    _controller.setLooping(true);
    _controller.initialize().then((_) => setState(() {}));
    _controller.play();
  }

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return SingleChildScrollView(
      child: Column(
        children: <Widget>[
          Container(padding: const EdgeInsets.only(top: 20.0)),
          const Text('Play DRM PlayReady'),
          Container(
            padding: const EdgeInsets.all(20),
            child: AspectRatio(
              aspectRatio: _controller.value.aspectRatio,
              child: Stack(
                alignment: Alignment.bottomCenter,
                children: <Widget>[
                  VideoPlayer(_controller),
                  ClosedCaption(text: _controller.value.caption.text),
                  _ControlsOverlay(controller: _controller),
                  VideoProgressIndicator(_controller, allowScrubbing: true),
                ],
              ),
            ),
          ),
        ],
      ),
    );
  }
}

class _TrackSelectionTest extends StatefulWidget {
  @override
  State<_TrackSelectionTest> createState() => _TrackSelectionTestState2();
}

class _TrackSelectionTestState2 extends State<_TrackSelectionTest> {
  late VideoPlayerController _controller;

  @override
  void initState() {
    super.initState();

    _controller = VideoPlayerController.network(
        'https://bitdash-a.akamaihd.net/content/sintel/hls/playlist.m3u8');

    _controller.addListener(() {
      if (_controller.value.hasError) {
        print(_controller.value.errorDescription);
      }
      setState(() {});
    });
    _controller.setLooping(true);
    _controller.initialize().then((_) => setState(() {}));
    _controller.play();
  }

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return SingleChildScrollView(
      child: Column(
        children: <Widget>[
          Container(padding: const EdgeInsets.only(top: 20.0)),
          const Text('track selections test'),
          Container(
            padding: const EdgeInsets.all(20),
            child: AspectRatio(
              aspectRatio: _controller.value.aspectRatio,
              child: Stack(
                alignment: Alignment.bottomCenter,
                children: <Widget>[
                  VideoPlayer(_controller),
                  ClosedCaption(text: _controller.value.caption.text),
                  _ControlsOverlay(controller: _controller),
                  VideoProgressIndicator(_controller, allowScrubbing: true),
                ],
              ),
            ),
          ),
          _GetTrackSelectionButton(controller: _controller),
        ],
      ),
    );
  }
}

class _ControlsOverlay extends StatelessWidget {
  const _ControlsOverlay({required this.controller});

  static const List<Duration> _exampleCaptionOffsets = <Duration>[
    Duration(seconds: -10),
    Duration(seconds: -3),
    Duration(seconds: -1, milliseconds: -500),
    Duration(milliseconds: -250),
    Duration.zero,
    Duration(milliseconds: 250),
    Duration(seconds: 1, milliseconds: 500),
    Duration(seconds: 3),
    Duration(seconds: 10),
  ];
  static const List<double> _examplePlaybackRates = <double>[
    0.25,
    0.5,
    1.0,
    1.5,
    2.0,
    3.0,
    5.0,
    10.0,
  ];

  final VideoPlayerController controller;

  @override
  Widget build(BuildContext context) {
    return Stack(
      children: <Widget>[
        AnimatedSwitcher(
          duration: const Duration(milliseconds: 50),
          reverseDuration: const Duration(milliseconds: 200),
          child: controller.value.isPlaying
              ? const SizedBox.shrink()
              : Container(
                  color: Colors.black26,
                  child: const Center(
                    child: Icon(
                      Icons.play_arrow,
                      color: Colors.white,
                      size: 100.0,
                      semanticLabel: 'Play',
                    ),
                  ),
                ),
        ),
        GestureDetector(
          onTap: () {
            controller.value.isPlaying ? controller.pause() : controller.play();
          },
        ),
        Align(
          alignment: Alignment.topLeft,
          child: PopupMenuButton<Duration>(
            initialValue: controller.value.captionOffset,
            tooltip: 'Caption Offset',
            onSelected: (Duration delay) {
              controller.setCaptionOffset(delay);
            },
            itemBuilder: (BuildContext context) {
              return <PopupMenuItem<Duration>>[
                for (final Duration offsetDuration in _exampleCaptionOffsets)
                  PopupMenuItem<Duration>(
                    value: offsetDuration,
                    child: Text('${offsetDuration.inMilliseconds}ms'),
                  )
              ];
            },
            child: Padding(
              padding: const EdgeInsets.symmetric(
                // Using less vertical padding as the text is also longer
                // horizontally, so it feels like it would need more spacing
                // horizontally (matching the aspect ratio of the video).
                vertical: 12,
                horizontal: 16,
              ),
              child: Text('${controller.value.captionOffset.inMilliseconds}ms'),
            ),
          ),
        ),
        Align(
          alignment: Alignment.topRight,
          child: PopupMenuButton<double>(
            initialValue: controller.value.playbackSpeed,
            tooltip: 'Playback speed',
            onSelected: (double speed) {
              controller.setPlaybackSpeed(speed);
            },
            itemBuilder: (BuildContext context) {
              return <PopupMenuItem<double>>[
                for (final double speed in _examplePlaybackRates)
                  PopupMenuItem<double>(
                    value: speed,
                    child: Text('${speed}x'),
                  )
              ];
            },
            child: Padding(
              padding: const EdgeInsets.symmetric(
                // Using less vertical padding as the text is also longer
                // horizontally, so it feels like it would need more spacing
                // horizontally (matching the aspect ratio of the video).
                vertical: 12,
                horizontal: 16,
              ),
              child: Text('${controller.value.playbackSpeed}x'),
            ),
          ),
        ),
      ],
    );
  }
}

class _GetTrackSelectionButton extends StatelessWidget {
  const _GetTrackSelectionButton({required this.controller});

  final VideoPlayerController controller;

  @override
  Widget build(BuildContext context) {
    return Padding(
      padding: const EdgeInsets.only(top: 20.0),
      child: MaterialButton(
          child: const Text('Get Track Selection'),
          onPressed: () async {
            final List<TrackSelection>? tracks =
                await controller.trackSelections;
            if (tracks == null) {
              return;
            }
            // ignore: use_build_context_synchronously
            await showDialog(
              context: context,
              builder: (_) => _TrackSelectionDialog(
                controller: controller,
                videoTrackSelections: tracks
                    .where((TrackSelection track) =>
                        track.trackType == TrackSelectionType.video)
                    .toList(),
                audioTrackSelections: tracks
                    .where((TrackSelection track) =>
                        track.trackType == TrackSelectionType.audio)
                    .toList(),
                textTrackSelections: tracks
                    .where((TrackSelection track) =>
                        track.trackType == TrackSelectionType.text)
                    .toList(),
              ),
            );
          }),
    );
  }
}

class _TrackSelectionDialog extends StatelessWidget {
  const _TrackSelectionDialog({
    required this.controller,
    required this.videoTrackSelections,
    required this.audioTrackSelections,
    required this.textTrackSelections,
  });

  final VideoPlayerController controller;
  final List<TrackSelection> videoTrackSelections;
  final List<TrackSelection> audioTrackSelections;
  final List<TrackSelection> textTrackSelections;

  @override
  Widget build(BuildContext context) {
    return DefaultTabController(
      length: 3,
      child: AlertDialog(
        titlePadding: EdgeInsets.zero,
        contentPadding: EdgeInsets.zero,
        title: TabBar(
          labelColor: Colors.black,
          tabs: <Widget>[
            if (videoTrackSelections.isNotEmpty) const Tab(text: 'Video'),
            if (audioTrackSelections.isNotEmpty) const Tab(text: 'Audio'),
            if (textTrackSelections.isNotEmpty) const Tab(text: 'Text'),
          ],
        ),
        content: SizedBox(
          height: 200,
          width: 200,
          child: TabBarView(
            children: <Widget>[
              if (videoTrackSelections.isNotEmpty)
                ListView.builder(
                    itemCount: videoTrackSelections.length,
                    itemBuilder: (BuildContext context, int index) {
                      return ListTile(
                        title: Text(
                            '${videoTrackSelections[index].width}x${videoTrackSelections[index].height},${(videoTrackSelections[index].bitrate! / 1000000).toStringAsFixed(2)}Mbps'),
                        onTap: () {
                          controller
                              .setTrackSelection(videoTrackSelections[index]);
                        },
                      );
                    }),
              if (audioTrackSelections.isNotEmpty)
                ListView.builder(
                    itemCount: audioTrackSelections.length,
                    itemBuilder: (BuildContext context, int index) {
                      return ListTile(
                        title: Text(
                            'language:${audioTrackSelections[index].language}'),
                        onTap: () {
                          controller
                              .setTrackSelection(audioTrackSelections[index]);
                        },
                      );
                    }),
              if (textTrackSelections.isNotEmpty)
                ListView.builder(
                    itemCount: textTrackSelections.length,
                    itemBuilder: (BuildContext context, int index) {
                      return ListTile(
                        title: Text(
                            'language:${textTrackSelections[index].language}'),
                        onTap: () {
                          controller
                              .setTrackSelection(textTrackSelections[index]);
                        },
                      );
                    }),
            ],
          ),
        ),
      ),
    );
  }
}
