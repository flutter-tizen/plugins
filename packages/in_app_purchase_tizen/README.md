# in_app_purchase_tizen

The Tizen implementation of [`in_app_purchase`](https://pub.dev/packages/in_app_purchase) based on the Tizen [Samsung Checkout](https://developer.samsung.com/smarttv/develop/guides/samsung-checkout/samsung-checkout.html) API.

## Supported devices

This plugin is supported only on Smart TVs running Tizen 4.0 or above.

## Required privileges

To use this plugin in a Tizen application, you may need to declare the following privileges in your `tizen-manifest.xml` file.

```xml
<privileges>
  <privilege>http://developer.samsung.com/privilege/billing</privilege>
  <privilege>http://tizen.org/privilege/appmanager.launch</privilege>
</privileges>
```

- The billing privilege (`http://developer.samsung.com/privilege/billing`) is required to connect to billing client.
- The appmanager.launch privilege (`http://tizen.org/privilege/appmanager.launch`) is required to allow the application to open other applications.

For detailed information on Tizen privileges and billing privileges, see [Tizen Docs: API Privileges](https://docs.tizen.org/application/dotnet/get-started/api-privileges) and [Billing API](https://developer.samsung.com/smarttv/develop/api-references/samsung-product-api-references/billing-api.html#).

## Preparation

This plugin relies on [Samsung Checkout DPI(Digital Product Inventory) portal](https://dpi.samsungcheckout.com/) for making in-app purchases.
There is a list of steps before start in-app purchase:

1. Registering your application at the [Samsung Apps TV Seller Office](https://seller.samsungapps.com/tv/).You do not need to complete the registration with your source code at this point. To be able to use the DPI portal, you need to proceed to the second step of the App Registration Page and set the "Billing Info" field to "Use" and the "Samsung Checkout" field to "Yes".You can save the registration at this point and return to it later when your source code is complete.You can see more information in [Application Publication Process](https://developer.samsung.com/tv-seller-office/application-publication-process.html).

2. Login to [Samsung Checkout DPI portal](https://dpi.samsungcheckout.com/) and configure product.You can see more information in [Samsung Checkout DPI Portal](https://developer.samsung.com/smarttv/develop/guides/samsung-checkout/samsung-checkout-dpi-portal.html#Overview).


## Usage

This package is not an _endorsed_ implementation of `in_app_purchase`. Therefore, you have to include `in_app_purchase_tizen` alongside `in_app_purchase` as dependencies in your `pubspec.yaml` file.

```yaml
dependencies:
  in_app_purchase: ^3.1.4
  in_app_purchase_tizen: ^1.0.0
```

Then you can import in_app_purchase and in_app_purchase_tizen in your Dart code:

```dart
import 'package:in_app_purchase/in_app_purchase.dart';
import 'package:in_app_purchase_tizen/in_app_purchase_tizen.dart';
```

## Supported APIs

- [x] `purchaseStream` (update purchase list)
- [x] `isAvailable` (return true if billing client is available)
- [x] `queryProductDetails` (request product list)
- [x] `buyNonConsumable` (items can only be bought once)
- [x] `buyConsumable` (items can be bought additional times)
- [ ] `completePurchase` (Andriod/IOS-only)
- [x] `restorePurchases` (restore previous purchase)
