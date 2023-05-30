# tizen_rpc_port

[![pub package](https://img.shields.io/pub/v/tizen_rpc_port.svg)](https://pub.dev/packages/tizen_rpc_port)

Tizen [RPC Port](https://docs.tizen.org/application/native/guides/app-management/rpc-port) APIs. Used to make remote procedure calls between Tizen applications.

> **Note**
>
> This plugin is typically used along with the TIDL (Tizen Interface Definition Language) compiler and a TIDL interface file. The [TIDL compiler](https://docs.tizen.org/application/native/guides/app-management/tidl) (`tidlc`) for Dart is currently under development and will be available soon.

## Usage

The Tizen RPC Port API supports remote procedure calls between client and server apps running on a single Tizen device. Each app may be written in any language (either C/C++, C#, or Dart) that is supported by the TIDL compiler. Running the following commands will generate Dart source files from a user-defined TIDL interface file (`message.tidl`).

```sh
tidlc -s -l dart -i message.tidl -o message_server
tidlc -p -l dart -i message.tidl -o message_client
```

The generated source files (`message_client.dart` and `message_server.dart`) depend on this package, so you need to add `tizen_rpc_port` as a dependency in the `pubspec.yaml` files of both client and server apps.

```yaml
depenedencies:
  tizen_rpc_port: ^0.1.3
```

Assuming that the name of the interface defined in your interface file is `Message`, the client must first call its `connect` method to connect to the server before making any remote invocation.

```dart
Message client = Message('com.example.server_app_id');
await client.connect();
```

For detailed usage, see the example apps of this package.

## Required privileges

The following privileges may be added to the client app's `tizen-manifest.xml` file to achieve full functionality.

```xml
<privileges>
    <privilege>http://tizen.org/privilege/appmanager.launch</privilege>
    <privilege>http://tizen.org/privilege/datasharing</privilege>
</privileges>
```

## Supported devices

This plugin is supported on Tizen devices running Tizen 6.5 or above.
