#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <netdb.h>
#define PORT 12346
  
char* extractURL(char buffer[]);
char* request(char buffer[]);
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr, char *uri, int size);
  
//This file is for the proxy accepting the client request and sending it to the server
//This file is not fully done, more information below

int main(int argc, char const *argv[])
{
    if (argc != 2) {
	fprintf(stderr, "Usage: %s <port number>\n", argv[0]);
	exit(0);
    }
    
    //this is creating a connection between client and proxy
    int client_fd;
    int new_socket;
    int valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[1024]= {0};

    // creating the socket for the browser(client)
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl (INADDR_ANY);
    address.sin_port = htons( PORT );
    
    if (bind(client_fd, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(client_fd, 1024) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((new_socket = accept(client_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    //Request from client
    valread = read( new_socket , buffer, 1024);

    // for log_entry;
    char* URL=extractURL(buffer);
    //logs the client request
    format_log_entry(log_entry, &address, URL, 0);
    
    
    // connecting to the server
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char bufferS[1024]={0};
    int sockfd, bytes, sent, received, total;
     sockfd = socket(AF_INET, SOCK_STREAM, 0);

    //sendMessage contains the GET request
    char* sendMessage=request(buffer);
    /* lookup the ip address */
    
    //host is hardcoded to google for testing sake
    char *host="google.com";    
    server = gethostbyname(host);

    /* fill in the structure */
    memset(&serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    memcpy(&serv_addr.sin_addr.s_addr,server->h_addr,server->h_length);

    /* connect the socket */
    connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr));
    
    //writes to the server but the browser keeps loading with this command
    write(sockfd , sendMessage , strlen(sendMessage));
    printf("Request message sent\n");
    
    //bufferS should contain the response from the server
    valread = read(sockfd , bufferS, 1024);
    printf("%s\n", bufferS );
    
    printf("%d", "The number of bytes read: ", valread);
    close(sockfd);
    
    //this code would send the response back to the client
    printf("Server response: sent");
    write(new_socket , bufferS , 1024);
    
    
    
    exit(0);
}

//this function returns the GET request that the client sent
//the format of the request may be causing the problem of infinitely loading
char* request(char buffer[]) {
    int spaceC=0;
    int z;
    for(z=0; z<1024; z++) {
        char c=buffer[z];
        if((isspace(c))!=0) {
                spaceC++;
        	if(spaceC==3) {
        		break;
        	}
        }
    }
    int c=0;
    char *str = malloc(z);
    while (c < (z)) {
      str[c] = buffer[c];
      c++;
    }
    str[c] = '\0';
    return str;
}
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr, 
		      char *uri, int size)
{
    time_t now;
    char time_str[MAXLINE];
    unsigned long host;
    unsigned char a, b, c, d;

    /* Get a formatted time string */
    now = time(NULL);
    strftime(time_str, MAXLINE, "%a %d %b %Y %H:%M:%S %Z", localtime(&now));

    /* 
     * Convert the IP address in network byte order to dotted decimal
     * form. Note that we could have used inet_ntoa, but chose not to
     * because inet_ntoa is a Class 3 thread unsafe function that
     * returns a pointer to a static variable (Ch 13, CS:APP).
     */
    host = ntohl(sockaddr->sin_addr.s_addr);
    a = host >> 24;
    b = (host >> 16) & 0xff;
    c = (host >> 8) & 0xff;
    d = host & 0xff;


    /* Return the formatted log entry string */
    sprintf(logstring, "%s: %d.%d.%d.%d %s", time_str, a, b, c, d, uri);
}
