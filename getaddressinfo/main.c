#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main(int argc, char* argv[])
{
    struct addrinfo hints, *result, *ptr;
    int status;
    char ipstr[INET6_ADDRSTRLEN];

    if (argc != 2) {
        fprintf(stderr, "usage: showip hostname\n");
        return 1;
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(argv[1], NULL, &hints, &result)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 2;
    }

    printf("IP addresses for %s:\n\n", argv[1]);

    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
        void* addr;
        char *ipver;

        if (ptr->ai_family == AF_INET) {
            struct sockaddr_in* ipv4 = (struct sockaddr_in*) ptr->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPv4";
        } else {
            struct sockaddr_in6* ipv6 = (struct sockaddr_in6*) ptr->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }

        inet_ntop(ptr->ai_family, addr, ipstr, sizeof ipstr);
        printf("    %s: %s\n", ipver, ipstr);
    }

    freeaddrinfo(result);

    return 0;
}