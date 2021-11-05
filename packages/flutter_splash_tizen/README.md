# flutter_splash_tizen

Flutter-tizen package that allows to add custom splash-screen images into your apps.
## Getting Started

First you should add the package to dependencies section in your `pubspec.yaml`

```yaml
dependencies:
  flutter_splash_tizen: ^0.0.1
```
After that run 
```
flutter-tizen pub get
```
in order to download all the dependencies.<br>

By adding 
```yaml
flutter_splash_tizen:
  image: test.png
```
section in your `pubspec.yaml` and running 
```
flutter-tizen pub run flutter_splash_tizen:create
```
the image from `tizen/shared/res/test.png` will be added as splash screen. Each call of `create` will override the previous. <br>

If you wish to remove the splash image from your app simply run
```
flutter-tizen pub run flutter_splash_tizen:remove
```
