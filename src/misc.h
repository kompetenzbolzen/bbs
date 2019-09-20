/*
 * misc.h
 * (c) 2019, Jonas Gunz, jonasgunz.de
 * <Description>
 * <License>
*/

#pragma once

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>

#ifdef _DEBUG
//DEBUG Macros
#warning "Compiling in DEBUG mode"
#define DEBUG_PRINTF( ... ) { \
	printf("%s:%d: ", __FILE__, __LINE__); \ 
	printf(__VA_ARGS__ ); }
#define PRINT_ERROR( str )  { \
	printf("%s:%d: %s: %s\n", __FILE__, __LINE__, str, strerror(errno)); }
#else
//Release Macros
#define DEBUG_PRINTF( ... ) { }
#define PRINT_ERROR( str ) { printf("%s: %s\n", str, strerror(errno)); }
#endif

#define ERROR_HELP( ... ) { \
	printf(__VA_ARGS__); \
	printf("bbs\n-p PORT: telnet port\n-i IP: telnet listen ip\n-s DEVIVE: modem serial device\n-b BAUD: serial baudrate\n-f FILE: use pidfile\n"); \
	exit(1); }

struct prog_params
{
	uint8_t 	telnet;
	uint16_t 	port;
	uint16_t 	backlog;
	char*		ip;
	char** 		run_argv;
	int 		run_argc;

	uint8_t		serial;
	char*		serial_port;
	uint32_t	serial_baudrate;

	uint8_t		fork;
	char*		pidfile;
};

struct prog_params parse_args(int argc, char* argv[]);

/*
 * Try writing to _fd for _retry times. Retries are reset after every succesful write.
 */
int try_write(int _fd, char *_buff, int _size, int _retry);

/*
 * Fork, exec(argc, argv) with new STDIO, return PID
 */
pid_t fork_run(int _stdin, int _stdout, int _stderr, int argc, char* argv[]);
