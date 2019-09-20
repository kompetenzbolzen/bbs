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

#include "misc.h"
#include "log.h"
#include "serial.h"

#define _AT "AT\r\n"
#define _AT_ANSWER "ATA\r\n"
#define _AT_ECHO_OFF "ATE0\r\n"
#define _AT_ECHO_ON "ATE1\r\n"
#define _AT_HANGUP "ATH\r\n"
#define _AT_CMD_MODE "+++\r\n"
#define _AT_MUTE "ATM0\r\n"
#define _AT_RESET_ON_DTR "AT&D3\r\n"

#define _MODEM_WAIT_RING

int modem_accept_wait(int fd);
/*
 * Waits for RING (ifdef _MODEM_WAIT_RING), accepts incoming calls. Return is non-zero when cennection fails.
 * */

int modem_command(int fd, char* cmd, int timeout_ms);
/*
 * Execute an AT command. return is non-zero if answer is not OK
 * */

int modem_run(int fd, int argc, char* argv[]);
/*
 * Run a program with modem as STDIO. checks if connection is still alive & process is still active.
 * will close fd on successful return
 * */

void dialup_server(struct prog_params params);
