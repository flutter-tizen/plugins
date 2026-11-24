// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:xml/xml.dart';
import 'package:yaml/yaml.dart';

final RegExp _version = RegExp(r'^\d+\.\d+$');
final RegExp _target = RegExp(r'^(common|mobile|tv)-(\d+\.\d+)$');
final RegExp _name = RegExp(r'^[a-zA-Z0-9_-]+$');
final RegExp _manifestValue = RegExp(r'^[a-zA-Z0-9][a-zA-Z0-9_.+-]*$');

/// A build profile and the exact API version to write into the manifest.
typedef ExampleTarget = ({String profile, String apiVersion});

/// Event steps indexed by pub package name and package-relative example path.
typedef ExampleScenarios = Map<String, Map<String, List<ExampleStep>>>;

/// Device OS version and the build targets selected for each pub package.
class ExampleTestRecipe {
  /// Validates the recipe and keeps OS and API versions as strings.
  ExampleTestRecipe.parse(String source) {
    final YamlMap yaml = _map(loadYaml(source), 'Recipe');
    if (yaml.keys.any((dynamic key) => key != 'platform_version' && key != 'plugins')) {
      throw const FormatException(
          'Use platform_version and plugins in the recipe, and --events for scenarios.');
    }
    final Object? version = yaml['platform_version'];
    if (version is! String || !_version.hasMatch(version)) {
      throw const FormatException('platform_version must be a quoted major.minor version.');
    }
    platformVersion = version;
    final YamlMap entries = _map(yaml['plugins'], 'plugins');
    for (final MapEntry<dynamic, dynamic> entry in entries.entries) {
      final Object? name = entry.key;
      final Object? values = entry.value;
      if (name is! String || !_name.hasMatch(name) || values is! YamlList) {
        throw FormatException('Expected a package name and target list in plugins: ${entry.key}');
      }
      final targets = <ExampleTarget>{};
      for (final Object? value in values) {
        final RegExpMatch? match = value is String ? _target.firstMatch(value) : null;
        if (match == null) {
          throw FormatException('$name: invalid target "$value". Expected e.g. common-11.0.');
        }
        if (!targets.add((profile: match[1]!, apiVersion: match[2]!))) {
          throw FormatException('$name: duplicate target "$value".');
        }
      }
      plugins[name] = targets.toList();
    }
    if (plugins.values.every((List<ExampleTarget> targets) => targets.isEmpty)) {
      throw const FormatException('The recipe must select at least one example target.');
    }
  }

  /// The OS version of the test device.
  late final String platformVersion;

  /// Build targets by pub package name. An empty list excludes a package.
  final Map<String, List<ExampleTarget>> plugins = <String, List<ExampleTarget>>{};
}

/// An ordered sequence of inputs followed by one screen capture.
class ExampleStep {
  /// Creates a named checkpoint after [events].
  const ExampleStep(this.id, this.events);

  /// The ID used in filenames and results.
  final String id;

  /// The inputs sent in order before capture.
  final List<ExampleEvent> events;
}

/// An input sent to the Tizen compositor.
sealed class ExampleEvent {
  const ExampleEvent();
}

/// A press and release of a named key.
class ExampleKey extends ExampleEvent {
  /// Creates an event for the compositor key [name].
  const ExampleKey(this.name);

  /// The key name understood by the compositor.
  final String name;
}

/// A touch press and release at a normalized point.
class ExampleTap extends ExampleEvent {
  /// Creates a tap at [x] and [y] in the range 0 to 1.
  const ExampleTap(this.x, this.y);

  /// Horizontal position relative to the display width.
  final double x;

  /// Vertical position relative to the display height.
  final double y;
}

/// Parses the optional event file. Example keys are package-relative paths.
ExampleScenarios parseExampleScenarios(String source) {
  final YamlMap scenarios = _map(_map(loadYaml(source), 'Event file')['scenarios'], 'scenarios');
  final result = <String, Map<String, List<ExampleStep>>>{};
  for (final MapEntry<dynamic, dynamic> package in scenarios.entries) {
    final Object? plugin = package.key;
    if (plugin is! String || !_name.hasMatch(plugin)) {
      throw FormatException('Invalid scenario package: $plugin');
    }
    final examples = <String, List<ExampleStep>>{};
    for (final MapEntry<dynamic, dynamic> example in _map(package.value, plugin).entries) {
      final Object? path = example.key;
      final Object? steps = example.value;
      if (path is! String || !path.split('/').every(_name.hasMatch) || steps is! YamlList) {
        throw FormatException('$plugin: expected a package-relative example path and step list.');
      }
      final stepIds = <String>{};
      examples[path] = <ExampleStep>[
        for (final Object? step in steps) _parseStep(step, '$plugin/$path', stepIds),
      ];
    }
    result[plugin] = examples;
  }
  return result;
}

ExampleStep _parseStep(Object? value, String path, Set<String> stepIds) {
  final YamlMap step = _map(value, '$path step');
  final Object? id = step['id'];
  final Object? events = step['events'];
  if (id is! String || !_name.hasMatch(id) || id == 'startup' || !stepIds.add(id)) {
    throw FormatException('$path: invalid or duplicate step ID "$id".');
  }
  if (step.length != 2 || events is! YamlList || events.isEmpty) {
    throw FormatException('$path/$id: expected a nonempty events list.');
  }
  return ExampleStep(id, <ExampleEvent>[
    for (final Object? event in events) _parseEvent(event, '$path/$id'),
  ]);
}

ExampleEvent _parseEvent(Object? value, String path) {
  final YamlMap event = _map(value, '$path event');
  final Object? key = event['key'];
  if (event.length == 1 && key is String && _name.hasMatch(key)) {
    return ExampleKey(key);
  }
  final Object? tap = event['tap'];
  if (event.length == 1 && tap is YamlMap) {
    final Object? x = tap['x'];
    final Object? y = tap['y'];
    if (tap.length == 2 && x is num && y is num && x >= 0 && x <= 1 && y >= 0 && y <= 1) {
      return ExampleTap(x.toDouble(), y.toDouble());
    }
  }
  throw FormatException('$path: expected a key name or tap with x and y between 0 and 1.');
}

YamlMap _map(Object? value, String label) {
  if (value is! YamlMap) {
    throw FormatException('$label must be a YAML map.');
  }
  return value;
}

/// A Tizen manifest with a mutable API version and IDs used by the runner.
class ExampleManifest {
  /// Parses the example's Tizen manifest XML.
  ExampleManifest.parse(String source) : _document = XmlDocument.parse(source) {
    if (_root.name.local != 'manifest') {
      throw const FormatException('Expected a Tizen manifest root element.');
    }
  }

  final XmlDocument _document;
  XmlElement get _root => _document.rootElement;

  /// The package identifier used by SDB uninstall.
  String get packageId => _attribute(_root, 'package');

  /// The expected filename of the TPK output.
  String get tpkName => '$packageId-${_attribute(_root, 'version')}.tpk';

  /// The UI application identifier used by the launcher.
  String get appId {
    final List<XmlElement> apps = _root.findElements('ui-application').toList();
    if (apps.length != 1) {
      throw const FormatException('Expected one ui-application in the example manifest.');
    }
    return _attribute(apps.single, 'appid');
  }

  /// The target API version written to the manifest root.
  String? get apiVersion => _root.getAttribute('api-version');

  set apiVersion(String? version) => _root.setAttribute('api-version', version);

  @override
  String toString() => _document.toXmlString();

  String _attribute(XmlElement element, String name) {
    final String? value = element.getAttribute(name);
    if (value == null || !_manifestValue.hasMatch(value)) {
      throw FormatException('Missing or invalid $name in the example manifest.');
    }
    return value;
  }
}
