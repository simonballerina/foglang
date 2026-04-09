#include <stdio.h>
#include <stdbool.h>

#define MAX_SIZE 128

// Define a structure for the stack
typedef struct {
    int arr[MAX_SIZE];  
    int top;        
} Stack;

// Function to initialize the stack
void initialize(Stack *stack) {
    stack->top = -1;  
}

// Function to check if the stack is empty
bool isEmpty(Stack *stack) {
    return stack->top == -1;  
}

// Function to check if the stack is full
bool isFull(Stack *stack) {
    return stack->top >= MAX_SIZE - 1;  
}

// Function to push an element onto the stack
void push(Stack *stack, int value) {
    if (isFull(stack)) {
        //printf("Stack Overflow\n");
        return;
    }
    stack->arr[++stack->top] = value;
    //printf("Pushed %d onto the stack\n", value);
}

// Function to pop an element from the stack
int pop(Stack *stack) {
    if (isEmpty(stack)) {
        //printf("Stack Underflow\n");
        return -1;
    }

    int popped = stack->arr[stack->top];
    stack->top--;
    //printf("Popped %d from the stack\n", popped);
    return popped;
}

// Function to peek the top element of the stack
int peek(Stack *stack) {
    if (isEmpty(stack)) {
        //printf("Stack is empty\n");
        return -1;
    }
    return stack->arr[stack->top];
}