#include "telnet.h"

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
			if (needle){ //Replace CR with space
				needle[0] = '\n';
				if ((needle - buff + 1) < buffsize)
					needle[1]='\0';
			}

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

void telnet_server(struct prog_params params)
{
	signal(SIGCHLD,SIG_IGN); //Ignore sigchld

	int server_socket, client_socket;
	struct sockaddr_in socket_address, client_address;
	socklen_t claddrsize = sizeof(client_address);
	
	if ( (server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1 )
	{
		printf("Error creating socket: %i: %s\n", errno, strerror(errno));
		exit(1);
	}

	memset (&socket_address, 0, sizeof(socket_address));

	socket_address.sin_family = AF_INET;
	socket_address.sin_port = htons( params.port );

	if ( (bind(server_socket, (struct sockaddr*) &socket_address, sizeof(socket_address))) == -1 )
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
		client_socket = accept(server_socket, (struct sockaddr*)&client_address, &claddrsize);
		DEBUG_PRINTF("Connection: %s\n", inet_ntoa(client_address.sin_addr));
		handle_connection(client_socket, client_address, params.run_argc, params.run_argv);
	}
}
