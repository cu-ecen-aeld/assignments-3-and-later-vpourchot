#include <sys/types.h>
#include <syslog.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdbool.h>
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

struct addrinfo *servinfo;
int listener_d,connect_d,fp;
char *buf,*file_buf,*writefile;

void error();
void handle_shut_down();
void bind_to_port();

int open_listener_socket();
int catch_signal();
int read_in();
int append_str();
int BUF_SIZE = 1024;

/* Opens a stream socket bound to port 9000,
 * failing and returning -1 if any of the socket connection
 * steps fail
 */


int main(int argc, char *argv[]){

        puts("test0");
	bool d_mode = false;
	if(argc == 2)
	{
		if (strcmp(argv[1], "-d") == 0)
			d_mode = true;
	}
//        if (catch_signal(SIGINT, handle_shut_down) == -1)  
//              error("SigInt");

//        if (catch_signal(SIGTERM, handle_shut_down) == -1)
//              error("SigTerm");
	signal(SIGINT, handle_shut_down);
	signal(SIGTERM, handle_shut_down);

	int status;
	struct addrinfo hints;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family =       AF_UNSPEC;
        hints.ai_socktype =     SOCK_STREAM;
        hints.ai_flags =        AI_PASSIVE;
//        int reuse =             1;

        if ((status = getaddrinfo(NULL, "9000", &hints, &servinfo) != 0))
	{
	        perror("getaddrinfo fail");
		return -1;
	}
    
        puts("test1");
        listener_d = socket(servinfo->ai_family, servinfo->ai_socktype, 0); 
        if (listener_d == -1) 
	{
		perror("Can't open socket");
		return -1;
	}

        puts("test2");
        if (setsockopt(listener_d, SOL_SOCKET, SO_REUSEADDR,&(int){1}, sizeof(int)) == -1)
	{
		perror("Can't set the reuse option on the socket");
		return -1;
	}

        int c = bind (listener_d, servinfo->ai_addr, servinfo->ai_addrlen);
        if (c == -1)
	{
		perror("Can't bind to socket");
		return -1;
	}
//        bind_to_port(listen_fd, SERVER_PORT);
	if(d_mode && c == 0)
	{
		int pid = fork();
		if(pid > 0 )
			exit(0);
	}

//	if (strcmp("-d", argv[1]) == 0)
//                daemon (0,1);

        puts("test3");
        if (listen(listener_d, 10) == -1) 
	{
		perror("Can't listen");
		return -1;
	}
//        struct sockaddr_storage client_addr;
//        unsigned int address_size = sizeof(client_addr);

        puts("Waiting for connection");

        writefile = "/var/tmp/aesdsocketdata";

        fp = open(writefile,O_WRONLY | O_CREAT | O_APPEND | O_TRUNC, 0666);
        if (fp == -1) 
	{
		perror("Directory does not exist");
		return -1;
	}
//	file_buf = malloc(BUF_SIZE);
	
	puts("test4");
/*        connect_d = accept(listener_d, servinfo->ai_addr, &servinfo->ai_addrlen);
        if (connect_d == -1)
                error("Can't open secondary socket");

        syslog(LOG_DEBUG,"Accepted connection from %d",connect_d);
	puts("test4");	
*/
        while (1) 
        {
		puts("loop0");

                connect_d = accept(listener_d, servinfo->ai_addr, &servinfo->ai_addrlen);
        	if (connect_d == -1)
		{
                	perror("Can't open secondary socket");
			return -1;
		}
       		syslog(LOG_DEBUG,"Accepted connection from %d",connect_d);
		
		buf = malloc(1);
		memset(buf, 0, 1);
		size_t buf_len = 0;
	  	file_buf = malloc(BUF_SIZE);
                memset(file_buf,0,BUF_SIZE);
		ssize_t read_d = 0;

		puts("loop1");  
//                int new_line = read_in(connect_d,fp,writefile);
//		if (new_line != 0)
//			error("Didn't finish stream\n");

		
		while((read_d = recv(connect_d,file_buf,BUF_SIZE,0)) > 0)
		{
			buf_len += read_d;
			buf = realloc(buf,(buf_len + read_d)+1);

			if (buf  == NULL)
			{
				exit(EXIT_FAILURE);
			}

			for (ssize_t i = 0; i < read_d; i++)
			{
				char c = file_buf[i];
				char tmpstr[2];
				tmpstr[0] = c;
				tmpstr[1] = 0;

				strcat(buf,tmpstr);

				if (c == '\n')
				{
					puts("if stmt append str");
					FILE *fp;
					
					fp = fopen(writefile, "a");
					if (fp == NULL)
					{
						syslog(LOG_ERR, "Error opening %s", writefile);
						exit(EXIT_FAILURE);
					}

					fputs(buf,fp);
					if (ferror(fp))
					{
						syslog(LOG_ERR, "Error writing %s", buf);
						exit(EXIT_FAILURE);
					}		
					fclose(fp);

			//		file_buf = (char *) malloc(BUF_SIZE);
			//		memset(file_buf,0,BUF_SIZE);
			 	 	size_t len = 0;
					char * line = NULL;
					ssize_t buffer;
					fp = fopen(writefile, "r");
					if (fp == NULL)
						error("file not founds");

/*
		if (f)
		{
			fseek (f, 0, SEEK_END);
			length = ftell(f);
			fseek (f,0,SEEK_SET);
			
			fread(file_buf,1,length,f);
//			file_buf[length-1] = '\0';
//			fclose(f);
		}
*/		
					while((buffer = getline(&line,&len,fp)) != -1)
					{
						ssize_t sent = send(connect_d, line, buffer, 0);
						if(sent == -1)
							error("unable to send");
						else
							printf("Sent: %s",line);
					}
/*		char *s = file_buf;
//		length = read(fp,file_buf,BUF_SIZE);
                printf("%s",s);	
		for (int i = 0; i < strlen(s); i++)
		{
			puts("send_loop");
                        int send_d = send(connect_d,&s[i],sizeof(&s[i]), 0);
			if (send_d == -1)
                                error("unable to send");

//			memset(file_buf,0,BUF_SIZE);
//                      length = read(fp,file_buf,sizeof(file_buf));
			
		}
*/				//	fclose(fp);
					break;
				}
			}
			memset(file_buf, 0, BUF_SIZE);
		}
		
		close(connect_d);
//		freeaddrinfo(servinfo);
		free(buf);
		free(file_buf);
		puts("loop_end");
        
	}
        if (remove(writefile) == 0)
                puts("file removed");
        else
                error("remove file fail");

/*	if (fp)
		close(fp);
        if (listener_d)
		close(listener_d);
        if (file_buf)
		free(file_buf);
	if (buf)
		free(buf);
        if (connect_d)
		close(connect_d);
*/
	return 0;

}



/**
 * Listens for and accepts a connection
 */

void error(char *msg)
{
	fprintf(stderr, "%s\n: %s\n", msg, strerror(errno));

        puts("shut_down_err\n");

/*        if (fp)
                close(fp);
        if (listener_d)
                close(listener_d);
        if (file_buf)
                free(file_buf);
        if (buf)
                free(buf);
        if (connect_d)
                close(connect_d);
*/
        if (remove(writefile) == 0)

                puts("file removed");
        else
                error("remove file fail");
        freeaddrinfo(servinfo);
        shutdown(listener_d, SHUT_RDWR);
        puts("Goodbye!");
        exit(-1);

}

/*
int open_listener_socket()
{
	int listener_d = socket(servinfo->ai_family, servinfo->ai_socktype, 0);
	if (listener_d == -1) 
		error("Can't open socket");

	return listener_d;
}


void bind_to_port(int socket, int port)
{
	
	memset(&hints, 0, sizeof(hints));

	hints.ai_family = 	AF_UNSPEC;
	hints.ai_socktype = 	SOCK_STREAM;
	hints.ai_flags = 	AI_PASSIVE;
	int reuse = 		1;

	if ((status = getaddrinfo(NULL, "9000", &hints, &servinfo) != 0))
			error("getaddrinfo fail");
	if (setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(int)) == -1)
		error("Can't set the reuse option on the socket");

	int c = bind (socket, servinfo->ai_addr, servinfo->ai_socktype);
	if (c == -1)
		error("Can't bind to socket");
}


int append_str(int fp, char *writefile,char *writestr) {

        ssize_t nr = write(fp,writestr,strlen(writestr));
        if (nr == -1) {
                error("unable to write to file");
        }
        int fsync(int fp);
	
//        int close (int fp);

        return 0;

}
*/

int read_in(int socket,int fp, char *writefile)
{
        char *s =       malloc(BUF_SIZE);
        memset(s,0,BUF_SIZE);
	int slen =      sizeof(s);
//        int c =         
	int c = recv(socket, s, slen, 0);
	s[c - 1] = '\n';
	printf("%s",s);
/*        while ((c > 0) && (s[c-1] != '\n'))
        {
                s += c;
                slen -= c;
                c = recv(socket, s, slen, 0);
        }

        if (c < 0)
                return c;
        else if (c == 0)
                s[0] = '\0';
        else
	{
		s[c-1] = '\0';
	}
*/	
//	strcat(s,(char*)'\n');	
//	char *new_ln = '\n';
        ssize_t nr = write(fp,s,strlen(s));
        if (nr == -1) 
                error("unable to write to file");
        
//	nr = write(fp,new_ln,strlen(new_ln));
  //      if (nr == -1)
    //            error("unable to write to file");

        int fsync(int fp);

//	if (append_str(fp,writefile,s) != 0)
//                error("Couldn't append test to file");
//        if (append_str(fp,writefile,'\n') != 0)
//                error("Couldn't append test to file");
	
        printf("%s",s);
//	free(s);
        return 0; //len - slen;

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
        
/*	if (fp)
                close(fp);
        if (listener_d)
                close(listener_d);
        if (file_buf)
                free(file_buf);
        if (buf)
                free(buf);
        if (connect_d)
                close(connect_d);
*/
	if (remove(writefile) == 0)
                puts("file removed");
        else
                error("remove file fail");	
	freeaddrinfo(servinfo);
	shutdown(listener_d, SHUT_RDWR);
	puts("Goodbye!");
        exit(sig);
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


