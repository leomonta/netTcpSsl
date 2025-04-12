#include "logger.h"

#include <stdarg.h>
#include <stdio.h>

#define RED   "\x1B[031m"
#define GRN   "\x1B[032m"
#define YEL   "\x1B[033m"
#define BLU   "\x1B[034m"
#define MAG   "\x1B[035m"
#define CYN   "\x1B[036m"
#define WHT   "\x1B[037m"
#define GRY   "\x1B[090m"
#define RESET "\x1B[0m"

#define INTERNAL_BUFFER_SIZE 8196

// default log level if not specified via 'set_log_level'
static char log_level = LOG_DEBUG;

void set_log_level(logLevel ll) {
	log_level = ll;
}

void logger(const logLevel ll, const char *file_name, const unsigned line_num, const char *function_name, const char *format, ...) {

	// ignore too low log levels
	if (log_level < ll) {
		return;
	}

	// stright up rawdogging it
	// needs this to prevent multiple threads to clutter stdout
	char log_buffer[INTERNAL_BUFFER_SIZE] = {0};

	va_list args;
	va_start(args, format);

	// shouldn't happen but to be safe
	const char *prefix = "[ UNKWN ]";

	switch (ll) {
	case LOG_DEBUG:
		prefix = BLU "[ DEBUG ]" RESET;
		break;
	case LOG_INFO:
		prefix = GRN "[  INFO ]" RESET;
		break;
	case LOG_WARNING:
		prefix = YEL "[WARNING]" RESET;
		break;
	case LOG_ERROR:
		prefix = MAG "[ ERROR ]" RESET;
		break;
	case LOG_FATAL:
		prefix = RED "[ FATAL ]" RESET;
		break;
	}

	//
	// [ DEBUG ] server.cpp:167  in void start(runtimeIn    [
	// this should return always
	auto printed_chars = snprintf(log_buffer, INTERNAL_BUFFER_SIZE, "%s %-10.10s:%-4d " GRY "in" RESET " %-20.20s    " RESET, prefix, file_name, line_num, function_name);
	vsnprintf(log_buffer + printed_chars, (size_t)(INTERNAL_BUFFER_SIZE - printed_chars), format, args);
	printf("%s", log_buffer);
	fflush(stdout); // ensure printing even with no \n

	va_end(args);
}
