// ignore_for_file: public_member_api_docs

import 'dart:async';

import 'package:flutter/material.dart';
import 'package:wearable_rotary/wearable_rotary.dart';

class CustomPageView extends StatefulWidget {
  const CustomPageView(this.scrollDirection, {Key? key}) : super(key: key);

  final Axis scrollDirection;

  @override
  State<CustomPageView> createState() => _CustomPageViewState();
}

class _CustomPageViewState extends State<CustomPageView> {
  StreamSubscription<RotaryEvent>? _rotarySubscription;
  final PageController _pager = PageController();
  int _currentPageIdx = 0;

  @override
  void initState() {
    super.initState();
    _rotarySubscription = rotaryEvents.listen((RotaryEvent event) {
      if (event == RotaryEvent.clockwise) {
        if (_currentPageIdx != Colors.primaries.length - 1) {
          _pager.animateToPage(
            ++_currentPageIdx,
            duration: const Duration(milliseconds: 300),
            curve: Curves.ease,
          );
        }
      } else if (event == RotaryEvent.counterClockwise) {
        if (_currentPageIdx != 0) {
          _pager.animateToPage(
            --_currentPageIdx,
            duration: const Duration(milliseconds: 300),
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
    final String title = widget.scrollDirection == Axis.vertical
        ? 'VerticalPageView'
        : 'HorizontalPageView';
    return Scaffold(
      appBar: AppBar(title: Text(title)),
      body: PageView.builder(
        controller: _pager,
        scrollDirection: widget.scrollDirection,
        itemCount: Colors.primaries.length,
        itemBuilder: (BuildContext context, int index) {
          return Padding(
            padding: const EdgeInsets.all(8.0),
            child: Container(color: Colors.primaries[index]),
          );
        },
        onPageChanged: (int index) => _currentPageIdx = index,
      ),
    );
  }
}
