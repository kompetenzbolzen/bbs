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

struct prog_params parse_args(int argc, char* argv[])
{
	struct prog_params ret;
	memset(&ret, 0, sizeof(ret));

	for (int i = 1; i < argc; i++)
	{
		int i_cpy = i; //i might be changed in loop

		if(argv[i_cpy][0] == '-')
		{
			for (int o = 1; o < strlen(argv[i_cpy]); o++)
			{
				switch (argv[i_cpy][o])
				{
					case 'h':
						ERROR_HELP("");
						break;
					case 'p':
						ret.telnet = 1;
						ret.port = atoi(argv[i_cpy + 1]);
						i++;
						break;
					case 'i':
						ret.telnet = 1;
						ret.ip = argv[i_cpy + 1];
						i++;
						break;
					case 's'://Serial modem
						ret.serial = 1;
						ret.serial_port = argv[i_cpy + 1];
						i++;
						break;
					case 'b':
						ret.serial = 1;
						ret.serial_baudrate = atoi(argv[i_cpy + 1]);
						i++;
						break;
					case 'f'://PID file for spawned children
						ret.fork = 1;
						ret.pidfile = argv[i_cpy + 1];
						i++;
						break;
					default:
						ERROR_HELP("Unrecognized Option: '%c'\n", argv[i_cpy][o]);
						break;
				};//switch
			}//for
		}//if
		else
		{
			//Copy the rest as arguments for prog to exec
			ret.run_argc = argc - i_cpy;
			ret.run_argv = &(argv[i_cpy]);
			break;
		}//else
	}//for

	if(ret.telnet == ret.serial)//run EITHER in telnet OR modem mode
		ERROR_HELP("Select either modem OR telnet.\n");

	return ret;
}
