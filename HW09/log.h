#ifndef LOG_H
#define LOG_H
#include <stdio.h>
#include <string.h>

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define LOG_DEBUG(fmt, ...) logger_log(LogLevel_DEBUG, __FILENAME__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)  logger_log(LogLevel_INFO , __FILENAME__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  logger_log(LogLevel_WARN , __FILENAME__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) logger_log(LogLevel_ERROR, __FILENAME__, __LINE__, fmt, ##__VA_ARGS__)

typedef enum {
    LogLevel_DEBUG,
    LogLevel_INFO,
    LogLevel_WARN,
    LogLevel_ERROR,
} LogLevel;

/**
 * Initialize the logger as a file logger.
 * If the filename is NULL, return without doing anything.
 *
 * @param[in] filename The name of the output file
 * @return Non-zero value upon success or 0 on error
 */
int logger_init(const char* filename);

/**
 * Get the log level that has been set.
 * The default log level is INFO.
 *
 * @return The log level
 */
LogLevel logger_getLevel(void);

/**
 * Set the log level.
 * Message levels lower than this value will be discarded.
 * The default log level is INFO.
 *
 * @param[in] level A log level
 */
void logger_setLevel(LogLevel level);

/**
 * @brief 
 * 
 * deinit logger
 * 
 */
void logger_exit();

/**
 * Log a message.
 * Make sure to call init_logger before starting logging.
 *
 * @param[in] level A log level
 * @param[in] file A file name string
 * @param[in] line A line number
 * @param[in] fmt A format string
 * @param[in] ... Additional arguments
 */
void logger_log(LogLevel level, const char* file, int line, const char* fmt, ...);

#endif