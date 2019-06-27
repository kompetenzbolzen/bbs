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

#define _AT "AT\r\n"
#define _AT_ANSWER "ATA\r\n"
#define _AT_ECHO_OFF "ATE0\r\n"
#define _AT_ECHO_ON "ATE1\r\n\”"
#define _AT_HANGUP "ATH\r\n"
#define _AT_CMD_MODE "+++"

int modem_accept_wait(int fd);

int modem_hang(int fd);

int modem_run(int fd, int argc, char* argv[]);
