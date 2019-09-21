/*
 * main.c
 * (c) 2019, Jonas Gunz, jonasgunz.de
 * <Description>
 * License: MIT
*/

#include "main.h"

int main(int argc, char* argv[])
{
	log_init_stdout(_LOG_DEBUG);

	signal(SIGCHLD,SIG_IGN); //Ignore sigchld
	struct prog_params params = parse_args(argc, argv);

	//Fork and write PID to pidfile
	if(params.fork)
	{
		FILE* pidfile = fopen(params.pidfile, "w");

		if(!pidfile)
		{
			LOGPRINTF(_LOG_ERROR,"Unable to open pidfile\n");
			exit(1);
		}
		pid_t pid = fork();

		if(pid < 0)
		{
			LOGPRINTF(_LOG_ERROR,"fork failed\n");
			exit(1);
		}
		else if(pid > 0)
		{
			fprintf(pidfile, "%i", pid);
			LOGPRINTF(_LOG_INFO,"Forked with PID %i\n\n", pid);
			fclose (pidfile);
			exit(0);
		}

		fclose (pidfile);

		//Close STDIO
		close (STDIN_FILENO);
		close (STDOUT_FILENO);
		close (STDERR_FILENO);
	}//if params.fork

	LOGPRINTF(_LOG_DEBUG, "%s, %i\n", params.run_argv[0], params.run_argc);

	if ( params.serial )
		dialup_server(params);
	else if (params.telnet)
		telnet_server(params);

	return 0;
}

