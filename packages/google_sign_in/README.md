# google_sign_in_tizen

[![pub package](https://img.shields.io/pub/v/google_sign_in_tizen.svg)](https://pub.dev/packages/google_sign_in_tizen)

The Tizen implementation of [`google_sign_in`](https://pub.dev/packages/google_sign_in).

## Usage

This package is not an _endorsed_ implementation of `google_sign_in`. Therefore, you have to include `google_sign_in_tizen` alongside `google_sign_in` as dependencies in your `pubspec.yaml` file.

```yaml
dependencies:
  google_sign_in: ^5.4.1
  google_sign_in_tizen: ^0.1.4
```

For detailed usage on how to use `google_sign_in`, see https://pub.dev/packages/google_sign_in#usage.

## Tizen integration

The implementation of `google_sign_in_tizen` is based on [Google Sign-In for TVs and Limited Input Devices](https://developers.google.com/identity/gsi/web/guides/devices). Note that this method only supports limited scopes of APIs as listed in [Allowed scopes](https://developers.google.com/identity/protocols/oauth2/limited-input-device#allowedscopes).

The plugin uses [OAuth 2.0 Device Authorization Grant](https://datatracker.ietf.org/doc/html/rfc8628) (aka Device Flow) which has a different authorization flow than [Authorization Code Grant with PKCE](https://datatracker.ietf.org/doc/html/rfc7636) used for endorsed platforms (Android, iOS, and web). While every step needed to authorize the app for "Authorization Code Grant with PKCE" happens on a single device, "Device Flow" requires the user to use a secondary device (PC, tablet, or desktop) to finalize the authorization.

To allow users to sign-in to their account using `google_sign_in`, you first need create credentials for your app ("Client ID" and "Client Secret"). See [register your application for TVs and limited input devices](https://developers.google.com/identity/gsi/web/guides/devices#get_a_client_id_and_client_secret) on how to create them.

You also need to add some additional code to your app to fully integrate `google_sign_in` on Tizen (explained below).

### Adding OAuth credentials

Unlike "Authorization Code Grant with PKCE", "Device Flow" requires a [client secret](https://developers.google.com/identity/protocols/oauth2/limited-input-device#step-4:-poll-googles-authorization-server) parameter during Google Sign-In. You must call `GoogleSignInTizen.setCredentials` function with your client's OAuth credentials before calling `google_sign_in`'s API.

```dart
import 'package:google_sign_in_tizen/google_sign_in_tizen.dart';

GoogleSignInTizen.setCredentials(
  clientId: 'YOUR_CLIENT_ID',
  clientSecret: 'YOUR_CLIENT_SECRET',
);
```

:warning: Security concerns

Storing a client secret in code is considered a bad practice as it exposes [security vulnerabilites](https://datatracker.ietf.org/doc/html/rfc8628#section-5.6), you should perform extra steps to protect your client credentials.

1. Do not upload credentials to public repositories.

   Anyone who has access to your credentials can impersonate your app. Make sure to not upload your production credentials in any source code repositories. See the comments in `credentials.dart` file in the [example](/example/) app .

2. Obfuscate code in production.

   Client secrets can still be extracted from binary with reverse engineering, but it can be made challenging with [code obfuscation](<https://en.wikipedia.org/wiki/Obfuscation_(software)>).

   ```bash
   flutter-tizen build tpk --obfuscate --split-debug-info=/<project-name>/<directory>
   ```

   See [Obfuscating Dart code](https://docs.flutter.dev/deployment/obfuscate) for more information.

### Assigning the plugin's navigatorKey object to MaterialApp or CupertinoApp

During Google Sign-In with Device Flow, the client displays a widget that shows [user_code](https://developers.google.com/identity/gsi/web/guides/devices#obtain_a_user_code_and_verification_url) and [verification url](https://developers.google.com/identity/gsi/web/guides/devices#obtain_a_user_code_and_verification_url) to the user and instructs them to visit the verification url in a user agent on a secondary device (for example, in a browser on their mobile phone) and enter the user code. The plugin will be able to show this widget when `GoogleSignInTizen.navigatorKey` is assigned to the `navigatorKey` parameter of `MaterialApp` or `CupertinoApp`.

```dart
import 'package:google_sign_in_tizen/google_sign_in_tizen.dart';

MaterialApp(
  title: 'Google Sign In',
  navigatorKey: GoogleSignInTizen.navigatorKey,
  home: const SignInDemo(),
);
```

If you need to assign a custom navigatorKey to `MaterialApp` (or `CupertinoApp`), you must set that key to be used by the plugin as well.

```dart
import 'package:google_sign_in_tizen/google_sign_in_tizen.dart';

GoogleSignInTizen.navigatorKey = GlobalKey<NavigatorState>();
MaterialApp(
  title: 'Google Sign In',
  navigatorKey: GoogleSignInTizen.navigatorKey,
  home: const SignInDemo(),
);
```

## Required privileges

The `http://tizen.org/privilege/internet` privilege is required to perform networking operations requested by your app during Google Sign-In. Add the required privilege in `tizen-manifest.xml` of your application.

```xml
<privileges>
  <privilege>http://tizen.org/privilege/internet</privilege>
</privileges>
```

## Supported devices

All devices running Tizen 5.5 or later.