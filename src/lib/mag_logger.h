//
// Created by Khubaib Umer on 18/01/2023.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef MAG_LOGGER_H_
#define MAG_LOGGER_H_

typedef enum {
    False = 0, True
} BOOL;

#define KB(x) ((x)*1000)
#define MB(x) (KB(x) * 1000)
#define GB(x) (MB(x) * 1000)

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#undef LOG_INFO
#undef LOG_DEBUG
typedef enum {
    LOG_TRACE = 0, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_NONE
} kLogLevel;

typedef struct {
    /// @fn trace, debug, info, warn, error
    /// @param fl filename
    /// @param fn caller function name
    /// @param ln caller line number
    /// @param fmt string format
    /// @param ... variadic list of arguments
    /// @brief Prints Trace Level logs
    void (*trace)(const char *fl, const char *fn, int ln, const char *fmt, ...);

    void (*debug)(const char *fl, const char *fn, int ln, const char *fmt, ...);

    void (*info)(const char *fl, const char *fn, int ln, const char *fmt, ...);

    void (*warn)(const char *fl, const char *fn, int ln, const char *fmt, ...);

    void (*error)(const char *fl, const char *fn, int ln, const char *fmt, ...);

    /* Logger Control */
    /// @fn update_log_level
    /// @param level  Requested Log Level
    /// @brief update log_level to specified
    ///		 must be called once after create_logger() is called
    BOOL (*update_log_level)(kLogLevel level);

    /// @fn is_initialized
    /// @brief Returns True is Logger is already initialized or false otherwise
    BOOL (*is_initialized)(void);

    /// @fn cycle_file
    /// @brief close the current file and start a new file
    void (*cycle_file)(void);
} logger_t;

/// @fn create_logger
/// @param service Name of the Service creating a Logger
/// @param path Path where Logfile will be created - NULL if console logging is required
/// @brief create a new logger object
EXTERN_C logger_t *create_logger(const char *service, const char *path);

/// @fn set_max_file_size
/// @param max set max file size
/// @brief Once file reaches the size mentioned here we create a new file and zips old file
/// 		-1 disabled file compression logic
EXTERN_C BOOL set_max_file_size(int64_t max);

/// @fn get_logger
/// @brief returns pointer to logger object already cleated
EXTERN_C logger_t *get_logger(void);

/// @fn close_logger
/// @brief closes the current open file and zips it and closes it
EXTERN_C BOOL close_logger(void);

#define M_TRACE(fmt, ...) get_logger()->trace(__FILENAME__, __func__, __LINE__, fmt, __VA_ARGS__)
#define M_DEBUG(fmt, ...) get_logger()->debug(__FILENAME__, __func__, __LINE__, fmt, __VA_ARGS__)
#define M_INFO(fmt, ...) get_logger()->info(__FILENAME__, __func__, __LINE__, fmt, __VA_ARGS__)
#define M_WARN(fmt, ...) get_logger()->warn(__FILENAME__, __func__, __LINE__, fmt, __VA_ARGS__)
#define M_ERROR(fmt, ...) get_logger()->error(__FILENAME__, __func__, __LINE__, fmt, __VA_ARGS__)

#define M_LOG(LEVEL, fmt, ...) M_##LEVEL(fmt, __VA_ARGS__)

#define M_LOGGER get_logger()

#endif //MAG_LOGGER_H_
