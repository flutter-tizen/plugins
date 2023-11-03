import 'dart:core';

import 'package:flutter/material.dart';
import 'package:flutter_webrtc/flutter_webrtc.dart';

import '../widgets/screen_select_dialog.dart';
import 'signaling.dart';

class CallSample extends StatefulWidget {
  CallSample({required this.host});
  static String tag = 'call_sample';
  final String host;

  @override
  _CallSampleState createState() => _CallSampleState();
}

class _CallSampleState extends State<CallSample> {
  _CallSampleState();
  Signaling? _signaling;
  List<dynamic> _peers = [];
  String? _selfId;
  final RTCVideoRenderer _localRenderer = RTCVideoRenderer();
  final RTCVideoRenderer _remoteRenderer = RTCVideoRenderer();
  bool _inCalling = false;
  Session? _session;
  bool _waitAccept = false;

  @override
  void initState() {
    super.initState();
    initRenderers();
    _connect(context);
  }

  void initRenderers() async {
    await _localRenderer.initialize();
    await _remoteRenderer.initialize();
  }

  @override
  void deactivate() {
    super.deactivate();
    _signaling?.close();
    _localRenderer.dispose();
    _remoteRenderer.dispose();
  }

  void _connect(BuildContext context) async {
    _signaling ??= Signaling(widget.host, context);
    await _signaling!.connect();
    _signaling?.onSignalingStateChange = (SignalingState state) {
      switch (state) {
        case SignalingState.ConnectionClosed:
        case SignalingState.ConnectionError:
        case SignalingState.ConnectionOpen:
          break;
      }
    };

    _signaling?.onCallStateChange = (Session session, CallState state) async {
      switch (state) {
        case CallState.CallStateNew:
          setState(() {
            _session = session;
          });
          break;
        case CallState.CallStateRinging:
          var accept = await _showAcceptDialog();
          if (accept!) {
            _accept();
            setState(() {
              _inCalling = true;
            });
          } else {
            _reject();
          }
          break;
        case CallState.CallStateBye:
          if (_waitAccept) {
            print('peer reject');
            _waitAccept = false;
            Navigator.of(context).pop(false);
          }
          setState(() {
            _localRenderer.srcObject = null;
            _remoteRenderer.srcObject = null;
            _inCalling = false;
            _session = null;
          });
          break;
        case CallState.CallStateInvite:
          _waitAccept = true;
          await _showInviteDialog();
          break;
        case CallState.CallStateConnected:
          if (_waitAccept) {
            _waitAccept = false;
            Navigator.of(context).pop(false);
          }
          setState(() {
            _inCalling = true;
          });
          break;
      }
    };

    _signaling?.onPeersUpdate = (event) {
      setState(() {
        _selfId = event['self'];
        _peers = event['peers'];
      });
    };

    _signaling?.onLocalStream = (stream) {
      _localRenderer.srcObject = stream;
      setState(() {});
    };

    _signaling?.onAddRemoteStream = (_, stream) {
      _remoteRenderer.srcObject = stream;
      setState(() {});
    };

    _signaling?.onRemoveRemoteStream = (_, stream) {
      _remoteRenderer.srcObject = null;
    };
  }

  Future<bool?> _showAcceptDialog() {
    return showDialog<bool?>(
      context: context,
      builder: (context) {
        return AlertDialog(
          title: Text('title'),
          content: Text('accept?'),
          actions: <Widget>[
            MaterialButton(
              child: Text(
                'Reject',
                style: TextStyle(color: Colors.red),
              ),
              onPressed: () => Navigator.of(context).pop(false),
            ),
            MaterialButton(
              child: Text(
                'Accept',
                style: TextStyle(color: Colors.green),
              ),
              onPressed: () => Navigator.of(context).pop(true),
            ),
          ],
        );
      },
    );
  }

  Future<bool?> _showInviteDialog() {
    return showDialog<bool?>(
      context: context,
      builder: (context) {
        return AlertDialog(
          title: Text('title'),
          content: Text('waiting'),
          actions: <Widget>[
            TextButton(
              child: Text('cancel'),
              onPressed: () {
                Navigator.of(context).pop(false);
                _hangUp();
              },
            ),
          ],
        );
      },
    );
  }

  void _invitePeer(BuildContext context, String peerId, bool useScreen) async {
    if (_signaling != null && peerId != _selfId) {
      _signaling?.invite(peerId, 'video', useScreen);
    }
  }

  void _accept() {
    if (_session != null) {
      _signaling?.accept(_session!.sid, 'video');
    }
  }

  void _reject() {
    if (_session != null) {
      _signaling?.reject(_session!.sid);
    }
  }

  void _hangUp() {
    if (_session != null) {
      _signaling?.bye(_session!.sid);
    }
  }

  void _switchCamera() {
    _signaling?.switchCamera();
  }

  Future<void> selectScreenSourceDialog(BuildContext context) async {
    MediaStream? screenStream;
    if (WebRTC.platformIsDesktop) {
      final source = await showDialog<DesktopCapturerSource>(
        context: context,
        builder: (context) => ScreenSelectDialog(),
      );
      if (source != null) {
        try {
          var stream =
              await navigator.mediaDevices.getDisplayMedia(<String, dynamic>{
            'video': {
              'deviceId': {'exact': source.id},
              'mandatory': {'frameRate': 30.0}
            }
          });
          stream.getVideoTracks()[0].onEnded = () {
            print(
                'By adding a listener on onEnded you can: 1) catch stop video sharing on Web');
          };
          screenStream = stream;
        } catch (e) {
          print(e);
        }
      }
    } else if (WebRTC.platformIsWeb) {
      screenStream =
          await navigator.mediaDevices.getDisplayMedia(<String, dynamic>{
        'audio': false,
        'video': true,
      });
    }
    if (screenStream != null) _signaling?.switchToScreenSharing(screenStream);
  }

  void _muteMic() {
    _signaling?.muteMic();
  }

  ListBody _buildRow(context, peer) {
    var self = peer['id'] == _selfId;
    return ListBody(children: <Widget>[
      ListTile(
        title: Text(self
            ? peer['name'] + ', ID: ${peer['id']} ' + ' [Your self]'
            : peer['name'] + ', ID: ${peer['id']} '),
        onTap: null,
        trailing: SizedBox(
            width: 100.0,
            child: Row(
                mainAxisAlignment: MainAxisAlignment.spaceBetween,
                children: <Widget>[
                  IconButton(
                    icon: Icon(self ? Icons.close : Icons.videocam,
                        color: self ? Colors.grey : Colors.black),
                    onPressed: () => _invitePeer(context, peer['id'], false),
                    tooltip: 'Video calling',
                  ),
                  IconButton(
                    icon: Icon(self ? Icons.close : Icons.screen_share,
                        color: self ? Colors.grey : Colors.black),
                    onPressed: () => _invitePeer(context, peer['id'], true),
                    tooltip: 'Screen sharing',
                  )
                ])),
        subtitle: Text('[ ${peer['user_agent']}'),
      ),
      Divider()
    ]);
  }

  Future<bool> _onWillPop(BuildContext context) async {
    if (_inCalling) {
      _hangUp();
    }
    return true;
  }

  @override
  Widget build(BuildContext context) {
    return WillPopScope(
      onWillPop: () => _onWillPop(context),
      child: Scaffold(
        appBar: AppBar(
          title: Text(
              'P2P Call Sample${_selfId != null ? ' [Your ID ($_selfId)] ' : ''}'),
          actions: <Widget>[
            IconButton(
              icon: const Icon(Icons.settings),
              onPressed: null,
              tooltip: 'setup',
            ),
          ],
        ),
        floatingActionButtonLocation: FloatingActionButtonLocation.centerFloat,
        floatingActionButton: _inCalling
            ? SizedBox(
                width: 240.0,
                child: Row(
                    mainAxisAlignment: MainAxisAlignment.spaceBetween,
                    children: <Widget>[
                      FloatingActionButton(
                        tooltip: 'Camera',
                        onPressed: _switchCamera,
                        child: const Icon(Icons.switch_camera),
                      ),
                      FloatingActionButton(
                        tooltip: 'Screen Sharing',
                        onPressed: () => selectScreenSourceDialog(context),
                        child: const Icon(Icons.desktop_mac),
                      ),
                      FloatingActionButton(
                        onPressed: _hangUp,
                        tooltip: 'Hangup',
                        backgroundColor: Colors.pink,
                        child: Icon(Icons.call_end),
                      ),
                      FloatingActionButton(
                        tooltip: 'Mute Mic',
                        onPressed: _muteMic,
                        child: const Icon(Icons.mic_off),
                      )
                    ]))
            : null,
        body: _inCalling
            ? OrientationBuilder(builder: (context, orientation) {
                return Container(
                  width: MediaQuery.of(context).size.width,
                  height: MediaQuery.of(context).size.height,
                  child: Row(
                    children: <Widget>[
                      Expanded(
                        flex: 1,
                        child: Container(
                          alignment: Alignment.center,
                          padding: const EdgeInsets.all(8.0),
                          decoration: BoxDecoration(color: Colors.black54),
                          child: RTCVideoView(_localRenderer),
                        ),
                      ),
                      Expanded(
                        flex: 1,
                        child: Container(
                          alignment: Alignment.center,
                          padding: const EdgeInsets.all(8.0),
                          decoration: BoxDecoration(color: Colors.black54),
                          child: RTCVideoView(_remoteRenderer),
                        ),
                      ),
                    ],
                  ),
                );
              })
            : ListView.builder(
                shrinkWrap: true,
                padding: const EdgeInsets.all(0.0),
                itemCount: _peers.length,
                itemBuilder: (context, i) {
                  return _buildRow(context, _peers[i]);
                }),
      ),
    );
  }
}
