/* 
 * proxy.c - CS:APP Web proxy
 *
 * TEAM MEMBERS:
 *     Tae-yeong Jeong, wwiiiii@kaist.ac.kr
 *  
 *  Concurrent Proxy - Using Threads for Parallelism
 *  Open listening socket for receiving connection from web browser once.
 *  
 *  When socket accepts query of client, make connection and then
 *  detach it from main thread.
 *
 *  Each task(child) thread has to close its own connection file descriptor
 *  and free pointer for passing arguments.
 *
 *  For thread safety, this program uses two semaphores;opencli and logsem.
 *  Former is for thread-safe use of open_clientfd function,
 *  latter is for thread-safe use of fprint function for proxy log.
 */ 

#include "csapp.h"
#include <string.h>

/*
 * Function prototypes and Global variables
 */

FILE * proxylog;
sem_t opencli;
sem_t logsem;

int parse_uri(char *uri, char *target_addr, char *path, int  *port);
int isGetMethod(char *str);
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr, char *uri, int size);
void *task(void * ptr);

void Pw(sem_t *sem);
void Vw(sem_t *sem);

ssize_t Rio_readlineb_w(rio_t *rp, void *usrbuf, size_t maxlen);
void Rio_writen_w(int fd, void *usrbuf, size_t n);
int Open_clientfd_ts(char *hostname, int port);

//struct for passing argument to task function
typedef struct data{
    pthread_t tid;
    int connfd;
    struct sockaddr_in sock;
} data;

//function for dealing with connection to web server
void *task(void * ptr)
{
    int connfd;
    struct sockaddr_in * sock;
    size_t n;
    int clientfd = -1;
    int port, contlen = 0;
    int logsize = 0, contsize = 0, isHeaderEnd = 0, isChunked;
    char bufServer[MAXLINE], hostname[MAXLINE], pathname[MAXLINE];
    char bufClient[MAXLINE], logtext[8000], urilog[8000];
    char query[MAXLINE];
    rio_t rioServer, rioClient;
    
    connfd = ((data *)ptr)->connfd;
    sock = &(((data *)ptr)->sock);
    
    if(pthread_detach(pthread_self()) != 0)
    {
        fprintf(stderr, "detach failed %dth thread\n", (int)((data *)ptr)->tid);
        if(clientfd>=0)close(clientfd);
        Close(connfd);
        Free(ptr);
        return NULL;
    }

    urilog[0] = '\0'; logtext[0] = '\0'; bufServer[0] = '\0';
    Rio_readinitb(&rioServer, connfd);//init browser
    if((n = Rio_readlineb_w(&rioServer, bufServer, MAXLINE))!=0)//read first line
    {
        if(n>1000) {Close(connfd); Free(ptr); return NULL;}
        if(n<0) {Close(connfd); Free(ptr); return NULL;}
        if(isGetMethod(bufServer))
        {
            if(strstr(bufServer,"http://") != NULL)
            {
                parse_uri(strstr(bufServer," ")+1,hostname,pathname,&port);
                printf("Try to open %s:%d...\n",hostname,port);
                clientfd = Open_clientfd(hostname, port);
                printf("Open with %s:%d with fd %d\n",hostname,port,clientfd);
                Rio_readinitb(&rioClient, clientfd);//init webserver
                strcpy(urilog, strstr(bufServer," ")+1);
                query[0] = '\0';
                strcat(query,"GET /");
                strcat(query, pathname);
            }
            else if(strstr(bufServer,"https://") != NULL)
            {
                parse_uri(strstr(bufServer," ")+1,hostname,pathname,&port);
                printf("Try to open %s:%d...\n",hostname,port);
                clientfd = Open_clientfd(hostname, port);
                printf("Open with %s:%d with fd %d\n",hostname,port,clientfd);
                Rio_readinitb(&rioClient, clientfd);//init webserver
                strcpy(urilog, strstr(bufServer," ")+1);
                query[0] = '\0';
                strcat(query,"GET /");
                strcat(query, pathname);
            }
            if(clientfd >= 0)
            {
                printf("bufServer(%d) : %s",(int)n,bufServer);
                n = strlen(query);
                printf("query is(%d) %s\n",(int)n,query);
                Rio_writen_w(clientfd, query, n);
                //Rio_writen_w(clientfd, bufServer, n);//send to webserver
            }
        }
        else
        {
            puts("Request is non-GET method...");
            Free(ptr);
            Close(connfd);
            return NULL;
        }
    }
    while((n = Rio_readlineb_w(&rioServer, bufServer, MAXLINE))!=0)//read from browser
    {
        if(n < 0) continue;
        if(clientfd >= 0)
        {
            printf("bufServer(%d)\n",(int)n);
            Rio_writen_w(clientfd, bufServer, n);//send to webserver
        }
        if(!strcmp(bufServer,"\r\n")){
            puts("end of request");
            break;
        }
    }
    if(clientfd >= 0)
    {    
        contlen = -1; contsize =0; isHeaderEnd = 0; logsize = 0, isChunked = 0;
        while((n = Rio_readlineb_w(&rioClient, bufClient, MAXLINE))!=0)//read from webserver
        {
            if(n<0) continue;
            Rio_writen_w(connfd, bufClient, n);//send to browser
            if(!strncasecmp(bufClient,"Content-Length: ",16))
            {
                contlen = atoi(bufClient+16);
            }
            if(!strncasecmp(bufClient,"Transfer-Encoding : chunked",27))
            {
                isChunked = 1;
            }
            if(isHeaderEnd == 1) contsize += n;
            if(!strcmp(bufClient,"\r\n")) isHeaderEnd = 1;
            logsize += n;
            if(isChunked == 1){
                if(!strcmp(bufClient,"0\r\n")) break;
            }
            if((contlen != -1) &&  (contlen <= contsize))
            {
                puts("Connection Closed since content all received!");
                break;
            }
            
        }
    }
    if(strstr(urilog," ")!=NULL)*strstr(urilog," ") = '\0';
    format_log_entry(logtext, sock, urilog, logsize);
    if(logsize!=0){
        Pw(&logsem);
        fprintf(proxylog, logtext);
        fflush(proxylog);
        Vw(&logsem);
    }
    if(clientfd>=0)close(clientfd);
    Close(connfd);
    Free(ptr);
    return NULL;
}
/* 
 * main - Main routine for the proxy program 
 */
int main(int argc, char **argv)
{
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    char client_hostname[MAXLINE], client_port[MAXLINE];
    Signal(SIGPIPE, SIG_IGN);
    /* Check arguments */
    
    if (argc != 2) {
	fprintf(stderr, "Usage: %s <port number>\n", argv[0]);
	exit(0);
    }
    
    if((proxylog = fopen("proxy.log","w")) == NULL)
    {
        fprintf(stderr, "Fail to open log file\n");
        exit(0);
    }
    
    sem_init(&opencli,0, 1);
    sem_init(&logsem, 0, 1);

    listenfd = Open_listenfd(atoi(argv[1]));
    
    while(1)
    {
        data * taskdata;
        clientlen = sizeof(struct sockaddr_storage);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        getnameinfo((SA*)&clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0);
        taskdata = Malloc(sizeof(data));
        
        taskdata->connfd = connfd;
        memcpy(&(taskdata->sock), (struct sockaddr_in *)&clientaddr, sizeof(struct sockaddr_in));

        printf("Connected to (%s, %s)\n", client_hostname, client_port);

        pthread_create(&(taskdata->tid),NULL,task,taskdata);
    }
    
    Close(listenfd);
    fclose(proxylog);
    exit(0);
}


/*
 * parse_uri - URI parser
 * 
 * Given a URI from an HTTP proxy GET request (i.e., a URL), extract
 * the host name, path name, and port.  The memory for hostname and
 * pathname must already be allocated and should be at least MAXLINE
 * bytes. Return -1 if there are any problems.
 */
int parse_uri(char *uri, char *hostname, char *pathname, int *port)
{
    char *hostbegin;
    char *hostend;
    char *pathbegin;
    int len;

    if (strncasecmp(uri, "http://", 7) != 0) {
	hostname[0] = '\0';
	return -1;
    }
       
    /* Extract the host name */
    hostbegin = uri + 7;
    hostend = strpbrk(hostbegin, " :/\r\n\0");
    len = hostend - hostbegin;
    strncpy(hostname, hostbegin, len);
    hostname[len] = '\0';
    
    /* Extract the port number */
    *port = 80; /* default */
    if (*hostend == ':')   
	*port = atoi(hostend + 1);
    
    /* Extract the path */
    pathbegin = strchr(hostbegin, '/');
    if (pathbegin == NULL) {
	pathname[0] = '\0';
    }
    else {
	pathbegin++;	
	strcpy(pathname, pathbegin);
    }

    return 0;
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
    sprintf(logstring, "%s: %d.%d.%d.%d %s %d\n", time_str, a, b, c, d, uri, size);
}

/*
 *  Decide whether query is about GET method
 */

int isGetMethod(char *str)
{
    if(strncasecmp(str, "GET ", 4) == 0)
    {
        return 1;
    }
    return 0;
}

//Non-terminating version of wrapper of rio_readlineb
ssize_t Rio_readlineb_w(rio_t *rp, void*usrbuf, size_t maxlen)
{
    ssize_t rc;
    if((rc = rio_readlineb(rp, usrbuf, maxlen)) < 0)
    {
        fprintf(stderr, "%s: %s","Rio_readlineb_w error",strerror(errno));
    } 
    return rc;
}

//Non-terminating version of wrapper of rio_writen
void Rio_writen_w(int fd, void *usrbuf, size_t n)
{
    if (rio_writen(fd, usrbuf, n) != n)
        fprintf(stderr, "%s: %s","Rio_writen_w error",strerror(errno));
}

//Thread-safe version of open_clientfd
int Open_clientfd_ts(char *hostname, int port)
{
    int ret;
    Pw(&opencli);
    ret = Open_clientfd(hostname, port);
    Vw(&opencli);
    return ret;
}

//Non-terminating wrapper of sem_wait
void Pw(sem_t *sem)
{
    if(sem_wait(sem) <0)
        fprintf(stderr,"P error");
}

//Non-terminating wrapper of sem_post
void Vw(sem_t *sem)
{
    if(sem_post(sem) <0)
        fprintf(stderr,"V error");
}
