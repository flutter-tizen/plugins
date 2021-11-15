import 'dart:io';
import 'package:yaml/yaml.dart';
import 'package:xml/xml.dart';

XmlDocument? loadXMLFileSync(String path) {
  File file = File(path);
  if (file.existsSync()) {
    return XmlDocument.parse(file.readAsStringSync());
  }
  return null;
}

void main() {
  String tizenManifestPath = "tizen/tizen-manifest.xml";

  XmlDocument? tizenManifest = loadXMLFileSync(tizenManifestPath);
  if (tizenManifest == null)
    throw FormatException("could not read tizen-manifext.xml!");

  XmlNode el = tizenManifest.root;

  XmlElement? uiApp = el.getElement("manifest")?.getElement("ui-application");
  if (uiApp == null) {
    throw FormatException("error when reading $tizenManifestPath");
  }
  XmlElement? splashScreens = uiApp.getElement("splash-screens");

  if (splashScreens != null) {
    splashScreens.children.clear();
    File(tizenManifestPath)
        .writeAsStringSync(el.toXmlString(pretty: true, indent: '    ') + '\n');
  }
}
