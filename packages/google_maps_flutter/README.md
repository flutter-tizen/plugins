# google_maps_flutter_tizen

[![pub package](https://img.shields.io/pub/v/google_maps_flutter_tizen.svg)](https://pub.dev/packages/google_maps_flutter_tizen)

The Tizen implementation of [google_maps_flutter](https://pub.dev/packages/google_maps_flutter).

## Required privileges

To use this plugin, add below lines under the `<manifest>` section in your `tizen-manifest.xml` file.

```xml
<privileges>
  <privilege>http://tizen.org/privilege/internet</privilege>
</privileges>
```

## Usage

This package is not an _endorsed_ implementation of `google_maps_flutter`. Therefore, you have to include `google_maps_flutter_tizen` alongside `google_maps_flutter` as dependencies in your `pubspec.yaml` file.

```yaml
dependencies:
  google_maps_flutter: ^2.1.7
  google_maps_flutter_tizen: ^0.1.3
```

For detailed usage, see https://pub.dev/packages/google_maps_flutter#sample-usage.

In addition, you need a Maps JavaScript API Key to use this plugin. You can get an API key at [this page](https://developers.google.com/maps/documentation/javascript/get-api-key) and specify it in the `assets/map.html` file of your app.

```js
<script src="https://maps.googleapis.com/maps/api/js?key=YOUR_API_KEY">
```
