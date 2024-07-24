#ifndef COMMON_H
#define COMMON_H

typedef struct ip{
    char address[100];
    int port;
    char protocol[4];
}ip;

typedef enum command{
    add,
    del,
    show,
    terminate,
    chlog
}command;

typedef struct daemon_command{
    command command_type;
    ip ip_info;
    int log_level;
}daemon_command;

ip create_ip(char* address, int port, char* protocol);

daemon_command create_daemon_command(command command_type, char* address, int port, char* protocol);
void print_ip_info(ip ip);
ip parse_ip_info(char* input);

#endif
