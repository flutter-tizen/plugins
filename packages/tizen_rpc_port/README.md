# rpc_port_tizen

[![pub package](https://img.shields.io/pub/v/rpc_port_tizen.svg)](https://pub.dev/packages/rpc_port_tizen)

This plugin adds support for communication between Flutter applications on Tizen platform.

## Usage

First, import the package in Dart file.

```dart
import 'package:rpc_port_proxy_tizen/rpc_port_proxy.dart';
```

### Prerequisites

To enable your application to use `RpcPortProxy` API,
the application has to request permission by adding the following priviege to `tizen-manifest.xml` file:
```
<privileges>
   <privilege>http://tizen.org/privilege/appmanager.launch</privilege>
   <privilege>http://tizen.org/privilege/datasharing</privilege>
</privileges>
```

### Implement a proxy class

`RpcPortProxy` is abstract class.
To use the api, you need to inherit `RpcPortProxy` and implement the class.
Below is an example of an implementation.
You can implement some callbacks.

```dart
class MyProxy extends RpcPortProxy {
  MyProxy(String appid, String portName) : super(appid, portName);

  @override
  Future<void> onConnectedEvent(String receiver, String portName) async {
    // This callback is invoked when the proxy is connected with the stub.
  }

  @override
  Future<void> onDisconnectedEvent(String receiver, String portName) async {
    // This callback is invoked when the proxy is disconnected with the stub.
  }

  @override
  Future<void> onRejectedEvent(String receiver, String portName) async {
    // This callback is invoked when the connect() is failed.
  }

  @override
  Future<void> onReceivedEvent(String receiver, String portName, Parel parcel) async {
    // This callback is invoked when the data is received from the stub.
  }
}
```

### Create proxy

Use `RpcPort(appid, portName)` method to create a proxy.

```dart
String appid = 'org.tizen.example.StubApp'
String portName = 'servicePort';
RpcPortProxy proxy = RpcPortProxy(appid, portName);
```

### Connect to remote port

To connect port with the stub app, use `RpcPortProxy.connect()` method.

```dart
proxy.connect();
```
If `connect()` is success, then invoked `onConnectedEvent()`,
else then invoked `onRejectedEvent()`

### Get port

You can get the port that can send parcel to stub.

```dart
final port = proxy.getPort(PortType.main);
final parcel = Parcel();
parcel.writeString("Hello world");
port.send(parcel);
```

### Send parcel

To send parcel to stub app, use `RpcPortProxy.send()` method.

```dart
final port = proxy.getPort(PortType.main);
final parcel = Parcel();
parcel.writeString("Hello world");
port.send(parcel);
```

### Send message with local port

To send message with local port information, use `RemotePort.send()` method. Local port received by remote application can be used to send a response.

```dart
final message = 'This is a string message';
try{
  await remotePort.sendWithLocalPort(message, localPort);
} catch (error) {
  print('Could not send message: $error');
}
```

### Supported data types

This plugin uses Flutter's [StandardMessageCodec](https://api.flutter.dev/flutter/services/StandardMessageCodec-class.html) to encode transferred data into binary. The data can contain any value or collection type supported by Flutter's standard method codec.

* `null`
* `bool`
* `int`, `double`
* `String`
* `Uint8List`, `Int32List`, `Int64List`, `Float64List`
* Lists and Maps of the above

To learn more about the Tizen Message Port API, visit [Tizen Docs: Message Port](https://docs.tizen.org/application/native/guides/app-management/message-port).
