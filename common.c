#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

student create_student(char* name, int id, float grade){
    student new_student;
    new_student.name = strdup(name); 
    new_student.id = id;
    new_student.grade = grade;
    return new_student;
}

void free_student(student student){
    free(student.name);
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