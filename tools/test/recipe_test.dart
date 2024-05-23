// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter_tizen_plugin_tools/src/recipe.dart';
import 'package:flutter_tizen_plugin_tools/src/tizen_sdk.dart';
import 'package:test/test.dart';
import 'package:yaml/yaml.dart';

void main() {
  late String yamlString;
  setUp(() {
    yamlString = '''
plugins:
  a: ["mobile-6.0"]
  b: ["mobile-6.0", "tv-6.0"]

  c: []
''';
  });

  test('correctly parses recipe', () {
    final Recipe recipe = Recipe.fromYaml(loadYaml(yamlString) as YamlMap);

    expect(recipe.contains('c'), true);
    expect(recipe.isExcluded('c'), true);

    expect(recipe.contains('d'), false);
    expect(recipe.isExcluded('d'), false);

    expect(recipe.getProfiles('a').first.toString(), 'mobile-6.0');
    expect(recipe.getProfiles('b').map((Profile profile) => profile.toString()),
        unorderedEquals(<String>['mobile-6.0', 'tv-6.0']));
  });
}
