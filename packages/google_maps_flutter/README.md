# google_maps_flutter_tizen

The Tizen implementation of the [google_maps_flutter](https://pub.dev/packages/google_maps_flutter).

## Required privileges

To use this plugin, add below lines under the `<manifest>` section in your `tizen-manifest.xml` file,

```xml
<privileges>
  <privilege>http://tizen.org/privilege/internet</privilege>
</privileges>
```

## Usage

This package is not an _endorsed_ implementation of `google_maps_flutter`. Therefore, you have to include `google_maps_flutter_tizen` alongside `google_maps_flutter` as dependencies in your `pubspec.yaml` file.

```yaml
dependencies:
  google_maps_flutter: ^2.0.8
  google_maps_flutter_tizen: ^0.1.0
```

In addition, you need a Maps JavaScript API Key to use this plugin. Please refer to the site below to get the API key.  
<https://developers.google.com/maps/documentation/javascript/get-api-key>

To use 'newCameraPosition' API, please add 'v=beta' option in map.html as shown below:
```
<script async
    src="https://maps.googleapis.com/maps/api/js?v=beta&key=YOUR_API_KEY">
</script>
```
