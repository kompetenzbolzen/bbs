#include <errno.h>
#include <fcntl.h> 
#include <string.h>
#include <termios.h>
#include <unistd.h>

/*
 * set baud rate of fd to speed
 */
int set_interface_attribs (int fd, int speed, int parity);

/*
 * sets blocking, duh
 */
void set_blocking (int fd, int should_block);
