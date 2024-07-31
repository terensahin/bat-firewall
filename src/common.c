#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

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

int is_valid_ip(const char *ip) {
    struct sockaddr_in sa;
    return (inet_pton(AF_INET, ip, &(sa.sin_addr)) != 0);
}

firewall_rule parse_ip_info(char* input) {
    if (input == NULL) {
        fprintf(stderr, "Error: Input string is NULL\n");
        exit(EXIT_FAILURE);
    }

    char *address = strtok(input, " ");
    if (address == NULL) {
        fprintf(stderr, "Error: Failed to parse address\n");
        exit(EXIT_FAILURE);    }

    if (!is_valid_ip(address)) {
        fprintf(stderr, "Error: Invalid IP address\n");
        exit(EXIT_FAILURE);    }

    char *port_str = strtok(NULL, " ");
    if (port_str == NULL) {
        fprintf(stderr, "Error: Failed to parse port\n");
        exit(EXIT_FAILURE);    }
    int port = atoi(port_str);
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Error: Invalid port number\n");
        exit(EXIT_FAILURE);    }

    char *protocol = strtok(NULL, " ");
    if (protocol == NULL) {
        fprintf(stderr, "Error: Failed to parse protocol\n");
        exit(EXIT_FAILURE);    }

    if (strcmp(protocol, "TCP") != 0 && strcmp(protocol, "UDP") != 0 && strcmp(protocol, "tcp") != 0 && strcmp(protocol, "udp") != 0) {
        fprintf(stderr, "Error: Invalid protocol\n");
        exit(EXIT_FAILURE);    }

    return create_ip(address, port, protocol);
}
