#pragma once

#if defined(__cplusplus)
extern "C" {            // Prevents name mangling of functions
#endif

// quick macro to use the automatic __FILE__ and __LINE__ gcc defines
#define llog(level, format, ...) logger(level, __FILE_NAME__, __LINE__, __PRETTY_FUNCTION__, format __VA_OPT__(, ) __VA_ARGS__)

typedef enum : char {
	LOG_FATAL,
	LOG_ERROR,
	LOG_WARNING,
	LOG_INFO,
	LOG_DEBUG
} logLevel;

/**
 * Set the local log_level variable to prevent any log message 'lower' than the one selected from being printed
 *
 * @param[in] ll the new log level to use
 */
void set_log_level(logLevel ll);

/**
 * logs a message to the standard output, with info about log level, file, line number, and function name
 * the variadic arguments are passed, alongside format, to vprintf
 * it is suggested to use the `llog` for automatic file name, line number, and function name detection
 *
 * internally it keeps a local buffer where the printf functions output to, then a single call to normal printf is executed and then an fflush(stdout)
 * this makes it reasonably thread safe
 *
 * @param[in] ll the log level of the message
 * @param[in] file_name the file to show at the log message
 * @param[in] line_num the line number to show in the log message
 * @param[in] function_name the function name to show in the log message
 * @param[in] format the orintf format to use when printing the variadic arguments
 * @param[in] ... the variadic arguments to pass to printf
 *
 */
void logger(const logLevel ll, const char *file_name, const unsigned line_num, const char *function_name, const char *format, ...);

#if defined(__cplusplus)
}
#endif
