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

void main() {
  var doc = loadYamlFileSync("pubspec.yaml")?['flutter_splash_tizen'];
  if (doc == null) throw const FormatException("could not read pubspec.yaml!");

  String? image = doc["image"];
  if (image == null) {
    throw const FormatException("could not find image section!");
  }
  const String tizenManifestPath = "tizen/tizen-manifest.xml";

  XmlDocument? tizenManifest = loadXMLFileSync(tizenManifestPath);
  if (tizenManifest == null) {
    throw const FormatException("could not read tizen-manifext.xml!");
  }
  XmlNode el = tizenManifest.root;

  XmlElement? uiApp = el.getElement("manifest")?.getElement("ui-application");
  if (uiApp == null) {
    throw const FormatException("error when reading $tizenManifestPath");
  }
  XmlElement? splashScreens = uiApp.getElement("splash-screens");
  if (splashScreens == null) {
    splashScreens = XmlElement(XmlName("splash-screens"));
    uiApp.children.add(splashScreens);
  }

  splashScreens.children.clear();
  XmlElement splashScreen = XmlElement(XmlName("splash-screen"));
  splashScreen.setAttribute("src", image);
  splashScreen.setAttribute("type", "img");
  splashScreen.setAttribute("indicator-display", "false");
  splashScreen.setAttribute("orientation", "portrait");
  splashScreens.children.add(splashScreen);

  File(tizenManifestPath)
      .writeAsStringSync(el.toXmlString(pretty: true, indent: '    ') + '\n');
}
