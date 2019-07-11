/*
 * modem.c
 * (c) 2019, Jonas Gunz, jonasgunz.de
 * <Description>
 * <License>
*/

#include "modem.h"

int modem_accept_wait(int fd)
{
	if(!fd)
		return 1;

	char buff[128];
	int ret;		//for retturn values
	struct pollfd fds;	//poll struct
	int cnt = 0;		//read byte counter

	fds.fd = fd;
	fds.events = POLLIN;

	modem_command(fd, _AT_ECHO_OFF, 1000);
	modem_command(fd, _AT_MUTE, 1000);
	modem_command(fd, _AT_RESET_ON_DTR, 1000);

#ifdef _MODEM_WAIT_RING
	while ( 1 ) { //wait for RING
		ret = poll(&fds, 1, 2000); //poll in 2s interval
		usleep(5000);
		if(ret) {
			cnt = read ( fd, buff, 128 );
			if(strstr(buff, "RING"))
				break;
		}
	}
#else
#warning "Wait for RING disabled"
#endif
	printf("Modem RINGING\n");
	int ok = 5;
	int timeout = 60000;
	write (fd, _AT_ANSWER, strlen(_AT_ANSWER)); //Accept incoming call
	cnt = 0;
	while(1)
	{
		ret = poll(&fds, 1, timeout); //Connection timeout 1 minute
		usleep(5000); //Wait for data
		if(!ret)
			break;

		if(cnt >= 128)
			break;

		cnt += read (fd, &buff[cnt - 1], 128 - cnt + 1);

		printf("%s\n", buff);

		if(strstr(buff, "CONNECT")){
			ok = 0;
			break;
		}
		else if(strstr(buff, "NO CARRIER")) //Don't timeout on error
			break;
	}

	if(poll(&fds, 1, 1)) //empty the buffer
			read(fd, &buff, 128);

	return ok;
}

int modem_command(int fd, char* cmd, int timeout_ms)
{
	char buff[128];
	int ret;		//for retturn values
	struct pollfd fds;	//poll struct
	int cnt = 0;		//read byte counter

	fds.fd = fd;
	fds.events = POLLIN;

	write(fd, cmd, strlen(cmd) );

	int ok = 1;
	while(1) {
		ret = poll(&fds, 1, timeout_ms);
		usleep(5000); //Wait for data to fully come in
		if (ret)
			cnt += read(fd, &buff[cnt - 1], 128 - cnt + 1);
		else 
			break;

		if(cnt >= 128)
			break;


		if( strstr(buff, "OK") ) {
			ok = 0;
			break;
		}
	}

	if(poll(&fds, 1, 1)) //Empty the buffer
			read(fd, &buff, 128);

	printf("Command %s: %s\n", cmd, ok ? "FAIL" : "OK");

	return ok;
}

int modem_run(int fd, int argc, char* argv[])
{
	int in[2];
	int out[2];

	//1: write, 0: read
	if(pipe(in) == -1)
		return 1;
	
	if(pipe(out) == -1)
		return 1;
	
	pid_t pid = fork_run(in[0], out[1], out[1], argc, argv);//fork();
	
/*	if(pid == 0) {//child
		close (in[1]);
		close (out[0]);

		dup2 (in[0],  STDIN_FILENO);
		dup2 (out[1], STDOUT_FILENO);
		dup2 (out[1], STDERR_FILENO);

		char* arv[argc + 1];

		for(int i = 0; i < argc; i++)
			arv[i] = argv[i];

		arv[argc] = NULL;

		execv(argv[0], arv);

		printf("EXEC ERROR %i: %s\r\n", errno, strerror(errno));
		exit(1);
	}*/
	if (pid < 0) {//error
		return 2;
	}

	int buffsize = 128;
	char buff[ buffsize ];

	close (in[0]);
	close (out[1]);

	//setup poll to listen for input
	struct pollfd fds[2];
	fds[0].fd = out[0];
	fds[0].events = POLLIN;
	fds[1].fd = fd;
	fds[1].events = POLLIN;

	printf("Forked with PID %i\n", pid);

	while(1)
	{
		int ret = poll (fds, 2, 100);
		usleep(5000); //Wait for data to fully come in, helps prevent truncation of status returns which helps with parsing
		if ( fds[0].revents & POLLIN ) {
			int cnt = read (out[0], buff, buffsize);
			if(cnt) {
				//replace +++ to not trigger modem command mode
				char *str = strstr(buff, "+++");
				if(str)
					str[1] = '*';

				if(try_write(fd, buff, cnt, 100)) {
					printf("Consecutive write errors while writing to serial device.\n");
					break;
				}
			}
		}
		if ( fds[1].revents & POLLIN ) {
			int cnt = read (fd, buff, buffsize);
			if(cnt) {
				//search for modem error message
				char *str = strstr(buff, "NO CARRIER");
				if(str){ //Exit if message found
					kill(pid,SIGTERM);
					break;
				}

				if(try_write(in[1], buff, cnt, 100)) {
					printf("Consecutive write errors while writing to STDIN.\n");
					break;
				}
			}
		}

		if(kill(pid,0)) //Check if child is still alive, if not return.
			break;
	}

	printf("Connection closed.\n");
	close(fd); //Auto closes connection
	return 0;
}
