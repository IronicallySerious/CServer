#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<arpa/inet.h>
#include<string.h>
#include<sys/stat.h>
#include<fcntl.h>

#define PACKET 1024

char ROOT[1000];
char webpage[] =
"HTTP/1.1 200 OK\r\n"
"Content-Type: text/html; charset=UTF-8\r\n\r\n"
"<!DOCTYPE html>\r\n";

struct request
{
	char path[1000];
	char type[5];
};
struct request request_parser (char * req) {
	struct request parsed_request;
	int len;
	strcpy(parsed_request.path, ROOT);
	strcpy(parsed_request.type, strtok(req," \t"));
	strcat(parsed_request.path, strtok(NULL," \t"));
	len = strlen(parsed_request.path);
	if(parsed_request.path[len-1] == '/') {
		strcat(parsed_request.path, "index.html");
	}
	return parsed_request;
}
int listener,client;
void request_handler ( int client) {
	char req[10000],data[PACKET];
	int file,bytes;
	memset((void*) req, (int)'\0', 10000);
	recv(client, req, 10000, 0);
	struct request parsed_request = request_parser(req);
	printf("%s\n", parsed_request.path);
	if ((file=open(parsed_request.path, O_RDONLY))!=-1)
	{
		send(client, "HTTP/1.0 200 OK\n\n", 17, 0);
		while ((bytes = read(file, data, PACKET)) > 0 )
			write (client, data, bytes);
	}
	else
		write(client, "HTTP/1.0 404 Not Found\n", 23);
	close(client);
}

int main()
{
    struct sockaddr_in server_addr, client_addr;
    socklen_t sin_len = sizeof(client_addr);
    int fd_server, fd_client;
    char buf[2048];
    int on =1;
    getcwd(ROOT, sizeof(ROOT));
    printf("Current Public Directory is : %s\n", ROOT);
    fd_server = socket(AF_INET, SOCK_STREAM, 0);
    if(fd_server<0)
    {
        perror("socket");
        exit(1);
    }

    setsockopt(fd_server, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8000);

    if(bind(fd_server, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1)
    {
        perror("bind");
        close(fd_server);
        exit(1);
    }

    if(listen(fd_server, 10) == -1)
    {
        perror("listen");
        close(fd_server);
        exit(1);
    }

    while(1)
    {
        fd_client = accept(fd_server, (struct sockaddr *) &client_addr, &sin_len);

        if(fd_client == -1)
        {
            perror("Connection failed...\n");
            continue;
        }

        printf("Connection established\n");

        if(!fork())
        {
            /* Child Process */
            close(fd_server);
            memset(buf, 0, 2047);
            read(fd_client, buf, 2047);
            char data[PACKET];

            int file,bytes;
            struct request parsed_request = request_parser(buf);
            printf("%s\n", parsed_request.path);
            if ((file=open(parsed_request.path, O_RDONLY))!=-1)
            {
                send(fd_client, webpage, sizeof(webpage), 0);
                while ((bytes = read(file, data, PACKET)) > 0 )
                {
                    write(fd_client, data, bytes);
                }
            }

            close(fd_client);
            printf("Closing...\n");
            exit(0);
        }
        /* Parent Process */
        close(fd_client);
    }
    return 0;
}