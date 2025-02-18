import 'dart:async';
import 'dart:convert';
import 'dart:io';
import 'dart:math';

class SimpleWebSocket {
  SimpleWebSocket(this._url);
  final String _url;
  WebSocket? _socket;
  Function()? onOpen;
  Function(dynamic msg)? onMessage;
  Function(int? code, String? reaso)? onClose;

  Future<void> connect() async {
    try {
      //_socket = await WebSocket.connect(_url);
      _socket = await _connectForSelfSignedCert(_url);
      onOpen?.call();
      _socket!.listen(
        (data) {
          onMessage?.call(data);
        },
        onDone: () {
          onClose?.call(_socket!.closeCode, _socket!.closeReason);
        },
      );
    } catch (e) {
      onClose?.call(500, e.toString());
    }
  }

  void send(data) {
    if (_socket != null) {
      _socket!.add(data);
      print('send: $data');
    }
  }

  void close() {
    if (_socket != null) _socket!.close();
  }

  Future<WebSocket> _connectForSelfSignedCert(url) async {
    try {
      var r = Random();
      var key = base64.encode(List<int>.generate(8, (_) => r.nextInt(255)));
      var client = HttpClient(context: SecurityContext());
      client.badCertificateCallback = (
        X509Certificate cert,
        String host,
        int port,
      ) {
        print(
          'SimpleWebSocket: Allow self-signed certificate => $host:$port. ',
        );
        return true;
      };

      var request = await client.getUrl(
        Uri.parse(url),
      ); // form the correct url here
      request.headers.add('Connection', 'Upgrade');
      request.headers.add('Upgrade', 'websocket');
      request.headers.add(
        'Sec-WebSocket-Version',
        '13',
      ); // insert the correct version here
      request.headers.add('Sec-WebSocket-Key', key.toLowerCase());

      var response = await request.close();
      // ignore: close_sinks
      var socket = await response.detachSocket();
      var webSocket = WebSocket.fromUpgradedSocket(
        socket,
        protocol: 'signaling',
        serverSide: false,
      );

      return webSocket;
    } catch (e) {
      rethrow;
    }
  }
}
