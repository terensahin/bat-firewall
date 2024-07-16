#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <errno.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>

#define BUF_SIZE 100
#define SV_SOCK_PATH "/tmp/platform"

int main(int argc, char *argv[])
{
    struct sockaddr_un addr;
    int sfd;
    ssize_t numRead;
    char buf[BUF_SIZE];

    /* Create client socket */
    sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sfd == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    /* Using memset() to zero out the entire structure, rather than initializing individual fields,
    ensures that any nonstandard fields that are provided by some implementations are also initialized to 0.*/
    memset(&addr, 0, sizeof(struct sockaddr_un));

    /* Socket type is set to AF_UNIX */
    addr.sun_family = AF_UNIX;
    /* Construct server address*/
    strncpy(addr.sun_path, SV_SOCK_PATH, sizeof(addr.sun_path) - 1);
    /* Connect to server*/
    if (connect(sfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1)
    {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    /* Copy stdin to socket */
    while ((numRead = read(STDIN_FILENO, buf, BUF_SIZE)) > 0)
    {
        int numWrite = write(sfd, buf, numRead);
        if (numWrite != numRead)
            exit(EXIT_FAILURE);
        // if (numWrite != 0)
        //     break;
    }

    if (numRead == -1)
    {
        perror("read");
        exit(EXIT_FAILURE);
    }

    /* Closes our socket; server sees EOF */
    exit(EXIT_SUCCESS);
}
