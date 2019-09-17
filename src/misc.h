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

/*
 * Try writing to _fd for _retry times. Retries are reset after every succesful write.
 */
int try_write(int _fd, char *_buff, int _size, int _retry);

/*
 * Fork, exec(argc, argv) with new STDIO, return PID
 */
pid_t fork_run(int _stdin, int _stdout, int _stderr, int argc, char* argv[]);
