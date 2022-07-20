# google_sign_in_tizen

[![pub package](https://img.shields.io/pub/v/google_sign_in_tizen.svg)](https://pub.dev/packages/google_sign_in_tizen)

The Tizen implementation of [`google_sign_in`](https://github.com/flutter/plugins/tree/master_archive/packages/google_sign_in/google_sign_in).

## Usage

This package is not an _endorsed_ implementation of `google_sign_in`. Therefore, you have to include `google_sign_in_tizen` alongside `google_sign_in` as dependencies in your `pubspec.yaml` file.

```yaml
dependencies:
  google_sign_in: ^5.4.0
  google_sign_in_tizen: ^1.0.0
```

For detailed usage on how to use `google_sign_in`, see https://pub.dev/packages/google_sign_in#usage.

## Tizen integration

To access Google Sign-In for Tizen, you'll need to make sure to [register your application for TVs and limited input devices](https://developers.google.com/identity/gsi/web/guides/devices). You also need to add some additional code to your app to fully integrate `google_sign_in` on Tizen (explained below).

### Adding OAuth credentials

`google_sign_in_tizen` uses [OAuth 2.0 Device Authorization Grant](https://datatracker.ietf.org/doc/html/rfc8628) (device flow) which has a different authorization flow than [Authorization Code Grant with PKCE](https://datatracker.ietf.org/doc/html/rfc7636) used for endorsed platforms (Android, iOS, and web). One of the key differences is requiring [client secret](https://developers.google.com/identity/protocols/oauth2/limited-input-device#step-4:-poll-googles-authorization-server) parameter during token request. You must call the `setCredentials` function with your client's OAuth credentials before calling `google_sign_in`'s API.

```dart
import `package:google_sign_in_tizen/google_sign_in_tizen.dart as tizen`

tizen.setCredentials(
  clientId: 'YOUR_CLIENT_ID',
  clientSecret: 'YOUR_CLIENT_SECRET',
);
```

:warning: Security concerns

Storing client secret in code is considered bad practice as it exposes [security vulnerabilites](https://datatracker.ietf.org/doc/html/rfc8628#section-5.6). Therefore Google allows a [limited set of scopes](https://developers.google.com/identity/protocols/oauth2/limited-input-device#allowedscopes) that can be requested for device flow. Nevertheless, you can perform extra steps to protect your credentials better.

1. Do not upload credentials to public repositories.

Anyone who has access to your credentials can impersonate your app. Make sure to not upload your production credentials in any source code repositories. See the [example app](/example/) for a simple demonstration.

2. Obfuscate code in production.

Client secrets can still be extracted from binary with reverse engineering, but it can be made challenging with [code obfuscation](https://en.wikipedia.org/wiki/Obfuscation_(software)).

```bash
flutter-tizen build tpk --obfuscate --split-debug-info=/<project-name>/<directory>
```

See [Obfuscating Dart code](https://docs.flutter.dev/deployment/obfuscate) for more information.

### Assigning the plugin's navigatorKey object to MaterialApp or CupertinoApp

During Google Sign-In with device flow, the client displays a widget that shows [user_code](https://developers.google.com/identity/gsi/web/guides/devices#obtain_a_user_code_and_verification_url) and [verification url](https://developers.google.com/identity/gsi/web/guides/devices#obtain_a_user_code_and_verification_url) to the user and instructs them to visit the verification url in a user agent on a secondary device (for example, in a browser on their mobile phone) and enter the user code. The plugin will be able to show this widget only when the plugin's navigatorKey object is assigned to the `navigatorKey` parameter of `MaterialApp` or `CupertinoApp` using the `getNavigatorKey` function.

```dart
import `package:google_sign_in_tizen/google_sign_in_tizen.dart as tizen`

MaterialApp(
  title: 'Google Sign In',
  navigatorKey: tizen.getNavigatorKey(),
  home: const SignInDemo(),
);
```

If you need to assign a custom navigatorKey to `MaterialApp`'s (or `CupertinoApp`'s) `navigatorKey` parameter, you must set that key to be used by the plugin as well using `setNavigatorKey` function.

```dart
import `package:google_sign_in_tizen/google_sign_in_tizen.dart as tizen`

final GlobalKey<NavigatorState> customNavigatorKey = GlobalKey<NavigatorState>(); 
tizen.setNavigatorKey(customNavigatorKey);
MaterialApp(
  title: 'Google Sign In',
  navigatorKey: tizen.getNavigatorKey(),
  home: const SignInDemo(),
);
```
