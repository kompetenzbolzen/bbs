/*
 * misc.c
 * (c) 2019, Jonas Gunz, jonasgunz.de
 * <Description>
 * <License>
*/

#include "misc.h"

pid_t fork_run(int _stdin, int _stdout, int _stderr, int argc, char* argv[])
{
	pid_t pid = fork();
	
	if(pid == 0) {//child
		dup2 (_stdin,  STDIN_FILENO);
		dup2 (_stdout, STDOUT_FILENO);
		dup2 (_stderr, STDERR_FILENO);

		char* arv[argc + 1];

		for(int i = 0; i < argc; i++)
			arv[i] = argv[i];

		arv[argc] = NULL;

		execv(argv[0], arv);

		printf("EXEC ERROR %i: %s\r\n", errno, strerror(errno));
		exit(1);
	}
	else {
		return pid;
	}
}


int try_write(int _fd, char *_buff, int _size, int _retry)
{
	if( _size <= 0)
		return 0;

	int written = 0;
	int cntr = 0;

	while (written != _size) {
		int ret = write ( _fd, _buff + written, _size - written);
		
		if(ret < 0)
			return 1;
		else {
			written += ret; 
			cntr = 0; //reset fail counter
		}

		if(++cntr > _retry)
			return 1;
	}
	return 0;
}
