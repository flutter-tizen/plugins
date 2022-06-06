// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:ui';

import 'package:flutter/material.dart';
import 'package:flutter/rendering.dart';

///The widget which punch a hole on flutter UI
class Hole extends LeafRenderObjectWidget {
  ///The constructor of Hole widget
  const Hole({Key? key}) : super(key: key);

  @override
  HoleBox createRenderObject(BuildContext context) => HoleBox();
}

///The render object of Hole widget
class HoleBox extends RenderBox {
  @override
  bool get sizedByParent => true;

  @override
  bool get alwaysNeedsCompositing => true;

  @override
  bool get isRepaintBoundary => true;

  @override
  void performResize() {
    size = constraints.biggest;
  }

  @override
  bool hitTestSelf(Offset position) {
    return true;
  }

  @override
  void paint(PaintingContext context, Offset offset) {
    context.addLayer(HoleLayer(rect: offset & size));
  }
}

///A composite layer that draw a rect with blend mode.
class HoleLayer extends Layer {
  ///Repesent position of hole widget.
  final Rect rect;

  ///The constructor of hole layer.
  HoleLayer({
    required this.rect,
  });

  @override
  void addToScene(SceneBuilder builder, [Offset layerOffset = Offset.zero]) {
    builder.addPicture(layerOffset, _createHolePicture(rect));
  }

  Picture _createHolePicture(Rect holeRect) {
    PictureRecorder recorder = PictureRecorder();
    Canvas canvas = Canvas(recorder);
    Paint paint = Paint();
    paint.color = Colors.transparent;
    paint.blendMode = BlendMode.src;
    canvas.drawRect(
        Rect.fromLTWH(holeRect.topLeft.dx, holeRect.topLeft.dy, holeRect.width,
            holeRect.height),
        paint);
    return recorder.endRecording();
  }
}
