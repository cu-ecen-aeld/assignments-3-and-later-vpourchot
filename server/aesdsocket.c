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
char *buf,*file_buf;

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

        puts("test0");

        if (catch_signal(SIGINT, handle_shut_down) == -1)  
              error("SigInt");

        if (catch_signal(SIGTERM, handle_shut_down) == -1)
              error("SigTerm");
	
    
        puts("test1");
        listen_fd =             open_listener_socket();
    
        puts("test2");
        bind_to_port(listen_fd, SERVER_PORT);

	if (strcmp("-d", argv[1]) == 0)
                daemon (0,1);

        puts("test3");
        if (listen(listen_fd, 10) == -1) 
                error("Can't listen");

        struct sockaddr_storage client_addr;
        unsigned int address_size = sizeof(client_addr);

        puts("Waiting for connection");

        char *writefile =       "/var/tmp/aesdsocketdata";

        fp = open(writefile,O_WRONLY | O_CREAT | O_APPEND, 0666);
        if (fp == -1) 
                error("Directory does not exist");
	
	puts("test4");

	connect_d = accept(listen_fd, (struct sockaddr *)&client_addr, &address_size);
	if (connect_d == -1)
		error("Can't open secondary socket");

        syslog(LOG_DEBUG,"Accepted connection from %d",connect_d);

        while (1) 
        {
                if (!buf)
			buf = malloc(sizeof(buf));
		printf("%s\n",buf);
                puts("loop1");  
                int new_line = read_in(connect_d,buf, sizeof(buf),fp,writefile);
		if (new_line != 0)
			error("Didn't finish stream\n");
/*	       
	        int slen =      sizeof(buf);
	        char *s = 	buf;
       		int c =         recv(connect_d, s, slen, 0);
		printf("%d\n%s\n",c,s);
	        while (c > 0) // && (s[c-1] != '\n'))
        	{
                	puts("while_2\n");
			s += c;
			slen -= c;
			printf("%s",s);
//	                slen -= c;
//	                c = recv(connect_d,packet_buf, slen, 0);
        	
		
			ssize_t nr = write(fp,s,strlen(s));
        
			if (nr == -1)               		
				syslog(LOG_ERR,"Invalid Number of arguments: No String Provided %s",buf);
			int fsync(int fp);
	 		c = recv(connect_d,s, slen, 0);
		}
*/
//		if (append_str(fp,writefile,packet_buf) != 0)
//			error("Couldn't append test to file");
		puts("loop2");
/*		if (buf)
			free(buf);
		
		puts("loop3");
*/		file_buf = (char *) malloc(sizeof(file_buf));
		long length;
		FILE *f = fopen(writefile, "rb");

		if (f)
		{
			fseek (f, 0, SEEK_END);
			length = ftell(f);
			fseek (f,0,SEEK_SET);
			
			if (file_buf)
				fread(file_buf,1,length,f);
			fclose(f);
		}
		file_buf[length - 1] = '\n';
		printf("%s",file_buf);
		if (file_buf)
		{
			if (send(connect_d,file_buf,strlen(file_buf), 0) == -1)
                        	error("send");
			free(file_buf);
		}

		puts("loop_end");
        
	}
        if (remove(writefile) == 0)
                puts("file removed");
        else
                error("remove file fail");

	if (fp)
		close(fp);
        if (listen_fd)
		close(listen_fd);
        if (file_buf)
		free(file_buf);
	if (buf)
		free(buf);
        if (connect_d)
		close(connect_d);

	return 0;


}



/**
 * Listens for and accepts a connection
 */

void error(char *msg)
{
	fprintf(stderr, "%s\n: %s\n", msg, strerror(errno));

        puts("shut_down_err\n");

        if (fp)
                close(fp);
        if (listen_fd)
                close(listen_fd);
        if (file_buf)
                free(file_buf);
        if (buf)
                free(buf);
        if (connect_d)
                close(connect_d);
	
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
	
//        int close (int fp);

        return 0;

}


int read_in(int socket, char *buf, int len ,int fp, char *writefile)
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
                buf[0] = '\0';
        else
                s[c-1] = '\0';

	if (append_str(fp,writefile,buf) != 0)
                error("Couldn't append test to file");

        printf("%s",s);
        return 0; //len - slen;

}

/*
int read_in(int socket, char *buf, int len) //,int fp, char *writefile)
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
		
//	if (append_str(fp,writefile,buf) != 0)
//                error("Couldn't append test to file");


        if (c < 0)  
                return c;
        else if (c == 0)
                buf[0] = '\n';
        else
                s[c-1] = '\n';
	
//	int close(int fp);
		
	printf("%s",s);
        return len - slen;

}
*/

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
        if (fp)
                close(fp);
        if (listen_fd)
                close(listen_fd);
        if (file_buf)
                free(file_buf);
        if (buf)
                free(buf);
        if (connect_d)
                close(connect_d);
	
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


