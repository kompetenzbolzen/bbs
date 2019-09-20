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
	
	LOGPRINTF(_LOG_DEBUG, "debug");
	LOGPRINTF(_LOG_NOTE, "note");
	LOGPRINTF(_LOG_WARNING, "warn");
	LOGPRINTF(_LOG_ERROR, "Error");

	signal(SIGCHLD,SIG_IGN); //Ignore sigchld
	struct prog_params params = parse_args(argc, argv);

	//Fork and write PID to pidfile
	if(params.fork)
	{
		FILE* pidfile = fopen(params.pidfile, "w");
		
		if(!pidfile)
		{
			PRINT_ERROR("Unable to open pidfile for writing");
			exit(1);
		}
		pid_t pid = fork();

		if(pid < 0)
		{
			PRINT_ERROR("fork() failed");
			exit(1);
		}
		else if(pid > 0)
		{
			fprintf(pidfile, "%i", pid);
			printf("Forked with PID %i\n", pid);
			fclose (pidfile);
			exit(0);
		}

		fclose (pidfile);

		//Close STDIO
		close (STDIN_FILENO);
		close (STDOUT_FILENO);
		close (STDERR_FILENO);
	}//if params.fork

	DEBUG_PRINTF("%s, %i\n", params.run_argv[0], params.run_argc);

	if ( params.serial )
		dialup_server(params);
	else if (params.telnet)
		telnet_server(params);

	return 0;
}

