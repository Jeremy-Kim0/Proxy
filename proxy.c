/*
 * proxy.c - A Simple Sequential Web proxy
 *
 * Course Name: 14:332:456-Network Centric Programming
 * Assignment 2
 * Student Name: Jeremy Kim
 * 
 * IMPORTANT: Give a high level description of your code here. You
 * must also provide a header comment at the beginning of each
 * function that describes what that function does.
 *This program is the main program. The first two requirements are completed here and the  next two are on the file client.c. proxy.c works by creating a connection with a client, in this case Mozilla firefox which sends a request. Information such as URL and IP are exracted from the request. I kept client.c and proxy.c because I was having problems with writing to the server. 
 */ 

#include "csapp.h"
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

/*
 * Function prototypes
 */
char* extractURL(char buffer[]);
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr, char *uri, int size);
/* 
 * main - Main routine for the proxy program 
 */
int main(int argc, char **argv)
{
    /* Check arguments */
    if (argc != 2) {
	fprintf(stderr, "Usage: %s <port number>\n", argv[0]);
	exit(0);
    }
    
    int server_fd;
    int new_socket;
    int valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[1024]= {0};
    char log_entry[MAXLINE];
    char* uri;
   
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    //initalizing the structures
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl (INADDR_ANY);
    address.sin_port = htons(  PORT );
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 1024) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    
    //the proxy reads the request from the client
    valread = read( new_socket , buffer, 1024);
    printf("The request from the client: ");
    printf("%s\n",buffer );
    char* URL=extractURL(buffer);
    
    //takes a log entry of the request
    //
    format_log_entry(log_entry, &address, URL, 0);
    
    printf("IP: %s\n", inet_ntoa(address.sin_addr)); 
    printf("URL: %s\n", URL); 
    
    exit(0);
}


/*
 * format_log_entry - Create a formatted log entry in logstring. 
 * 
 * The inputs are the socket address of the requesting client
 * (sockaddr), the URI from the request (uri), and the size in bytes
 * of the response from the server (size).
 */
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
//this function returns the URL from the HTTP request to be passed into entry_log
char* extractURL(char buffer[]) {
    int startI;
    int endI;

    for(int j=0; j<1024; j++) {
	char c=buffer[j];
	if(c=='h') {
		startI=j;
		break;
	} 
		
    }
    startI++;
    endI=startI;

    int z;
    for(z=endI; z<1024; z++) {
        char c=buffer[z];
        if((isspace(c))!=0) {
        	break;
        }
    }
    z++;
    int c=0;
    char *str = malloc(z-startI);
    while (c < (z-startI)) {
      str[c] = buffer[startI+c-1];
      c++;
    }
    str[c] = '\0';
    return str;
}

