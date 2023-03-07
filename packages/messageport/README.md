# messageport_tizen

[![pub package](https://img.shields.io/pub/v/messageport_tizen.svg)](https://pub.dev/packages/messageport_tizen)

This plugin adds support for communication between Flutter applications on Tizen platform.

## Usage

First, import the package in Dart file.

```dart
import 'package:messageport_tizen/messageport_tizen.dart';
```

### Create local port

Use `LocalPort.create()` method to create a local port.

```dart
String portName = 'servicePort';
LocalPort localPort = await LocalPort.create(portName);
```

To register callback to be called when message arrives to the local port, use `LocalPort.register()` method.

```dart
void onMessage(Object message, [RemotePort remotePort]) {
  // Handle the received message.
}
...
localPort.register(onMessage);
```

Use `LocalPort.unregister()` to unregister port when it is no longer needed.

```dart
localPort.unregister();
```

### Connect to remote port

To connect to already registered port in remote application, use `RemotePort.connect()` method.

```dart
String portName = 'servicePort';
String remoteAppId = 'remote.app.id';
RemotePort remotePort = await RemotePort.connect(remoteAppId, portName);
```

### Send message

To send message to remote applcation, use `RemotePort.send()` method.

```dart
final message = {'a': 1, 'b': 2, 'c': 3};
await remotePort.send(message);
```

### Send message with local port

To send message with local port information, use `RemotePort.send()` method. Local port received by remote application can be used to send a response.

```dart
final message = 'This is a string message';
await remotePort.sendWithLocalPort(message, localPort);
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
