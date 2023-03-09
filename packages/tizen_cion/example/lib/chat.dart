// ignore_for_file: public_member_api_docs, always_specify_types

import 'package:flutter/material.dart';

class ChatMessage {
  ChatMessage({required this.messageContent, required this.messageType});
  String messageContent;
  String messageType;
}

class ChatBubble extends StatelessWidget {
  const ChatBubble(
      {super.key, required this.messageContent, required this.messageType});
  final String messageContent;
  final String messageType;

  @override
  Widget build(BuildContext context) {
    return Container(
      margin: const EdgeInsets.symmetric(vertical: 10),
      padding: const EdgeInsets.all(16),
      decoration: BoxDecoration(
        color: messageType == 'receiver' ? Colors.grey[300] : Colors.blue,
        borderRadius: BorderRadius.only(
          topLeft: const Radius.circular(30),
          topRight: const Radius.circular(30),
          bottomLeft: messageType == 'receiver'
              ? Radius.zero
              : const Radius.circular(30),
          bottomRight: messageType == 'receiver'
              ? const Radius.circular(30)
              : Radius.zero,
        ),
      ),
      child: Text(
        messageContent,
        style: TextStyle(
          color: messageType == 'receiver' ? Colors.black : Colors.white,
          fontSize: 16,
        ),
      ),
    );
  }
}
