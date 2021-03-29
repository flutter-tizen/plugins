#ifndef __LOG_H__
#define __LOG_H__

#include <dlog.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "CameraPlugin"

#define LOG(prio, fmt, arg...)                                                 \
  dlog_print(prio, LOG_TAG, "%s: %s(%d) > " fmt, __FILE__, __func__, __LINE__, \
             ##arg)

#define LOG_DEBUG(fmt, args...) LOG(DLOG_DEBUG, fmt, ##args)
#define LOG_INFO(fmt, args...) LOG(DLOG_INFO, fmt, ##args)
#define LOG_WARN(fmt, args...) LOG(DLOG_WARN, fmt, ##args)
#define LOG_ERROR(fmt, args...) LOG(DLOG_ERROR, fmt, ##args)

#define LOG_ERROR_IF(expr, fmt, arg...) \
  do {                                  \
    if (expr) {                         \
      LOG_ERROR(fmt, ##arg);            \
    }                                   \
  } while (0)

#define RETV_LOG_ERROR_IF(expr, val, fmt, arg...) \
  do {                                            \
    if (expr) {                                   \
      LOG_ERROR(fmt, ##arg);                      \
      return (val);                               \
    }                                             \
  } while (0)

#endif  // __LOG_H__
