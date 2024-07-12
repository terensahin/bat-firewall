
#ifndef STUDENT_VECTOR_2_H
#define STUDENT_VECTOR_2_H

typedef struct student{
    char* name;
    int age;
    int student_id;
}student;

typedef struct student_vector{
    student* students;
    int size;
    int student_count;
} student_vector;

student create_student(char* name, int age, int student_id);
void vector_initialize(student_vector* vector);
void vector_add(student_vector* vector, student new_student);
void vector_delete(student_vector* vector, int index);
student vector_get(student_vector* vector, int index);
void test_add(student_vector* vector);
void test_delete(student_vector* vector);
void test_get(student_vector* vector);
void test_vector();
#endif
