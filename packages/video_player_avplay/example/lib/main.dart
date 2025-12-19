// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: public_member_api_docs, avoid_print, use_build_context_synchronously

/// An example of using the plugin, controlling lifecycle and playback of the
/// video.
library;

import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:http/http.dart' as http;
import 'package:video_player_avplay/video_player.dart';
import 'package:video_player_avplay/video_player_platform_interface.dart';

void main() {
  runApp(MaterialApp(home: _App()));
}

class _App extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return DefaultTabController(
      length: 10,
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
              Tab(icon: Icon(Icons.cloud), text: 'Track'),
              Tab(icon: Icon(Icons.cloud), text: 'Asset'),
              Tab(icon: Icon(Icons.live_tv), text: 'Live'),
              Tab(icon: Icon(Icons.local_florist), text: 'ChangeURLTest'),
              Tab(icon: Icon(Icons.abc), text: 'PictureCaptionTest'),
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
            _TrackTest(),
            _AssetVideo(),
            _LiveRemoteVideo(),
            _TestRemoteVideo(),
            _PictureCaptionVideo(),
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
      'https://test-streams.mux.dev/x36xhzz/x36xhzz.m3u8',
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
          const Text('With Hls'),
          Container(
            padding: const EdgeInsets.all(20),
            child: AspectRatio(
              aspectRatio: _controller.value.aspectRatio,
              child: Stack(
                alignment: Alignment.bottomCenter,
                children: <Widget>[
                  VideoPlayer(_controller),
                  ClosedCaption(textCaption: _controller.value.textCaption),
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
  // Optional
  final Map<StreamingPropertyType, String> _streamingProperties =
      <StreamingPropertyType, String>{
    /// You can set multiple parameters at once, please separate them with ";".
    StreamingPropertyType.adaptiveInfo:
        'MAX_RESOLUTION=3840X2160;MAX_FRAMERATE=60;UPDATE_SAME_LANGUAGE_CODE=1;OPEN_SUBTITLE_STYLE=TRUE',

    /// update token [before] dash-player prepare done.
    StreamingPropertyType.dashToken: 'YWJyVHlwZT1CUi1BVkMtREFTSC',
    StreamingPropertyType.openHttpHeader: 'TRUE',
    StreamingPropertyType.openManifest: 'TRUE',
  };

  @override
  void initState() {
    super.initState();
    _controller = VideoPlayerController.network(
      'https://dash.akamaized.net/dash264/TestCasesUHD/2b/11/MultiRate.mpd',
      formatHint: VideoFormat.dash,
      streamingProperty: _streamingProperties,
    );

    _controller.addListener(() {
      if (_controller.value.hasError) {
        print(_controller.value.errorDescription);
      }
      if (_controller.value.hasAdInfo) {
        print(_controller.value.adInfo);
      }
      if (_controller.value.hasManifestUpdated) {
        print(_controller.value.manifestInfo);
      }
      setState(() {});
    });
    _controller.setLooping(true);
    _controller.initialize().then((_) {
      setState(() {});
      // update token [after] dash-player prepare done.
      _controller.updateDashToken('YWJyVHlwZT1CUi1BVkMtREFTSC');
      // New features: get the following properties.
      _controller.getStreamingProperty(StreamingPropertyType.audioStreamInfo);
      _controller
          .getStreamingProperty(StreamingPropertyType.subtitleStreamInfo);
      _controller.getStreamingProperty(StreamingPropertyType.videoStreamInfo);
      _controller.getData(<DashPlayerProperty>{DashPlayerProperty.httpHeader});
    });
    _controller.play();
  }

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    const TextStyle customTextStyle =
        TextStyle(fontSize: 30, color: Colors.green);
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
                  ClosedCaption(
                      textCaption: _controller.value.textCaption,
                      customTextStyle: customTextStyle),
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
      'https://media.w3.org/2010/05/bunny/trailer.mp4',
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
          const Text('With remote mp4'),
          Container(
            padding: const EdgeInsets.all(20),
            child: AspectRatio(
              aspectRatio: _controller.value.aspectRatio,
              child: Stack(
                alignment: Alignment.bottomCenter,
                children: <Widget>[
                  VideoPlayer(_controller),
                  ClosedCaption(textCaption: _controller.value.textCaption),
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
      formatHint: VideoFormat.dash,
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
                  ClosedCaption(textCaption: _controller.value.textCaption),
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
            'https://test.playready.microsoft.com/service/rightsmanager.asmx',
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
                  ClosedCaption(textCaption: _controller.value.textCaption),
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

class _TrackTest extends StatefulWidget {
  @override
  State<_TrackTest> createState() => _TrackTestState();
}

class _TrackTestState extends State<_TrackTest> {
  late VideoPlayerController _controller;

  @override
  void initState() {
    super.initState();

    _controller = VideoPlayerController.network(
      'https://devstreaming-cdn.apple.com/videos/streaming/examples/img_bipbop_adv_example_fmp4/master.m3u8',
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
          const Text('track selections test'),
          Container(
            padding: const EdgeInsets.all(20),
            child: AspectRatio(
              aspectRatio: _controller.value.aspectRatio,
              child: Stack(
                alignment: Alignment.bottomCenter,
                children: <Widget>[
                  VideoPlayer(_controller),
                  ClosedCaption(textCaption: _controller.value.textCaption),
                  _ControlsOverlay(controller: _controller),
                  VideoProgressIndicator(_controller, allowScrubbing: true),
                ],
              ),
            ),
          ),
          _GetVideoTrackButton(controller: _controller),
          _GetAudioTrackButton(controller: _controller),
          _GetTextTrackButton(controller: _controller),
        ],
      ),
    );
  }
}

class _AssetVideo extends StatefulWidget {
  @override
  State<_AssetVideo> createState() => _AssetVideoState();
}

class _AssetVideoState extends State<_AssetVideo> {
  late VideoPlayerController _controller;

  @override
  void initState() {
    super.initState();

    _controller = VideoPlayerController.asset('assets/Butterfly-209.mp4');

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
          const Text('With assets mp4'),
          Container(
            padding: const EdgeInsets.all(20),
            child: AspectRatio(
              aspectRatio: _controller.value.aspectRatio,
              child: Stack(
                alignment: Alignment.bottomCenter,
                children: <Widget>[
                  VideoPlayer(_controller),
                  ClosedCaption(textCaption: _controller.value.textCaption),
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
              : const ColoredBox(
                  color: Colors.black26,
                  child: Center(
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
                  ),
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
                  PopupMenuItem<double>(value: speed, child: Text('${speed}x')),
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

class _GetVideoTrackButton extends StatelessWidget {
  const _GetVideoTrackButton({required this.controller});

  final VideoPlayerController controller;

  @override
  Widget build(BuildContext context) {
    return Padding(
      padding: const EdgeInsets.only(top: 20.0),
      child: MaterialButton(
        child: const Text('Get Video Track'),
        onPressed: () async {
          final List<VideoTrack>? videotracks = await controller.videoTracks;
          if (videotracks == null) {
            return;
          }
          await showDialog<void>(
            context: context,
            builder: (BuildContext context) {
              return AlertDialog(
                title: const Text('Video'),
                content: SizedBox(
                  height: 200,
                  width: 200,
                  child: ListView.builder(
                    itemCount: videotracks.length,
                    itemBuilder: (BuildContext context, int index) {
                      return ListTile(
                        title: Text(
                          '${videotracks[index].width}x${videotracks[index].height},${(videotracks[index].bitrate / 1000000).toStringAsFixed(2)}Mbps',
                        ),
                        onTap: () {
                          controller.setTrackSelection(videotracks[index]);
                        },
                      );
                    },
                  ),
                ),
              );
            },
          );
        },
      ),
    );
  }
}

class _GetAudioTrackButton extends StatelessWidget {
  const _GetAudioTrackButton({required this.controller});

  final VideoPlayerController controller;

  @override
  Widget build(BuildContext context) {
    return Padding(
      padding: const EdgeInsets.only(top: 20.0),
      child: MaterialButton(
        child: const Text('Get Audio Track'),
        onPressed: () async {
          final List<AudioTrack>? audioTracks = await controller.audioTracks;
          if (audioTracks == null) {
            return;
          }
          await showDialog<void>(
            context: context,
            builder: (BuildContext context) {
              return AlertDialog(
                title: const Text('Audio'),
                content: SizedBox(
                  height: 200,
                  width: 200,
                  child: ListView.builder(
                    itemCount: audioTracks.length,
                    itemBuilder: (BuildContext context, int index) {
                      return ListTile(
                        title: Text('language:${audioTracks[index].language}'),
                        onTap: () {
                          controller.setTrackSelection(audioTracks[index]);
                        },
                      );
                    },
                  ),
                ),
              );
            },
          );
        },
      ),
    );
  }
}

class _GetTextTrackButton extends StatelessWidget {
  const _GetTextTrackButton({required this.controller});

  final VideoPlayerController controller;

  @override
  Widget build(BuildContext context) {
    return Padding(
      padding: const EdgeInsets.only(top: 20.0),
      child: MaterialButton(
        child: const Text('Get Text Track'),
        onPressed: () async {
          final List<TextTrack>? textTracks = await controller.textTracks;
          if (textTracks == null) {
            return;
          }
          await showDialog<void>(
            context: context,
            builder: (BuildContext context) {
              return AlertDialog(
                title: const Text('Text'),
                content: SizedBox(
                  height: 200,
                  width: 200,
                  child: ListView.builder(
                    itemCount: textTracks.length,
                    itemBuilder: (BuildContext context, int index) {
                      return ListTile(
                        title: Text('language:${textTracks[index].language}'),
                        onTap: () {
                          controller.setTrackSelection(textTracks[index]);
                        },
                      );
                    },
                  ),
                ),
              );
            },
          );
        },
      ),
    );
  }
}

class _LiveRemoteVideo extends StatefulWidget {
  @override
  State<_LiveRemoteVideo> createState() => _LiveRomoteVideoState();
}

class _LiveRomoteVideoState extends State<_LiveRemoteVideo> {
  late VideoPlayerController _controller;

  @override
  void initState() {
    super.initState();
    _controller = VideoPlayerController.network(
      'https://hlive.ktv.go.kr/live/klive_h.stream/playlist.m3u8',
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
          const Text('Playing Live TV'),
          Container(
            padding: const EdgeInsets.all(20),
            child: AspectRatio(
              aspectRatio: _controller.value.aspectRatio,
              child: Stack(
                alignment: Alignment.bottomCenter,
                children: <Widget>[
                  VideoPlayer(_controller),
                  ClosedCaption(textCaption: _controller.value.textCaption),
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

class _TestRemoteVideo extends StatefulWidget {
  @override
  State<_TestRemoteVideo> createState() => _TestRemoteVideoState();
}

class _TestRemoteVideoState extends State<_TestRemoteVideo> {
  late VideoPlayerController _controller;

  @override
  void initState() {
    super.initState();
    _controller = VideoPlayerController.network(
      'https://test-streams.mux.dev/x36xhzz/x36xhzz.m3u8',
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
    _controller.setRestoreData(
      restoreDataSource: restoreDataSource,
      resumeTime: restoreTime,
    );
  }

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  DataSource restoreDataSource() {
    final DataSource dataSource = DataSource(
      sourceType: DataSourceType.network,
      uri: 'https://media.w3.org/2010/05/bunny/trailer.mp4',
    );
    return dataSource;
  }

  int restoreTime() {
    /// if resumeTime >= 0 , it will restore from resumeTime
    /// if resumeTime is not set or <0, it will restore from the time when suspend is called
    const int resumeTime = 0;
    return resumeTime;
  }

  @override
  Widget build(BuildContext context) {
    return SingleChildScrollView(
      child: Column(
        children: <Widget>[
          Container(padding: const EdgeInsets.only(top: 20.0)),
          const Text('ChangeURLTest'),
          Container(
            padding: const EdgeInsets.all(20),
            child: AspectRatio(
              aspectRatio: _controller.value.aspectRatio,
              child: Stack(
                alignment: Alignment.bottomCenter,
                children: <Widget>[
                  VideoPlayer(_controller),
                  ClosedCaption(textCaption: _controller.value.textCaption),
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

class _PictureCaptionVideo extends StatefulWidget {
  @override
  State<_PictureCaptionVideo> createState() => _PictureCaptionVideoState();
}

class _PictureCaptionVideoState extends State<_PictureCaptionVideo> {
  late VideoPlayerController _controller;

  @override
  void initState() {
    super.initState();
    _controller = VideoPlayerController.network(
      'https://livesim2.dashif.org/vod/testpic_2s/imsc1_img.mpd',
      formatHint: VideoFormat.dash,
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
          const Text('Picture Caption Test'),
          Container(
            padding: const EdgeInsets.all(20),
            child: AspectRatio(
              aspectRatio: _controller.value.aspectRatio,
              child: Stack(
                alignment: Alignment.bottomCenter,
                children: <Widget>[
                  VideoPlayer(_controller),
                  ClosedCaption(
                    textCaption: _controller.value.textCaption,
                    pictureCaption: _controller.value.pictureCaption,
                  ),
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
