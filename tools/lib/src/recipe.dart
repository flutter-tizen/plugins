// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:yaml/yaml.dart';

import 'tizen_sdk.dart';

/// A class that holds data parsed from recipe file.
class Recipe {
  Recipe._(Map<String, List<Profile>> profilesPerPackage)
      : _profilesPerPackage = profilesPerPackage;

  /// Creates a [Recipe] instance by parsing [yamlMap].
  factory Recipe.fromYaml(YamlMap yamlMap) {
    final Map<String, YamlList> plugins =
        (yamlMap['plugins'] as YamlMap).cast<String, YamlList>();
    final Map<String, List<Profile>> profilesPerPackage =
        <String, List<Profile>>{};
    for (final MapEntry<String, YamlList> plugin in plugins.entries) {
      profilesPerPackage[plugin.key] = plugin.value
          .map((dynamic profile) => Profile.fromString(profile as String))
          .toList();
    }
    return Recipe._(profilesPerPackage);
  }

  final Map<String, List<Profile>> _profilesPerPackage;

  /// Returns `true` if [plugin] was specified in the recipe file but
  /// with an empty profile list.
  bool isExcluded(String plugin) =>
      contains(plugin) && _profilesPerPackage[plugin]!.isEmpty;

  /// Returns `true` if [plugin] was specified in the recipe file.
  bool contains(String plugin) => _profilesPerPackage.containsKey(plugin);

  /// Returns a list of profiles to test on for [plugin].
  List<Profile> getProfiles(String plugin) {
    if (!contains(plugin)) {
      throw ArgumentError('Package $plugin is not included in recipe.');
    }
    return _profilesPerPackage[plugin]!;
  }
}
