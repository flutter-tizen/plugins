import 'dart:io';
import 'package:yaml/yaml.dart';
import 'package:xml/xml.dart';

Map? loadYamlFileSync(String path) {
  File file = File(path);
  if (file.existsSync()) {
    return loadYaml(file.readAsStringSync());
  }
  return null;
}

XmlDocument? loadXMLFileSync(String path) {
  File file = File(path);
  if (file.existsSync()) {
    return XmlDocument.parse(file.readAsStringSync());
  }
  return null;
}
