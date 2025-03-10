# in_app_purchase_tizen

The Tizen implementation of [`in_app_purchase`](https://pub.dev/packages/in_app_purchase) based on the [Samsung Checkout](https://developer.samsung.com/smarttv/develop/guides/samsung-checkout/samsung-checkout.html) API.

## Supported devices

This plugin is only supported on Samsung Smart TVs running Tizen 6.0 and above.

## Required privileges

To use this plugin in a Tizen application, you need to declare the following privileges in your `tizen-manifest.xml` file.

```xml
<privileges>
  <privilege>http://developer.samsung.com/privilege/billing</privilege>
  <privilege>http://developer.samsung.com/privilege/sso.partner</privilege>
  <privilege>http://tizen.org/privilege/appmanager.launch</privilege>
</privileges>
```

The sso.partner privilege is required by the [Sso API](https://developer.samsung.com/smarttv/develop/api-references/samsung-product-api-references/sso-api.html) to internally obtain the user's custom ID (UID). Your app must be signed with a [partner-level certificate](https://docs.tizen.org/application/dotnet/get-started/certificates/creating-certificates) to use this privilege.

## Preparation

Follow these steps before setting up in-app purchases for your application:

1. [Register your application](https://github.com/flutter-tizen/flutter-tizen/blob/master/doc/publish-app.md) at the [Samsung Apps TV Seller Office](https://seller.samsungapps.com/tv) if you haven't registered yet. You do not need to complete the registration process at this point. Go to the **Billing Info** page of the app and set the **Samsung Checkout** checkbox to ON. You can return back to this page and finish the registration process when the final version of your app is ready.

2. Log in to the [Samsung Checkout DPI Portal](https://dpi.samsungcheckout.com) and register your in-app items. You can find your **App ID** and **Security Key** in the [**App Details Setting**](https://dpi.samsungcheckout.com/settings/appdetails) page. These values will be used as request parameters in your app code.

## Usage

This package is not an _endorsed_ implementation of `in_app_purchase`. Therefore, you have to include `in_app_purchase_tizen` alongside `in_app_purchase` as dependencies in your `pubspec.yaml` file.

```yaml
dependencies:
  in_app_purchase: ^3.2.1
  in_app_purchase_tizen: ^0.1.1
```

Then you can import `in_app_purchase` and `in_app_purchase_tizen` in your Dart code:

```dart
import 'package:in_app_purchase/in_app_purchase.dart';
import 'package:in_app_purchase_tizen/in_app_purchase_tizen.dart';
```

You must call `setRequestParameters` to set required parameters before making any plugin API call.

```dart
final InAppPurchaseTizenPlatformAddition platformAddition = _inAppPurchase
    .getPlatformAddition<InAppPurchaseTizenPlatformAddition>();
platformAddition.setRequestParameters(
  appId: 'your_dpi_app_id',
  pageSize: 20,
  pageNum: 1,
  securityKey: 'your_security_key',
);

final ProductDetailsResponse response =
    await _inAppPurchase.queryProductDetails(<String>{});
```

For detailed usage, see https://pub.dev/packages/in_app_purchase#usage and the [example](example/lib) app.

For more information on the Samsung Checkout API, visit the following pages.

- [Samsung Developers: Implementing the Purchase Process](https://developer.samsung.com/smarttv/develop/guides/samsung-checkout/implementing-the-purchase-process.html)
- [Samsung Developers: Billing API References](https://developer.samsung.com/smarttv/develop/api-references/samsung-product-api-references/billing-api.html)

## Supported APIs

- [x] `InAppPurchase.purchaseStream`
- [x] `InAppPurchase.isAvailable`
- [x] `InAppPurchase.queryProductDetails`
- [x] `InAppPurchase.buyNonConsumable`
- [x] `InAppPurchase.buyConsumable`
- [ ] `InAppPurchase.completePurchase` (Andriod/iOS-only)
- [x] `InAppPurchase.restorePurchases`
