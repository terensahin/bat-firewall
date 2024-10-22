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
#include <sys/ioctl.h>

#include "airbatd.h"
#include "../c_vector.h"
#include "../common.h"
#include "log.h"
#include "../batdev.h"

FILE *logfp;
firewall_rule *vector;

/* Create or set the backup file when shutdown happens */
void backup_shutdown(){
    log_trace("shotdown back up started");
    FILE *file = fopen(BACKUP_FILE, "w");

    fprintf(file, "%d\n", vector_get_size(vector));
    for(int i = 0; i < vector_get_size(vector); i++){
        firewall_rule *tmprule = vector_at(vector, i);
        fprintf(file, "%s,%d,%s\n", tmprule->address, tmprule->port, tmprule->protocol);
    }
    vector_free(vector);
    log_trace("vector freed");
    fclose(file);
    log_trace("log file is closed");
    fclose(logfp);
}

/* Read the backup file and fill the vector */
void backup_start(){
    FILE *file = fopen(BACKUP_FILE, "a+");
    if (file == NULL) {
        log_error("back up file could not be opened");
        return;
    }

    int vector_size;
    if (fscanf(file, "%d\n", &vector_size) != 1) {
        fclose(file);
        log_warn("Backup is empty or scanning error");
        return;
    }

    int readed_number;
    firewall_rule tmprule;
    for(int i = 0; i < vector_size; i++){
        readed_number = fscanf(file, "%[^,],%d,%s\n", tmprule.address, &tmprule.port, tmprule.protocol);
        if(readed_number != 3){
            log_warn("Unexpected backup style");
            break;
        }
        vector = vector_push_back(vector, &tmprule);
    }

    ioctl_func();

    fclose(file);
    log_trace("vector backed up");
    return;
}

/* Function for the ioctl call */
int ioctl_set_msg(int file_desc){
    int ret_val;

    int vector_size = vector_get_size(vector);

    ret_val = ioctl(file_desc, IOCTL_SET_SIZE, &vector_size);

    if (ret_val < 0) {
        printf("ioctl_set_msg failed 1:%d\n", ret_val);
    }

    ret_val = ioctl(file_desc, IOCTL_SET_RULE, vector);

    if (ret_val < 0) {
        printf("ioctl_set_msg failed 2:%d\n", ret_val);
    }

    return ret_val;
}

/* Function that opens our module device, communicates with ioctl, then closes the device */
/* All rules are dumped to kernel module */
int ioctl_func(){
    int file_desc, ret_val;

    log_trace("Sending data to the kernel");

    file_desc = open(DEVICE_PATH, O_RDWR);
    if (file_desc < 0) {
        log_error("Can't open device file: %s, error:%d\n", DEVICE_PATH, file_desc);
        exit(EXIT_FAILURE);
    }
    ret_val = ioctl_set_msg(file_desc);
    if (ret_val){
        log_error("ioctl_set_msg failed:%d\n", ret_val);
        close(file_desc);
        exit(EXIT_FAILURE);
    }

    close(file_desc);
    return 0;

}

/* Handler function for shutdown */
void shutdown_handler(){
    backup_shutdown();
    exit(EXIT_FAILURE);
}

/* Set handlers for a SIGNAL */
void setup_signal_handler(int signal, void (*handler)(int)) {
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = handler;
    if (sigaction(signal, &sa, NULL) == -1) {
        perror("error sigaction");
        exit(EXIT_FAILURE);
    }
}

static void skeleton_daemon(){
    pid_t pid; /* Used pid_t for portability */

    pid = fork(); /* This fork is to leave from process group */

    if (pid < 0){  /* fork error */
        log_error("error in first fork");
        exit(EXIT_FAILURE);
    }

    if (pid > 0){ /* Parent exits */
        exit(EXIT_SUCCESS);
    }

    /*child continues*/
    if (setsid() < 0){ /* Child process becomes leader of his own session */
        log_error("error creating session");
        exit(EXIT_FAILURE);
    }

    setup_signal_handler(SIGHUP, SIG_IGN);
    setup_signal_handler(SIGCHLD, SIG_IGN);
    setup_signal_handler(SIGTERM, shutdown_handler);
    setup_signal_handler(SIGSEGV, shutdown_handler);
    setup_signal_handler(SIGABRT, shutdown_handler);

    pid = fork(); /* This fork is to prevent access to the terminal */

    if (pid < 0){  /* fork error */
        log_error("error in first fork");
        exit(EXIT_FAILURE);
    }

    if (pid > 0){ /* Parent exits */
        exit(EXIT_SUCCESS);
    }

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
    log_trace("file descriptors of daemon are closed");

    int fd = open("/dev/null", O_RDWR); /* 0,1,2 file desciptors are pointed to null, so functions using in or stdout does not generate error */
    if (fd != STDIN_FILENO)             /* 'fd' should be 0 */
        exit(EXIT_FAILURE);
    if (dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO)
        exit(EXIT_FAILURE);
    if (dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO)
        exit(EXIT_FAILURE);
    log_trace("in stdout and stderr is mapped to dev/null");
    log_trace("Daemon is successfully created");
    return;
}

int create_socket(){
    struct sockaddr_un socket_address;
    int socket_fd;

    socket_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (socket_fd == -1)
    {
        log_error("socket could not be created");
        exit(EXIT_FAILURE);
    }

    if (strlen(SV_SOCK_PATH) > sizeof(socket_address.sun_path) - 1){
        log_error("socket path name is too long");
        exit(EXIT_FAILURE);
    }

    if (remove(SV_SOCK_PATH) == -1 && errno != ENOENT){
        log_error("existing socket could not be removed");
        exit(EXIT_FAILURE);
    }
    log_trace("socket is created");

    memset(&socket_address, 0, sizeof(struct sockaddr_un)); /* Cleared the socket_address */
    socket_address.sun_family = AF_UNIX;                    /* UNIX domain address */
    strncpy(socket_address.sun_path, SV_SOCK_PATH, sizeof(socket_address.sun_path) - 1);

    if (bind(socket_fd, (struct sockaddr *)&socket_address, sizeof(struct sockaddr_un)) == -1)
    {
        if (errno == EADDRINUSE)
        {
            log_error("socket is in use");
            exit(EXIT_FAILURE);
        }
        log_error("bind error");
        exit(EXIT_FAILURE);
    }
    log_trace("socket is binded");
    return socket_fd;
}

void execute_command(daemon_command command, char* response, ssize_t *command_len){
    switch (command.command_type)
    {
    case add: { /* Add a rule to vector */
        firewall_rule tmprule = command.rule_info;
        vector = vector_push_back(vector, &tmprule);
        snprintf(response, BUF_SIZE, "Successfully added IP: Address: %s, Port: %d, Protocol: %s\n" ,\
            command.rule_info.address, command.rule_info.port, command.rule_info.protocol);
        ioctl_func(); /* All rules are dumped to kernel module */
        break;
    }

    case del: { /* Delete a rule from vector with given index */
        int delete_index = command.rule_info.port;
	    vector_erase(vector, delete_index);
        snprintf(response, BUF_SIZE, "Successfully deleted IP with index %d\n", delete_index);
        ioctl_func(); /* All rules are dumped to kernel module */
        break;
    }
    case show: { /* Dump all rules to user terminal */
        snprintf(response, BUF_SIZE, "All IPs:\n" );
        for(int i = 0; i < vector_get_size(vector); i++){
            firewall_rule *tmprule = vector_at(vector, i);
            char tmpbuf[BUF_SIZE];
            snprintf(tmpbuf, BUF_SIZE, "Index: %d, Address: %s, Port: %d, Protocol: %s\n", i, tmprule->address, tmprule->port, tmprule->protocol);
            strcat(response, tmpbuf);
        }
        break;
    }
    case terminate: {/* Terminate the deamon */
        snprintf(response, BUF_SIZE, "Terminating daemon\n" );
        backup_shutdown();
        break;
    }
    case chlog: { /* Change log levels */
        snprintf(response, BUF_SIZE, "Changed debug log\n" );
        log_set_level(command.log_level);
        break;
    }
    default: {
        snprintf(response, BUF_SIZE, "Unknown command\n" );
        break;
    }
    }


    *command_len = strlen(response);
    return;
}

FILE *set_log_levels(){
    log_set_level(0);
    log_set_quiet(1);
    FILE *fp;
    fp = fopen(LOG_FILE, "ab");
    if(fp == NULL){
        return NULL;
    }
    log_add_fp(fp, 0);

    log_trace("initialized log levels");
    return fp;
}


int main()
{
    skeleton_daemon();

    logfp = set_log_levels();
    if (logfp == NULL){
        exit(EXIT_FAILURE);
    }

    vector = vector_initialize(sizeof(firewall_rule), NULL);
    log_trace("Vector is initialized");
    backup_start();

    int socket_fd = create_socket();

    struct sockaddr_un claddr;
    socklen_t len = sizeof(struct sockaddr_un);
    ssize_t command_bytes;
    daemon_command command;
    char response[BUF_SIZE];
    while (1) {
        log_trace("waiting for command");
        if (recvfrom(socket_fd, &command, sizeof(daemon_command), 0, (struct sockaddr *) &claddr, &len) == -1){ /* Receive command from cli process */
            log_error("command could not be read properly");
            exit(EXIT_FAILURE);
        }

        execute_command(command, response, &command_bytes);

        if (sendto(socket_fd, response, command_bytes, 0, (struct sockaddr *) &claddr, len) != command_bytes){ /* Send response back to cli process */
            log_error("response could not be sent properly");
            exit(EXIT_FAILURE);
        }

        if(command.command_type == terminate){
            return EXIT_SUCCESS;
        }

        log_trace("response is sent");
    }
    return EXIT_SUCCESS;
}
