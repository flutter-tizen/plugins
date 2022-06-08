// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// github.com:flutter/flutter.git@57a688c1f04d56eaa40beeb9f44e549eaf0ce54d
// packages/flutter/lib/src/rendering/platform_view.dart
// packages/flutter/lib/src/widgets/platform_view.dart
// packages/flutter/lib/src/services/platform_views.dart
// Imported from above files, the content has been minimally modified.
// (e.g. Rename keyword 'Android' -> 'Tizen' )

// ignore_for_file: public_member_api_docs

part of '../ewk_webview_flutter_tizen.dart';

enum _PlatformViewState {
  uninitialized,
  resizing,
  ready,
}

class TizenView extends StatefulWidget {
  const TizenView({
    super.key,
    required this.viewType,
    this.onPlatformViewCreated,
    this.hitTestBehavior = PlatformViewHitTestBehavior.opaque,
    this.layoutDirection,
    this.gestureRecognizers,
    this.creationParams,
    this.creationParamsCodec,
    this.clipBehavior = Clip.hardEdge,
  })  : assert(viewType != null),
        assert(hitTestBehavior != null),
        assert(creationParams == null || creationParamsCodec != null);

  final String viewType;
  final PlatformViewCreatedCallback? onPlatformViewCreated;
  final PlatformViewHitTestBehavior hitTestBehavior;
  final TextDirection? layoutDirection;
  final Set<Factory<OneSequenceGestureRecognizer>>? gestureRecognizers;
  final dynamic creationParams;
  final MessageCodec<dynamic>? creationParamsCodec;
  final Clip clipBehavior;

  @override
  State<TizenView> createState() => _TizenWebViewState();
}

class _TizenWebViewState extends State<TizenView> {
  int? _id;
  late TizenViewController _controller;
  TextDirection? _layoutDirection;
  bool _initialized = false;
  FocusNode? _focusNode;

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
        gestureRecognizers: widget.gestureRecognizers ?? _emptyRecognizersSet,
        clipBehavior: widget.clipBehavior,
      ),
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
    super.didChangeDependencies();
    final TextDirection newLayoutDirection = _findLayoutDirection();
    final bool didChangeLayoutDirection =
        _layoutDirection != newLayoutDirection;
    _layoutDirection = newLayoutDirection;

    _initializeOnce();
    if (didChangeLayoutDirection) {
      _controller.setLayoutDirection(_layoutDirection!);
    }
  }

  @override
  void didUpdateWidget(TizenView oldWidget) {
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
      _controller.setLayoutDirection(_layoutDirection!);
    }
  }

  TextDirection _findLayoutDirection() {
    assert(
        widget.layoutDirection != null || debugCheckHasDirectionality(context));
    return widget.layoutDirection ?? Directionality.of(context);
  }

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  void _createNewTizenWebView() {
    _id = platformViewsRegistry.getNextPlatformViewId();
    _controller = PlatformViewsServiceTizen.initTizenView(
      id: _id!,
      viewType: widget.viewType,
      layoutDirection: _layoutDirection!,
      creationParams: widget.creationParams,
      creationParamsCodec: widget.creationParamsCodec,
      onFocus: () {
        _focusNode!.requestFocus();
      },
    );
    if (widget.onPlatformViewCreated != null) {
      _controller
          .addOnPlatformViewCreatedListener(widget.onPlatformViewCreated!);
    }
  }

  void _onFocusChange(bool isFocused) {
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
    SystemChannels.textInput.invokeMethod<void>(
      'TextInput.setPlatformViewClient',
      <String, dynamic>{'platformViewId': _id},
    ).catchError((dynamic e) {
      if (e is MissingPluginException) {
        return;
      }
    });
  }
}

enum _TizenViewState {
  waitingForSize,
  creating,
  created,
  disposed,
}

class TizenViewController extends PlatformViewController {
  TizenViewController._({
    required this.viewId,
    required String viewType,
    required TextDirection layoutDirection,
    dynamic creationParams,
    MessageCodec<dynamic>? creationParamsCodec,
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

  final MessageCodec<dynamic>? _creationParamsCodec;

  final List<PlatformViewCreatedCallback> _platformViewCreatedCallbacks =
      <PlatformViewCreatedCallback>[];

  static int pointerAction(int pointerId, int action) {
    return ((pointerId << 8) & 0xff00) | (action & 0xff);
  }

  int? _textureId;

  int? get textureId => _textureId;

  /// The current offset of the platform view.
  Offset _off = Offset.zero;

  Future<Size> setSize(Size size) async {
    assert(_state != _TizenViewState.disposed,
        'Tizen view is disposed. View id: $viewId');
    assert(_state != _TizenViewState.waitingForSize,
        'Tizen view must have an initial size. View id: $viewId');
    assert(size != null);
    assert(!size.isEmpty);

    final Map<Object?, Object?>? meta =
        await SystemChannels.platform_views.invokeMapMethod<Object?, Object?>(
      'resize',
      <String, dynamic>{
        'id': viewId,
        'width': size.width,
        'height': size.height,
      },
    );
    assert(meta != null);
    assert(meta!.containsKey('width'));
    assert(meta!.containsKey('height'));
    return Size(meta!['width']! as double, meta['height']! as double);
  }

  Future<void> setOffset(Offset off) async {
    if (off == _off) {
      return;
    }

    if (_state != _TizenViewState.created) {
      return;
    }

    _off = off;

    await SystemChannels.platform_views.invokeMethod<void>(
      'offset',
      <String, dynamic>{
        'id': viewId,
        'top': off.dy,
        'left': off.dx,
      },
    );
  }

  Future<void> _sendCreateMessage({Size? size}) async {
    if (size == null) {
      return;
    }

    assert(!size.isEmpty,
        'trying to create $TizenViewController without setting a valid size.');

    final Map<String, dynamic> args = <String, dynamic>{
      'id': viewId,
      'viewType': _viewType,
      'width': size.width,
      'height': size.height,
      'direction': _layoutDirection == TextDirection.ltr ? 0 : 1,
    };
    if (_creationParams != null) {
      final ByteData paramsByteData =
          _creationParamsCodec!.encodeMessage(_creationParams)!;
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

  Future<void> create({Size? size}) async {
    assert(_state != _TizenViewState.disposed,
        'trying to create a disposed Tizen view');
    await _sendCreateMessage(size: size);

    _state = _TizenViewState.created;
    for (final PlatformViewCreatedCallback callback
        in _platformViewCreatedCallbacks) {
      callback(viewId);
    }
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
    return SystemChannels.platform_views
        .invokeMethod<void>('clearFocus', viewId);
  }

  @override
  Future<void> dispose() async {
    if (_state == _TizenViewState.creating ||
        _state == _TizenViewState.created) {
      await _sendDisposeMessage();
    }
    _platformViewCreatedCallbacks.clear();
    _state = _TizenViewState.disposed;
    PlatformViewsServiceTizen._instance._focusCallbacks.remove(viewId);
  }
}

class PlatformViewsServiceTizen {
  PlatformViewsServiceTizen._() {
    SystemChannels.platform_views.setMethodCallHandler(_onMethodCall);
  }
  static final PlatformViewsServiceTizen _instance =
      PlatformViewsServiceTizen._();

  Future<void> _onMethodCall(MethodCall call) {
    switch (call.method) {
      case 'viewFocused':
        final int id = call.arguments as int;
        if (_focusCallbacks.containsKey(id)) {
          _focusCallbacks[id]!();
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
    required int id,
    required String viewType,
    required TextDirection layoutDirection,
    dynamic creationParams,
    MessageCodec<dynamic>? creationParamsCodec,
    VoidCallback? onFocus,
  }) {
    assert(id != null);
    assert(viewType != null);
    assert(layoutDirection != null);
    assert(creationParams == null || creationParamsCodec != null);

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

/// A render object for an Tizen view.
///
/// [RenderTizenView] is responsible for sizing, displaying and passing touch events to Tizen
///
/// See also:
///
///  * [PlatformViewsService] which is a service for controlling platform views.
class RenderTizenView extends PlatformViewRenderBox {
  /// Creates a render object for an Tizen view.
  RenderTizenView({
    required TizenViewController viewController,
    required PlatformViewHitTestBehavior hitTestBehavior,
    required Set<Factory<OneSequenceGestureRecognizer>> gestureRecognizers,
    Clip clipBehavior = Clip.hardEdge,
  })  : assert(viewController != null),
        assert(hitTestBehavior != null),
        assert(gestureRecognizers != null),
        assert(clipBehavior != null),
        _viewController = viewController,
        _clipBehavior = clipBehavior,
        super(
            controller: viewController,
            hitTestBehavior: hitTestBehavior,
            gestureRecognizers: gestureRecognizers) {
    updateGestureRecognizers(gestureRecognizers);
    _viewController.addOnPlatformViewCreatedListener(_onPlatformViewCreated);
    this.hitTestBehavior = hitTestBehavior;
  }

  _PlatformViewState _state = _PlatformViewState.uninitialized;

  Size? _currentTextureSize;

  @override
  TizenViewController get controller => _viewController;

  TizenViewController _viewController;

  /// Sets a new Tizen view controller.
  ///
  /// `viewController` must not be null.
  @override
  set controller(TizenViewController viewController) {
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

  /// {@macro flutter.material.Material.clipBehavior}
  ///
  /// Defaults to [Clip.hardEdge], and must not be null.
  Clip get clipBehavior => _clipBehavior;
  Clip _clipBehavior = Clip.hardEdge;
  set clipBehavior(Clip value) {
    assert(value != null);
    if (value != _clipBehavior) {
      _clipBehavior = value;
      markNeedsPaint();
      markNeedsSemanticsUpdate();
    }
  }

  void _onPlatformViewCreated(int id) {
    markNeedsSemanticsUpdate();
  }

  @override
  bool get sizedByParent => true;

  @override
  bool get alwaysNeedsCompositing => true;

  @override
  bool get isRepaintBoundary => true;

  @override
  Size computeDryLayout(BoxConstraints constraints) {
    return constraints.biggest;
  }

  @override
  void performResize() {
    super.performResize();
    _sizePlatformView();
  }

  Future<void> _sizePlatformView() async {
    if (_state == _PlatformViewState.resizing || size.isEmpty) {
      return;
    }

    _state = _PlatformViewState.resizing;
    markNeedsPaint();

    Size targetSize;
    do {
      targetSize = size;
      if (_viewController.isCreated) {
        _currentTextureSize = await _viewController.setSize(targetSize);
      } else {
        await _viewController.create(size: targetSize);
        _currentTextureSize = targetSize;
      }
    } while (size != targetSize);

    _state = _PlatformViewState.ready;
    markNeedsPaint();
  }

  @override
  void paint(PaintingContext context, Offset offset) {
    if (_viewController.textureId == null || _currentTextureSize == null)
      return;

    final bool isTextureLargerThanWidget =
        _currentTextureSize!.width > size.width ||
            _currentTextureSize!.height > size.height;
    if (isTextureLargerThanWidget && clipBehavior != Clip.none) {
      _clipRectLayer.layer = context.pushClipRect(
        true,
        offset,
        offset & size,
        _paintTexture,
        clipBehavior: clipBehavior,
        oldLayer: _clipRectLayer.layer,
      );
      return;
    }
    _clipRectLayer.layer = null;
    _paintTexture(context, offset);
  }

  final LayerHandle<ClipRectLayer> _clipRectLayer =
      LayerHandle<ClipRectLayer>();

  @override
  void dispose() {
    _clipRectLayer.layer = null;
    super.dispose();
  }

  void _paintTexture(PaintingContext context, Offset offset) {
    if (_currentTextureSize == null) {
      return;
    }

    context.addLayer(TextureLayer(
      rect: offset & _currentTextureSize!,
      textureId: _viewController.textureId!,
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
    required this.controller,
    required this.hitTestBehavior,
    required this.gestureRecognizers,
    this.clipBehavior = Clip.hardEdge,
  })  : assert(controller != null),
        assert(hitTestBehavior != null),
        assert(gestureRecognizers != null);

  final TizenViewController controller;
  final PlatformViewHitTestBehavior hitTestBehavior;
  final Set<Factory<OneSequenceGestureRecognizer>> gestureRecognizers;
  final Clip clipBehavior;

  @override
  RenderObject createRenderObject(BuildContext context) => RenderTizenView(
        viewController: controller,
        hitTestBehavior: hitTestBehavior,
        gestureRecognizers: gestureRecognizers,
        clipBehavior: clipBehavior,
      );

  @override
  void updateRenderObject(BuildContext context, RenderTizenView renderObject) {
    renderObject.controller = controller;
    renderObject.hitTestBehavior = hitTestBehavior;
    renderObject.updateGestureRecognizers(gestureRecognizers);
    renderObject.clipBehavior = clipBehavior;
  }
}
