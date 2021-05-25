# messageport_tizen

This plugin adds support communication between Flutter applications on Tizen platform.


## Getting Started

First, imports the package in dart file.

```dart
import 'package:messageport_tizen/messageport_tizen.dart';
```

### Required privileges

No additional privileges needed.

### Create local port

Use `TizenMessageport.createLocalPort` method to create local port.

```dart
String portName = 'servicePort';
TizenLocalPort localPort = await TizenMessageport.createLocalPort(portName);
```

To register callback when message arrives to the local port, use `TizenLocalPort.register()` method.

```dart
  void onMessage(Object message, [TizenRemotePort remotePort]) {
    print('Message received: ' + message.toString());
  }
  ...
  localPort.register(onMessage);
```

### Connect to remote port

To connect to already registered port in remote application, use `TizenMessageport.connectToRemotePort()` method.

```dart
  String portName = 'servicePort';
  String remoteAppId = 'remote.app.id';
  TizenRemotePort remotePort = await TizenMessageport.connectToRemotePort(remoteAppId, portName);
```

### Send message

To send message to remote applcation use `TizenRemotePort.send()` method.

```dart
  String message = "This is message";
  try{
    remotePort.send(message);
  } catch (e) {
    print('Could not send message: ${e.toString()}");
  }
```

### Send message with local port

To send message with local port use `TizenRemotePort.send()` method. Local port received by remote application can be use to send a response.

```dart
  String message = "This is message";
  try{
    remotePort.sendWithLocalPort(message, localPort);
  } catch (e) {
    print('Could not send message: ${e.toString()}");
  }
```

To learn more about how to use this plugin, please check example application.

## Releated Info

To learn more about Tizen Message Port API visit following guides:

- [MessagePort Native Guide](https://docs.tizen.org/application/native/guides/app-management/message-port/)
