#include "runner.h"

#include <dlog.h>

#include "generated_plugin_registrant.h"

class App : public FlutterServiceApp {
 public:
  bool OnCreate() {
    dlog_print(DLOG_DEBUG, "WorkmanagerTizenPlugin", "Start Service");
    if (FlutterServiceApp::OnCreate()) {
      RegisterPlugins(this);
    }
    return IsRunning();
  }

  void OnTerminate() {
    dlog_print(DLOG_DEBUG, "WorkmanagerTizenPlugin", "Terminate Service");
  }
};

int main(int argc, char *argv[]) {
  App app;
  app.SetDartEntrypoint("callbackDispatcher");
  return app.Run(argc, argv);
}
