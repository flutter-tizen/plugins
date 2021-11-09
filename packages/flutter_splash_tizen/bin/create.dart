import 'file_utils.dart';
import 'dart:io';
import 'package:xml/xml.dart';

void main() {
  var doc = loadYamlFileSync("pubspec.yaml");
  if(doc == null)
    throw FormatException("could not read pubspec.yaml!");

  String color = doc["color"] ?? ""; //[TODO] generation of image in this color?
  String image = doc["image"] ?? "";

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
  
  File(tizenManifestPath).writeAsStringSync(el.toXmlString());
}
