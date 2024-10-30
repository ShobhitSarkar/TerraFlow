#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include "util.h"

/*
Function to allocate space for a 2D array 
Initializes all the elements to 0
*/
void *allocateArray(int rows, int columns){
    double **array; 
    int i, j; 

    array = (double **)malloc(rows *sizeof(double *)); 

    if (array == NULL){ // case for when memory allocation fails
        return NULL; 
    }

    for (i = 0; i < rows; i++){
        array[i] = (double *)calloc(columns, sizeof(double)); 
        if (array[i] == NULL){
            for (j = 0; j < i; j++){
                free(array[j]); 
            }
            free(array); 
            return NULL; 
        }
    }

    return (void *) array; 
}

int listInit (List* l, int max_elmt_size){
    l -> max_size = 10; 
    l -> max_element_size = max_elmt_size; 
    l -> size = 0; 
    l -> data = malloc(l -> max_size * l -> max_element_size); 

    return l-> data != NULL; 
}

void listAddEnd(List* l, void* elmt){
    if (l -> size == l -> max_size){ // doubling the size of the array
        int new_max_size = l -> max_size * 2; 
        void* new_data = realloc(l->data, new_max_size * l->max_element_size);

        if (new_data == NULL){
            return; // handle memory allocation failure
        }

        l -> data = new_data; 
        l -> max_size = new_max_size; 
    }

    memcpy((char*)l->data + l->size * l->max_element_size, elmt, l->max_element_size);
    l -> size++; 
}

void *listGet(List* l, int index){
    if (index < 0 || index >= l -> size){
        return NULL;
    }

    return (char*)l->data + index * l->max_element_size; 
}