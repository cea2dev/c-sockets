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

void sigchld_handler(int s)
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

int main(void)
{
    int sockfd, new_fd;
    struct addrinfo hints, *servinfo, *ptr;
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    struct sigaction sig_action;
    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    int rv;

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
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, ptr->ai_addr, ptr->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo);

    if (ptr == NULL) {
        fprintf(stderr, "server: failed to find");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sig_action.sa_handler = sigchld_handler;
    sigemptyset(&sig_action.sa_mask);
    sig_action.sa_flags = SA_RESTART;

    if (sigaction(SIGCHLD, &sig_action, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections...\n");

    while(1) {
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr*)&their_addr, &sin_size);

        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family, 
            get_in_addr((struct sockaddr*)&their_addr),
            s, sizeof s);
        
        printf("server: got connection from %s\n", s);

        if (!fork()) {
            close(sockfd);

            if (send(new_fd, "Hello, world!\n", 14, 0) == -1) {
                perror("send");
            }

            close(new_fd);
            exit(0);
        }

        close(new_fd);
    }

    return 0;
}