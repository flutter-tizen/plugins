// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:yaml/yaml.dart';

import 'tizen_sdk.dart';

/// A class that holds data parsed from recipe file.
class Recipe {
  Recipe._(Map<String, List<Profile>> profilesPerPackages,
      {List<String> excluded = const <String>[]})
      : _profilesPerPackage = profilesPerPackages,
        _excluded = excluded;

  /// Creates a [Recipe] instance by parsing [yamlMap].
  factory Recipe.fromYaml(YamlMap yamlMap) {
    final Map<String, YamlList> plugins =
        (yamlMap['plugins'] as YamlMap).cast<String, YamlList>();
    final List<String> excluded = <String>[];
    final Map<String, List<Profile>> profilesPerPackage =
        <String, List<Profile>>{};
    for (final MapEntry<String, YamlList> plugin in plugins.entries) {
      if (plugin.value.isEmpty) {
        excluded.add(plugin.key);
      } else {
        profilesPerPackage[plugin.key] = plugin.value
            .map((dynamic profile) => Profile.fromString(profile as String))
            .toList();
      }
    }
    return Recipe._(
      profilesPerPackage,
      excluded: excluded,
    );
  }

  final List<String> _excluded;
  final Map<String, List<Profile>> _profilesPerPackage;

  /// Returns `true` if [plugin] was specified in the recipe file but
  /// with an empty profile list.
  bool isExcluded(String plugin) => _excluded.contains(plugin);

  /// Returns `true` if [plugin] was specified in the recipe file.
  bool isRecognized(String plugin) =>
      _profilesPerPackage.containsKey(plugin) || _excluded.contains(plugin);

  /// Returns a list of profiles to test on for [plugin].
  List<Profile> getProfiles(String plugin) =>
      _profilesPerPackage[plugin] ?? <Profile>[];
}
