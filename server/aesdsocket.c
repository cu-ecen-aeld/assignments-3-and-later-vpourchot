#include <sys/types.h>
#include <syslog.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <signal.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netdb.h>

#define SERVER_PORT 9000
#define SA struct sockaddr

int listen_fd, connect_d, fp;
char *buf;

void error();
void handle_shut_down();
void bind_to_port();

int open_listener_socket();
int catch_signal();
int read_in();
int append_str();

/* Opens a stream socket bound to port 9000,
 * failing and returning -1 if any of the socket connection
 * steps fail
 */


int main(int argc, char *argv[]){

	if (strcmp("-d", argv[1]) == 0)
		if (daemon (0,1) != 0)
			error("Daemon failed");

        puts("test0");
	int sig = 1;

        if (catch_signal(SIGINT, handle_shut_down) == -1)  
              sig = 0;
//		error("Can't set the interrupt handler");
    
        puts("test1");
        listen_fd =             open_listener_socket();
    
        puts("test2");
        bind_to_port(listen_fd, SERVER_PORT);

        puts("test3");
        if (listen(listen_fd, 100) == -1) 
                error("Can't listen");

        struct sockaddr_storage client_addr;
        unsigned int address_size = sizeof(client_addr);

        puts("Waiting for connection");

        buf = malloc(sizeof(buf));
    
        char *writefile =       "/var/tmp/aesdsocketdata";
//      char *writestr;

        fp = creat(writefile,0666);
        if (fp == -1) 
                error("Directory does not exist");

        while (sig == 1) 
        {
                puts("while_loop");
                connect_d = accept(listen_fd, (struct sockaddr *)&client_addr, &address_size);
                if (connect_d == -1) 
                        error("Can't open secondary socket");
                puts("while_loop2");
                syslog(LOG_DEBUG,"Accepted connection from %d",connect_d);
    
                puts("while_loop1");    
                int new_line = read_in(connect_d, buf, sizeof(buf));
		if (new_line != 0)
			error("Didn't finish stream");
		
		if (append_str(fp,writefile,buf) != 0)
			error("Couldn't append test to file");

		if (send(connect_d, buf, strlen(buf), 0) == -1)
                        error("send");

               /* if (remove(writefile) == 0)
                        puts("file removed");
                else
                        error("remove file fail");
                */
		puts("while_loop_end");
                close(fp);
                close(listen_fd);
                free(buf);
                close(connect_d);
		
		return 0;
        }

	return 0;


}



/**
 * Listens for and accepts a connection
 */

void error(char *msg)
{
	fprintf(stderr, "%s/n: %s/n", msg, strerror(errno));
	if (buf)
	{
		free(buf);
                puts("shut_down_err");
                ssize_t n = read(fp, buf, BUFSIZ);

                send(connect_d, &n, sizeof(n),0);
                close(fp);
                close(listen_fd);
                free(buf);
                close(connect_d);
        }
	
	exit(-1);
}


int open_listener_socket()
{
	int listener_d = socket(PF_INET, SOCK_STREAM, 0);
	if (listener_d == -1) 
		error("Can't open socket");

	return listener_d;
}


void bind_to_port(int socket, int port)
{

	struct sockaddr_in name;
//        bzero(&servaddr, sizeof(servaddr));

	name.sin_family = 	PF_INET;
	name.sin_port = 	(in_port_t)htons(port);
	name.sin_addr.s_addr = 	htonl(INADDR_ANY);
	int reuse = 		1;

	if (setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(int)) == -1)
		error("Can't set the reuse option on the socket");

	int c = bind (socket, (struct sockaddr *) &name, sizeof(name));
	if (c == -1)
		error("Can't bind to socket");
}

int append_str(int fp, char *writefile,char *writestr) {

        ssize_t nr = write(fp,writestr,strlen(writestr));
        if (nr == -1) {
              syslog(LOG_ERR,"Invalid Number of arguments: No String Provided %s",writestr);
                return 1;
        }
        int fsync(int fp);

        int close (int fp);

        return 0;

}


int read_in(int socket, char *buf, int len)
{
        char *s =       buf;
        int slen =      len;
        int c =         recv(socket, s, slen, 0); 

        while ((c > 0) && (s[c-1] != '\n'))
        {
                s += c;
                slen -= c;
                c = recv(socket, s, slen, 0); 

        }

        if (c < 0)  
                return c;
        else if (c == 0)
                buf[0] = '\n';
        else
                s[c-1] = '\n';
	
	printf("%s",s);
        return len - slen;

}

int catch_signal(int sig, void (*handler)(int))
{
	struct sigaction action;
	action.sa_handler = handler;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;
	return sigaction (sig, &action, NULL);
}

void handle_shut_down(int sig)
{
	printf("%d\n",sig);
	if (fp && listen_fd)
	{
	        puts("shut_down");
		ssize_t n = read(fp, buf, BUFSIZ);

                send(connect_d, &n, sizeof(n),0);
                close(fp);
		close(listen_fd);
		free(buf);
		close(connect_d);
	}
	
	puts("Goodbye!");
        exit(0);
}



/* Logs message to the syslog "Accepted Connection from xxx"
 * where xxxx is the IP address of the connected client
 */






/**
 * Receives data over the connection and appends to file /var/tmp/aesdsocketdata,
 * creating this file if it doesn't exist
 */




/**
 * Returns the full content of /var/tmp/aesdsocketdata to the client
 * as soon as the received data packet completes
 */




/**
 * Logs message to the syslog "Caught signal, exiting" when SIGINT or SIGTERM is received
 */




/**
 * Exits when SIGINT or SIGTERM is received, completeing an open connection operations,
 * closing open sockets, and
 * deleting the file /var/tmp/aesdsocketdata
 */


