import 'package:flutter/material.dart';
import 'package:flutter/widgets.dart';

import '../model/main_item.dart';

/// Main item widget.
class MainItemWidget extends StatefulWidget {
  /// Main item widget.
  const MainItemWidget(this.item, this.onTap, {Key? key}) : super(key: key);

  /// item data.
  final MainItem item;

  /// onTap action (typically run or open).
  final Function(MainItem item) onTap; // = Function(MainItem item);

  @override
  _MainItemWidgetState createState() => _MainItemWidgetState();
}

class _MainItemWidgetState extends State<MainItemWidget> {
  @override
  Widget build(BuildContext context) {
    return ListTile(
        title: Text(widget.item.title),
        subtitle: Text(widget.item.description),
        onTap: _onTap);
  }

  void _onTap() {
    widget.onTap(widget.item);

    //print(widget.item.route);
    //Navigator.pushNamed(context, widget.item.route);
  }
}
