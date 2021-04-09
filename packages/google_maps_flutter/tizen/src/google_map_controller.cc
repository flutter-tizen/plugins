#include "google_map_controller.h"

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
      webview_instance_(nullptr),
      width_(width),
      height_(height),
      tbm_surface_(nullptr),
      is_mouse_lbutton_down_(false) {
  SetTextureId(FlutterRegisterExternalTexture(texture_registrar_));
  InitWebView();

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

  webview_instance_->LoadURL(url);
}

GoogleMapController::~GoogleMapController() { Dispose(); }

void GoogleMapController::InitWebView() {
  if (webview_instance_ != nullptr) {
    webview_instance_->Destroy();
    webview_instance_ = nullptr;
  }
  float scale_factor = 1;

  webview_instance_ = (LWE::WebContainer*)createWebViewInstance(
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
  auto settings = webview_instance_->GetSettings();
  settings.SetUserAgentString(
      "Mozilla/5.0 (like Gecko/54.0 Firefox/54.0) Mobile");
  webview_instance_->SetSettings(settings);
}

std::string GoogleMapController::GetChannelName(int view_id) {
  return "plugins.flutter.io/google_maps_" + std::to_string(view_id);
}

void GoogleMapController::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue>& method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  if (!webview_instance_) {
    return;
  }
  const auto method_name = method_call.method_name();
  const auto& arguments = *method_call.arguments();

  LOG_DEBUG("GoogleMapController::HandleMethodCall : %s \n ",
            method_name.c_str());

  if (method_name.compare("loadUrl") == 0) {
    if (std::holds_alternative<flutter::EncodableMap>(arguments)) {
      auto settings = std::get<flutter::EncodableMap>(arguments);
    }
  }
}

void GoogleMapController::Dispose() {
  FlutterUnregisterExternalTexture(texture_registrar_, GetTextureId());

  if (webview_instance_) {
    webview_instance_->Destroy();
    webview_instance_ = nullptr;
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
    webview_instance_->DispatchMouseDownEvent(
        LWE::MouseButtonValue::LeftButton,
        LWE::MouseButtonsValue::LeftButtonDown, x, y);
    is_mouse_lbutton_down_ = true;
  } else if (type == 1) {  // move event
    webview_instance_->DispatchMouseMoveEvent(
        is_mouse_lbutton_down_ ? LWE::MouseButtonValue::LeftButton
                               : LWE::MouseButtonValue::NoButton,
        is_mouse_lbutton_down_ ? LWE::MouseButtonsValue::LeftButtonDown
                               : LWE::MouseButtonsValue::NoButtonDown,
        x, y);
  } else if (type == 2) {  // up event
    webview_instance_->DispatchMouseUpEvent(
        LWE::MouseButtonValue::NoButton, LWE::MouseButtonsValue::NoButtonDown,
        x, y);
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
