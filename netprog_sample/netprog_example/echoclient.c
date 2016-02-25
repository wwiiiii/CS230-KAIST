/*
 * echoclient.c - An echo client
 */
/* $begin echoclientmain */
#include "csapp.h"

int main(int argc, char **argv) 
{
    int clientfd;
    int temp;
    char *host, *port, buf[MAXLINE], buf2[MAXLINE];
    rio_t rio;

    if (argc != 3) {
	fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
	exit(0);
    }
    host = argv[1];
    port = argv[2];

    clientfd = Open_clientfd(host, port);
    Rio_readinitb(&rio, clientfd);

    while (Fgets(buf, MAXLINE, stdin) != NULL) {
    temp = strlen(buf);
    buf[temp] = '\r'; buf[temp+1] ='\n'; buf[temp+2] = '\0';
	Rio_writen(clientfd, buf, strlen(buf));
	Rio_readlineb(&rio, buf2, MAXLINE);
	Fputs(buf2, stdout);
    }
    Close(clientfd); //line:netp:echoclient:close
    exit(0);
}
/* $end echoclientmain */
