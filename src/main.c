#include "main.h"
#include "serial.h"
#include "modem.h"

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
		}//else
	}//for

	if(ret.telnet == ret.serial)//run EITHER in telnet OR modem mode
		ERROR_HELP("Select either modem OR telnet.\n");

	return ret;
}

void handle_connection(int _socket, struct sockaddr_in _addr)
{
	pid_t pid = fork();
	if( pid > 0 )
	{
		close(_socket);
		return;
	}
	else if ( pid < 0 )
		return;

	//This is here for later
	//This should handle a premature closed socket from client to not create unused processes
	pid = fork();
	if(pid > 0)
	{
		close(_socket);
		//Instead of waiting for pid, check if process is running AND socket is still open
		wait(0);
		exit(0);
	}
	else if(pid < 0)
		exit(1);
	
	//Redirect STDIO to socket
	close(STDIN_FILENO);
	dup(_socket);
	close(STDOUT_FILENO);
	dup(_socket);

	//RUUNNNN!
	execl("/usr/bin/whoami", "/usr/bin/whoami", NULL);

	PRINT_ERROR("EXEC failed");

	close(_socket);
	exit(0);
}


int main(int argc, char* argv[])
{
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
		}

		fclose (pidfile);
	}//if params.fork

	if ( params.serial )
		dialup_server(params);
	else if (params.telnet)
		telnet_server(params);

	return 0;
}

void dialup_server(struct prog_params params)
{

	int fd = open (params.serial_port, O_RDWR | O_NOCTTY | O_SYNC);
	if (fd < 0)
	{
	        printf ("error %d opening %s: %s\n", errno, params.serial_port, strerror (errno));
	        return;
	}
	
	///TODO Hardcoded Baudrate. change!
	set_interface_attribs (fd, params.serial_baudrate, 0);  // set speed to 115,200 bps, 8n1 (no parity)
	set_blocking (fd, 0);                // set no blocking

	int ret = modem_accept_wait(fd);
	printf("maw(): %i\n", ret);

	if(ret)
		return;

	printf("Connection!\n");

	modem_run(fd, 0, NULL);
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
		handle_connection(client_socket, client_address);
	}
}
