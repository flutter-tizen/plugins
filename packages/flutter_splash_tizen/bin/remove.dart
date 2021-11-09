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

  XmlElement? uiApp = el.getElement("manifest")?.getElement("ui-application");
  if (uiApp == null) {
    throw FormatException("error when reading $tizenManifestPath");
  }
  XmlElement? splashScreens = uiApp.getElement("splash-screens");

  if (splashScreens != null) {
    splashScreens.children.clear();
    File(tizenManifestPath).writeAsStringSync(el.toXmlString());
  }
}
