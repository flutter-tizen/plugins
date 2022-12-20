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
    final Map<String, YamlList> packages =
        (yamlMap['plugins'] as YamlMap).cast<String, YamlList>();
    final Map<String, List<Profile>> profilesPerPackage =
        <String, List<Profile>>{};
    for (final MapEntry<String, YamlList> package in packages.entries) {
      profilesPerPackage[package.key] = package.value
          .map((dynamic profile) => Profile.fromString(profile as String))
          .toList();
    }
    return Recipe._(profilesPerPackage);
  }

  final Map<String, List<Profile>> _profilesPerPackage;

  /// Returns `true` if [package] was specified in the recipe file but
  /// with an empty profile list.
  bool isExcluded(String package) =>
      contains(package) && _profilesPerPackage[package]!.isEmpty;

  /// Returns `true` if [package] was specified in the recipe file.
  bool contains(String package) => _profilesPerPackage.containsKey(package);

  /// Returns a list of profiles to test on for [package].
  List<Profile> getProfiles(String package) {
    if (!contains(package)) {
      throw ArgumentError('Package $package not found in the recipe.');
    }
    return _profilesPerPackage[package]!;
  }
}
