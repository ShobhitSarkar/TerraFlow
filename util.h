#ifndef UTIL_H
#define UTIL_H

// Function prototype for allocating 2D array
void *allocateArray(int rows, int columns);

// Access macro for 2D array
#define ACCESS_2D(row, col, num_cols) ((row) * (num_cols) + (col))

typedef struct{
    int max_size; 
    int max_element_size; 
    void* data; 
    int size; 
} List; 

int listInit(List* l, int max_elmt_size); 
void listAddEnd(List* l, void* elmt); 
void *listGet(List* l, int index); 


#endif // UTIL_H