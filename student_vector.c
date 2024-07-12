#include "student_vector.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

student create_student(char* name, int age, int student_id){
    student new_student;
    new_student.name = strdup(name); 
    new_student.age = age;
    new_student.student_id = student_id;
    return new_student;
}

void vector_initialize(student_vector* vector){
    vector->size = 1;
    vector->student_count = 0;
    vector->students = (student*)calloc(vector->size, sizeof(student));
}

void vector_add(student_vector* vector, student new_student){
    if(vector->size == vector->student_count){
        vector->size *= 2;
        vector->students = realloc(vector->students, vector->size * sizeof(student));
    }
    vector->students[vector->student_count] = new_student;
    vector->student_count += 1;
}

void vector_delete(student_vector* vector, int index) {
    if (index > vector->student_count - 1){
        return;
    }
    free(vector->students[index].name);
    for(int i = index; i < vector->student_count - 1; i++){
        vector->students[i] = vector->students[i+1];
    }
    vector->student_count -= 1;
}

student vector_get(student_vector* vector, int index){
    if (index > vector->student_count - 1){
        return;
    }
    return vector->students[index];
}

void test_add(student_vector* vector){ 
    student student1 = create_student("baran", 5, 200);
    student student2 = create_student("ali", 7, 200);
    student student3 = create_student("ayşe", 9, 200);

    vector_add(vector, student1);
    vector_add(vector, student2);
    vector_add(vector, student3);

    if(strcmp(vector->students[0].name, "baran") != 0){
        printf("Vector Add Fail!\n");
        return;
    }

    printf("Vector try!\n");
    if(strcmp(vector->students[1].name, "ali") != 0){
        printf("Vector Add Fail!\n");
        return;
    }
    if(strcmp(vector->students[2].name, "ayşe") != 0){
        printf("Vector Add Fail!\n");
        return;
    }
    if(vector->students[0].age != 5){
        printf("Vector Add Fail!\n");
        return;
    }
    if(vector->students[1].age != 7){
        printf("Vector Add Fail!\n");
        return;
    }
    if(vector->students[2].age != 9){
        printf("Vector Add Fail!\n");
        return;
    }
    printf("Vector Add Success!\n");
}

void test_delete(student_vector* vector){ 
    vector_delete(vector, 1);
    if(strcmp(vector->students[0].name, "baran") != 0){
        printf("%s\n", vector->students[0].name);
        printf("Vector Delete Fail 1!\n");
        return;
    }
    if(strcmp(vector->students[1].name, "ayşe") != 0){
        printf("%s\n", vector->students[1].name);
        printf("Vector Delete Fail 2!\n");
        return;
    }
    vector_delete(vector, 1);
    if(strcmp(vector->students[0].name, "baran") != 0){
        printf("%s\n", vector->students[0].name);
        printf("Vector Delete Fail 3!\n");
        return;
    }
    if(vector->student_count != 1){
        printf("Vector Delete Fail 4!\n");
        printf("%d\n", vector->student_count);
        return;
    }
    printf("Vector Delete Success!\n");
}

void test_get(student_vector* vector){ 
    student std1 = vector_get(vector, 0);
    if(strcmp(vector->students[0].name, "baran") != 0){
        printf("%s\n", vector->students[0].name);
        printf("Vector Get Fail 1!\n");
        return;
    }
    printf("Vector Get Success!\n");
}

void test_vector(){
    student_vector* vector;
    vector = (student_vector*)malloc(sizeof(student_vector));
    vector_initialize(vector);
    
    test_add(vector);
    test_delete(vector);
    test_get(vector);

    for(int i = 0; i < vector->student_count; i++){
        free(vector_get(vector, i).name);
    }
    free(vector->students);
    free(vector);
}