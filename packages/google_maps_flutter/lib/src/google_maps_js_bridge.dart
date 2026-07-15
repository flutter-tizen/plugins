// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:io';

import 'package:webview_flutter/webview_flutter.dart';

/// The duration (in milliseconds) of mouse-down before it is treated as a
/// long press by the JS side of the map.
const int kGoogleMapsControllerLongPressDuration = 1000;

/// A handle to a JavaScript object living inside the Google Maps WebView.
///
/// Wraps the JS-side variable name so it can be interpolated into further
/// JavaScript snippets (via [toString]) without callers hand-building
/// variable names.
class JsRef {
  /// Creates a handle to the JS-side variable called [name].
  JsRef(this.name);

  /// The JS-side variable name this handle refers to.
  final String name;

  @override
  String toString() => name;
}

/// Base class for events dispatched from the Google Maps JavaScript runtime
/// back into Dart through a [GoogleMapsJsBridge].
abstract class MapsJsEvent {
  /// Const constructor for subclasses.
  const MapsJsEvent();
}

/// Fired when the map's `bounds_changed` listener fires.
class BoundsChangedJsEvent extends MapsJsEvent {
  /// Creates a [BoundsChangedJsEvent].
  const BoundsChangedJsEvent();
}

/// Fired when the map's `idle` listener fires.
class IdleJsEvent extends MapsJsEvent {
  /// Creates an [IdleJsEvent].
  const IdleJsEvent();
}

/// Fired when the map's `tilesloaded` listener fires.
class TilesLoadedJsEvent extends MapsJsEvent {
  /// Creates a [TilesLoadedJsEvent].
  const TilesLoadedJsEvent();
}

/// Base class for [MapsJsEvent]s that carry a raw JSON [message] payload.
abstract class MessageJsEvent extends MapsJsEvent {
  /// Creates a [MessageJsEvent] carrying the raw [message] payload.
  const MessageJsEvent(this.message);

  /// The raw (JSON-encoded) message payload posted from the JS side.
  final String message;
}

/// Fired when the map is clicked.
class ClickJsEvent extends MessageJsEvent {
  /// Creates a [ClickJsEvent].
  const ClickJsEvent(super.message);
}

/// Fired when the map is long-pressed.
class LongPressJsEvent extends MessageJsEvent {
  /// Creates a [LongPressJsEvent].
  const LongPressJsEvent(super.message);
}

/// Fired when a marker is clicked.
class MarkerClickJsEvent extends MessageJsEvent {
  /// Creates a [MarkerClickJsEvent].
  const MarkerClickJsEvent(super.message);
}

/// Fired when a marker cluster is clicked.
class ClusterClickJsEvent extends MessageJsEvent {
  /// Creates a [ClusterClickJsEvent].
  const ClusterClickJsEvent(super.message);
}

/// Fired when a marker drag starts.
class MarkerDragStartJsEvent extends MessageJsEvent {
  /// Creates a [MarkerDragStartJsEvent].
  const MarkerDragStartJsEvent(super.message);
}

/// Fired while a marker is being dragged.
class MarkerDragJsEvent extends MessageJsEvent {
  /// Creates a [MarkerDragJsEvent].
  const MarkerDragJsEvent(super.message);
}

/// Fired when a marker drag ends.
class MarkerDragEndJsEvent extends MessageJsEvent {
  /// Creates a [MarkerDragEndJsEvent].
  const MarkerDragEndJsEvent(super.message);
}

/// Fired when a polyline is clicked.
class PolylineClickJsEvent extends MessageJsEvent {
  /// Creates a [PolylineClickJsEvent].
  const PolylineClickJsEvent(super.message);
}

/// Fired when a polygon is clicked.
class PolygonClickJsEvent extends MessageJsEvent {
  /// Creates a [PolygonClickJsEvent].
  const PolygonClickJsEvent(super.message);
}

/// Fired when a circle is clicked.
class CircleClickJsEvent extends MessageJsEvent {
  /// Creates a [CircleClickJsEvent].
  const CircleClickJsEvent(super.message);
}

/// Fired when a ground overlay is clicked.
class GroundOverlayClickJsEvent extends MessageJsEvent {
  /// Creates a [GroundOverlayClickJsEvent].
  const GroundOverlayClickJsEvent(super.message);
}

/// Mediates all interaction between Dart and the Google Maps JavaScript API
/// running inside a WebView.
///
/// This is the single seam through which JS commands are built and JS→Dart
/// events are dispatched, replacing ad hoc `runJavaScript` calls and
/// hand-built JS strings scattered across the plugin.
abstract class GoogleMapsJsBridge {
  /// The underlying WebView controller. Exposed so callers can build the
  /// [WebViewWidget] that hosts this bridge's JS runtime.
  WebViewController get controller;

  /// Broadcasts events received from the JS side.
  Stream<MapsJsEvent> get events;

  /// Loads the map HTML shell and wires up the JS→Dart event channels.
  ///
  /// Completes once the page has finished loading.
  Future<void> load();

  /// Creates the top-level `map` JS variable using [optionsJs] (a JS object
  /// literal), plus its built-in map-level listeners.
  Future<void> createMap(String optionsJs);

  /// Creates a JS object via `new <constructorExpression>`, assigns it to
  /// the JS-side variable [varName], and returns a [JsRef] handle to it.
  Future<JsRef> createObject(String varName, String constructorExpression);

  /// Assigns `ref[property] = value` on the JS side.
  Future<void> setProperty(JsRef ref, String property, Object? value);

  /// Reads `ref.property` from the JS side.
  Future<Object?> getProperty(JsRef ref, String property);

  /// Calls `ref.method(...args)` on the JS side, discarding the result.
  Future<void> callMethod(JsRef ref, String method, List<Object?> args);

  /// Calls `ref.method(...args)` on the JS side and returns the result.
  Future<Object?> callMethodReturning(
    JsRef ref,
    String method,
    List<Object?> args,
  );

  /// Registers `ref.addListener(eventName, ...)` on the JS side, so that
  /// [payloadJs] (a JS expression, evaluated with `event` bound to the
  /// listener's callback argument) is posted to [channel] whenever it fires.
  Future<void> addListener(
    JsRef ref,
    String eventName,
    String channel,
    String payloadJs,
  );

  /// Escape hatch for JS not yet expressed in terms of the methods above.
  Future<void> runJavaScript(String script);

  /// Escape hatch for JS not yet expressed in terms of the methods above,
  /// returning the raw result.
  Future<Object> runJavaScriptReturningResult(String script);

  /// Releases the resources held by this bridge.
  void dispose();
}

/// A [GoogleMapsJsBridge] backed by a [WebViewController].
class WebViewGoogleMapsJsBridge implements GoogleMapsJsBridge {
  /// Creates a bridge over [controller], or a fresh [WebViewController] if
  /// none is provided.
  WebViewGoogleMapsJsBridge({WebViewController? controller})
      : controller = controller ?? WebViewController();

  @override
  final WebViewController controller;

  final StreamController<MapsJsEvent> _events =
      StreamController<MapsJsEvent>.broadcast();
  final Completer<bool> _pageFinished = Completer<bool>();

  @override
  Stream<MapsJsEvent> get events => _events.stream;

  @override
  Future<void> load() {
    String path = Platform.environment['AUL_ROOT_PATH'] ?? '';
    path += '/res/flutter_assets/assets/map.html';
    controller
      ..setNavigationDelegate(
        NavigationDelegate(
          onPageFinished: (String url) {
            if (!_pageFinished.isCompleted) {
              _pageFinished.complete(true);
            }
          },
        ),
      )
      ..setJavaScriptMode(JavaScriptMode.unrestricted)
      ..addJavaScriptChannel(
        'BoundChanged',
        onMessageReceived: (JavaScriptMessage message) {
          _events.add(const BoundsChangedJsEvent());
        },
      )
      ..addJavaScriptChannel(
        'Idle',
        onMessageReceived: (JavaScriptMessage message) {
          _events.add(const IdleJsEvent());
        },
      )
      ..addJavaScriptChannel(
        'Tilesloaded',
        onMessageReceived: (JavaScriptMessage message) {
          _events.add(const TilesLoadedJsEvent());
        },
      )
      ..addJavaScriptChannel(
        'Click',
        onMessageReceived: (JavaScriptMessage message) {
          _events.add(ClickJsEvent(message.message));
        },
      )
      ..addJavaScriptChannel(
        'LongPress',
        onMessageReceived: (JavaScriptMessage message) {
          _events.add(LongPressJsEvent(message.message));
        },
      )
      ..addJavaScriptChannel(
        'MarkerClick',
        onMessageReceived: (JavaScriptMessage message) {
          _events.add(MarkerClickJsEvent(message.message));
        },
      )
      ..addJavaScriptChannel(
        'ClusterClick',
        onMessageReceived: (JavaScriptMessage message) {
          _events.add(ClusterClickJsEvent(message.message));
        },
      )
      ..addJavaScriptChannel(
        'MarkerDragStart',
        onMessageReceived: (JavaScriptMessage message) {
          _events.add(MarkerDragStartJsEvent(message.message));
        },
      )
      ..addJavaScriptChannel(
        'MarkerDrag',
        onMessageReceived: (JavaScriptMessage message) {
          _events.add(MarkerDragJsEvent(message.message));
        },
      )
      ..addJavaScriptChannel(
        'MarkerDragEnd',
        onMessageReceived: (JavaScriptMessage message) {
          _events.add(MarkerDragEndJsEvent(message.message));
        },
      )
      ..addJavaScriptChannel(
        'PolylineClick',
        onMessageReceived: (JavaScriptMessage message) {
          _events.add(PolylineClickJsEvent(message.message));
        },
      )
      ..addJavaScriptChannel(
        'PolygonClick',
        onMessageReceived: (JavaScriptMessage message) {
          _events.add(PolygonClickJsEvent(message.message));
        },
      )
      ..addJavaScriptChannel(
        'CircleClick',
        onMessageReceived: (JavaScriptMessage message) {
          _events.add(CircleClickJsEvent(message.message));
        },
      )
      ..addJavaScriptChannel(
        'GroundOverlayClick',
        onMessageReceived: (JavaScriptMessage message) {
          _events.add(GroundOverlayClickJsEvent(message.message));
        },
      )
      ..loadFile(path);

    return _pageFinished.future;
  }

  @override
  Future<void> createMap(String optionsJs) async {
    final String command = '''
      map = new google.maps.Map(document.getElementById('map'), $optionsJs);
      map.addListener('bounds_changed', (event) => { BoundChanged.postMessage(''); });
      map.addListener('idle', (event) => { Idle.postMessage(''); });
      map.addListener('click', (event) => { Click.postMessage(JSON.stringify(event)); });
      map.addListener('tilesloaded', (evnet) => { Tilesloaded.postMessage(''); });

      let longPressTimeout;
      map.addListener('mousedown', (e) => {
                longPressTimeout = setTimeout(() => {
                    LongPress.postMessage(JSON.stringify(e));
                }, $kGoogleMapsControllerLongPressDuration);
            });
      map.addListener('mouseup', () => { clearTimeout(longPressTimeout); });
      map.addListener('mouseout', () => { clearTimeout(longPressTimeout); });

      const makeClusterEvent = function(clusterManagerId, event, cluster) {
          var result = '{"id": "' + clusterManagerId +'"';
          result += ', "cluster": {"count":' + cluster.count
          result += ', "position":' + JSON.stringify(cluster.position)
          result += ', "bounds":' + JSON.stringify(cluster.bounds);
          result += ', "markers": [';
          var i = 0;
          for (; i < cluster.markers.length - 1; i++) {
            result += cluster.markers[i].id;
            result += ', ';
          }
          result += cluster.markers[i].id;
          result += ']}}';

          return result;
        }
    ''';
    await controller.runJavaScript(command);
  }

  @override
  Future<JsRef> createObject(
    String varName,
    String constructorExpression,
  ) async {
    await controller.runJavaScript('var $varName = $constructorExpression;');
    return JsRef(varName);
  }

  @override
  Future<void> setProperty(
    JsRef ref,
    String property,
    Object? value,
  ) async {
    await controller.runJavaScript(
      "JSON.stringify($ref['$property'] = $value)",
    );
  }

  @override
  Future<Object?> getProperty(JsRef ref, String property) async {
    return controller.runJavaScriptReturningResult('$ref.$property');
  }

  @override
  Future<void> callMethod(
    JsRef ref,
    String method,
    List<Object?> args,
  ) async {
    await controller.runJavaScript(
      'JSON.stringify($ref.$method.apply($ref, $args))',
    );
  }

  @override
  Future<Object?> callMethodReturning(
    JsRef ref,
    String method,
    List<Object?> args,
  ) async {
    return controller.runJavaScriptReturningResult(
      '$ref.$method.apply($ref, $args)',
    );
  }

  @override
  Future<void> addListener(
    JsRef ref,
    String eventName,
    String channel,
    String payloadJs,
  ) async {
    await controller.runJavaScript(
      "$ref.addListener('$eventName', (event) => $channel.postMessage($payloadJs));",
    );
  }

  @override
  Future<void> runJavaScript(String script) async {
    await controller.runJavaScript(script);
  }

  @override
  Future<Object> runJavaScriptReturningResult(String script) async {
    return controller.runJavaScriptReturningResult(script);
  }

  @override
  void dispose() {
    _events.close();
  }
}
