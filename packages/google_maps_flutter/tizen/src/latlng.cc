#include "latlng.h"

#include <algorithm>

LatLng::LatLng(double latitude, double longitude) {
  latitude_ = std::min(std::max(latitude, -90.0), 90.0);
  longitude_ = std::min(std::max(longitude, -180.0), 180.0);
}

bool LatLng::Equals(const LatLng& op) {
  return latitude_ == op.latitude_ && longitude_ == op.longitude_;
}

LatLngBounds::LatLngBounds(LatLng ne, LatLng sw)
    : northeast_(ne), southwest_(sw) {}
