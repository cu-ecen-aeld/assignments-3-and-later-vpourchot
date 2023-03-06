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

void 	error();
void 	handle_shut_down();
void 	bind_to_port();
struct 	addrinfo *servinfo;
char	*writefile;
char	*buf;
char	*file_buf;
bool	buf_free;
bool	term;
bool	connected;
bool	file_buf_free;
bool 	no_queue;
bool	str_search();
int	listener_d;
int	connect_d;
int	queue;
int 	open_listener_socket();
int 	catch_signal();
int 	read_in();
void	append_str();
int 	BUF_SIZE = 1024;

/* Opens a stream socket bound to port 9000,
 * failing and returning -1 if any of the socket connection
 * steps fail
 */


int main(int argc, char *argv[])
{
	term = false;
	no_queue = false;
	connected = false;
        puts("test0");
	bool d_mode = false;
	if(argc == 2)
	{
		if (strcmp(argv[1], "-d") == 0)
			d_mode = true;
	}
	buf_free = true;
	file_buf_free = true;
	
	signal(SIGINT, handle_shut_down);
	signal(SIGTERM, handle_shut_down);

	
	int status;
	struct addrinfo hints;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family =       AF_UNSPEC;
        hints.ai_socktype =     SOCK_STREAM;
        hints.ai_flags =        AI_PASSIVE;
        int reuse =             1;

        if ((status = getaddrinfo(NULL, "9000", &hints, &servinfo) != 0))
	{
	        perror("getaddrinfo fail");
		return -1;
	}
    
        puts("test1");
        listener_d = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol); 
        if (listener_d == -1) 
	{
		perror("Can't open socket");
		return -1;
	}

        puts("test2");
        if (setsockopt(listener_d, SOL_SOCKET, SO_REUSEADDR,&reuse, sizeof(int)) == -1)
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
	if(d_mode && c == 0)
	{
		int pid = fork();
		if(pid > 0 )
			exit(0);
	}

	char ipv4[INET_ADDRSTRLEN];
        puts("test3");
        if (listen(listener_d, 10) == -1) 
	{
		perror("Can't listen");
		return -1;
	}

        puts("Waiting for connection");

        writefile = "/var/tmp/aesdsocketdata";

        FILE *fp_buf = fopen(writefile, "w+");
	if (fp_buf == NULL) 
	{
		perror("Directory does not exist");
		return -1;
	}
	fclose(fp_buf);
	puts("test4");
        while (!term || !no_queue) 
        {
		puts("loop0");

                connect_d = accept(listener_d, servinfo->ai_addr, &servinfo->ai_addrlen);
        	if (connect_d == -1)
		{
			if (errno == EWOULDBLOCK)
			{
				puts("EWOULDBLOCK");
				no_queue = true;
			}
                	perror("Can't open secondary socket");
			return -1;
		}
		connected = true;
       		syslog(LOG_INFO,"Accepted connection from %s\n",inet_ntop(AF_INET,&servinfo,ipv4, INET_ADDRSTRLEN));
		
		buf = malloc(1);
		buf_free = false;
		memset(buf, 0, 1);
		size_t buf_len = 0;
	  	file_buf = malloc(BUF_SIZE);
		file_buf_free = false;
                memset(file_buf,0,BUF_SIZE);
		ssize_t read_d = 0;

		puts("loop1");  
		
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
					bool status = str_search(writefile, buf);
					append_str(writefile, buf, status);
					
					FILE * fp_buf;	
			 	 	size_t len = 0;
					char * line = NULL;
					ssize_t buffer;
					fp_buf = fopen(writefile, "r");
					if (fp_buf == NULL)
						error("file not founds");

					while((buffer = getline(&line,&len,fp_buf)) != -1)
					{
						ssize_t sent = send(connect_d, line, buffer, 0);
						if(sent == -1)
							error("unable to send");
						else
							printf("Sent: %s",line);
					}
					if (line)
						free(line);
					if (fp_buf)
						fclose(fp_buf);
					break;
				}
			}
			if (queue == 0)
				no_queue = true;

			memset(file_buf, 0, BUF_SIZE);
		}
		
           	free(file_buf);
        	file_buf_free = true;
                free(buf);
		buf_free = true;

		if (connect_d)
                	close(connect_d);	
		syslog(LOG_INFO, "Closed connection from %s\n", inet_ntop(AF_INET,&servinfo,ipv4,INET_ADDRSTRLEN));
		puts("loop_end");
        
	}
	
	if (remove(writefile) == 0)
                puts("file removed");
        else
                error("remove file fail");
	freeaddrinfo(servinfo);
	shutdown(listener_d, SHUT_RDWR);
	puts("Shutdown Goodbye");

	return 0;

}


bool str_search(char *writefile, char *str)
{

	FILE *fp_read;
	int line_num = 1;
//	int find_result = 0;
	char temp[1024];

	if ((fp_read = fopen(writefile, "r")) == NULL)
		error("unreadable file");
	while(fgets(temp, 1024, fp_read) != NULL) 
	{
		if((strstr(temp, str)) != NULL)
		{
			fclose(fp_read);
			return true;
		}
		line_num++;
	}
	fclose(fp_read);
	return false;
}




/**
 * Listens for and accepts a connection
 */

void error(char *msg)
{
	fprintf(stderr, "%s\n: %s\n", msg, strerror(errno));

        puts("shut_down_err\n");

        if (listener_d)
                close(listener_d);
        if (connect_d)
                close(connect_d);

        if (remove(writefile) == 0)

                puts("file removed");
        else
                error("remove file fail");
        if (servinfo)
		freeaddrinfo(servinfo);
        shutdown(listener_d, SHUT_RDWR);
        puts("Goodbye!");
        exit(-1);

}


void append_str(char *writefile,char *writestr, bool *status) 
{
	if (status)
	{
		FILE *fp = fopen(writefile, "w");
		queue = 5;
	        if (fp == NULL)
	                error("Can't open file");

        	fputs(writestr, fp);
        	if (ferror(fp))
                	error("error writing file");
		
		fclose(fp);
	}
	else
	{
		FILE * fp = fopen(writefile, "a");
		queue--;
		if (fp == NULL)
			error("Can't open file");

		fputs(writestr, fp);
		if (ferror(fp))
			error("error writing file");
		fclose(fp);
	}
}


int read_in(int socket,int fp, char *writefile)
{
        char *s =       malloc(BUF_SIZE);
        memset(s,0,BUF_SIZE);
	int slen =      sizeof(s);
	int c = recv(socket, s, slen, 0);
	s[c - 1] = '\n';
	printf("%s",s);
        ssize_t nr = write(fp,s,strlen(s));
        if (nr == -1) 
                error("unable to write to file");
        
        int fsync(int fp);

        printf("%s",s);
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
	printf("Signal Caught: %d\n",sig);
	if (!connected)
	{

        	syslog(LOG_INFO, "Caught signal, exiting\n");
        	if (!file_buf_free)
                	free(file_buf);
        	if (!buf_free)
                	free(buf);
	        if (connect_d)
	                close(connect_d);

		if (remove(writefile) == 0)
        	        puts("file removed");
        	else
                	error("remove file fail");	

	
		freeaddrinfo(servinfo);
		shutdown(listener_d, SHUT_RDWR);
		puts("Goodbye!");
//        	exit(sig);
	}
	else
		term = true;
	exit(sig);
}
