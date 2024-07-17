#ifndef COMMON_H
#define COMMON_H

typedef struct student{
    char* name;
    int id;
    float grade;
}student;

typedef enum command{
    add,
    del,
    show
}command;

typedef struct daemon_command{
    command command_type;
    student student_info;
}daemon_command;

student create_student(char* name, int id, float grade);
void free_student(student student);
daemon_command create_daemon_command(command command_type, char* name, int id, float grade);
void print_student_info(student student);

#endif