# tizen_window_manager

 [![pub package](https://img.shields.io/pub/v/tizen_window_manager.svg)](https://pub.dev/packages/tizen_window_manager)

Tizen window manager APIs. Used to control windows and get window geometry information.

> **Warning**
> `WindowManager.activate()` and `WindowManager.lower()` require flutter-tizen `3.35.3-tizen.1.1.0` (released 2025-10-24) or later. Earlier embedders do not handle these requests on the `tizen/internal/window` channel and throw `MissingPluginException`. `WindowManager.getGeometry()` works on all versions.

## Usage

To use this package, add `tizen_window_manager` as a dependency in your `pubspec.yaml` file.

```yaml
dependencies:
  tizen_window_manager: ^0.2.0
```

## API Reference

The `WindowManager` class provides methods to control window behavior on the Tizen platform.

### Controls windows

Activates the window and brings it to the foreground.

```dart
await WindowManager.activate();
```

Lowers the window and sends it to the background.

```dart
await WindowManager.lower();
```

### Retrieving window geometry

Gets the geometry(position and size) of the window.

Returns a `Map<String, int>` containing:
- `x`: The x coordinate of the window
- `y`: The y coordinate of the window  
- `width`: The width of the window
- `height`: The height of the window

```dart
Map<String, int> geometry = await WindowManager.getGeometry();
print('Window position: (${geometry['x']}, ${geometry['y']})');
print('Window size: ${geometry['width']}x${geometry['height']}');
```
