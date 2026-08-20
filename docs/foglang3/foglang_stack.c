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

        stack->data = realloc(stack->data, stack->capacity*2*stack->element_size);
        if (!stack->data) goto malloc_error;
    }
    
    memcpy((char*)stack->data + stack->top*stack->element_size, value, stack->element_size);
    
    (stack->top)++;

    return;
    malloc_error:
        printf("Memory allocation failed\n");
        exit(1);
}

void stack_pop(Stack* stack, void* out) {
    memcpy(out, (char*)stack->data+stack->top*stack->element_size, stack->element_size);
        
    (stack->top)--;
    return;
}