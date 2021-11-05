import 'dart:io';
import 'package:yaml/yaml.dart';
import 'package:xml/xml.dart';

Map? loadYamlFileSync(String path) {
  File file = File(path);
  if (file.existsSync() == true) {
    return loadYaml(file.readAsStringSync());
  }
  return null;
}

XmlDocument loadXMLFileSync(String path) {
  File file = File(path);
  if (file.existsSync() == true) {
    return XmlDocument.parse(file.readAsStringSync());
  }
  throw FormatException("could not read $path");
}

void writeToFileSync(String path, String data) {
  File file = File(path);
  if (file.existsSync() == true) {
    file.writeAsStringSync(data);
  } else {
    throw FormatException("could not write to $path");
  }
}
