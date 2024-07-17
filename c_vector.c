#include "c_vector.h"

void *vector_initialize(void *vec, size_t element_size, void (*destuctor_function)(void *))
{
    vector_metadata *metadata = (vector_metadata *)malloc(sizeof(vector_metadata));
    metadata->capacity = 0;
    metadata->size = 0;
    metadata->element_size = element_size;
    metadata->elem_destructor_func = destuctor_function;
    return ((void *)&((vector_metadata *)(metadata))[1]);
}

void *vector_meta_to_vec(vector_metadata *metadata)
{
    return ((void *)&((vector_metadata *)(metadata))[1]);
}

vector_metadata *vector_get_metadata(void *vec)
{
    return &((vector_metadata *)vec)[-1];
}

int vector_get_size(void *vec)
{
    return vec ? vector_get_metadata(vec)->size : 0;
}

size_t vector_get_element_size(void *vec)
{
    return vec ? vector_get_metadata(vec)->element_size : 0;
}

int vector_get_capacity(void *vec)
{
    return vec ? vector_get_metadata(vec)->capacity : 0;
}

void vector_set_size(void *vec, int new_size)
{
    if (vec)
        vector_get_metadata(vec)->size = new_size;
    return;
}

void vector_set_element_size(void *vec, size_t new_element_size)
{
    if (vec)
        vector_get_metadata(vec)->element_size = new_element_size;
    return;
}

void vector_set_capacity(void *vec, int new_capacity)
{
    if (vec)
        vector_get_metadata(vec)->capacity = new_capacity;
    return;
}

void *vector_resize(void *vec, int new_capacity)
{
    size_t new_vector_size_in_bytes = new_capacity * vector_get_element_size(vec) + sizeof(vector_metadata);
    vector_metadata *metadata = vector_get_metadata(vec);
    metadata = realloc(metadata, new_vector_size_in_bytes);
    metadata->capacity = new_capacity;
    return vector_meta_to_vec(metadata);
}

int vector_calc_new_size(int oldsize)
{
    return (oldsize == 0) ? 1 : oldsize * 2;
}

void *vector_insert(void *vec, void *element_ptr, int index)
{
    if (index < 0 || index > vector_get_size(vec))
    {
        printf("invalid index to insert!");
        return NULL;
    }
    if (vector_get_size(vec) == vector_get_capacity(vec))
    {
        int new_size = vector_calc_new_size(vector_get_size(vec));
        vec = vector_resize(vec, new_size);
    }
    void *new_element_address = vec + index * vector_get_element_size(vec);
    memmove(new_element_address + vector_get_element_size(vec), new_element_address,
            (vector_get_size(vec) - index) * vector_get_element_size(vec));
    memcpy(new_element_address, element_ptr, vector_get_element_size(vec));
    vector_set_size(vec, vector_get_size(vec) + 1);
    return vec;
}

void *vector_push_back(void *vec, void *element_ptr)
{
    if (vector_get_size(vec) == vector_get_capacity(vec))
    {
        int new_size = vector_calc_new_size(vector_get_size(vec));
        vec = vector_resize(vec, new_size);
    }
    void *new_element_address = vec + vector_get_size(vec) * vector_get_element_size(vec);
    memcpy(new_element_address, element_ptr, vector_get_element_size(vec));
    vector_set_size(vec, vector_get_size(vec) + 1);
    return vec;
}

void vector_erase(void *vec, int index)
{
    if (index >= vector_get_size(vec))
    {
        printf("invalid index to remove!");
        return;
    }
    void *deleted_element_address = vec + index * vector_get_element_size(vec);
    if (vector_get_metadata(vec)->elem_destructor_func != NULL)
        vector_get_metadata(vec)->elem_destructor_func(deleted_element_address);
    memmove(deleted_element_address, deleted_element_address + vector_get_metadata(vec)->element_size, (vector_get_size(vec) - index - 1) * vector_get_metadata(vec)->element_size);
    vector_set_size(vec, vector_get_size(vec) - 1);
    memset(vec + vector_get_size(vec) * vector_get_element_size(vec), 0, vector_get_element_size(vec));
}

void vector_pop_back(void *vec)
{
    vector_erase(vec, vector_get_size(vec) - 1);
}

void *vector_at(void *vec, int index)
{
    if (index < 0 || index >= vector_get_size(vec))
    {
        printf("invalid index to get!");
        return NULL;
    }
    void *element_address = vec + index * vector_get_element_size(vec);
    // void *result = malloc(vector_get_element_size(vec));
    // memcpy(result, element_address, vector_get_element_size(vec));
    return element_address;
}

void *vector_front(void *vec)
{
    return vector_at(vec, 0);
}

void *vector_back(void *vec)
{
    return vector_at(vec, vector_get_size(vec) - 1);
}

void vector_free(void *vec)
{
    if (vec)
    {
        for (int i = 0; i < vector_get_size(vec); i++)
        {
            int *free_address = vec + i * vector_get_element_size(vec);
            if(vector_get_metadata(vec)->elem_destructor_func != NULL)
                vector_get_metadata(vec)->elem_destructor_func(free_address);
        }
        free(vector_get_metadata(vec));
    }
    return;
}