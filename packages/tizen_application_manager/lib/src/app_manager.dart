import 'dart:ffi';
import 'package:ffi/ffi.dart';
import 'package:flutter/services.dart';

typedef _app_manager_get_app_context = Int32 Function(
    Pointer<Utf8>, Pointer<Pointer<_ContextHandle>>);
typedef _app_manager_is_running = Int32 Function(Pointer<Utf8>, Pointer<Int8>);
typedef _app_context_get_package_id = Int32 Function(
    Pointer<_ContextHandle>, Pointer<Pointer<Utf8>>);
typedef _app_context_get_pid = Int32 Function(
    Pointer<_ContextHandle>, Pointer<Int32>);
typedef _app_context_get_app_state = Int32 Function(
    Pointer<_ContextHandle>, Pointer<Int32>);
typedef _app_manager_terminate_app = Int32 Function(Pointer<_ContextHandle>);
typedef _app_manager_resume_app = Int32 Function(Pointer<_ContextHandle>);
typedef _app_context_destroy = Int32 Function(Pointer<_ContextHandle>);

class _ContextHandle extends Opaque {}

const int TIZEN_ERROR_NONE = 0;
const int TIZEN_ERROR_IO_ERROR = -5;
const int TIZEN_ERROR_OUT_OF_MEMORY = -12;
const int TIZEN_ERROR_PERMISSION_DENIED = -13;
const int TIZEN_ERROR_INVALID_PARAMETER = -22;
const int TIZEN_ERROR_UNKNOW = -1073741824;
const int TIZEN_ERROR_NOT_SUPPORTED = -1073741824 + 2;
const int TIZEN_ERROR_APPLICATION_MANAGER = -0x01110000;
const int APP_MANAGER_ERROR_NO_SUCH_APP =
    TIZEN_ERROR_APPLICATION_MANAGER | 0x01;
const int APP_MANAGER_ERROR_DB_FAILED = TIZEN_ERROR_APPLICATION_MANAGER | 0x03;
const int APP_MANAGER_ERROR_INVALID_PACKAGE =
    TIZEN_ERROR_APPLICATION_MANAGER | 0x04;
const int APP_MANAGER_ERROR_APP_NO_RUNNING =
    TIZEN_ERROR_APPLICATION_MANAGER | 0x05;
const int APP_MANAGER_ERROR_REQUEST_FAILED =
    TIZEN_ERROR_APPLICATION_MANAGER | 0x06;

const Map<int, String> _errorCodes = <int, String>{
  TIZEN_ERROR_NONE: 'APP_MANAGER_ERROR_NONE',
  TIZEN_ERROR_IO_ERROR: 'APP_MANAGER_ERROR_IO_ERROR',
  TIZEN_ERROR_OUT_OF_MEMORY: 'APP_MANAGER_ERROR_OUT_OF_MEMORY',
  TIZEN_ERROR_PERMISSION_DENIED: 'APP_MANAGER_ERROR_PERMISSION_DENIED',
  TIZEN_ERROR_INVALID_PARAMETER: 'APP_MANAGER_ERROR_INVALID_PARAMETER',
  TIZEN_ERROR_UNKNOW: 'APP_MANAGER_ERROR_IO_ERROR ',
  TIZEN_ERROR_NOT_SUPPORTED: 'APP_MANAGER_ERROR_NOT_SUPPORTED ',
  APP_MANAGER_ERROR_NO_SUCH_APP: 'APP_MANAGER_ERROR_NO_SUCH_APP',
  APP_MANAGER_ERROR_DB_FAILED: 'APP_MANAGER_ERROR_DB_FAILED',
  APP_MANAGER_ERROR_INVALID_PACKAGE: 'APP_MANAGER_ERROR_INVALID_PACKAGE',
  APP_MANAGER_ERROR_APP_NO_RUNNING: 'APP_MANAGER_ERROR_APP_NO_RUNNING',
  APP_MANAGER_ERROR_REQUEST_FAILED: 'APP_MANAGER_ERROR_REQUEST_FAILED',
};

String _getErrorCode(int returnCode) => _errorCodes.containsKey(returnCode)
    ? _errorCodes[returnCode]!
    : '$returnCode';

AppManager? _appManagerInstance;
AppManager get appManager => _appManagerInstance ??= AppManager();

class AppManager {
  AppManager() {
    final libAppMananger =
        DynamicLibrary.open('libcapi-appfw-app-manager.so.0');
    getAppContext = libAppMananger
        .lookup<NativeFunction<_app_manager_get_app_context>>(
            'app_manager_get_app_context')
        .asFunction();
    appIsRunning = libAppMananger
        .lookup<NativeFunction<_app_manager_is_running>>(
            'app_manager_is_running')
        .asFunction();
    getPackageId = libAppMananger
        .lookup<NativeFunction<_app_context_get_package_id>>(
            'app_context_get_package_id')
        .asFunction();
    getProcessId = libAppMananger
        .lookup<NativeFunction<_app_context_get_pid>>('app_context_get_pid')
        .asFunction();
    getAppState = libAppMananger
        .lookup<NativeFunction<_app_context_get_app_state>>(
            'app_context_get_app_state')
        .asFunction();
    terminateApp = libAppMananger
        .lookup<NativeFunction<_app_manager_terminate_app>>(
            'app_manager_terminate_app')
        .asFunction();
    resumeApp = libAppMananger
        .lookup<NativeFunction<_app_manager_resume_app>>(
            'app_manager_resume_app')
        .asFunction();
    destroyContext = libAppMananger
        .lookup<NativeFunction<_app_context_destroy>>('app_context_destroy')
        .asFunction();
  }

  late int Function(Pointer<Utf8>, Pointer<Pointer<_ContextHandle>>)
      getAppContext;
  late int Function(Pointer<Utf8>, Pointer<Int8>) appIsRunning;
  late int Function(Pointer<_ContextHandle>, Pointer<Pointer<Utf8>>)
      getPackageId;
  late int Function(Pointer<_ContextHandle>, Pointer<Int32>) getProcessId;
  late int Function(Pointer<_ContextHandle>, Pointer<Int32>) getAppState;
  late int Function(Pointer<_ContextHandle>) terminateApp;
  late int Function(Pointer<_ContextHandle>) resumeApp;
  late int Function(Pointer<_ContextHandle>) destroyContext;
}

class AppContext {
  Pointer<_ContextHandle> _handle = nullptr;
  int _handleAddress = 0;
  late String _appId;

  AppContext(String appId, int handleAddress) {
    if (appId.isEmpty) {
      throw PlatformException(
        code: _getErrorCode(TIZEN_ERROR_INVALID_PARAMETER),
        message: 'appId is empty',
      );
    }

    _appId = appId;
    if (handleAddress == 0) {
      final Pointer<Pointer<_ContextHandle>> pHandle = malloc();
      final appIdP = appId.toNativeUtf8();
      try {
        var ret = appManager.getAppContext(appIdP, pHandle);
        if (ret != 0) {
          throw PlatformException(
            message: 'Failed to excute app_manager_get_app_context $appId',
            code: _getErrorCode(ret),
          );
        }
        _handle = pHandle.value;
        _handleAddress = pHandle.value.address;
      } finally {
        malloc.free(appIdP);
        malloc.free(pHandle);
      }
    } else {
      // if have app_context_h
      _handle = Pointer.fromAddress(handleAddress);
      _handleAddress = handleAddress;
    }
  }

  void destroy() {
    if (_handle != nullptr) {
      print('destroy _handle:${_handle.toString()}');
      var ret = appManager.destroyContext(_handle);
      _handle = nullptr;
      if (ret != 0) {
        throw PlatformException(
          message: 'Failed to excute app_context_destroy',
          code: _getErrorCode(ret),
        );
      }
    }
  }

  int getHandleAddress() {
    return _handleAddress;
  }

  bool isAppRunning() {
    var isRun = false;
    final Pointer<Int8> out = malloc();
    final appIdP = _appId.toNativeUtf8();
    try {
      var ret = appManager.appIsRunning(appIdP, out);
      if (ret != 0) {
        throw PlatformException(
          message: 'Failed to excute app_manager_is_running $_appId',
          code: _getErrorCode(ret),
        );
      }

      if (out != nullptr) {
        isRun = out.value != 0 ? true : false;
      }
      return isRun;
    } finally {
      malloc.free(appIdP);
      malloc.free(out);
    }
  }

  String getPackageId() {
    final Pointer<Pointer<Utf8>> packageIdP = malloc();
    try {
      _handleCheck();
      var ret = appManager.getPackageId(_handle, packageIdP);
      if (ret != 0) {
        throw PlatformException(
          message: 'Failed to excute app_context_get_package_id',
          code: _getErrorCode(ret),
        );
      }

      return packageIdP.value.toDartString();
    } finally {
      malloc.free(packageIdP);
    }
  }

  int getProcessId() {
    final Pointer<Int32> processIdP = malloc();
    try {
      _handleCheck();
      var ret = appManager.getProcessId(_handle, processIdP);
      if (ret != 0) {
        throw PlatformException(
          message: 'Failed to excute app_context_get_pid',
          code: _getErrorCode(ret),
        );
      }
      return processIdP.value;
    } finally {
      malloc.free(processIdP);
    }
  }

  int getAppState() {
    final Pointer<Int32> stateP = malloc();
    try {
      _handleCheck();
      var ret = appManager.getAppState(_handle, stateP);
      if (ret != 0) {
        throw PlatformException(
          message: 'Failed to excute app_context_get_app_state',
          code: _getErrorCode(ret),
        );
      }
      return stateP.value;
    } finally {
      malloc.free(stateP);
    }
  }

  void terminate() {
    print('terminate() : $_appId');
    _handleCheck();
    var ret = appManager.terminateApp(_handle);
    if (ret != 0) {
      throw PlatformException(
        message: 'Failed to excute app_manager_terminate_app',
        code: _getErrorCode(ret),
      );
    }
  }

  void resume() {
    print('resume() : $_appId');
    _handleCheck();
    var ret = appManager.resumeApp(_handle);
    if (ret != 0) {
      throw PlatformException(
        message: 'Failed to excute app_manager_resume_app',
        code: _getErrorCode(ret),
      );
    }
  }

  void _handleCheck() {
    if (_handle == nullptr) {
      print('handle == nullptr');
      throw PlatformException(
          code: _getErrorCode(TIZEN_ERROR_INVALID_PARAMETER),
          message: 'context handle is null!');
    }
  }
}
