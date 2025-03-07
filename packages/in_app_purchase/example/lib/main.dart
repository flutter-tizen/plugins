// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:flutter/material.dart';
import 'package:in_app_purchase/in_app_purchase.dart';
import 'package:in_app_purchase_tizen/in_app_purchase_tizen.dart';

void main() {
  WidgetsFlutterBinding.ensureInitialized();

  runApp(_MyApp());
}

// To try without auto-consume, change `true` to `false` here.
const bool _kAutoConsume = true;

const String _kAppId = '3201504002021';
const int _kPageSize = 20;
const int _kPageNum = 1;
// Do not expose your DPI security key. You can use a key management server to retrieve it for greater security.
const String _kSecurityKey = 'YxE757K+aDWHJXa0QMnL5AJmItefoEizvv8L7WPJAMs=';

class _MyApp extends StatefulWidget {
  @override
  State<_MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<_MyApp> {
  final InAppPurchase _inAppPurchase = InAppPurchase.instance;
  late StreamSubscription<List<PurchaseDetails>> _subscription;
  List<String> _notFoundIds = <String>[];
  List<ProductDetails> _products = <ProductDetails>[];
  List<PurchaseDetails> _purchases = <PurchaseDetails>[];
  bool _isAvailable = false;
  bool _purchasePending = false;
  bool _loading = true;
  String? _queryProductError;
  String _countryCode = '';

  @override
  void initState() {
    final Stream<List<PurchaseDetails>> purchaseUpdated =
        _inAppPurchase.purchaseStream;
    _subscription = purchaseUpdated.listen(
      (List<PurchaseDetails> purchaseDetailsList) {
        _listenToPurchaseUpdated(purchaseDetailsList);
      },
      onDone: () {
        _subscription.cancel();
      },
      onError: (Object error) {
        // handle error here.
      },
    );
    initStoreInfo();
    super.initState();
  }

  Future<void> initStoreInfo() async {
    // Tizen specific API:
    // You need to set necessary parameters before calling any plugin API.
    final InAppPurchaseTizenPlatformAddition platformAddition = _inAppPurchase
        .getPlatformAddition<InAppPurchaseTizenPlatformAddition>();
    platformAddition.setRequestParameters(
      appId: _kAppId,
      pageSize: _kPageSize,
      pageNum: _kPageNum,
      securityKey: _kSecurityKey,
    );

    final bool isAvailable = await _inAppPurchase.isAvailable();
    if (!isAvailable) {
      setState(() {
        _isAvailable = isAvailable;
        _products = <ProductDetails>[];
        _purchases = <PurchaseDetails>[];
        _notFoundIds = <String>[];
        _purchasePending = false;
        _loading = false;
      });
      return;
    }

    // The `identifiers` argument is not used on Tizen.
    // Use `InAppPurchaseTizenPlatformAddition.setRequestParameters` instead.
    final ProductDetailsResponse productDetailResponse =
        await _inAppPurchase.queryProductDetails(<String>{});
    if (productDetailResponse.error != null) {
      setState(() {
        _queryProductError = productDetailResponse.error!.message;
        _isAvailable = isAvailable;
        _products = productDetailResponse.productDetails;
        _purchases = <PurchaseDetails>[];
        _notFoundIds = productDetailResponse.notFoundIDs;
        _purchasePending = false;
        _loading = false;
      });
      return;
    }

    if (productDetailResponse.productDetails.isEmpty) {
      setState(() {
        _queryProductError = null;
        _isAvailable = isAvailable;
        _products = productDetailResponse.productDetails;
        _purchases = <PurchaseDetails>[];
        _notFoundIds = productDetailResponse.notFoundIDs;
        _purchasePending = false;
        _loading = false;
      });
      return;
    }

    setState(() {
      _isAvailable = isAvailable;
      _products = productDetailResponse.productDetails;
      _notFoundIds = productDetailResponse.notFoundIDs;
      _purchasePending = false;
      _loading = false;
    });
  }

  @override
  void dispose() {
    _subscription.cancel();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final List<Widget> stack = <Widget>[];
    if (_queryProductError == null) {
      stack.add(
        ListView(
          children: <Widget>[
            _buildConnectionCheckTile(),
            _buildProductList(),
            _buildRestoreButton(),
          ],
        ),
      );
    } else {
      stack.add(Center(child: Text(_queryProductError!)));
    }
    if (_purchasePending) {
      stack.add(
        const Stack(
          children: <Widget>[
            Opacity(
              opacity: 0.3,
              child: ModalBarrier(dismissible: false, color: Colors.grey),
            ),
            Center(child: CircularProgressIndicator()),
          ],
        ),
      );
    }

    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(title: const Text('IAP Example')),
        body: Stack(children: stack),
      ),
    );
  }

  Card _buildConnectionCheckTile() {
    if (_loading) {
      return const Card(child: ListTile(title: Text('Trying to connect...')));
    }
    final Widget storeHeader = ListTile(
      leading: Icon(
        _isAvailable ? Icons.check : Icons.block,
        color:
            _isAvailable ? Colors.green : ThemeData.light().colorScheme.error,
      ),
      title: Text(
        'The store is ${_isAvailable ? 'available' : 'unavailable'}.',
      ),
    );
    final List<Widget> children = <Widget>[storeHeader];

    if (!_isAvailable) {
      children.addAll(<Widget>[
        const Divider(),
        ListTile(
          title: Text(
            'Not connected',
            style: TextStyle(color: ThemeData.light().colorScheme.error),
          ),
          subtitle: const Text(
            'Unable to connect to the payments processor. Are you signed in with your Samsung account on this device?',
          ),
        ),
      ]);
    }
    return Card(child: Column(children: children));
  }

  Card _buildProductList() {
    if (_loading) {
      return const Card(
        child: ListTile(
          leading: CircularProgressIndicator(),
          title: Text('Fetching products...'),
        ),
      );
    }
    if (!_isAvailable) {
      return const Card();
    }
    const ListTile productHeader = ListTile(title: Text('Products for Sale'));
    final List<ListTile> productList = <ListTile>[];
    if (_notFoundIds.isNotEmpty) {
      productList.add(
        ListTile(
          title: Text(
            '[${_notFoundIds.join(", ")}] not found',
            style: TextStyle(color: ThemeData.light().colorScheme.error),
          ),
          subtitle: const Text(
            'This app needs special configuration to run. Please see README.md for instructions.',
          ),
        ),
      );
    }

    productList.addAll(
      _products.map((ProductDetails productDetails) {
        return ListTile(
          title: Text(productDetails.title),
          subtitle: Text(productDetails.description),
          trailing: TextButton(
            style: TextButton.styleFrom(
                backgroundColor: Colors.green[800],
                foregroundColor: Colors.white),
            onPressed: () {
              final PurchaseParam purchaseParam = PurchaseParam(
                productDetails: productDetails,
              );

              if (productDetails is SamsungCheckoutProductDetails) {
                if (productDetails.itemDetails.itemType ==
                    ItemType.consumable) {
                  _inAppPurchase.buyConsumable(
                    purchaseParam: purchaseParam,
                    // ignore: avoid_redundant_argument_values
                    autoConsume: _kAutoConsume,
                  );
                } else {
                  _inAppPurchase.buyNonConsumable(purchaseParam: purchaseParam);
                }
              }
            },
            child: Text(productDetails.price),
          ),
        );
      }),
    );

    return Card(
      child: Column(
        children: <Widget>[productHeader, const Divider()] + productList,
      ),
    );
  }

  Widget _buildRestoreButton() {
    if (_loading) {
      return Container();
    }

    return Padding(
      padding: const EdgeInsets.all(4.0),
      child: Row(
        mainAxisAlignment: MainAxisAlignment.end,
        children: <Widget>[
          TextButton(
            style: TextButton.styleFrom(
                backgroundColor: Theme.of(context).colorScheme.primary,
                foregroundColor: Colors.white),
            onPressed: () => _inAppPurchase.restorePurchases(),
            child: const Text('Restore purchases'),
          ),
        ],
      ),
    );
  }

  void showPendingUI() {
    setState(() {
      _purchasePending = true;
    });
  }

  Future<void> deliverProduct(PurchaseDetails purchaseDetails) async {
    // IMPORTANT!! Always verify purchase details before delivering the product.
    setState(() {
      _purchases.add(purchaseDetails);
      _purchasePending = false;
    });
  }

  void handleError(IAPError error) {
    setState(() {
      _purchasePending = false;
    });
  }

  Future<bool> _verifyPurchase(PurchaseDetails purchaseDetails) {
    // IMPORTANT!! Always verify a purchase before delivering the product.

    // Tizen specific verify purchase:
    // If `PurchaseDetails.status` is `purchased`, need to verify purchase.
    final InAppPurchaseTizenPlatformAddition platformAddition = _inAppPurchase
        .getPlatformAddition<InAppPurchaseTizenPlatformAddition>();
    return platformAddition.verifyPurchase(purchaseDetails: purchaseDetails);
  }

  void _handleInvalidPurchase(PurchaseDetails purchaseDetails) {
    // handle invalid purchase here if  _verifyPurchase` failed.
  }

  Future<void> _listenToPurchaseUpdated(
    List<PurchaseDetails> purchaseDetailsList,
  ) async {
    for (final PurchaseDetails purchaseDetails in purchaseDetailsList) {
      if (purchaseDetails.status == PurchaseStatus.pending) {
        showPendingUI();
      } else {
        if (purchaseDetails.status == PurchaseStatus.error) {
          handleError(purchaseDetails.error!);
        } else if (purchaseDetails.status == PurchaseStatus.purchased ||
            purchaseDetails.status == PurchaseStatus.restored) {
          final bool valid = await _verifyPurchase(purchaseDetails);
          if (valid) {
            unawaited(deliverProduct(purchaseDetails));
          } else {
            _handleInvalidPurchase(purchaseDetails);
            return;
          }
        }
      }
    }
  }
}
