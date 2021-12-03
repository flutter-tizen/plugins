# messageport_tizen

[![pub package](https://img.shields.io/pub/v/messageport_tizen.svg)](https://pub.dev/packages/messageport_tizen)

This plugin adds support for communication between Flutter applications on Tizen platform.

## Getting Started

First, import the package in dart file.

```dart
import 'package:messageport_tizen/messageport_tizen.dart';
```

### Create local port

Use `TizenMessagePort.createLocalPort` method to create local port.

```dart
String portName = 'servicePort';
LocalPort localPort = await TizenMessagePort.createLocalPort(portName);
```

To register callback when message arrives to the local port, use `LocalPort.register()` method.

```dart
  void onMessage(Object message, [RemotePort remotePort]) {
    print('Message received: ' + message.toString());
  }
  ...
  localPort.register(onMessage);
```

Use `LocalPort.unregister()` to unregister port when it is no longer needed.

```dart
  localPort.unregister();
```

### Connect to remote port

To connect to already registered port in remote application, use `TizenMessagePort.connectToRemotePort()` method.

```dart
  String portName = 'servicePort';
  String remoteAppId = 'remote.app.id';
  RemotePort remotePort = await TizenMessagePort.connectToRemotePort(remoteAppId, portName);
```

### Send message

To send message to remote applcation use `RemotePort.send()` method.

```dart
  final message = {"a": 1, "b": 2, "c": 3};
  try{
    await remotePort.send(message);
  } catch (e) {
    print('Could not send message: ${e.toString()}");
  }
```

### Send message with local port

To send message with local port use `RemotePort.send()` method. Local port received by remote application can be used to send a response.

```dart
  final message = "This is a string message";
  try{
    await remotePort.sendWithLocalPort(message, localPort);
  } catch (e) {
    print('Could not send message: ${e.toString()}");
  }
```

### Supported data types

Data types that can be transferred using this plugin:
* null
* bool
* numbers: int, double
* String
* Uint8List, Int32List, Int64List, Float64List
* List of supported values
* Map from supported values to supported values

Plugin uses Flutter's [StandardMessageCodec](https://api.flutter.dev/flutter/services/StandardMessageCodec-class.html) to encode transferred data into binary.
Data can contain any value or collection type supported by Flutter's standard method codec.

To learn more about how to use this plugin, please check example application.

## Releated Info

To learn more about Tizen Message Port API visit following guides:

- [MessagePort Native Guide](https://docs.tizen.org/application/native/guides/app-management/message-port/)
