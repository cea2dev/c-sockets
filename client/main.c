#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MYPORT "3490"
#define BACKLOG 10
#define MAXDATASIZE 100

void sigchld_hander(int s)
{
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}

void* get_in_addr(struct sockaddr* sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*) sa)->sin_addr);
    } else {
        return &(((struct sockaddr_in6*) sa)->sin6_addr);
    }
}

int main(int argc, char* argv[])
{
    int sockfd, numBytes;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *ptr;
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    struct sigaction sig_action;
    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    if (argc != 2) {
        fprintf(stderr, "usage: client hostname\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    for (ptr = servinfo; ptr != NULL; ptr = ptr->ai_next) {
        if ((sockfd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, ptr->ai_addr, ptr->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    inet_ntop(ptr->ai_family, get_in_addr((struct sockaddr* ) ptr->ai_addr), s, sizeof s);
    
    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo);

    if ((numBytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1) {
        perror("recv");
        exit(1);
    }

    if (ptr == NULL) {
        fprintf(stderr, "client: failed to connect");
        exit(1);
    }

    buf[numBytes] = '\0';

    printf("client: received '%s'\n",buf);
    
    close(sockfd);

    return 0;
}