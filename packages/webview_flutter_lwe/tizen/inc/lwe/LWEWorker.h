/*
 * Copyright (c) 2023-present Samsung Electronics Co., Ltd
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 *  USA
 */

#ifndef __LWE_WORKER__
#define __LWE_WORKER__

#ifndef LWE_EXPORT
#ifdef _MSC_VER
#define LWE_EXPORT __declspec(dllexport)
#else
#define LWE_EXPORT __attribute__((visibility("default")))
#endif
#endif

#include <functional>
#include <string>

namespace LWE {

class LWE_EXPORT WorkerClient {
 public:
  /*
   * Register worker data directory path.
   *
   * Be sure to set the same data path as the shared or service worker server.
   * If you do not register data directory, the data directory path is
   * set to '${HOME}/starfish-worker-data' or /tmp/starfish-worker-data.
   *
   * This method must be invoked after LWE::Initialize() is invoked.
   */
  static void RegisterDataDirectoryPath(const std::string &dataDirectoryPath);

  /*
   * Register service worker server process executor callback function.
   * The callback function should return the success or failure of the
   * processor execution.
   *
   * This method must be invoked after LWE::Initialize() is invoked.
   */
  static void RegisterServiceWorkerProcessExecutor(
      const std::function<bool()> &fn);
};

enum class LWE_EXPORT WorkerProcessState {
  None,
  Terminated,
};

class LWE_EXPORT ServiceWorker {
 public:
  /*
   * Initialize service worker server.
   *
   * Be sure to set the same data path as the worker client.
   * If you set data directory path to an empty path, it is set to
   * '${HOME}/starfish-worker-data' or /tmp/starfish-worker-data.
   */
  static void Initialize(const std::string &dataDirectoryPath);

  static void RegisterOnStatusChangedHandler(
      const std::function<void(WorkerProcessState)> &cb);

  static void Finalize();
};

class LWE_EXPORT SharedWorker {
 public:
  /*
   * Initialize shared worker server.
   *
   * Be sure to set the same data path as the worker client.
   * If you set data directory path to an empty path, it is set to
   * '${HOME}/starfish-worker-data' or /tmp/starfish-worker-data.
   */
  static void Initialize(const std::string &dataDirectoryPath);

  static void RegisterOnStatusChangedHandler(
      const std::function<void(WorkerProcessState)> &cb);

  static void Finalize();
};

}  // namespace LWE

#endif
