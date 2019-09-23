#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>

#define _LOG_ERROR 	1
#define _LOG_WARNING 	2
#define _LOG_NOTE 	3
#define _LOG_DEBUG 	4

unsigned int log_loglevel;
int log_fd;

const char* log_loglevel_str[5];

#define LOGPRINTF(l,...) {\
	if((l) <= log_loglevel){\
		time_t current = time (NULL);\
		struct tm *tma = localtime(&current);\
		char* date = asctime(tma);\
		date[strlen(date) - 1] = '\0';\
		printf("[%s] %s: ", date,  log_loglevel_str[(l)]);\
		if((l) == _LOG_ERROR)\
			printf("%s:", strerror(errno));\
		if((l) == _LOG_DEBUG)\
			printf("%s:%d: ", __FILE__, __LINE__);\
		printf(__VA_ARGS__);\
		printf("\n");\
		fsync(STDOUT_FILENO);\
	}\
}


/**
 * Opens logfile, writes filedes to _fd
 * */
int log_init_file(char* _file, unsigned int _verbosity);

/**
 * Configures LOG macros for stdout
 */
int log_init_stdout(unsigned int _verbosity);

int log_close();
