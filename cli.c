#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <errno.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>

#define BUF_SIZE 100
#define SV_SOCK_PATH "/tmp/platform"

void shift_buffer(char* buf, int buf_size, int start_index, int shift_amount){
    for (int i = buf_size; i >= start_index; i--) {
        buf[i + shift_amount] = buf[i];
    }
}

void help_usage(){
    printf("boyle kullanilacak\n");
}

int main(int argc, char *argv[])
{

    if(argc < 2) help_usage();

    int is_command_received = 0;
    char buf[BUF_SIZE];
    char commandbuf[BUF_SIZE];
    int write_index = 0;
    int commands_last_index = 0;

    for (int i = 1; i < argc; i++){
        if(strcmp(argv[i], "--help") == 0){
            help_usage();
            return 0;
        }  
        if (strncmp(argv[i], "-", 1) == 0){
            for(int j = 0; j < strlen(argv[i] + 1); j++){
                if ((strncmp(argv[i] + j + 1, "a", 1) == 0) || (strncmp(argv[i] + j + 1, "d", 1) == 0) || (strncmp(argv[i] + j + 1, "s", 1) == 0)){
                    if(is_command_received){
                        printf("More than one commands are not allowed\n");
                        return 1;
                    }
                    is_command_received = 1;

                    shift_buffer(buf, BUF_SIZE, commands_last_index, 3);

                    buf[commands_last_index++] = '-';
                    memcpy(buf + commands_last_index++, argv[i] + j + 1, 1);
                    buf[commands_last_index++] = ',';

                    write_index += 3;
                } else {
                    printf("There is no such a command as %s (%.*s is not a valid command), --help for help\n", argv[i], 1, argv[i] + j + 1);
                    return 1;
                }
            }
        }
        else{
            snprintf(buf + write_index, sizeof(argv[i])+1, "%s,", argv[i]);
            write_index += strlen(argv[i]) + 1; 
        }
        printf("for: %s\n", buf);
    }
    buf[write_index] = '\0';

    struct sockaddr_un addr;
    int sfd;
    ssize_t numRead;

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

    int numWrite = write(sfd, buf, strlen(buf));

    if (numWrite != write_index){
        perror("error writing");
        exit(EXIT_FAILURE);
    }
    /* Closes our socket; server sees EOF */
    exit(EXIT_SUCCESS);
}
