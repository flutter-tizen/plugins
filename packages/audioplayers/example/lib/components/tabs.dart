import 'package:flutter/material.dart';

class Tabs extends StatelessWidget {
  final Map<String, Widget> tabs;
  const Tabs({super.key, required this.tabs});

  @override
  Widget build(BuildContext context) {
    return DefaultTabController(
      length: tabs.length,
      child: Scaffold(
        body: Column(
          crossAxisAlignment: CrossAxisAlignment.stretch,
          children: [
            TabBar(
              labelColor: Colors.black,
              tabs: tabs.keys.map((key) => Tab(text: key)).toList(),
            ),
            Expanded(
              child: TabBarView(
                children: tabs.values.toList(),
              ),
            ),
          ],
        ),
      ),
    );
  }
}
