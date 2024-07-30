#ifndef COMMON_H
#define COMMON_H

typedef struct firewall_rule{
    char address[50];
    int port;
    char protocol[4];
}firewall_rule;

typedef enum command{
    add,
    del,
    show,
    terminate,
    chlog
}command;

typedef struct daemon_command{
    command command_type;
    firewall_rule rule_info;
    int log_level;
}daemon_command;

firewall_rule create_ip(char* address, int port, char* protocol);

daemon_command create_daemon_command(command command_type, char* address, int port, char* protocol);
void print_ip_info(firewall_rule firewall_rule);
firewall_rule parse_ip_info(char* input);

#endif
