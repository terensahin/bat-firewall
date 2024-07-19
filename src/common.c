#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

student create_student(char* name, int id, float grade){
    student new_student;
    strcpy(new_student.name, name);
    new_student.id = id;
    new_student.grade = grade;
    return new_student;
}

void free_student(student student){
    return;
}

daemon_command create_daemon_command(command command_type, char* name, int id, float grade){
    daemon_command new_daemon_command;
    new_daemon_command.command_type = command_type;
    new_daemon_command.student_info = create_student(name, id, grade);
    return new_daemon_command;
}

void print_student_info(student student){
    printf("Name: %s, ID: %d, Grade: %f", student.name, student.id, student.grade);
}

student parse_student_info(char* input){
    char *name = strtok(input, " ");
    int id = atoi(strtok(NULL, " "));
    char *endptr;
    float grade = strtof(strtok(NULL, " "), &endptr);
    if (*endptr != '\0') {
        printf("Conversion error, non-convertible part: %s\n", endptr);
    }
    return create_student(name, id, grade);
}