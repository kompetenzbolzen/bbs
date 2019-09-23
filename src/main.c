/*
 * main.c
 * (c) 2019, Jonas Gunz, jonasgunz.de
 * <Description>
 * License: MIT
*/

#include "main.h"

int main(int argc, char* argv[])
{
	struct prog_params params = parse_args(argc, argv);
	
	log_init_stdout( _LOG_DEBUG );

	signal( SIGCHLD, SIG_IGN ); //Ignore sigchld
	signal( SIGKILL, terminate);
	signal( SIGINT, terminate);

	//Fork and write PID to pidfile
	if(params.fork)
	{
		FILE* pidfile = fopen(params.pidfile, "w");

		if(!pidfile)
		{
			LOGPRINTF( _LOG_ERROR, "Unable to open pidfile\n" );
			exit(1);
		}
		pid_t pid = fork();

		if(pid < 0)
		{
			LOGPRINTF( _LOG_ERROR, "fork failed\n" );
			exit(1);
		}
		else if(pid > 0)
		{
			fprintf( pidfile, "%i", pid );
			LOGPRINTF( _LOG_NOTE, "Forked with PID %i\n\n", pid );
			
			if( log_fd == STDIN_FILENO )
				LOGPRINTF( _LOG_WARNING, "STDIO will be closed but no logfile is set. No logging possible\n" );
			
			fclose ( pidfile );
			exit( 0 );
		}

		fclose (pidfile);

		//Close STDIO
		close ( STDIN_FILENO );
		close ( STDOUT_FILENO );
		close ( STDERR_FILENO );
	}//if params.fork

	LOGPRINTF(_LOG_DEBUG, "command: %s, %i\n", params.run_argv[0], params.run_argc);

	if ( params.serial )
		dialup_server( params );
	else if ( params.telnet )
		telnet_server( params );

	return 0;
}

void terminate()
{
	log_close();
	exit ( 1 );
}
