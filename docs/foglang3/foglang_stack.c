#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "foglang.h"


void stack_init(Stack* stack, int element_size, int capacity){
    stack->data = malloc(capacity*element_size);
    if (!stack->data) goto malloc_error;

    stack->capacity = capacity;
    stack->element_size = element_size;
    stack->top = 0;

    return;
    malloc_error:
        printf("Memory allocation failed\n");
        exit(1);
}


void stack_push(Stack* stack, void* value){
    if (stack->top >= stack->capacity) {
        int new_capacity = stack->capacity * 2;
        void* new_data = realloc(stack->data, new_capacity * stack->element_size);
        if (!new_data) goto malloc_error;
        stack->data = new_data;
        stack->capacity = new_capacity;
    }
    
    memcpy((char*)stack->data + stack->top*stack->element_size, value, stack->element_size);
    
    (stack->top)++;

    return;
    malloc_error:
        printf("Memory allocation failed\n");
        exit(1);
}

void stack_pop(Stack* stack, void* out) {
    // copy the last pushed element (top - 1)
    if (stack->top <= 0) return;
    memcpy(out, (char*)stack->data + (stack->top - 1) * stack->element_size, stack->element_size);
    stack->top--;
    return;
}