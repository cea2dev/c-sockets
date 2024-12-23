#include <stdio.h>
#include <poll.h>

int main(void)
{
    struct pollfd pfds[1];

    pfds[0].fd = 0;
    pfds[0].events = POLLIN;

    printf("Hit RETURN or wait 2.5 seconds\n");

    int num_events = poll(pfds, 1, 2500);

    if (num_events == 0) {
        printf("Poll timed out\n");
    } else {
        if (pfds[0].revents & POLLIN) {
            printf("File descriptor %d is ready to read\n", pfds[0].fd);
        } else {
            printf("Unexpected event occurred: %d\n", pfds[0].revents);
        }
    }

    return 0;
}