import 'dart:html';

class SimpleWebSocket {
  SimpleWebSocket(this._url) {
    _url = _url.replaceAll('https:', 'wss:');
  }
  String _url;
  WebSocket? _socket;
  Function()? onOpen;
  Function(dynamic msg)? onMessage;
  Function(int code, String reason)? onClose;

  Future<void> connect() async {
    try {
      _socket = WebSocket(_url);
      _socket!.onOpen.listen((e) {
        onOpen?.call();
      });

      _socket!.onMessage.listen((e) {
        onMessage?.call(e.data);
      });

      _socket!.onClose.listen((e) {
        onClose!.call(e.code!, e.reason!);
      });
    } catch (e) {
      onClose?.call(500, e.toString());
    }
  }

  void send(data) {
    if (_socket != null && _socket!.readyState == WebSocket.OPEN) {
      _socket!.send(data);
      print('send: $data');
    } else {
      print('WebSocket not connected, message $data not sent');
    }
  }

  void close() {
    if (_socket != null) {
      _socket!.close();
    }
  }
}
