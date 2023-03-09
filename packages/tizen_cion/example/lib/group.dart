// ignore_for_file: public_member_api_docs

import 'dart:convert';

import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:tizen_cion/tizen_cion.dart';

import 'chat.dart';

class GroupApp extends StatefulWidget {
  const GroupApp({super.key});

  @override
  State<GroupApp> createState() => _GroupAppState();
}

class _GroupAppState extends State<GroupApp> {
  List<ChatMessage> messages = List<ChatMessage>.empty(growable: true);

  final TextEditingController _textEditingController = TextEditingController();

  final Group group1 = Group('mytopic');
  final Group group2 = Group('mytopic');

  @override
  void initState() {
    super.initState();
    initPlatformState();
  }

  Future<void> group2Unsubscribe() async {
    group2.unsubscribe();
  }

  Future<void> group1Init() async {
    print('group 1 init');

    try {
      await group1.subscribe(onJoined: (PeerInfo peer) async {
        _createBubble('[group1] onJoined');
      }, onLeft: (PeerInfo peer) async {
        _createBubble('[group1] onLeft');
      }, onReceived: (PeerInfo peer, DataPayload payload) async {
        final String data = const AsciiDecoder().convert(payload.data);
        _createBubble('[group1] onReceived: $data');
      });
    } catch (e) {
      _createBubble(e.toString());
    }
  }

  Future<void> group2Init() async {
    print('group 2 init');

    try {
      await group2.subscribe(onJoined: (PeerInfo peer) async {
        _createBubble('[group2] onJoined', type: 'receiver');
      }, onLeft: (PeerInfo peer) async {
        _createBubble('[group2] onLeft', type: 'receiver');
      }, onReceived: (PeerInfo peer, DataPayload payload) async {
        final String data = const AsciiDecoder().convert(payload.data);

        _createBubble('[group2] onReceived: $data', type: 'receiver');
        group2.publish(payload);
      });
    } catch (e) {
      _createBubble(e.toString(), type: 'receiver');
    }
  }

  Future<void> initPlatformState() async {
    group1Init();
    group2Init();
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
                      _groupSend(_textEditingController.text);
                    },
                  ),
                ],
              ),
              ElevatedButton(
                child: const Text('Go to Server & Client sample'),
                onPressed: () {
                  Navigator.pop(context);
                },
              )
            ]),
          )
        ],
      ),
    );
  }

  void _groupSend(String messageContent) {
    final Payload payload = DataPayload(
        Uint8List.fromList(const AsciiEncoder().convert(messageContent)));
    group1.publish(payload);
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
