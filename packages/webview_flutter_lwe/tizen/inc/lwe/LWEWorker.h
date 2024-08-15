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

enum class LWE_EXPORT WorkerProcessState {
  None,
  Terminated,
};

/**
 * \brief Perform initialization or cleanup of service worker.
 */
class LWE_EXPORT ServiceWorker {
 public:
  /**
   * \brief Sets the preference for the engine version to be used.
   *
   * \param preferUpdatedVersion If true, an updated engine version will be
   * used if available.
   *
   * \remark Must be called before the Initialize.
   */
  static void SetVersionPreference(bool preferUpdatedVersion);

  /**
   * \brief Initialize service worker server.
   * Be sure to set the same data path as the lightweight web engine.
   *
   * \code{.cpp}
   *     LWE::ServiceWorker::Initialize("/tmp/Starfish_storage");
   * \endcode
   *
   * \param storageDirectoryPath Directory path for storage.
   *
   */
  static void Initialize(const std::string &storageDirectoryPath);

  /**
   * \brief Register callback that is called when the worker process state
   * changes.
   *
   * \code{.cpp}
   * LWE::ServiceWorker::RegisterOnStatusChangedHandler(
   *     [](LWE::WorkerProcessState state) {
   *         if (state == LWE::WorkerProcessState::Terminated) {
   *             printf("Terminated worker process\n");
   *         }
   *     }
   * );
   * \endcode
   *
   * \param cb state handling callback.
   *
   */
  static void RegisterOnStatusChangedHandler(
      const std::function<void(WorkerProcessState)> &cb);

  /**
   * \brief Perform lightweight web engine service worker cleanup.
   * Called once when the lightweight web engine service worker is no longer
   * in use.
   *
   * \code{.cpp}
   *     LWE::ServiceWorker::Finalize();
   * \endcode
   *
   */
  static void Finalize();
};

/**
 * \brief Perform initialization or cleanup of Shared worker.
 */
class LWE_EXPORT SharedWorker {
 public:
  /**
   * \brief Sets the preference for the engine version to be used.
   *
   * \param preferUpdatedVersion If true, an updated engine version will be
   * used if available.
   *
   * \remark Must be called before the Initialize.
   */
  static void SetVersionPreference(bool preferUpdatedVersion);

  /**
   * \brief Initialize shared worker server.
   * Be sure to set the same data path as the lightweight web engine.
   *
   * \code{.cpp}
   *     LWE::SharedWorker::Initialize("/tmp/Starfish_storage");
   * \endcode
   *
   * \param storageDirectoryPath Directory path for storage.
   *
   */
  static void Initialize(const std::string &storageDirectoryPath);

  /**
   * \brief Register callback that is called when the worker process state
   * changes.
   *
   * \code{.cpp}
   * LWE::SharedWorker::RegisterOnStatusChangedHandler(
   *     [](LWE::WorkerProcessState state) {
   *         if (state == LWE::WorkerProcessState::Terminated) {
   *             printf("Terminated worker process\n");
   *         }
   *     }
   * );
   * \endcode
   *
   * \param cb state handling callback.
   *
   */
  static void RegisterOnStatusChangedHandler(
      const std::function<void(WorkerProcessState)> &cb);

  /**
   * \brief Perform lightweight web engine shared worker cleanup.
   * Called once when the lightweight web engine shared worker is no longer
   * in use.
   *
   * \code{.cpp}
   *     LWE::SharedWorker::Finalize();
   * \endcode
   *
   */
  static void Finalize();
};

}  // namespace LWE

#endif
