#include "billing_service_proxy.h"

#include <dlfcn.h>

#include "log.h"

FuncGetProductslist service_billing_get_products_list = nullptr;
FuncGetpurchaselist service_billing_get_purchase_list = nullptr;
FuncServiceBillingIsServiceAvailable service_billing_is_service_available =
    nullptr;
FuncBillingBuyItem service_billing_buyitem = nullptr;
FuncBillingSetBuyItemCb service_billing_set_buyitem_cb = nullptr;

void *Dlsym(void *handle, const char *name) {
  if (!handle) {
    LOG_ERROR("[DrmManagerService] dlsym failed, handle is null");
    return nullptr;
  }
  return dlsym(handle, name);
}

void *OpenBillingApi() { return dlopen("libbilling_api.so", RTLD_LAZY); }

void *OpenSsoApi() { return dlopen("libsso_api.so", RTLD_LAZY); }

int InitBillingApi(void *handle) {
  service_billing_get_products_list = reinterpret_cast<FuncGetProductslist>(
      Dlsym(handle, "service_billing_get_products_list"));
  if (service_billing_get_products_list == nullptr) {
    return 0;
  }

  service_billing_get_purchase_list = reinterpret_cast<FuncGetpurchaselist>(
      Dlsym(handle, "service_billing_get_purchase_list"));
  if (service_billing_get_purchase_list == nullptr) {
    return 0;
  }

  service_billing_is_service_available =
      reinterpret_cast<FuncServiceBillingIsServiceAvailable>(
          Dlsym(handle, "service_billing_is_service_available"));
  if (service_billing_is_service_available == nullptr) {
    return 0;
  }

  service_billing_buyitem = reinterpret_cast<FuncBillingBuyItem>(
      Dlsym(handle, "service_billing_buyitem"));
  if (service_billing_buyitem == nullptr) {
    return 0;
  }

  service_billing_set_buyitem_cb = reinterpret_cast<FuncBillingSetBuyItemCb>(
      Dlsym(handle, "service_billing_set_buyitem_cb"));
  if (service_billing_set_buyitem_cb == nullptr) {
    return 0;
  }
}

int CloseBillingApi(void *handle) {
  if (handle == nullptr) {
    LOG_ERROR("[BillingApi] handle is null");
    return -1;
  }
  return dlclose(handle);
}
