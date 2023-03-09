// ignore_for_file: public_member_api_docs

import 'dart:convert';
import 'dart:io';

import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:path_provider/path_provider.dart';
import 'package:tizen_cion/tizen_cion.dart';

import 'chat.dart';
import 'group.dart';

class ClientServerApp extends StatefulWidget {
  const ClientServerApp({super.key});

  @override
  State<ClientServerApp> createState() => _ClientServerAppState();
}

class _ClientServerAppState extends State<ClientServerApp> {
  List<ChatMessage> messages = List<ChatMessage>.empty(growable: true);

  final TextEditingController _textEditingController = TextEditingController();

  final Client client = Client('testService');
  final Server server = Server('testService', 'exampleServer');

  String _msg = '';

  @override
  void initState() {
    super.initState();
    initPlatformState();
  }

  Future<void> clientInit() async {
    try {
      await client.tryDiscovery(onDiscovered: (PeerInfo peer) async {
        if (peer.displayName != 'exampleServer') {
          return;
        }

        client.stopDiscovery();
        final ConnectionResult result = await client.connect(peer,
            onDisconnected: (PeerInfo peer) async {
          setState(() {
            _msg = 'Peer has been disconnected \n'
                ' appid: ${peer.appid}\n'
                ' displayName: ${peer.displayName}\n'
                ' deviceId: ${peer.deviceId}\n'
                ' deviceName: ${peer.deviceName}\n'
                ' uuid: ${peer.uuid}';

            _createBubble('[client] $_msg');
          });
        }, onReceived: (PeerInfo peer, Payload payload,
                PayloadTransferStatus status) async {
          final FilePayload filePayload = payload as FilePayload;
          if (status == PayloadTransferStatus.success) {
            final Directory dir = await getTemporaryDirectory();
            final String userDataPath = '${dir.path}received.txt';
            filePayload.saveAsFile(userDataPath);
            final File receivedFile = File(userDataPath);
            receivedFile.openRead();
            final String message = await receivedFile.readAsString();
            _createBubble('[client] filePayload receive msg: $message');
          }
        });

        if (result.status != ConnectionStatus.ok) {
          throw Exception('Connection failed: ${result.reason}');
        }

        _createBubble('[client] PeerInfo has been discovered. \n'
            ' appid: ${peer.appid}\n'
            ' displayName: ${peer.displayName}\n'
            ' deviceId: ${peer.deviceId}\n'
            ' deviceName: ${peer.deviceName}\n'
            ' uuid: ${peer.uuid}');
      });
    } catch (e) {
      _createBubble(e.toString());
    }
  }

  Future<void> serverInit() async {
    try {
      await server.listen(onConnectionRequest: (PeerInfo peer) async {
        server.accept(peer);

        _createBubble(
            '[server] peer:\n'
            ' appid: ${peer.appid}\n'
            ' deviceId: ${peer.deviceId}\n'
            ' deviceName: ${peer.deviceName}\n'
            ' uuid: ${peer.uuid}',
            type: 'receiver');
      }, onDisconnected: (PeerInfo peer) async {
        _createBubble('[server] Peer has been disconnected', type: 'receiver');
      }, onReceived:
          (PeerInfo peer, Payload payload, PayloadTransferStatus status) async {
        if (status == PayloadTransferStatus.success) {
          final DataPayload dataPayload = payload as DataPayload;
          final String message = const AsciiDecoder().convert(dataPayload.data);
          _createBubble('[server] $message', type: 'receiver');

          final Directory dir = await getTemporaryDirectory();
          final String userDataPath = '${dir.path}test.txt';
          final File file = File(userDataPath);
          file.createSync();
          await file.writeAsString(message);

          final Payload filePayload = FilePayload(userDataPath);
          server.sendAll(filePayload);
          file.deleteSync();
        }
      });
    } catch (e) {
      _createBubble(e.toString());
    }
  }

  Future<void> initPlatformState() async {
    const String platformVersion = '7.0';

    setState(() {
      _msg = platformVersion;
    });

    clientInit();
    serverInit();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Cion example app'),
      ),
      body: Column(
        children: <Widget>[
          Flexible(
            child: ListView.builder(
              reverse: true,
              itemCount: messages.length,
              itemBuilder: (BuildContext context, int index) {
                return ChatBubble(
                  messageContent: messages[index].messageContent,
                  messageType: messages[index].messageType,
                );
              },
            ),
          ),
          const Divider(
            height: 1.0,
          ),
          Container(
            decoration: BoxDecoration(
              color: Theme.of(context).cardColor,
            ),
            child: Column(children: <Widget>[
              Row(
                children: <Widget>[
                  IconButton(
                    icon: const Icon(Icons.photo),
                    onPressed: () {},
                  ),
                  Flexible(
                    child: TextField(
                      controller: _textEditingController,
                      decoration: const InputDecoration.collapsed(
                        hintText: 'Type your message...',
                      ),
                    ),
                  ),
                  IconButton(
                    icon: const Icon(Icons.send),
                    onPressed: () {
                      _clientSend(_textEditingController.text);
                    },
                  ),
                ],
              ),
              ElevatedButton(
                child: const Text('Go to Group sample'),
                onPressed: () {
                  Navigator.push<dynamic>(
                    context,
                    MaterialPageRoute<dynamic>(
                        builder: (BuildContext context) => const GroupApp()),
                  );
                },
              )
            ]),
          )
        ],
      ),
    );
  }

  void _clientSend(String messageContent) {
    final Payload payload = DataPayload(
        Uint8List.fromList(const AsciiEncoder().convert(messageContent)));
    client.send(payload);
    _createBubble(messageContent);
  }

  void _createBubble(String messageContent, {String type = 'sender'}) {
    _textEditingController.clear();

    final ChatMessage message = ChatMessage(
      messageContent: messageContent,
      messageType: type,
    );

    setState(() {
      messages.insert(0, message);
    });
  }
}
