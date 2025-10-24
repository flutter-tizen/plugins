# tizen_window_manager

 [![pub package](https://img.shields.io/pub/v/tizen_window_manager.svg)](https://pub.dev/packages/tizen_window_manager)

Tizen window manager APIs. Used to control windows and get window geometry information.

## Usage

To use this package, add `tizen_window_manager` as a dependency in your `pubspec.yaml` file.

```yaml
dependencies:
  tizen_window_manager: ^0.1.0
```

> [!NOTE]
> This plugin only works when the [RendererType](https://github.com/flutter-tizen/flutter-tizen/blob/master/embedding/csharp/Tizen.Flutter.Embedding/FlutterApplication.cs#L110) is [EGL](https://github.com/flutter-tizen/flutter-tizen/blob/master/embedding/csharp/Tizen.Flutter.Embedding/FlutterApplication.cs#L24). Calling this plugin's API when the RendererType is [EvasGL](https://github.com/flutter-tizen/flutter-tizen/blob/master/embedding/csharp/Tizen.Flutter.Embedding/FlutterApplication.cs#L20) will cause the app to crash.

## API Reference

The `TizenWindowManagerPlugin` class provides methods to control window behavior on the Tizen platform.

### Controls windows

Activates the window and brings it to the foreground.

```dart
final windowManager = TizenWindowManagerPlugin();
await windowManager.activate();
```

Lowers the window and sends it to the background.

```dart
final windowManager = TizenWindowManagerPlugin();
await windowManager.lower();
```

### Retrieving window geometry

Gets the geometry(position and size) of the window.

Returns a `Map<String, int>` containing:
- `x`: The x coordinate of the window
- `y`: The y coordinate of the window  
- `width`: The width of the window
- `height`: The height of the window

```dart
final windowManager = TizenWindowManagerPlugin();
Map<String, int> geometry = await windowManager.getGeometry();
print('Window position: (${geometry['x']}, ${geometry['y']})');
print('Window size: ${geometry['width']}x${geometry['height']}');
```
