#ifndef FLUTTER_PLUGIN_GOOGLE_MAPS_FLUTTER_TIZEN_CONVERT_H_
#define FLUTTER_PLUGIN_GOOGLE_MAPS_FLUTTER_TIZEN_CONVERT_H_

#include <map>
#include <string>

#include "latlng.h"

class Convert {
 public:
  static bool GetBound(const std::string& str, double& south, double& west,
                       double& north, double& east);
};

#endif  // FLUTTER_PLUGIN_GOOGLE_MAPS_FLUTTER_TIZEN_CONVERT_H_
