import 'file_utils.dart';
import 'package:xml/xml.dart';

void main() {
  var doc = loadYamlFileSync("pubspec.yaml")?['flutter_splash_tizen'];
  String tizenManifestPath = "tizen/tizen-manifest.xml";

  XmlDocument tizenManifest = loadXMLFileSync(tizenManifestPath);
  XmlNode el = tizenManifest.root;

  XmlElement? splashScreens = el
      .getElement("manifest")
      ?.getElement("ui-application")
      ?.getElement("splash-screens");
  if (splashScreens == null) {
    throw FormatException("error when reading $tizenManifestPath");
  }
  splashScreens.children.clear();
  writeToFileSync(tizenManifestPath, el.toXmlString());
}
