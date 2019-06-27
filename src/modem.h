/*
 * modem.h
 * (c) 2019, Jonas Gunz, jonasgunz.de
 * <Description>
 * <License>
*/

#include <unistd.h>
#include <poll.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

#define _AT "AT\r\n"
#define _AT_ANSWER "ATA\r\n"
#define _AT_ECHO_OFF "ATE0\r\n"
#define _AT_ECHO_ON "ATE1\r\n\”"
#define _AT_HANGUP "ATH\r\n"
#define _AT_CMD_MODE "+++\r\n"
#define _AT_MUTE "ATM0\r\n"
#define _AT_RESET_ON_DTR "AT&D3\r\n"

int modem_accept_wait(int fd);

int modem_command(int fd, char* cmd, int timeout_ms);

int modem_run(int fd, int argc, char* argv[]);
