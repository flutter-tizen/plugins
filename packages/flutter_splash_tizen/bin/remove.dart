import 'dart:io';
import 'package:xml/xml.dart';

void main() {
  const String tizenManifestPath = "tizen/tizen-manifest.xml";
  File tizenManifestFile = File(tizenManifestPath);
  if (!tizenManifestFile.existsSync()) {
    throw const FormatException("could not read tizen-manifext.xml!");
  }
  XmlDocument? tizenManifest =
      XmlDocument.parse(tizenManifestFile.readAsStringSync());
  XmlNode el = tizenManifest.root;

  XmlElement? uiApp = el.getElement("manifest")?.getElement("ui-application");
  if (uiApp == null) {
    throw const FormatException("error when reading $tizenManifestPath");
  }
  XmlElement? splashScreens = uiApp.getElement("splash-screens");

  if (splashScreens != null) {
    splashScreens.children.clear();
    File(tizenManifestPath)
        .writeAsStringSync(el.toXmlString(pretty: true, indent: '    ') + '\n');
  }
}
