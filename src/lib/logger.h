#ifndef __LOGGER_H__
#define __LOGGER_H__

#include "mag_logger.h"

#define log_trace(fmt, ...) M_LOG(TRACE, fmt, __VA_ARGS__)
#define log_debug(fmt, ...) M_LOG(DEBUG, fmt, __VA_ARGS__)
#define log_info(fmt, ...)  M_LOG(INFO,  fmt, __VA_ARGS__)
#define log_warn(fmt, ...)  M_LOG(WARN, fmt, __VA_ARGS__)
#define log_error(fmt, ...) M_LOG(ERROR, fmt, __VA_ARGS__)

#endif // __LOGGER_H__