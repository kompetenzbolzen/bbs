/*
 * main.h
 * (c) 2019, Jonas Gunz, jonasgunz.de
 * <Description>
 * License: MIT
*/

#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#include "misc.h"
#include "log.h"
#include "serial.h"
#include "modem.h"
#include "telnet.h"

#define _DEF_MAX_BACKLOG 	20
#define _DEF_PORT 		23
#define _DEF_IP			0
#define _DEF_CONFIG_FILE	"/etc/bbs.conf"

int main(int argc, char* argv[]);

void terminate();
