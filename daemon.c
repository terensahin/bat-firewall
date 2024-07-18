#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <errno.h>
#include <sys/un.h>
#include "c_vector.h"
#include "common.h"

static const char *LOG_FILE = "/home/baranbolo/Desktop/platform_i/daemon.log";
static const char *ERR_FILE = "home/baranbolo/Desktop/platform_i/error.log";
static const char *BACKUP_FILE = "home/baranbolo/Desktop/platform_i/backup.log";

#define SV_SOCK_PATH "/tmp/ud_ucase"
#define BUF_SIZE 1024
#define BACKLOG 5
#define MAXIMUM_MESSAGE_COUNT 10

static FILE *logfp;

int *vector;

ssize_t logMessage(const char *format)
{
    return fprintf(logfp, "%s", format); /* Writes to opened log file */
}

void logOpen(const char *logFilename, char* open_mode)
{
    mode_t m;

    m = umask(077);                  /* To recover old mask */
    logfp = fopen(logFilename, open_mode); /* File is opened with permissions 700 */
    umask(m);

    if (logfp == NULL) /* If opening the log fails */
        exit(EXIT_FAILURE);

    setbuf(logfp, NULL); /* Disable stdio buffering */
}

void logClose(void)
{
    fclose(logfp);
}

void backup_shutdown(){
    logOpen(BACKUP_FILE, "w");
    char buf[16];
    for(int i = 0; i < vector_get_size(vector); i++){
        if(i == vector_get_size(vector) - 1)
            snprintf(buf, sizeof(int)+1, "%d", *((int *)vector_at(vector, i)));
        else
            snprintf(buf, sizeof(int)+1, "%d,", *((int *)vector_at(vector, i)));
        logMessage(buf);
    }
    logClose();
}

void backup_start(){
    vector = vector_initialize(vector, sizeof(student), NULL);
    FILE *file = fopen(BACKUP_FILE, "a+");
    if (file == NULL) {
        return;
    }

    char buf[128];
    size_t items_read = fread(buf, 1, sizeof(buf), file);

    char str[20];
    snprintf(str, 20, "%ld\n", items_read);

    char *token;
    token = strtok(buf, ",");
    while (token != NULL){
        int number = atoi(token);
        vector = vector_push_back(vector, &number);
        token = strtok(NULL, ",");
    }
    fclose(file);
    return;
}

static void skeleton_daemon()
{
    pid_t pid; /* Used pid_t for portability */

    pid = fork(); /* This fork is to leave from process group */

    if (pid < 0) /* fork error */
        exit(EXIT_FAILURE);

    if (pid > 0)
    { /* Parent exits */
        exit(EXIT_SUCCESS);
    }

    /*child continues*/
    if (setsid() < 0) /* Child process becomes leader of his own session */
        exit(EXIT_FAILURE);

    struct sigaction sa; /* Ignore SIGCHLD and SIGHUP signals */
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = SIG_IGN;
    if (sigaction(SIGHUP, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    sigemptyset(&sa.sa_mask); /* Set handler for SIGTERM signal (backup on shutdown) */
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = backup_shutdown;
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    pid = fork(); /* This fork is to prevent access to the terminal */

    if (pid < 0) /* fork error*/
        exit(EXIT_FAILURE);

    if (pid > 0) /* Parent exits */
        exit(EXIT_SUCCESS);

    /* Grandchild continues */

    umask(077); /* Newly opened files will have permission 700
                 so that only deamon process can rwe them */

    chdir("/"); /* Changing working directory to be able to unmount the initial directory */

    int maxfd = sysconf(_SC_OPEN_MAX); /* Get maximum possible file desciptor a process can have */
    if (maxfd == -1)                   /* If not defined, make a guess */
        maxfd = 8192;

    for (int i = maxfd - 1; i >= 0; i--) /* Closed all desciptors */
    {
        close(i);
    }

    int fd = open("/dev/null", O_RDWR); /* 0,1,2 file desciptors are pointed to null, so functions using stdin or stdout does not generate error */
    if (fd != STDIN_FILENO)             /* 'fd' should be 0 */
        exit(EXIT_FAILURE);
    if (dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO)
        exit(EXIT_FAILURE);
    if (dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO)
        exit(EXIT_FAILURE);
}

int create_socket()
{
    struct sockaddr_un socket_address;
    int socket_fd;

    socket_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (socket_fd == -1)
    {
        perror("Error creating socket!");
        exit(EXIT_FAILURE);
    }

    if (strlen(SV_SOCK_PATH) > sizeof(socket_address.sun_path) - 1)
        exit(EXIT_FAILURE);

    if (remove(SV_SOCK_PATH) == -1 && errno != ENOENT)
        exit(EXIT_FAILURE);

    memset(&socket_address, 0, sizeof(struct sockaddr_un)); /* Cleared the socket_address */
    socket_address.sun_family = AF_UNIX;                    /* UNIX domain address */
    strncpy(socket_address.sun_path, SV_SOCK_PATH, sizeof(socket_address.sun_path) - 1);

    if (bind(socket_fd, (struct sockaddr *)&socket_address, sizeof(struct sockaddr_un)) == -1)
    {
        if (errno == EADDRINUSE)
        {
            perror("socket in use");
            exit(EXIT_FAILURE);
        }
        perror("binding error ");

        exit(EXIT_FAILURE);
    }

    // change
    // if (listen(socket_fd, BACKLOG) == -1)
    //     exit(EXIT_FAILURE);

    return socket_fd;
}

void execute_command(daemon_command command, char* response, ssize_t *command_len){
    switch (command.command_type)
    {
    case add:
        student tmpstd = command.student_info;
        vector = vector_push_back(vector, &tmpstd);
        snprintf(response, BUF_SIZE, "Successfully added student: Name: %s, ID: %d, Grade: %f\n" ,\
            command.student_info.name, command.student_info.id, command.student_info.grade);
        break;
    case del:
        int delete_id = command.student_info.id;
        for(int i = vector_get_size(vector) - 1; i >= 0; i--){
            student *tmpstd = vector_at(vector, i);
            if(tmpstd->id == delete_id){
                vector_erase(vector, i);
            }
        }
        snprintf(response, BUF_SIZE, "Successfully deleted student with id %d\n", delete_id);
        break;
    case show:
        snprintf(response, BUF_SIZE, "All students:\n" );
        for(int i = 0; i < vector_get_size(vector); i++){
            student *tmpstd = vector_at(vector, i);
            char tmpbuf[128];
            snprintf(tmpbuf, 128, "Name: %s, ID: %d, Grade: %f\n", tmpstd->name, tmpstd->id, tmpstd->grade);
            strcat(response, tmpbuf);
        }
        break;
    default:
        snprintf(response, BUF_SIZE, "Unknown command\n" );
        break;
    }
    *command_len = strlen(response);
    return;
}

int main(int argc, char *argv[])
{
    skeleton_daemon();

    logOpen(LOG_FILE, "a");

    backup_start();

    int socket_fd = create_socket();

    struct sockaddr_un claddr;
    socklen_t len = sizeof(struct sockaddr_un);
    ssize_t command_bytes;
    daemon_command command;
    char response[BUF_SIZE];
    while (1) {
        if (recvfrom(socket_fd, &command, sizeof(daemon_command), 0, (struct sockaddr *) &claddr, &len) == -1)
            exit(EXIT_FAILURE);

        execute_command(command, response, &command_bytes);

        if (sendto(socket_fd, response, command_bytes, 0, (struct sockaddr *) &claddr, len) != command_bytes){
            exit(EXIT_FAILURE);
        }
    }
    return EXIT_SUCCESS;
}
