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
#include "student_vector.h"

static const char *LOG_FILE = "/home/baranbolo/Desktop/airties_staj/daemon_processes/daemon.log";

static FILE *logfp;   

void logMessage(const char *format, ...)
{
    fprintf(logfp, "%s\n", format); /* Writes to opened log file */
}

void logOpen(const char *logFilename)
{
    mode_t m;

    m = umask(077);  /* To recover old mask */
    logfp = fopen(logFilename, "a");  /* File is opened with permissions 700 */
    umask(m);

    if (logfp == NULL)   /* If opening the log fails */
        exit(EXIT_FAILURE);

    setbuf(logfp, NULL);   /* Disable stdio buffering */

    logMessage("Opened log file");
}

void logClose(void)
{
    logMessage("Closing log file");
    fclose(logfp); 
}

static void skeleton_daemon()
{
    pid_t pid;  /* Used pid_t for portability */

    pid = fork(); /* This fork is to leave from process group */

    if (pid < 0)   /* fork error */
        exit(EXIT_FAILURE);

    if (pid > 0){   /* Parent exits */
        exit(EXIT_SUCCESS);
    }

    /*child continues*/
    if (setsid() < 0) /* Child process becomes leader of his own session */
        exit(EXIT_FAILURE);

    struct sigaction sa;  /* Ignore SIGCHLD and SIGHUP signals */
    sigemptyset(&sa.sa_mask); 
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = SIG_IGN;
    if (sigaction(SIGHUP, &sa, NULL) == -1)
        exit(EXIT_FAILURE);
    if (sigaction(SIGCHLD, &sa, NULL) == -1)
        exit(EXIT_FAILURE);

    pid = fork();   /* This fork is to prevent access to the terminal */

    if (pid < 0)    /* fork error*/
        exit(EXIT_FAILURE);

    if (pid > 0)    /* Parent exits */
        exit(EXIT_SUCCESS);

    /* Grandchild continues */

    umask(077);    /* Newly opened files will have permission 700
                    so that only deamon process can rwe them */

    chdir("/");    /* Changing working directory to be able to unmount the initial directory */

    int maxfd = sysconf(_SC_OPEN_MAX);  /* Get maximum possible file desciptor a process can have */
    if (maxfd == -1)    /* If not defined, make a guess */
        maxfd = 8192;  

    for (int i = maxfd - 1; i>=0; i--)   /* Closed all desciptors */
    {
        close(i);
    }

    int fd = open("/dev/null", O_RDWR);  /* 0,1,2 file desciptors are pointed to null, so functions using stdin or stdout does not generate error */
    if (fd != STDIN_FILENO)         /* 'fd' should be 0 */
        exit(EXIT_FAILURE);
    if (dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO)
        exit(EXIT_FAILURE);
    if (dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO)
        exit(EXIT_FAILURE);
}

int main(int argc, char* argv[])
{
    for(int i = 0; i < argc; i++){  /* If -t command line argument is given, the vector is tested */
        printf("%s", argv[i]);
        if(strcmp(argv[i], "-t") == 0){
            printf(" activates test mode\n");
            test_vector(); /* student_vector.c */
        }
        else{
            printf("\n");
        }
    }

    skeleton_daemon();

    int i = 0;
    while (1)
    {
        logOpen(LOG_FILE);
        logMessage("BURDAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
        logClose();
        sleep (5);
        i++;
        if(i >= 5) break;
    }
    return EXIT_SUCCESS;
}