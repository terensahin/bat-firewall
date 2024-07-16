#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <errno.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>

#define BUF_SIZE 100
#define SV_SOCK_PATH "/tmp/platform"


void usage(){
    printf("boyle kullanilacak\n");
}

int main(int argc, char *argv[])
{

    if(argc < 2) usage();


    int is_command_received = 0;
    char buf[BUF_SIZE];
    char commandbuf[BUF_SIZE];
    int index = 0;

 
    for (int i = 1; i < argc; i++){

        if(strcmp(argv[i], "--help") == 0){
            // TODO fill
            return 0;
        }  
        /* If -t command line argument is given, the vector is tested */
        if ((strcmp(argv[i], "-a") == 0) || (strcmp(argv[i], "-d") == 0) || (strcmp(argv[i], "-s") == 0)){
            if(is_command_received){
                printf("More than one commands are not allowed\n");
                return 1;
            }
            is_command_received = 1;
            snprintf(buf + index, sizeof(argv[i])+1, "%s,", argv[i]); 
            index += strlen(argv[i]) + 1;
        }
        else{
            snprintf(buf + index, sizeof(argv[i])+1, "%s,", argv[i]);
            index += strlen(argv[i]) + 1; 
        }
    }

    buf[index] = '\0';
    printf("%s\n", buf);
    fflush(stdout);


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

    /* Copy stdin to socket */
    // while ((numRead = read(STDIN_FILENO, buf, BUF_SIZE)) > 0)
    // {
        int numWrite = write(sfd, buf, strlen(buf));
        // if (numWrite != numRead) exit(EXIT_FAILURE);
        // if (numWrite != 0)
        //     break;
    // }

    if (numRead == -1)
    {
        perror("read");
        exit(EXIT_FAILURE);
    }

    /* Closes our socket; server sees EOF */
    exit(EXIT_SUCCESS);
}
