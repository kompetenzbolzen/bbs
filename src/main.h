#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <arpa/inet.h>

#define _DEF_MAX_BACKLOG 	20
#define _DEF_PORT 		23
#define _DEF_IP			0
#define _DEF_CONFIG_FILE	"/etc/bbs.conf"

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
};

struct prog_params parse_args(int argc, char* argv[]);

void handle_connection(int _socket, struct sockaddr_in _addr);

int main(int argc, char* argv[]);
