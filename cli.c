#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <errno.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>
#include "common.h"

#define BUF_SIZE 1024
#define SV_SOCK_PATH "/tmp/ud_ucase"

void help_usage(){
    printf("Usage: cli [OPTIONS] [ARGUMENTS]\n");
    printf("OPTIONS:\n");
    printf("\t-a: Add student\n");
    printf("\t-d: Delete with ID\n");
    printf("\t-s: Search\n");
    printf("ARGUMENTS:\n");
    printf("\t[ARGUMENTS]: Arguments to be sent to the server\n");
    printf("EXAMPLE USAGE:\n");
    printf("\tcli -a \"george 2521 3.75\"\n");
    printf("\tcli -d 2521\n");
    printf("\tcli -s\n");
    exit(0);
}

int send_command_to_daemon(daemon_command command, struct sockaddr_un *claddr){
    struct sockaddr_un svaddr;
    int socket_fd;

    /* Create client socket */
    socket_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (socket_fd == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    /* Using memset() to zero out the entire structure, rather than initializing individual fields,
    ensures that any nonstandard fields that are provided by some implementations are also initialized to 0.*/
    memset(claddr, 0, sizeof(struct sockaddr_un));

    /* Socket type is set to AF_UNIX */
    (*claddr).sun_family = AF_UNIX;
    /* Construct server address*/
    snprintf((*claddr).sun_path, sizeof((*claddr).sun_path), "/tmp/ud_ucase_cl.%ld", (long) getpid());
    /* Connect to server*/
    if (bind(socket_fd, (struct sockaddr *) claddr, sizeof(struct sockaddr_un)) == -1)
        exit(EXIT_FAILURE);

    memset(&svaddr, 0, sizeof(struct sockaddr_un));
    svaddr.sun_family = AF_UNIX;
    strncpy(svaddr.sun_path, SV_SOCK_PATH, sizeof(svaddr.sun_path) - 1);

    if (sendto(socket_fd, &command, sizeof(daemon_command), 0, (struct sockaddr *) &svaddr, sizeof(struct sockaddr_un))\
         != sizeof(daemon_command))
        exit(EXIT_FAILURE);

    return socket_fd;
}

void wait_response(int socket_fd, struct sockaddr_un claddr){
    ssize_t read_bytes;
    char resp[BUF_SIZE];
    read_bytes = recvfrom(socket_fd, resp, BUF_SIZE, 0, NULL, NULL);
    if (read_bytes == -1)
        exit(EXIT_FAILURE);
    printf("Response: %.*s",(int) read_bytes, resp);

    remove(claddr.sun_path);            /* Remove client socket pathname */
    return;
}

int main(int argc, char *argv[])
{

    if(argc < 2) help_usage();

    daemon_command command;

    int opt;
    while ((opt = getopt(argc, argv, "a:d:sht")) != -1) {
        switch (opt) {
            case 'h':
                help_usage();
                break;
            case 'a':
                command.command_type = add;
                command.student_info = parse_student_info(optarg);
                printf("%s %d %f\n", command.student_info.name, command.student_info.id, command.student_info.grade);
                break;
            case 'd':
                command.command_type = del;
                command.student_info = (student){" ", atoi(optarg), 0};
                printf("%s %d %f\n", command.student_info.name, command.student_info.id, command.student_info.grade);
                break;
            case 's':
                command.command_type = show;
                command.student_info = (student){" ", 0, 0};
                break;
            case 't':
                command.command_type = terminate;
                command.student_info = (student){" ", 0, 0};
                break;
            case '?': // Case for unknown options or missing arguments
                help_usage();
                return EXIT_FAILURE;
            default:
                help_usage();
                return EXIT_FAILURE;
        }
    }

    struct sockaddr_un claddr;
    int socket_fd;

    socket_fd = send_command_to_daemon(command, &claddr);
    wait_response(socket_fd, claddr);

    exit(EXIT_SUCCESS);
}
