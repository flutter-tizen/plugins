import 'dart:async';

import 'package:flutter/material.dart';
import 'package:wearable_rotary/wearable_rotary.dart';

class CustomPageView extends StatefulWidget {
  final Axis scrollDirection;

  CustomPageView(this.scrollDirection);

  @override
  _CustomPageViewState createState() => _CustomPageViewState();
}

class _CustomPageViewState extends State<CustomPageView> {
  StreamSubscription<RotaryEvent>? _rotarySubscription;
  PageController _pager = PageController(initialPage: 0);
  int _currentPageIdx = 0;

  @override
  void initState() {
    super.initState();
    _rotarySubscription = rotaryEvent.listen((RotaryEvent event) {
      if (event == RotaryEvent.clockwise) {
        if (_currentPageIdx != Colors.primaries.length - 1) {
          _pager.animateToPage(
            ++_currentPageIdx,
            duration: Duration(milliseconds: 500),
            curve: Curves.ease,
          );
        }
      } else if (event == RotaryEvent.counterClockwise) {
        if (_currentPageIdx != 0) {
          _pager.animateToPage(
            --_currentPageIdx,
            duration: Duration(milliseconds: 500),
            curve: Curves.ease,
          );
        }
      }
    });
  }

  @override
  void dispose() {
    super.dispose();
    _rotarySubscription?.cancel();
    _pager.dispose();
  }

  @override
  Widget build(BuildContext context) {
    String _title = widget.scrollDirection == Axis.vertical
        ? 'verticalPageView'
        : 'HorizontalPageView';
    return Scaffold(
      appBar: AppBar(
        title: Text(_title),
      ),
      body: PageView.builder(
        controller: _pager,
        scrollDirection: widget.scrollDirection,
        itemCount: Colors.primaries.length,
        itemBuilder: (context, index) {
          return Padding(
            padding: const EdgeInsets.all(8.0),
            child: Container(color: Colors.primaries[index]),
          );
        },
        onPageChanged: (index) => {
          _currentPageIdx = index,
        },
      ),
    );
  }
}
