/*
 * main.c
 * (c) 2019, Jonas Gunz, jonasgunz.de
 * <Description>
 * License: MIT
*/

#include "main.h"

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

void handle_connection(int _socket, struct sockaddr_in _addr, int argc, char* argv[])
{
	pid_t pid = fork();
	if( pid != 0 ) {
		close(_socket);
		return;
	}

	int in[2];
	int out[2];

	//1: write, 0: read
	if(pipe(in) == -1)
		return;
	
	if(pipe(out) == -1)
		return;
	
	pid = fork_run(in[0], out[1], out[1], argc, argv);
	if (pid < 0)
		return;

	const int buffsize = 128;
	char buff[ buffsize + 1];
	buff[buffsize] = '\0';

	//close unused pipes
	close (in[0]);
	close (out[1]);

	//setup poll to listen for input
	struct pollfd fds[2];
	fds[0].fd = out[0];
	fds[0].events = POLLIN;
	fds[1].fd = _socket;
	fds[1].events = POLLIN;

	LOGPRINTF(_LOG_NOTE, "%i: Connected to %s", pid, inet_ntoa(_addr.sin_addr));

	while(1)
	{
		int ret = poll (fds, 2, 100);
		if ( fds[0].revents & POLLIN ) {
			const int cnt = read (out[0], buff, buffsize);
			if(try_write(_socket, buff, cnt, 100)) {
				LOGPRINTF(_LOG_ERROR, "%i: Consecutive write errors while writing to socket.", pid);
				break;
			}
		}
		if ( fds[1].revents & POLLIN ) {
			const int cnt = read (_socket, buff, buffsize);
			
			if(cnt == 0)
				break;

			char *needle = strstr(buff, "\r");
			if (needle) //Replace CR with space
				*needle = ' ';

			if(try_write(in[1], buff, cnt, 100)) {
				LOGPRINTF(_LOG_ERROR, "%i: Consecutive write errors while writing to STDIN.", pid);
				break;
			}

		}

		if (ret < 0){
			LOGPRINTF(_LOG_ERROR, "Poll error\n");
			break;
		}

		if(kill(pid,0)) //Check if child is still alive, if not return.
			break;
	}

	LOGPRINTF(_LOG_NOTE, "%i: Connection closed.", pid);

	kill(pid,SIGKILL);

	close(_socket);
	exit(1);
}


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

void dialup_server(struct prog_params params)
{
	printf("Starting dialup server\n");

	while(1) {
		//Serial port is reopened for every new connection to reset modem over DTR
		int fd = open (params.serial_port, O_RDWR | O_NOCTTY | O_SYNC);
		if (fd < 0) {
			PRINT_ERROR("Failed to open serial port");
		        return;
		}
	
		set_interface_attribs (fd, params.serial_baudrate, 0);
		set_blocking (fd, 0);

		int ret = modem_accept_wait(fd);

		if(ret) {
			printf("Modem error %i\n", ret);
			close(fd);
			break;
		}

		DEBUG_PRINTF("Connection\n");

		modem_run(fd, params.run_argc, params.run_argv);
		close (fd);
	}
}

void telnet_server(struct prog_params params)
{
	signal(SIGCHLD,SIG_IGN); //Ignore sigchld

	int server_socket, client_socket;
	struct sockaddr_in socket_address, client_address;
	size_t claddrsize = sizeof(client_address);
	
	if ( (server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1 )
	{
		printf("Error creating socket: %i: %s\n", errno, strerror(errno));
		exit(1);
	}

	memset (&socket_address, 0, sizeof(socket_address));

	socket_address.sin_family = AF_INET;
	socket_address.sin_port = htons( params.port );

	if ( (bind(server_socket, &socket_address, sizeof(socket_address))) == -1 )
	{
		printf("Error binding socket: %i: %s\n", errno, strerror(errno));
		exit(1);
	}

	if ( (listen(server_socket, 10)) == -1 )
	{
		printf("Error listening socket: %i: %s\n", errno, strerror(errno));
		exit(1);
	}

	while(1)
	{
		client_socket = accept(server_socket, &client_address, &claddrsize);
		DEBUG_PRINTF("Connection: %s\n", inet_ntoa(client_address.sin_addr));
		handle_connection(client_socket, client_address, params.run_argc, params.run_argv);
	}
}
