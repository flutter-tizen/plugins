import 'file_utils.dart';
import 'package:xml/xml.dart';
import 'dart:io';

void main() {
  var doc = loadYamlFileSync("pubspec.yaml")?['flutter_splash_tizen'];
  if(doc == null)
    throw FormatException("could not read pubspec.yaml!");

  String tizenManifestPath = "tizen/tizen-manifest.xml";

  XmlDocument? tizenManifest = loadXMLFileSync(tizenManifestPath);
  if(tizenManifest == null)
    throw FormatException("could not read tizen-manifext.xml!");
    
  XmlNode el = tizenManifest.root;

  XmlElement? splashScreens = el
      .getElement("manifest")
      ?.getElement("ui-application")
      ?.getElement("splash-screens");
  if (splashScreens == null) {
    throw FormatException("error when reading $tizenManifestPath");
  }
  splashScreens.children.clear();
  File(tizenManifestPath).writeAsStringSync(el.toXmlString());
}
