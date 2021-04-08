import 'dart:async';
import 'dart:typed_data';

import 'package:flutter/foundation.dart';
import 'package:flutter/gestures.dart';
import 'package:flutter/services.dart';
import 'package:flutter/widgets.dart';
import 'package:flutter/semantics.dart';
import 'package:flutter/rendering.dart';

import 'package:webview_flutter/webview_flutter.dart';
import 'package:webview_flutter/platform_interface.dart';
import 'package:webview_flutter/src/webview_method_channel.dart';

enum _TizenViewState {
  waitingForSize,
  creating,
  created,
  disposed,
}

enum PlatformViewHitTestBehavior {
  opaque,
  translucent,
  transparent,
}

enum _PlatformViewState {
  uninitialized,
  resizing,
  ready,
}

class TizenViewController extends PlatformViewController {
  TizenViewController._({
    @required this.viewId,
    @required String viewType,
    @required TextDirection layoutDirection,
    dynamic creationParams,
    MessageCodec<dynamic> creationParamsCodec,
    bool waitingForSize = true,
  })  : assert(viewId != null),
        assert(viewType != null),
        assert(layoutDirection != null),
        assert(creationParams == null || creationParamsCodec != null),
        _viewType = viewType,
        _layoutDirection = layoutDirection,
        _creationParams = creationParams,
        _creationParamsCodec = creationParamsCodec,
        _state = waitingForSize
            ? _TizenViewState.waitingForSize
            : _TizenViewState.creating;

  @override
  final int viewId;

  final String _viewType;

  TextDirection _layoutDirection;

  _TizenViewState _state;

  final dynamic _creationParams;

  final MessageCodec<dynamic> _creationParamsCodec;

  final List<PlatformViewCreatedCallback> _platformViewCreatedCallbacks =
      <PlatformViewCreatedCallback>[];

  static int pointerAction(int pointerId, int action) {
    return ((pointerId << 8) & 0xff00) | (action & 0xff);
  }

  int _textureId;

  int get textureId => _textureId;

  Size _size;

  Future<void> setSize(Size size) async {
    assert(_state != _TizenViewState.disposed,
        'trying to size a disposed Tizen View. View id: $viewId');

    assert(size != null);
    assert(!size.isEmpty);

    if (_state == _TizenViewState.waitingForSize) {
      _size = size;
      return create();
    }
    await SystemChannels.platform_views
        .invokeMethod<void>('resize', <String, dynamic>{
      'id': viewId,
      'width': size.width,
      'height': size.height,
    });
  }

  Future<void> _sendCreateMessage() async {
    assert(!_size.isEmpty,
        'trying to create $TizenViewController without setting a valid size.');

    final Map<String, dynamic> args = <String, dynamic>{
      'id': viewId,
      'viewType': _viewType,
      'width': _size.width,
      'height': _size.height,
      'direction': _layoutDirection == TextDirection.ltr ? 0 : 1,
    };
    if (_creationParams != null) {
      final ByteData paramsByteData =
          _creationParamsCodec.encodeMessage(_creationParams);
      args['params'] = Uint8List.view(
        paramsByteData.buffer,
        0,
        paramsByteData.lengthInBytes,
      );
    }
    _textureId =
        await SystemChannels.platform_views.invokeMethod<int>('create', args);
  }

  Future<void> _sendDisposeMessage() {
    return SystemChannels.platform_views
        .invokeMethod<void>('dispose', <String, dynamic>{
      'id': viewId,
      'hybrid': false,
    });
  }

  Future<void> create() async {
    assert(_state != _TizenViewState.disposed,
        'trying to create a disposed Tizen view');
    await _sendCreateMessage();

    _state = _TizenViewState.created;
    for (final PlatformViewCreatedCallback callback
        in _platformViewCreatedCallbacks) {
      callback(viewId);
    }
  }

  @Deprecated('Call `controller.viewId` instead. '
      'This feature was deprecated after v1.20.0-2.0.pre.')
  int get id => viewId;

  set pointTransformer(PointTransformer transformer) {
    assert(transformer != null);
  }

  bool get isCreated => _state == _TizenViewState.created;

  void addOnPlatformViewCreatedListener(PlatformViewCreatedCallback listener) {
    assert(listener != null);
    assert(_state != _TizenViewState.disposed);
    _platformViewCreatedCallbacks.add(listener);
  }

  /// Removes a callback added with [addOnPlatformViewCreatedListener].
  void removeOnPlatformViewCreatedListener(
      PlatformViewCreatedCallback listener) {
    assert(_state != _TizenViewState.disposed);
    _platformViewCreatedCallbacks.remove(listener);
  }

  Future<void> setLayoutDirection(TextDirection layoutDirection) async {
    assert(_state != _TizenViewState.disposed,
        'trying to set a layout direction for a disposed UIView. View id: $viewId');

    if (layoutDirection == _layoutDirection) {
      return;
    }

    assert(layoutDirection != null);
    _layoutDirection = layoutDirection;

    if (_state == _TizenViewState.waitingForSize) {
      return;
    }

    await SystemChannels.platform_views
        .invokeMethod<void>('setDirection', <String, dynamic>{
      'id': viewId,
      'direction': layoutDirection == TextDirection.ltr ? 0 : 1,
    });
  }

  @override
  Future<void> dispatchPointerEvent(PointerEvent event) async {
    if (event is PointerHoverEvent) {
      return;
    }

    int eventType = 0;
    if (event is PointerDownEvent) {
      eventType = 0;
    } else if (event is PointerMoveEvent) {
      eventType = 1;
    } else if (event is PointerUpEvent) {
      eventType = 2;
    } else {
      throw UnimplementedError('Not Implemented');
    }
    await SystemChannels.platform_views
        .invokeMethod<dynamic>('touch', <String, dynamic>{
      'id': viewId,
      'event': <dynamic>[
        eventType, // int, pointer event type
        event.buttons, // int, mouse button type (left, right, middle)
        event.localPosition.dx, // double, global position x
        event.localPosition.dy, // double, global position y
        event.localDelta.dx, // double, moved position x
        event.localDelta.dy, // double, moved position y
      ]
    });
  }

  @override
  Future<void> clearFocus() {
    if (_state != _TizenViewState.created) {
      return Future<void>.value();
    }
    print('TizenViewController::clearFocus() : $viewId');
    return SystemChannels.platform_views
        .invokeMethod<void>('clearFocus', viewId);
  }

  @override
  Future<void> dispose() async {
    print('TizenViewController::dispose()');
    if (_state == _TizenViewState.creating || _state == _TizenViewState.created)
      await _sendDisposeMessage();
    _platformViewCreatedCallbacks.clear();
    _state = _TizenViewState.disposed;
    PlatformViewsServiceTizen._instance._focusCallbacks.remove(viewId);
  }
}

class TizenView extends StatefulWidget {
  const TizenView({
    Key key,
    @required this.viewType,
    this.onPlatformViewCreated,
    this.hitTestBehavior = PlatformViewHitTestBehavior.opaque,
    this.layoutDirection,
    this.gestureRecognizers,
    this.creationParams,
    this.creationParamsCodec,
  })  : assert(viewType != null),
        assert(hitTestBehavior != null),
        assert(creationParams == null || creationParamsCodec != null),
        super(key: key);

  final String viewType;
  final PlatformViewCreatedCallback onPlatformViewCreated;
  final PlatformViewHitTestBehavior hitTestBehavior;
  final TextDirection layoutDirection;
  final Set<Factory<OneSequenceGestureRecognizer>> gestureRecognizers;
  final dynamic creationParams;
  final MessageCodec<dynamic> creationParamsCodec;

  @override
  State<TizenView> createState() => _TizenWebViewState();
}

class PlatformViewsServiceTizen {
  PlatformViewsServiceTizen._() {
    SystemChannels.platform_views.setMethodCallHandler(_onMethodCall);
  }
  static final PlatformViewsServiceTizen _instance =
      PlatformViewsServiceTizen._();

  Future<void> _onMethodCall(MethodCall call) {
    print('TizenView::_onMethodCall() - ${call.method}');
    switch (call.method) {
      case 'viewFocused':
        final int id = call.arguments as int;
        print('viewFocused: id - $id');
        if (_focusCallbacks.containsKey(id)) {
          if (_focusCallbacks[id] != null) {
            _focusCallbacks[id]();
          } else {
            throw FlutterError('FocusCallbacks[$id] must not be null.');
          }
        }
        break;
      default:
        throw UnimplementedError(
            "${call.method} was invoked but isn't implemented by PlatformViewsService");
    }
    return Future<void>.value();
  }

  final Map<int, VoidCallback> _focusCallbacks = <int, VoidCallback>{};

  static TizenViewController initTizenView({
    @required int id,
    @required String viewType,
    @required TextDirection layoutDirection,
    dynamic creationParams,
    MessageCodec<dynamic> creationParamsCodec,
    VoidCallback onFocus,
  }) {
    assert(id != null);
    assert(viewType != null);
    assert(layoutDirection != null);
    assert(creationParams == null || creationParamsCodec != null);

    print(
        'PlatformViewsServiceTizen::initTizenView [id:$id] [onFocus:$onFocus]');

    final TizenViewController controller = TizenViewController._(
      viewId: id,
      viewType: viewType,
      layoutDirection: layoutDirection,
      creationParams: creationParams,
      creationParamsCodec: creationParamsCodec,
    );

    _instance._focusCallbacks[id] = onFocus ?? () {};
    return controller;
  }
}

typedef _HandlePointerEvent = Future<void> Function(PointerEvent event);

// This recognizer constructs gesture recognizers from a set of gesture recognizer factories
// it was give, adds all of them to a gesture arena team with the _PlatformViewGestureRecognizer
// as the team captain.
// As long as the gesture arena is unresolved, the recognizer caches all pointer events.
// When the team wins, the recognizer sends all the cached pointer events to `_handlePointerEvent`, and
// sets itself to a "forwarding mode" where it will forward any new pointer event to `_handlePointerEvent`.
class _PlatformViewGestureRecognizer extends OneSequenceGestureRecognizer {
  _PlatformViewGestureRecognizer(
    _HandlePointerEvent handlePointerEvent,
    this.gestureRecognizerFactories, {
    PointerDeviceKind kind,
  }) : super(kind: kind) {
    team = GestureArenaTeam();
    team.captain = this;
    _gestureRecognizers = gestureRecognizerFactories.map(
      (Factory<OneSequenceGestureRecognizer> recognizerFactory) {
        final OneSequenceGestureRecognizer gestureRecognizer =
            recognizerFactory.constructor();
        gestureRecognizer.team = team;
        // The below gesture recognizers requires at least one non-empty callback to
        // compete in the gesture arena.
        // https://github.com/flutter/flutter/issues/35394#issuecomment-562285087
        if (gestureRecognizer is LongPressGestureRecognizer) {
          gestureRecognizer.onLongPress ??= () {};
        } else if (gestureRecognizer is DragGestureRecognizer) {
          gestureRecognizer.onDown ??= (_) {};
        } else if (gestureRecognizer is TapGestureRecognizer) {
          gestureRecognizer.onTapDown ??= (_) {};
        }
        return gestureRecognizer;
      },
    ).toSet();
    _handlePointerEvent = handlePointerEvent;
  }

  _HandlePointerEvent _handlePointerEvent;

  // Maps a pointer to a list of its cached pointer events.
  // Before the arena for a pointer is resolved all events are cached here, if we win the arena
  // the cached events are dispatched to `_handlePointerEvent`, if we lose the arena we clear the cache for
  // the pointer.
  final Map<int, List<PointerEvent>> cachedEvents = <int, List<PointerEvent>>{};

  // Pointer for which we have already won the arena, events for pointers in this set are
  // immediately dispatched to `_handlePointerEvent`.
  final Set<int> forwardedPointers = <int>{};

  // We use OneSequenceGestureRecognizers as they support gesture arena teams.
  // TODO(amirh): get a list of GestureRecognizers here.
  // https://github.com/flutter/flutter/issues/20953
  final Set<Factory<OneSequenceGestureRecognizer>> gestureRecognizerFactories;
  Set<OneSequenceGestureRecognizer> _gestureRecognizers;

  @override
  void addAllowedPointer(PointerDownEvent event) {
    startTrackingPointer(event.pointer, event.transform);
    for (final OneSequenceGestureRecognizer recognizer in _gestureRecognizers) {
      recognizer.addPointer(event);
    }
  }

  @override
  String get debugDescription => 'Platform view';

  @override
  void didStopTrackingLastPointer(int pointer) {}

  @override
  void handleEvent(PointerEvent event) {
    if (!forwardedPointers.contains(event.pointer)) {
      _cacheEvent(event);
    } else {
      _handlePointerEvent(event);
    }
    stopTrackingIfPointerNoLongerDown(event);
  }

  @override
  void acceptGesture(int pointer) {
    _flushPointerCache(pointer);
    forwardedPointers.add(pointer);
  }

  @override
  void rejectGesture(int pointer) {
    stopTrackingPointer(pointer);
    cachedEvents.remove(pointer);
  }

  void _cacheEvent(PointerEvent event) {
    if (!cachedEvents.containsKey(event.pointer)) {
      cachedEvents[event.pointer] = <PointerEvent>[];
    }
    cachedEvents[event.pointer].add(event);
  }

  void _flushPointerCache(int pointer) {
    cachedEvents.remove(pointer)?.forEach(_handlePointerEvent);
  }

  @override
  void stopTrackingPointer(int pointer) {
    super.stopTrackingPointer(pointer);
    forwardedPointers.remove(pointer);
  }

  void reset() {
    forwardedPointers.forEach(super.stopTrackingPointer);
    forwardedPointers.clear();
    cachedEvents.keys.forEach(super.stopTrackingPointer);
    cachedEvents.clear();
    resolve(GestureDisposition.rejected);
  }
}

class RenderTizenView extends RenderBox with _PlatformViewGestureMixin {
  /// Creates a render object for an Tizen view.
  RenderTizenView({
    @required TizenViewController viewController,
    @required PlatformViewHitTestBehavior hitTestBehavior,
    @required Set<Factory<OneSequenceGestureRecognizer>> gestureRecognizers,
  })  : assert(viewController != null),
        assert(hitTestBehavior != null),
        assert(gestureRecognizers != null),
        _viewController = viewController {
    _viewController.pointTransformer = (Offset offset) => globalToLocal(offset);
    updateGestureRecognizers(gestureRecognizers);
    _viewController.addOnPlatformViewCreatedListener(_onPlatformViewCreated);
    this.hitTestBehavior = hitTestBehavior;
  }

  _PlatformViewState _state = _PlatformViewState.uninitialized;

  TizenViewController get viewcontroller => _viewController;
  TizenViewController _viewController;

  set viewController(TizenViewController viewController) {
    assert(_viewController != null);
    assert(viewController != null);
    if (_viewController == viewController) {
      return;
    }
    _viewController.removeOnPlatformViewCreatedListener(_onPlatformViewCreated);
    _viewController = viewController;
    _sizePlatformView();
    if (_viewController.isCreated) {
      markNeedsSemanticsUpdate();
    }
    _viewController.addOnPlatformViewCreatedListener(_onPlatformViewCreated);
  }

  void _onPlatformViewCreated(int id) {
    markNeedsSemanticsUpdate();
  }

  void updateGestureRecognizers(
      Set<Factory<OneSequenceGestureRecognizer>> gestureRecognizers) {
    _updateGestureRecognizersWithCallBack(
        gestureRecognizers, _viewController.dispatchPointerEvent);
  }

  @override
  bool get sizedByParent => true;

  @override
  bool get alwaysNeedsCompositing => true;

  @override
  bool get isRepaintBoundary => true;

  @override
  void performResize() {
    size = constraints.biggest;
    _sizePlatformView();
  }

  Size _currentTizenViewSize;

  Future<void> _sizePlatformView() async {
    if (_state == _PlatformViewState.resizing || size.isEmpty) {
      return;
    }
    _state = _PlatformViewState.resizing;
    markNeedsPaint();

    Size targetSize;
    do {
      targetSize = size;
      await _viewController.setSize(targetSize);
      _currentTizenViewSize = targetSize;
    } while (size != targetSize);

    _state = _PlatformViewState.ready;
    markNeedsPaint();
  }

  @override
  void paint(PaintingContext context, Offset offset) {
    if (_viewController.textureId == null) {
      return;
    }
    if (size.width < _currentTizenViewSize.width ||
        size.height < _currentTizenViewSize.height) {
      context.pushClipRect(true, offset, offset & size, _paintTexture);
      return;
    }

    _paintTexture(context, offset);
  }

  void _paintTexture(PaintingContext context, Offset offset) {
    context.addLayer(TextureLayer(
      rect: offset & _currentTizenViewSize,
      textureId: _viewController.textureId,
      freeze: _state == _PlatformViewState.resizing,
    ));
  }

  @override
  void describeSemanticsConfiguration(SemanticsConfiguration config) {
    super.describeSemanticsConfiguration(config);

    config.isSemanticBoundary = true;

    if (_viewController.isCreated) {
      config.platformViewId = _viewController.viewId;
    }
  }
}

class _TizenPlatformTextureView extends LeafRenderObjectWidget {
  const _TizenPlatformTextureView({
    Key key,
    @required this.controller,
    @required this.hitTestBehavior,
    @required this.gestureRecognizers,
  })  : assert(controller != null),
        assert(hitTestBehavior != null),
        assert(gestureRecognizers != null),
        super(key: key);

  final TizenViewController controller;
  final PlatformViewHitTestBehavior hitTestBehavior;
  final Set<Factory<OneSequenceGestureRecognizer>> gestureRecognizers;

  @override
  RenderObject createRenderObject(BuildContext context) => RenderTizenView(
        viewController: controller,
        hitTestBehavior: hitTestBehavior,
        gestureRecognizers: gestureRecognizers,
      );

  @override
  void updateRenderObject(BuildContext context, RenderTizenView renderObject) {
    renderObject.viewController = controller;
    renderObject.hitTestBehavior = hitTestBehavior;
    renderObject.updateGestureRecognizers(gestureRecognizers);
  }
}

class _TizenWebViewState extends State<TizenView> {
  int _id;
  TizenViewController _controller;
  TextDirection _layoutDirection;
  bool _initialized = false;
  FocusNode _focusNode;

  static final Set<Factory<OneSequenceGestureRecognizer>> _emptyRecognizersSet =
      <Factory<OneSequenceGestureRecognizer>>{};

  @override
  Widget build(BuildContext context) {
    return Focus(
      focusNode: _focusNode,
      onFocusChange: _onFocusChange,
      child: _TizenPlatformTextureView(
          controller: _controller,
          hitTestBehavior: widget.hitTestBehavior,
          gestureRecognizers:
              widget.gestureRecognizers ?? _emptyRecognizersSet),
    );
  }

  void _initializeOnce() {
    if (_initialized) {
      return;
    }
    _initialized = true;
    _createNewTizenWebView();
    _focusNode = FocusNode(debugLabel: 'TizenWebView(id: $_id)');
  }

  @override
  void didChangeDependencies() {
    print('_TizenWebViewState::didChangeDependencies()');
    super.didChangeDependencies();
    final TextDirection newLayoutDirection = _findLayoutDirection();
    final bool didChangeLayoutDirection =
        _layoutDirection != newLayoutDirection;
    _layoutDirection = newLayoutDirection;

    _initializeOnce();
    if (didChangeLayoutDirection) {
      _controller.setLayoutDirection(_layoutDirection);
    }
  }

  @override
  void didUpdateWidget(TizenView oldWidget) {
    print('_TizenWebViewState::didUpdateWidget()');
    super.didUpdateWidget(oldWidget);

    final TextDirection newLayoutDirection = _findLayoutDirection();
    final bool didChangeLayoutDirection =
        _layoutDirection != newLayoutDirection;
    _layoutDirection = newLayoutDirection;

    if (widget.viewType != oldWidget.viewType) {
      _controller.dispose();
      _createNewTizenWebView();
      return;
    }

    if (didChangeLayoutDirection) {
      _controller.setLayoutDirection(_layoutDirection);
    }
  }

  TextDirection _findLayoutDirection() {
    assert(
        widget.layoutDirection != null || debugCheckHasDirectionality(context));
    return widget.layoutDirection ?? Directionality.of(context);
  }

  @override
  void dispose() {
    print('_TizenWebViewState::dispose()');
    _controller.dispose();
    super.dispose();
  }

  void _createNewTizenWebView() {
    _id = platformViewsRegistry.getNextPlatformViewId();
    _controller = PlatformViewsServiceTizen.initTizenView(
      id: _id,
      viewType: widget.viewType,
      layoutDirection: _layoutDirection,
      creationParams: widget.creationParams,
      creationParamsCodec: widget.creationParamsCodec,
      onFocus: () {
        print('_TizenWebViewState::_createNewTizenWebView() - onFocus()');
        _focusNode.requestFocus();
      },
    );
    if (widget.onPlatformViewCreated != null) {
      _controller
          .addOnPlatformViewCreatedListener(widget.onPlatformViewCreated);
    }
  }

  void _onFocusChange(bool isFocused) {
    print('_TizenWebViewState::_onFocusChange(isFocused:$isFocused)');
    if (!_controller.isCreated) {
      return;
    }

    if (!isFocused) {
      _controller.clearFocus().catchError((dynamic e) {
        if (e is MissingPluginException) {
          return;
        }
      });
      return;
    }
    SystemChannels.textInput
        .invokeMethod<void>(
      'TextInput.setPlatformViewClient',
      _id,
    )
        .catchError((dynamic e) {
      if (e is MissingPluginException) {
        return;
      }
    });
  }
}

class TizenWebView implements WebViewPlatform {
  static void register() {
    WebView.platform = TizenWebView();
  }

  @override
  Widget build({
    BuildContext context,
    CreationParams creationParams,
    @required WebViewPlatformCallbacksHandler webViewPlatformCallbacksHandler,
    WebViewPlatformCreatedCallback onWebViewPlatformCreated,
    Set<Factory<OneSequenceGestureRecognizer>> gestureRecognizers,
  }) {
    assert(webViewPlatformCallbacksHandler != null);
    return GestureDetector(
      onLongPress: () {},
      excludeFromSemantics: true,
      child: TizenView(
        viewType: 'plugins.flutter.io/webview',
        onPlatformViewCreated: (int id) {
          if (onWebViewPlatformCreated == null) {
            return;
          }
          onWebViewPlatformCreated(MethodChannelWebViewPlatform(
              id, webViewPlatformCallbacksHandler));
        },
        gestureRecognizers: gestureRecognizers,
        layoutDirection: TextDirection.rtl,
        creationParams:
            MethodChannelWebViewPlatform.creationParamsToMap(creationParams),
        creationParamsCodec: const StandardMessageCodec(),
      ),
    );
  }

  @override
  Future<bool> clearCookies() => MethodChannelWebViewPlatform.clearCookies();
}

bool _factoryTypesSetEquals<T>(Set<Factory<T>> a, Set<Factory<T>> b) {
  if (a == b) {
    return true;
  }
  if (a == null || b == null) {
    return false;
  }
  return setEquals(_factoriesTypeSet(a), _factoriesTypeSet(b));
}

Set<Type> _factoriesTypeSet<T>(Set<Factory<T>> factories) {
  return factories.map<Type>((Factory<T> factory) => factory.type).toSet();
}

/// The Mixin handling the pointer events and gestures of a platform view render box.
mixin _PlatformViewGestureMixin on RenderBox implements MouseTrackerAnnotation {
  /// How to behave during hit testing.
  // Changing _hitTestBehavior might affect which objects are considered hovered over.
  set hitTestBehavior(PlatformViewHitTestBehavior value) {
    if (value != _hitTestBehavior) {
      _hitTestBehavior = value;
      if (owner != null) {
        markNeedsPaint();
      }
    }
  }

  PlatformViewHitTestBehavior _hitTestBehavior;

  _HandlePointerEvent _handlePointerEvent;

  /// {@macro  flutter.rendering.platformView.updateGestureRecognizers}
  ///
  /// Any active gesture arena the `PlatformView` participates in is rejected when the
  /// set of gesture recognizers is changed.
  void _updateGestureRecognizersWithCallBack(
      Set<Factory<OneSequenceGestureRecognizer>> gestureRecognizers,
      _HandlePointerEvent handlePointerEvent) {
    assert(gestureRecognizers != null);
    assert(
      _factoriesTypeSet(gestureRecognizers).length == gestureRecognizers.length,
      'There were multiple gesture recognizer factories for the same type, there must only be a single '
      'gesture recognizer factory for each gesture recognizer type.',
    );
    if (_factoryTypesSetEquals(
        gestureRecognizers, _gestureRecognizer?.gestureRecognizerFactories)) {
      return;
    }
    _gestureRecognizer?.dispose();
    _gestureRecognizer =
        _PlatformViewGestureRecognizer(handlePointerEvent, gestureRecognizers);
    _handlePointerEvent = handlePointerEvent;
  }

  _PlatformViewGestureRecognizer _gestureRecognizer;

  @override
  bool hitTest(BoxHitTestResult result, {Offset position}) {
    if (_hitTestBehavior == PlatformViewHitTestBehavior.transparent ||
        !size.contains(position)) {
      return false;
    }
    result.add(BoxHitTestEntry(this, position));
    return _hitTestBehavior == PlatformViewHitTestBehavior.opaque;
  }

  @override
  bool hitTestSelf(Offset position) =>
      _hitTestBehavior != PlatformViewHitTestBehavior.transparent;

  @override
  PointerEnterEventListener get onEnter => null;

  @override
  PointerExitEventListener get onExit => null;

  @override
  MouseCursor get cursor => MouseCursor.uncontrolled;

  @override
  bool get validForMouseTracker => true;

  @override
  void handleEvent(PointerEvent event, HitTestEntry entry) {
    if (event is PointerDownEvent) {
      _gestureRecognizer.addPointer(event);
    }
    if (event is PointerHoverEvent) {
      _handlePointerEvent?.call(event);
    }
  }

  @override
  void detach() {
    _gestureRecognizer.reset();
    super.detach();
  }
}
