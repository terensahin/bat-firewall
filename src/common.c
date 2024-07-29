#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

firewall_rule create_ip(char* address, int port, char* protocol){
    firewall_rule new_ip;
    strcpy(new_ip.address, address);
    new_ip.port = port;
    strcpy(new_ip.protocol, protocol);
    return new_ip;
}


daemon_command create_daemon_command(command command_type, char* address, int port, char* protocol){
    daemon_command new_daemon_command;
    new_daemon_command.command_type = command_type;
    new_daemon_command.rule_info = create_ip(address, port, protocol);
    return new_daemon_command;
}

firewall_rule parse_ip_info(char* input) {
    if (input == NULL) {
        fprintf(stderr, "Error: Input string is NULL\n");
        return create_ip("", -1, ""); // or an appropriate error value for your use case
    }

    char *address = strtok(input, " ");
    if (address == NULL) {
        fprintf(stderr, "Error: Failed to parse address\n");
        return create_ip("", -1, ""); // or an appropriate error value
    }

    char *port_str = strtok(NULL, " ");
    if (port_str == NULL) {
        fprintf(stderr, "Error: Failed to parse port\n");
        return create_ip("", -1, ""); // or an appropriate error value
    }
    int port = atoi(port_str);
    if (port <= 0) {
        fprintf(stderr, "Error: Invalid port number\n");
        return create_ip("", -1, ""); // or an appropriate error value
    }

    char *protocol = strtok(NULL, " ");
    if (protocol == NULL) {
        fprintf(stderr, "Error: Failed to parse protocol\n");
        return create_ip("", -1, ""); // or an appropriate error value
    }

    return create_ip(address, port, protocol);
}
