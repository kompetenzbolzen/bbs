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

int try_write(int _fd, char *_buff, int _size, int _retry);

pid_t fork_run(int _stdin, int _stdout, int _stderr, int argc, char* argv[]);
