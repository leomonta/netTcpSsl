#include "logger.hpp"

#include <pthread.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>

#define RED   "\x1B[031m"
#define GRN   "\x1B[032m"
#define YEL   "\x1B[0033m"
#define BLU   "\x1B[034m"
#define MAG   "\x1B[0035m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

char currLogLevel = LOG_ALL;

void setLogLevel(const char newLogLevel) {
	currLogLevel = newLogLevel;
}

void log(const char logLevel, const char *mex, ...) {

	if (currLogLevel > logLevel) {
		return;
	}

	va_list args;
	va_start(args, mex);

	const char *prefix = "[ UNKWN ]";

	switch (logLevel) {
	case LOG_ALL:
		prefix = "[  ALL  ]";
		break;
	case LOG_DEBUG:
		prefix = GRN "[ DEBUG ]";
		break;
	case LOG_INFO:
		prefix = CYN "[  INFO ]";
		break;
	case LOG_WARNING:
		prefix = YEL "[WARNING]";
		break;
	case LOG_ERROR:
		prefix = MAG "[ ERROR ]";
		break;
	case LOG_FATAL:
		prefix = RED "[ FATAL ]";
		break;
	}

	auto tid = gettid();

	//               0   123456789
	char TColor[] = "\x1B[38;5;000m";

	// where to modify the TColor variable
	auto index = 9;

	// how colors are looped
	// take the remainer of the division by 129 + 32 to ignore the first 32 colors
	int Tnum = tid % 191 + 1;

	for (int i = 0; i < 3; ++i) {

		char num = static_cast<char>(Tnum % 10) + '0';
		Tnum /= 10;
		TColor[index] = num;
		--index;
	}

	printf("\r%s" RESET " %s[THREAD %6d] " RESET, prefix, TColor, tid);
	vprintf(mex, args);
	fflush(stdout);

	va_end(args);
}
