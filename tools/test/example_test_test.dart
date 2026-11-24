// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter_tizen_plugin_tools/src/example_test_config.dart';
import 'package:flutter_tizen_plugin_tools/src/example_test_device.dart';
import 'package:test/test.dart';

void main() {
  test('keeps runtime OS separate from each manifest API version', () {
    final recipe = ExampleTestRecipe.parse('''
platform_version: "11.0"
plugins:
  webview_flutter_tizen: ["common-11.0", "common-10.1"]
  skipped: []
''');

    expect(recipe.platformVersion, '11.0');
    expect(recipe.plugins['webview_flutter_tizen'], <ExampleTarget>[
      (profile: 'common', apiVersion: '11.0'),
      (profile: 'common', apiVersion: '10.1'),
    ]);
    expect(recipe.plugins['skipped'], isEmpty);
  });

  test('rejects an invalid API version or scenarios embedded in the recipe', () {
    expect(
      () => ExampleTestRecipe.parse('platform_version: "11.0"\nplugins: {webview: [common-11.]}'),
      throwsFormatException,
    );
    expect(
      () => ExampleTestRecipe.parse(
        'platform_version: "11.0"\nplugins: {webview: [common-11.0]}\nscenarios: {}',
      ),
      throwsFormatException,
    );
  });

  test('loads inputs in file order for a nested example', () {
    final ExampleScenarios examples = parseExampleScenarios('''
scenarios:
  webview_flutter_tizen:
    example/client:
      - id: open_menu
        events:
          - tap: {x: 0.98, y: 0.04}
          - key: Return
''');

    final ExampleStep step = examples['webview_flutter_tizen']!['example/client']!.single;
    expect(step.id, 'open_menu');
    expect(step.events, <Matcher>[
      isA<ExampleTap>().having((ExampleTap tap) => tap.x, 'x', 0.98),
      isA<ExampleKey>().having((ExampleKey key) => key.name, 'name', 'Return'),
    ]);
  });

  test('rejects out-of-bounds tap and conflicting checkpoint IDs', () {
    expect(
      () => parseExampleScenarios('''
scenarios:
  webview_flutter_tizen:
    example:
      - id: tap
        events: [{tap: {x: 1.01, y: 0.5}}]
'''),
      throwsFormatException,
    );
    expect(
      () => parseExampleScenarios('''
scenarios:
  webview_flutter_tizen:
    example:
      - id: startup
        events: [{key: Return}]
'''),
      throwsFormatException,
    );
  });

  test('updates the root manifest API independently of application IDs', () {
    final manifest = ExampleManifest.parse('''
<?xml version="1.0" encoding="utf-8"?>
<manifest xmlns="http://tizen.org/ns/packages" package="org.tizen.myapp" version="1.0.0" api-version="6.0">
  <profile name="common"/>
  <ui-application appid="org.tizen.myapp.ui"/>
</manifest>
''');
    manifest.apiVersion = '11.0';
    final rebuilt = ExampleManifest.parse(manifest.toString());

    expect(rebuilt.apiVersion, '11.0');
    expect(rebuilt.appId, 'org.tizen.myapp.ui');
    expect(rebuilt.tpkName, 'org.tizen.myapp-1.0.0.tpk');
    expect(rebuilt.toString(), contains('<profile name="common"/>'));
  });

  test('reports Dart exceptions only for the example process', () {
    final ExampleLogIssue issue = ExampleLogIssue.parse(<String, dynamic>{
      'pid': 42,
      'msg': '══╡ EXCEPTION CAUGHT BY GESTURE ╞══',
    })!;
    expect(issue.exception, isTrue);
    expect(issue.crash, isFalse);
    expect(issue.concerns('org.tizen.myapp', 42), isTrue);
    expect(issue.concerns('org.tizen.myapp', 43), isFalse);
  });

  test('correlates fatal logs from the crash reporter with the example', () {
    final ExampleLogIssue issue = ExampleLogIssue.parse(<String, dynamic>{
      'pid': 11,
      'tag': 'CRASH_MANAGER',
      'msg': 'crash-manager.c: run(1279) > Creating report for pid 42, pkgid org.tizen.myapp',
    })!;
    expect(issue.exception, isFalse);
    expect(issue.crash, isTrue);
    expect(issue.concerns('unrelated.app', 42), isTrue);
    expect(issue.concerns('org.tizen.myapp', null), isTrue);
    expect(issue.concerns('org.tizen.myapp.other', 142), isFalse);
    expect(
        ExampleLogIssue.parse(<String, dynamic>{'pid': 42, 'msg': 'Recovered normally'}), isNull);
  });

  test('recognizes the WebView signal and ignores normal termination', () {
    expect(
      ExampleLogIssue.parse(<String, dynamic>{'pid': 42, 'msg': 'Received signal 11 SEGV_MAPERR'})
          ?.crash,
      isTrue,
    );
    expect(
      ExampleLogIssue.parse(<String, dynamic>{'pid': 42, 'msg': 'Received signal 15'}),
      isNull,
    );
  });
}
