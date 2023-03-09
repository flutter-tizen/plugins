# tizen_cion

[![pub package](https://img.shields.io/pub/v/tizen_cion.svg)](https://pub.dev/packages/tizen_cion)

The Cion API provides functionality to communicate with other devices.
The main features of the Cion API include:

 - Communicating with other applications in server-client style.
 - Communicating with other applications in group style.

>**Note**
>
> APIs will be used in generated source files by TIDLC which is an Interface Definition Language (IDL) compiler for Tizen.
> **[TIDL](https://docs.tizen.org/application/native/guides/app-management/tidl/) is recommended rather than using Cion APIs directly.**
> Usage example:
> ```
> Client: tidlc --cion -p -b -i CionTest.tidl -l C -o CionClient
> Server: tidlc --cion -s -b -i CionTest.tidl -l C -o CionServer
>```


## Cion APIs

Cion APIs compose of three set of APIs:

- **Server APIs**:  provide functions to connect a server, invoke functions in the server, and return the results from the server.

- **Client APIs**:  provide the way to call methods of the server, which executes the methods requested by clients.

- **Group APIs**:  provide the way to call methods of the other endpoints in the same group.

## Prerequisites

To use this package, add `tizen_cion` as a dependency in your `pubspec.yaml` file.

```yaml
depenedencies:
  tizen_cion: ^0.1.0
```

To use the `tizen_cion` packages, the application has to request permission by adding the following privilege to the `tizen-manifest.xml` file:
  ```
  <privileges>
     <privilege>http://tizen.org/privilege/datasharing</privilege>
     <privilege>http://tizen.org/privilege/internet</privilege>
  </privileges>
  ```

## Server API
Follow is the usage example of `Server` API.

```dart
class MyServer {
  MyServer() : _server = Server(serviceName, displayName);

  Future<void> _onConnectionRequestEvent(PeerInfo peer) async {
    _server.accept(peer);
  }

  Future<void> _onDisconnectedEvent(PeerInfo peer) async {
    // peer is disconnected
  }

  Future<void> _onReceivedEvent(
      PeerInfo peer, Payload payload, PayloadTransferStatus stauts) async {
    switch (payload.type) {
      case PayloadType.data:
        // dispatch DataPayload
        break;
      case PayloadType.file:
        // dispatch FilePayload
        break;
    }
  }

  Future<void> listen() async {
    return _server.listen(
        onConnectionRequest: _onConnectionRequestEvent,
        onDisconnected: _onDisconnectedEvent,
        onReceived: _onReceivedEvent);
  }

  void stop() {
    _server.stop();
  }

  Future<void> send(PeerInfo peer, Payload payload) async {
    _server.send(peer, payload);
  }

  static const String serviceName = 'exampleService';
  static const String displayName = 'exampleDisplay';
  final Server _server;
}

...
MyServer server;
server.listen();

```

## Client API
Follow is the usage example of `Client` API.

```dart
class MyClient {
  MyClient() : _client = Client(serviceName);

  Future<void> _onDisconnectedEvent(PeerInfo peer) async {
    // peer is disconnected
  }

  Future<void> _onReceivedEvent(
      PeerInfo peer, Payload payload, PayloadTransferStatus status) async {
    switch (payload.type) {
      case PayloadType.data:
        // dispatch DataPayload
        break;
      case PayloadType.file:
        // dispatch FilePayload
        break;
    }
  }

  Future<void> _onDiscoveredEvent(PeerInfo peer) async {
    stopDiscovery();
    final ConnectionResult result = await connect(peer);
    if (result.status == ConnectionStatus.ok) {
      // connect() success.
    } else {
      // connect() failure.
    }
  }

  Future<ConnectionResult> connect(PeerInfo peer) async {
    return _client.connect(peer,
        onDisconnected: _onDisconnectedEvent, onReceived: _onReceivedEvent);
  }

  Future<void> tryDiscovery() async {
    return _client.tryDiscovery(onDiscovered: _onDiscoveredEvent);
  }

  void stopDiscovery() {
    _client.stopDiscovery();
  }

  static const String serviceName = 'exampleService';
  final Client _client;
}

MyClient client;
client.tryDiscovery();
```

## Group API
Follow is the usage example of `Group` API.

```dart

class MyGroup {
  MyGroup() : _group = Group(topicName);

  Future<void> _onJoind(PeerInfo peer) async {
    // peer has been joined.
  }

  Future<void> _onLeft(PeerInfo peer) async {
    // peer has bben left.
  }

  Future<void> _onReceived(PeerInfo peer, DataPayload payload) async {
    // payload is received from the peer.
  }

  Future<void> subscribe() async {
    return _group.subscribe(
        onJoined: _onJoind, onLeft: _onLeft, onReceived: _onReceived);
  }

  void unsubscribe() {
    return _group.unsubscribe();
  }

  void publish(DataPayload payload) {
    _group.publish(payload);
  }

  final Group _group;
  static const String topicName = 'exampleTopic';
}
...
MyGroup group;
group.subscribe();
```
