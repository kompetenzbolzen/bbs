#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <poll.h>
#include <stdint.h>

#include "misc.h"
#include "log.h"

void telnet_server(struct prog_params params);

void handle_connection(int _socket, struct sockaddr_in _addr, int argc, char* argv[]);
