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
#include "c_dynamic_vector.h"

static const char *LOG_FILE = "/home/baranbolo/Desktop/platform_i/daemon.log";

#define SV_SOCK_PATH "/tmp/platform"
#define BUF_SIZE 100
#define BACKLOG 5

static FILE *logfp;

ssize_t logMessage(const char *format)
{
    return fprintf(logfp, "%s", format); /* Writes to opened log file */
}

void logOpen(const char *logFilename)
{
    mode_t m;

    m = umask(077);                  /* To recover old mask */
    logfp = fopen(logFilename, "a"); /* File is opened with permissions 700 */
    umask(m);

    if (logfp == NULL) /* If opening the log fails */
        exit(EXIT_FAILURE);

    setbuf(logfp, NULL); /* Disable stdio buffering */

}

void logClose(void)
{
    fclose(logfp);
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
    if (sigaction(SIGHUP, &sa, NULL) == -1)
        exit(EXIT_FAILURE);
    if (sigaction(SIGCHLD, &sa, NULL) == -1)
        exit(EXIT_FAILURE);

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

    socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
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

    if (listen(socket_fd, BACKLOG) == -1)
        exit(EXIT_FAILURE);

    return socket_fd;
}


int main(int argc, char *argv[])
{
    skeleton_daemon();

    int socket_fd = create_socket();

    int connection_fd;
    ssize_t numRead, numWrite;
    char buf[BUF_SIZE];

    int message_count = 0;
    while (1)
    {
        memset(buf, 0, sizeof(buf));
        connection_fd = accept(socket_fd, NULL, NULL);
        if (connection_fd == -1){
                logOpen(LOG_FILE);
                char str[20];
                snprintf(str, 20, "%d-%d", 1,1);
                logMessage(str);
                logClose();
            exit(EXIT_FAILURE);
        }


        while ((numRead = read(connection_fd, buf, BUF_SIZE)) > 0)
        {
            logOpen(LOG_FILE);
            numWrite = logMessage(buf);
            logClose();


            if(numRead != numWrite){
                logOpen(LOG_FILE);
                char str[20];
                snprintf(str, 20, "%ld-%ld\n", numWrite, numRead);
                logMessage(str);
                logClose();
                exit(EXIT_FAILURE);
            }
            message_count++;
            memset(buf, 0, sizeof(buf));
        }

        logOpen(LOG_FILE);
        char str[20];
        snprintf(str, 20, "%ld\n", numRead);
        logMessage(str);
        logClose();

        
        

        if (numRead == -1){
            perror("read");
            exit(EXIT_FAILURE);
        }
        if (close(connection_fd) == -1){
            perror("close");
            exit(EXIT_FAILURE);
        }

        if (message_count >= 5) exit(EXIT_SUCCESS);
    }
    return EXIT_SUCCESS;
}