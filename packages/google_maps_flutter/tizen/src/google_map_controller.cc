#include "google_map_controller.h"

#include "convert.h"
#include "log.h"
#include "lwe/LWEWebView.h"
#include "lwe/PlatformIntegrationData.h"

extern "C" size_t LWE_EXPORT createWebViewInstance(
    unsigned x, unsigned y, unsigned width, unsigned height,
    float devicePixelRatio, const char* defaultFontName, const char* locale,
    const char* timezoneID,
    const std::function<::LWE::WebContainer::ExternalImageInfo(void)>&
        prepareImageCb,
    const std::function<void(::LWE::WebContainer*, bool isRendered)>&
        renderedCb);

GoogleMapController::GoogleMapController(
    flutter::PluginRegistrar* registrar, int view_id,
    FlutterTextureRegistrar* texture_registrar, double width, double height,
    flutter::EncodableMap& params)
    : PlatformView(registrar, view_id),
      texture_registrar_(texture_registrar),
      mapview_(nullptr),
      width_(width),
      height_(height),
      tbm_surface_(nullptr),
      is_mouse_lbutton_down_(false),
      mapReadyResult_() {
  SetTextureId(FlutterRegisterExternalTexture(texture_registrar_));
  InitWebView();

  LOG_DEBUG("GoogleMapController::GoogleMapController [%s] \n ",
            GetChannelName(view_id).c_str());

  channel_ = std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
      GetPluginRegistrar()->messenger(), GetChannelName(view_id),
      &flutter::StandardMethodCodec::GetInstance());
  channel_->SetMethodCallHandler(
      [webview = this](const auto& call, auto result) {
        webview->HandleMethodCall(call, std::move(result));
      });

  std::string url = "https://seungsoo47.github.io/map.html";

  ////////////////////////////////////////////////
  // TODO: Add callbacks to webview_instance
  ////////////////////////////////////////////////

  mapview_->LoadURL(url);
}

GoogleMapController::~GoogleMapController() { Dispose(); }

void GoogleMapController::InitWebView() {
  if (mapview_ != nullptr) {
    mapview_->Destroy();
    mapview_ = nullptr;
  }
  float scale_factor = 1;

  mapview_ = (LWE::WebContainer*)createWebViewInstance(
      0, 0, width_, height_, scale_factor, "SamsungOneUI", "ko-KR",
      "Asia/Seoul",
      [this]() -> LWE::WebContainer::ExternalImageInfo {
        LWE::WebContainer::ExternalImageInfo result;
        if (!tbm_surface_) {
          tbm_surface_ =
              tbm_surface_create(width_, height_, TBM_FORMAT_ARGB8888);
        }
        result.imageAddress = (void*)tbm_surface_;
        return result;
      },
      [this](LWE::WebContainer* c, bool isRendered) {
        if (isRendered) {
          FlutterMarkExternalTextureFrameAvailable(
              texture_registrar_, GetTextureId(), tbm_surface_);
          tbm_surface_destroy(tbm_surface_);
          tbm_surface_ = nullptr;
        }
      });
  auto settings = mapview_->GetSettings();
  settings.SetUserAgentString(
      "Mozilla/5.0 (like Gecko/54.0 Firefox/54.0) Mobile");
  mapview_->SetSettings(settings);
}

std::string GoogleMapController::GetChannelName(int view_id) {
  return "plugins.flutter.io/google_maps_" + std::to_string(view_id);
}

std::string GoogleMapController::GetVisibleRegion() {
  std::string request = "JSON.stringify(map.getBounds()); ";
  std::string response = mapview_->EvaluateJavaScript(request);
  LOG_DEBUG("GetVisibleRegion() : %s", response.c_str());
  return response;
}

void GoogleMapController::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue>& method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  if (!mapview_) {
    return;
  }
  const auto name = method_call.method_name();
  // const auto& arguments = *method_call.arguments();

  LOG_DEBUG("GoogleMapController::HandleMethodCall() : %s", name.c_str());

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////
  if (!name.compare("map#waitForMap")) {
    if (mapview_) {
      result->Success();
      return;
    }
    mapReadyResult_ = std::move(result);
  } else if (!name.compare("map#update")) {
    // {
    // final Map<String, Object> data = new HashMap<>();
    // data.put("bearing", position.bearing);
    // data.put("target", latLngToJson(position.target));
    // data.put("tilt", position.tilt);
    // data.put("zoom", position.zoom);
    // }
    // -----------------------------------------------------------------------
    // Convert.interpretGoogleMapOptions(call.argument("options"), this);
    // result->Success(Convert.cameraPositionToJson(getCameraPosition()));
    // -----------------------------------------------------------------------
    result->Success(flutter::EncodableValue());

  } else if (!name.compare("map#getVisibleRegion")) {
    if (mapview_) {
      std::string str = GetVisibleRegion();
      double south = 0.0, west = 0.0, north = 0.0, east = 0.0;
      Convert::GetBound(str, south, west, north, east);

      LOG_DEBUG("south : %f, west : %f, north : %f, east: %f", south, west,
                north, east);
      flutter::EncodableMap data;
      flutter::EncodableList SW = {flutter::EncodableValue(south),
                                   flutter::EncodableValue(west)};
      flutter::EncodableList NE = {flutter::EncodableValue(north),
                                   flutter::EncodableValue(east)};
      data.insert(
          std::make_pair<flutter::EncodableValue, flutter::EncodableValue>(
              flutter::EncodableValue("southwest"),
              flutter::EncodableValue(SW)));
      data.insert(
          std::make_pair<flutter::EncodableValue, flutter::EncodableValue>(
              flutter::EncodableValue("northeast"),
              flutter::EncodableValue(NE)));
      result->Success(flutter::EncodableValue(data));
    } else {
      result->Error("GoogleMap uninitialized",
                    "getVisibleRegion called prior to map initialization");
    }
  } else if (!name.compare("map#getScreenCoordinate")) {
    // if (mapview_) {
    //   LatLng latLng = Convert.toLatLng(call.arguments);
    //   Point screenLocation =
    //   googleMap.getProjection().toScreenLocation(latLng);
    //   result->Success(Convert.pointToJson(screenLocation));
    // } else {
    //   result->Error("GoogleMap uninitialized",
    //                 "getScreenCoordinate called prior to map
    //                 initialization");
    // }
  } else if (!name.compare("map#getLatLng")) {
    // if (mapview_) {
    //   Point point = Convert.toPoint(call.arguments);
    //   LatLng latLng = googleMap.getProjection().fromScreenLocation(point);
    //   result->Success(Convert.latLngToJson(latLng));
    // } else {
    //   result->Error("GoogleMap uninitialized",
    //                 "getLatLng called prior to map initialization");
    // }
  } else if (!name.compare("map#takeSnapshot")) {
    // if (mapview_) {
    //   final MethodChannel.Result _result = result;
    //   googleMap.snapshot(new SnapshotReadyCallback() {
    //     @Override public void onSnapshotReady(Bitmap bitmap) {
    //       ByteArrayOutputStream stream = new ByteArrayOutputStream();
    //       bitmap.compress(Bitmap.CompressFormat.PNG, 100, stream);
    //       byte[] byteArray = stream.toByteArray();
    //       bitmap.recycle();
    //       _result->Success(byteArray);
    //     }
    //   });
    // } else {
    //   result->Error("GoogleMap uninitialized", "takeSnapshot");
    // }
  } else if (!name.compare("camera#move")) {
    // final CameraUpdate cameraUpdate =
    //     Convert.toCameraUpdate(call.argument("cameraUpdate"), density);
    // moveCamera(cameraUpdate);
    // result->Success();
  } else if (!name.compare("camera#animate")) {
    // final CameraUpdate cameraUpdate =
    //     Convert.toCameraUpdate(call.argument("cameraUpdate"), density);
    // animateCamera(cameraUpdate);
    // result->Success();
  } else if (!name.compare("markers#update")) {
    // List<Object> markersToAdd = call.argument("markersToAdd");
    // markersController.addMarkers(markersToAdd);
    // List<Object> markersToChange = call.argument("markersToChange");
    // markersController.changeMarkers(markersToChange);
    // List<Object> markerIdsToRemove = call.argument("markerIdsToRemove");
    // markersController.removeMarkers(markerIdsToRemove);
    // result->Success();
    // -----------------------------------------------------------------------
    result->Success();
  } else if (!name.compare("markers#showInfoWindow")) {
    // Object markerId = call.argument("markerId");
    // markersController.showMarkerInfoWindow((String)markerId, result);
  } else if (!name.compare("markers#hideInfoWindow")) {
    // Object markerId = call.argument("markerId");
    // markersController.hideMarkerInfoWindow((String)markerId, result);
  } else if (!name.compare("markers#isInfoWindowShown")) {
    // Object markerId = call.argument("markerId");
    // markersController.isInfoWindowShown((String)markerId, result);
  } else if (!name.compare("polygons#update")) {
    // List<Object> polygonsToAdd = call.argument("polygonsToAdd");
    // polygonsController.addPolygons(polygonsToAdd);
    // List<Object> polygonsToChange = call.argument("polygonsToChange");
    // polygonsController.changePolygons(polygonsToChange);
    // List<Object> polygonIdsToRemove = call.argument("polygonIdsToRemove");
    // polygonsController.removePolygons(polygonIdsToRemove);
    // result->Success();
    // -----------------------------------------------------------------------
    result->Success();
  } else if (!name.compare("polylines#update")) {
    // List<Object> polylinesToAdd = call.argument("polylinesToAdd");
    // polylinesController.addPolylines(polylinesToAdd);
    // List<Object> polylinesToChange = call.argument("polylinesToChange");
    // polylinesController.changePolylines(polylinesToChange);
    // List<Object> polylineIdsToRemove = call.argument("polylineIdsToRemove");
    // polylinesController.removePolylines(polylineIdsToRemove);
    // result->Success();
    // -----------------------------------------------------------------------
    result->Success();
  } else if (!name.compare("circles#update")) {
    // List<Object> circlesToAdd = call.argument("circlesToAdd");
    // circlesController.addCircles(circlesToAdd);
    // List<Object> circlesToChange = call.argument("circlesToChange");
    // circlesController.changeCircles(circlesToChange);
    // List<Object> circleIdsToRemove = call.argument("circleIdsToRemove");
    // circlesController.removeCircles(circleIdsToRemove);
    // result->Success();
    // -----------------------------------------------------------------------
    result->Success();
  } else if (!name.compare("map#isCompassEnabled")) {
    // result->Success(googleMap.getUiSettings().isCompassEnabled());
  } else if (!name.compare("map#isMapToolbarEnabled")) {
    // result->Success(googleMap.getUiSettings().isMapToolbarEnabled());
  } else if (!name.compare("map#getMinMaxZoomLevels")) {
    // List<Float> zoomLevels = new ArrayList<>(2);
    // zoomLevels.add(googleMap.getMinZoomLevel());
    // zoomLevels.add(googleMap.getMaxZoomLevel());
    // result->Success(zoomLevels);
  } else if (!name.compare("map#isZoomGesturesEnabled")) {
    // result->Success(googleMap.getUiSettings().isZoomGesturesEnabled());
  } else if (!name.compare("map#isLiteModeEnabled")) {
    // result->Success(options.getLiteMode());
  } else if (!name.compare("map#isZoomControlsEnabled")) {
    // result->Success(googleMap.getUiSettings().isZoomControlsEnabled());
  } else if (!name.compare("map#isScrollGesturesEnabled")) {
    // result->Success(googleMap.getUiSettings().isScrollGesturesEnabled());
  } else if (!name.compare("map#isTiltGesturesEnabled")) {
    // result->Success(googleMap.getUiSettings().isTiltGesturesEnabled());
  } else if (!name.compare("map#isRotateGesturesEnabled")) {
    // result->Success(googleMap.getUiSettings().isRotateGesturesEnabled());
  } else if (!name.compare("map#isMyLocationButtonEnabled")) {
    // result->Success(googleMap.getUiSettings().isMyLocationButtonEnabled());
  } else if (!name.compare("map#isTrafficEnabled")) {
    // result->Success(googleMap.isTrafficEnabled());
  } else if (!name.compare("map#isBuildingsEnabled")) {
    // result->Success(googleMap.isBuildingsEnabled());
  } else if (!name.compare("map#getZoomLevel")) {
    // result->Success(googleMap.getCameraPosition().zoom);
  } else if (!name.compare("map#setStyle")) {
    // String mapStyle = (String)call.arguments;
    // boolean mapStyleSet;
    // if (mapStyle == null) {
    //   mapStyleSet = googleMap.setMapStyle(null);
    // } else {
    //   mapStyleSet = googleMap.setMapStyle(new MapStyleOptions(mapStyle));
    // }
    // ArrayList<Object> mapStyleResult = new ArrayList<>(2);
    // mapStyleResult.add(mapStyleSet);
    // if (!mapStyleSet) {
    //   mapStyleResult.add(
    //       "Unable to set the map style. Please check console logs for
    //       errors.");
    // }
    // result->Success(mapStyleResult);
  } else if (!name.compare("tileOverlays#update")) {
    // List < Map < String,
    //     ? >> tileOverlaysToAdd = call.argument("tileOverlaysToAdd");
    // tileOverlaysController.addTileOverlays(tileOverlaysToAdd);
    // List < Map < String,
    //     ? >> tileOverlaysToChange = call.argument("tileOverlaysToChange");
    // tileOverlaysController.changeTileOverlays(tileOverlaysToChange);
    // List<String> tileOverlaysToRemove =
    // call.argument("tileOverlayIdsToRemove");
    // tileOverlaysController.removeTileOverlays(tileOverlaysToRemove);
    // result->Success();
    // -----------------------------------------------------------------------
    result->Success();
  } else if (!name.compare("tileOverlays#clearTileCache")) {
    // String tileOverlayId = call.argument("tileOverlayId");
    // tileOverlaysController.clearTileCache(tileOverlayId);
    // result->Success();
  } else if (!name.compare("map#getTileOverlayInfo")) {
    // String tileOverlayId = call.argument("tileOverlayId");
    // result->Success(tileOverlaysController.getTileOverlayInfo(tileOverlayId));
  } else {
    result->NotImplemented();
  }
  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////
}

void GoogleMapController::Dispose() {
  FlutterUnregisterExternalTexture(texture_registrar_, GetTextureId());

  if (mapview_) {
    mapview_->Destroy();
    mapview_ = nullptr;
  }
}

void GoogleMapController::Resize(double width, double height) {
  LOG_DEBUG("GoogleMapController::Resize width: %f height: %f \n", width,
            height);
  // TODO: implement this if necessary
}

void GoogleMapController::Touch(int type, int button, double x, double y,
                                double dx, double dy) {
  if (type == 0) {  // down event
    mapview_->DispatchMouseDownEvent(LWE::MouseButtonValue::LeftButton,
                                     LWE::MouseButtonsValue::LeftButtonDown, x,
                                     y);
    is_mouse_lbutton_down_ = true;
  } else if (type == 1) {  // move event
    mapview_->DispatchMouseMoveEvent(
        is_mouse_lbutton_down_ ? LWE::MouseButtonValue::LeftButton
                               : LWE::MouseButtonValue::NoButton,
        is_mouse_lbutton_down_ ? LWE::MouseButtonsValue::LeftButtonDown
                               : LWE::MouseButtonsValue::NoButtonDown,
        x, y);
  } else if (type == 2) {  // up event
    mapview_->DispatchMouseUpEvent(LWE::MouseButtonValue::NoButton,
                                   LWE::MouseButtonsValue::NoButtonDown, x, y);
    is_mouse_lbutton_down_ = false;
  } else {
    // TODO: Not implemented
  }
}

void GoogleMapController::SetDirection(int direction) {
  LOG_DEBUG("GoogleMapController::SetDirection direction: %d\n", direction);
  // TODO: implement this if necessary
}

void GoogleMapController::ClearFocus() {
  LOG_DEBUG("GoogleMapController::ClearFocus()");
  // TODO: implement this if necessary
}

void GoogleMapController::DispatchKeyDownEvent(Ecore_Event_Key* key) {}
void GoogleMapController::DispatchKeyUpEvent(Ecore_Event_Key* key) {}
void GoogleMapController::DispatchCompositionUpdateEvent(const char* str,
                                                         int size) {}
void GoogleMapController::DispatchCompositionEndEvent(const char* str,
                                                      int size) {}

void GoogleMapController::SetSoftwareKeyboardContext(
    Ecore_IMF_Context* context) {}
