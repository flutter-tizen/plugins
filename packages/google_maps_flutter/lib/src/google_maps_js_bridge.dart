// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:convert';
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

/// A piece of raw JavaScript code to be evaluated literally rather than
/// encoded as a string literal.
class JsExpression {
  /// Creates a [JsExpression] wrapping the raw [code].
  const JsExpression(this.code);

  /// The raw JavaScript code.
  final String code;

  @override
  String toString() => code;
}

/// Identifies the kind of a [MapsJsEvent] dispatched from the Google Maps
/// JavaScript runtime.
enum MapsJsEventType {
  /// The map's `bounds_changed` listener fired.
  boundsChanged,

  /// The map's `idle` listener fired.
  idle,

  /// The map's `tilesloaded` listener fired.
  tilesLoaded,

  /// The map was clicked.
  click,

  /// The map was long-pressed.
  longPress,

  /// A marker was clicked.
  markerClick,

  /// A marker cluster was clicked.
  clusterClick,

  /// A marker drag started.
  markerDragStart,

  /// A marker is being dragged.
  markerDrag,

  /// A marker drag ended.
  markerDragEnd,

  /// A polyline was clicked.
  polylineClick,

  /// A polygon was clicked.
  polygonClick,

  /// A circle was clicked.
  circleClick,

  /// A ground overlay was clicked.
  groundOverlayClick,
}

/// An event dispatched from the Google Maps JavaScript runtime back into
/// Dart through a [GoogleMapsJsBridge].
///
/// [message] carries the raw (JSON-encoded) payload posted from the JS side,
/// or `null` for a [type] that carries no payload.
typedef MapsJsEvent = ({MapsJsEventType type, String? message});

/// Mediates all interaction between Dart and the Google Maps JavaScript API
/// running inside a WebView.
///
/// This is the single seam through which JS commands are built and JS→Dart
/// events are dispatched, replacing ad hoc `runJavaScript` calls and
/// hand-built JS strings scattered across the plugin.
class GoogleMapsJsBridge {
  /// Creates a bridge over a fresh [WebViewController].
  GoogleMapsJsBridge() : controller = WebViewController();

  /// The JS-side channel name for each event type, registered in [load].
  static const Map<String, MapsJsEventType> _channelEventTypes =
      <String, MapsJsEventType>{
        'BoundChanged': MapsJsEventType.boundsChanged,
        'Idle': MapsJsEventType.idle,
        'Tilesloaded': MapsJsEventType.tilesLoaded,
        'Click': MapsJsEventType.click,
        'LongPress': MapsJsEventType.longPress,
        'MarkerClick': MapsJsEventType.markerClick,
        'ClusterClick': MapsJsEventType.clusterClick,
        'MarkerDragStart': MapsJsEventType.markerDragStart,
        'MarkerDrag': MapsJsEventType.markerDrag,
        'MarkerDragEnd': MapsJsEventType.markerDragEnd,
        'PolylineClick': MapsJsEventType.polylineClick,
        'PolygonClick': MapsJsEventType.polygonClick,
        'CircleClick': MapsJsEventType.circleClick,
        'GroundOverlayClick': MapsJsEventType.groundOverlayClick,
      };

  /// Event types whose JS side posts an empty payload, so [MapsJsEvent] is
  /// created with a `null` [MapsJsEvent.message] instead of `''`.
  static const Set<MapsJsEventType> _payloadlessEventTypes = <MapsJsEventType>{
    MapsJsEventType.boundsChanged,
    MapsJsEventType.idle,
    MapsJsEventType.tilesLoaded,
  };

  /// The underlying WebView controller. Exposed so callers can build the
  /// [WebViewWidget] that hosts this bridge's JS runtime.
  final WebViewController controller;

  final StreamController<MapsJsEvent> _events =
      StreamController<MapsJsEvent>.broadcast();
  final Completer<bool> _pageFinished = Completer<bool>();

  /// Broadcasts events received from the JS side.
  Stream<MapsJsEvent> get events => _events.stream;

  /// Adds [event] to [_events], unless this bridge has already been
  /// disposed.
  ///
  /// JS-side timers and in-flight `postMessage` calls can still invoke the
  /// channel callbacks below after [dispose] closes [_events], so emission
  /// must be guarded rather than left to throw on a closed controller.
  void _emit(MapsJsEvent event) {
    if (!_events.isClosed) {
      _events.add(event);
    }
  }

  /// Loads the map HTML shell and wires up the JS→Dart event channels.
  ///
  /// Completes once the page has finished loading.
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
      ..setJavaScriptMode(JavaScriptMode.unrestricted);

    for (final MapEntry<String, MapsJsEventType> entry
        in _channelEventTypes.entries) {
      final MapsJsEventType type = entry.value;
      controller.addJavaScriptChannel(
        entry.key,
        onMessageReceived: (JavaScriptMessage message) {
          _emit((
            type: type,
            message: _payloadlessEventTypes.contains(type)
                ? null
                : message.message,
          ));
        },
      );
    }

    controller.loadFile(path);

    return _pageFinished.future;
  }

  /// Creates the top-level `map` JS variable using [optionsJs] (a JS object
  /// literal), plus its built-in map-level listeners.
  Future<void> createMap(String optionsJs) async {
    final String command =
        '''
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

  /// Creates a JS object via `new <constructorExpression>`, assigns it to
  /// the JS-side variable [varName], and returns a [JsRef] handle to it.
  Future<JsRef> createObject(
    String varName,
    String constructorExpression,
  ) async {
    await controller.runJavaScript('var $varName = $constructorExpression;');
    return JsRef(varName);
  }

  /// Serializes [arg] for interpolation into a JavaScript snippet.
  ///
  /// [JsRef]s and [JsExpression]s are emitted as raw JS code via their
  /// [toString], so they refer to JS-side variables/expressions. Plain
  /// [String]s are JSON-encoded so they are safely quoted and escaped as JS
  /// string literals rather than being mistaken for raw code.
  String _serializeArg(Object? arg) {
    if (arg is JsRef || arg is JsExpression) {
      return arg.toString();
    }
    if (arg is String) {
      return jsonEncode(arg);
    }
    return arg.toString();
  }

  /// Assigns `ref[property] = value` on the JS side.
  Future<void> setProperty(JsRef ref, String property, Object? value) async {
    await controller.runJavaScript(
      "JSON.stringify($ref['$property'] = ${_serializeArg(value)})",
    );
  }

  /// Reads `ref.property` from the JS side.
  Future<Object?> getProperty(JsRef ref, String property) async {
    return controller.runJavaScriptReturningResult('$ref.$property');
  }

  /// Calls `ref.method(...args)` on the JS side, discarding the result.
  Future<void> callMethod(JsRef ref, String method, List<Object?> args) async {
    final String serializedArgs = '[${args.map(_serializeArg).join(', ')}]';
    await controller.runJavaScript(
      'JSON.stringify($ref.$method.apply($ref, $serializedArgs))',
    );
  }

  /// Calls `ref.method(...args)` on the JS side and returns the result.
  Future<Object?> callMethodReturning(
    JsRef ref,
    String method,
    List<Object?> args,
  ) async {
    final String serializedArgs = '[${args.map(_serializeArg).join(', ')}]';
    return controller.runJavaScriptReturningResult(
      '$ref.$method.apply($ref, $serializedArgs)',
    );
  }

  /// Registers `ref.addListener(eventName, ...)` on the JS side, so that
  /// [payloadJs] (a JS expression, evaluated with `event` bound to the
  /// listener's callback argument) is posted to [channel] whenever it fires.
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

  /// Escape hatch for JS not yet expressed in terms of the methods above.
  Future<void> runJavaScript(String script) async {
    await controller.runJavaScript(script);
  }

  /// Escape hatch for JS not yet expressed in terms of the methods above,
  /// returning the raw result.
  Future<Object> runJavaScriptReturningResult(String script) async {
    return controller.runJavaScriptReturningResult(script);
  }

  /// Releases the resources held by this bridge.
  void dispose() {
    _events.close();
  }
}
