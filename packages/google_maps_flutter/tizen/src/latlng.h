#ifndef FLUTTER_PLUGIN_GOOGLE_MAPS_FLUTTER_TIZEN_LATLNG_H_
#define FLUTTER_PLUGIN_GOOGLE_MAPS_FLUTTER_TIZEN_LATLNG_H_

class LatLng {
 public:
  LatLng(double latitude, double longitude);
  ~LatLng(){};

  double Latitude() { return latitude_; }
  double Longitude() { return longitude_; }

  bool Equals(const LatLng& op);
  bool operator==(const LatLng& op) { return Equals(op); }
  bool operator!=(const LatLng& op) { return !Equals(op); }

 private:
  double latitude_;
  double longitude_;
};

class LatLngBounds {
 public:
  LatLngBounds(LatLng ne, LatLng sw);
  ~LatLngBounds() {}

  LatLng northeast() { return northeast_; }
  LatLng southwest() { return southwest_; }

 private:
  LatLng northeast_;
  LatLng southwest_;
};

#endif  // FLUTTER_PLUGIN_GOOGLE_MAPS_FLUTTER_TIZEN_LATLNG_H_