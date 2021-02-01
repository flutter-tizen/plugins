# Share Tizen plugin

The Tizen implementation of [`share`](https://github.com/flutter/plugins/tree/master/packages/share).

## Usage

To use this plugin, add `share` and `share_tizen` as [dependencies in your pubspec.yaml file](https://flutter.io/platform-plugins/).

## Example

Import the library.

``` dart
import 'package:share/share.dart';
```

Then invoke the static `share` method anywhere in your Dart code.

``` dart
Share.share('check out my website https://example.com');
```

The `share` method also takes an optional `subject` that will be used when
sharing to email.

``` dart
Share.share('check out my website https://example.com', subject: 'Look what I made!');
```

To share one or multiple files invoke the static `shareFiles` method anywhere in your Dart code. Optionally you can also pass in `text` and `subject`.
``` dart
Share.shareFiles(['${directory.path}/image.jpg'], text: 'Great picture');
Share.shareFiles(['${directory.path}/image1.jpg', '${directory.path}/image2.jpg']);
```

## Notice
To use this plugin, you need to declare privileges in tizen-manifest.xml of your application.
``` xml
<privileges>
  <privilege>http://tizen.org/privilege/appmanager.launch</privilege>
</privileges>
```
This plugin only works with ``sms`` on ``Watch(emulator)``. Except in previous case, the required app does not exist and cannot be used.