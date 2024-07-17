
#ifndef C_VECTOR_H
#define C_VECTOR_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>

typedef struct vector_metadata{
    int capacity;
    int size;
    size_t element_size;
    void (*elem_destructor_func)(void *);
} vector_metadata;

void* vector_initialize(void *vec, size_t element_size, void (*destuctor_function)(void *));
void* vector_meta_to_vec(vector_metadata *metadata);
vector_metadata* vector_get_metadata(void *vec);
int vector_get_size(void *vec);
size_t vector_get_element_size(void *vec);
int vector_get_capacity(void *vec);
void vector_set_size(void *vec, int new_size);
void vector_set_element_size(void *vec, size_t new_element_size);
void vector_set_capacity(void *vec, int new_capacity);
void* vector_resize(void *vec, int new_capacity);
int vector_calc_new_size(int oldsize);
void* vector_insert(void *vec, void *element_ptr, int index);
void* vector_push_back(void *vec, void *element_ptr);
void vector_erase(void *vec, int index);
void vector_pop_back(void *vec);
void* vector_at(void *vec, int index);
void* vector_front(void *vec);
void* vector_back(void *vec);
void vector_free(void *vec);
#endif
