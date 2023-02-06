import 'dart:async';
import 'dart:core';
import 'dart:typed_data';

import 'package:flutter/material.dart';
import 'package:flutter_webrtc/flutter_webrtc.dart';

import 'signaling.dart';

class DataChannelSample extends StatefulWidget {
  DataChannelSample({required this.host});
  static String tag = 'call_sample';
  final String host;

  @override
  _DataChannelSampleState createState() => _DataChannelSampleState();
}

class _DataChannelSampleState extends State<DataChannelSample> {
  _DataChannelSampleState();
  Signaling? _signaling;
  List<dynamic> _peers = [];
  String? _selfId;
  bool _inCalling = false;
  RTCDataChannel? _dataChannel;
  Session? _session;
  Timer? _timer;
  var _text = '';

  @override
  void initState() {
    super.initState();
    _connect(context);
  }

  @override
  void deactivate() {
    super.deactivate();
    _signaling?.close();
    _timer?.cancel();
  }

  void _connect(BuildContext context) async {
    _signaling ??= Signaling(widget.host, context);
    await _signaling!.connect();
    _signaling?.onDataChannelMessage = (_, dc, RTCDataChannelMessage data) {
      setState(() {
        if (data.isBinary) {
          print('Got binary [${data.binary}]');
        } else {
          _text = data.text;
        }
      });
    };

    _signaling?.onDataChannel = (_, channel) {
      _dataChannel = channel;
    };

    _signaling?.onSignalingStateChange = (SignalingState state) {
      switch (state) {
        case SignalingState.ConnectionClosed:
        case SignalingState.ConnectionError:
        case SignalingState.ConnectionOpen:
          break;
      }
    };

    _signaling?.onCallStateChange = (Session session, CallState state) {
      switch (state) {
        case CallState.CallStateNew:
          {
            setState(() {
              _session = session;
              _inCalling = true;
            });
            _timer =
                Timer.periodic(Duration(seconds: 1), _handleDataChannelTest);
            break;
          }
        case CallState.CallStateBye:
          {
            setState(() {
              _inCalling = false;
            });
            _timer?.cancel();
            _dataChannel = null;
            _inCalling = false;
            _session = null;
            _text = '';
            break;
          }
        case CallState.CallStateInvite:
        case CallState.CallStateConnected:
        case CallState.CallStateRinging:
      }
    };

    _signaling?.onPeersUpdate = (event) {
      setState(() {
        _selfId = event['self'];
        _peers = event['peers'];
      });
    };
  }

  Future<void> _handleDataChannelTest(Timer timer) async {
    var text = 'Say hello ${timer.tick} times, from [$_selfId]';
    await _dataChannel
        ?.send(RTCDataChannelMessage.fromBinary(Uint8List(timer.tick + 1)));
    await _dataChannel?.send(RTCDataChannelMessage(text));
  }

  void _invitePeer(context, peerId) async {
    if (peerId != _selfId) {
      _signaling?.invite(peerId, 'data', false);
    }
  }

  void _hangUp() {
    _signaling?.bye(_session!.sid);
  }

  ListBody _buildRow(context, peer) {
    var self = peer['id'] == _selfId;
    return ListBody(children: <Widget>[
      ListTile(
        title: Text(self
            ? peer['name'] + ', ID: ${peer['id']} ' + ' [Your self]'
            : peer['name'] + ', ID: ${peer['id']} '),
        onTap: () => _invitePeer(context, peer['id']),
        trailing: Icon(Icons.sms),
        subtitle: Text('[ ${peer['user_agent']}]'),
      ),
      Divider()
    ]);
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text(
            'Data Channel Sample${_selfId != null ? ' [Your ID ($_selfId)] ' : ''}'),
        actions: <Widget>[
          IconButton(
            icon: const Icon(Icons.settings),
            onPressed: null,
            tooltip: 'setup',
          ),
        ],
      ),
      floatingActionButton: _inCalling
          ? FloatingActionButton(
              onPressed: _hangUp,
              tooltip: 'Hangup',
              child: Icon(Icons.call_end),
            )
          : null,
      body: _inCalling
          ? Center(
              child: Container(
                child: Text('Recevied => $_text'),
              ),
            )
          : ListView.builder(
              shrinkWrap: true,
              padding: const EdgeInsets.all(0.0),
              itemCount: _peers.length,
              itemBuilder: (context, i) {
                return _buildRow(context, _peers[i]);
              }),
    );
  }
}
