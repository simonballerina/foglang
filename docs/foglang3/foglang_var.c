#include "foglang.h"

void create_variable(Node* value, char* name, Scope* scope){

    int len = strlen(name);
    char* new_name = malloc(len+1);
    if (!new_name) goto malloc_error;
    memcpy(new_name, name, len+1);

    Variable v = {
        .name = new_name,
        .type = value->type
    };

    if (value->type == NODE_NUMBER) v.number = value->number.value;
    else if (value->type == NODE_STRING) {

        int len = strlen(value->string.string);
        char* new_str = malloc(len+1);
        if (!new_str) goto malloc_error;
        memcpy(new_str, value->string.string, len+1);
        
        v.string.string = new_str;
        v.string.is_svets = value->string.is_svets;
    }
    else if (value->type == NODE_LIST) {
        v.list.len = value->list.len;
        v.list.list = value->list.list;
    }

    if (scope->top >= scope->capacity) {
        Variable* tmp = realloc(scope->variables, (scope->capacity*2)*sizeof(Variable));
        if (!tmp) goto malloc_error;
        scope->variables = tmp;
        scope->capacity *= 2;
    }
    scope->variables[scope->top++] = v;
    
    return;

    malloc_error:
        printf("Memory allocation failed\n");
        exit(1);
}

void change_var_value(Scope* scope, char* name, Node* new_value){
    for (int i = 0; i < scope->top; i++){
        if (!strcmp(scope->variables[i].name, name)) {

            NodeType old_type = scope->variables[i].type;
            if (old_type == NODE_STRING){ // free when lists are implemented
                free(scope->variables[i].string.string);
            }

            NodeType type = new_value->type;
            if (type == NODE_STRING) {
                int len = strlen(new_value->string.string);
                char* new_str = malloc(len+1);
                if (!new_str) goto malloc_error;
                memcpy(new_str, new_value->string.string, len+1);
                
                scope->variables[i].string.string = new_str;
                scope->variables[i].string.is_svets = new_value->string.is_svets;
            }

            else if (type == NODE_NUMBER) scope->variables[i].number = new_value->number.value;
            else if (type == NODE_LIST) {
                scope->variables[i].list.len = new_value->list.len;
                scope->variables[i].list.list = new_value->list.list;
            }

            scope->variables[i].type = type;
        }
    }

    return;
    malloc_error:
        printf("Memory allocation failed\n");
        exit(1);
}

UnnamedVariable get_var_value(Scope* scope, char* name){ // Returns variable with type TERMINATOR if not found
    for (int i = 0; i < scope->top; i++){
        if (!strcmp(scope->variables[i].name, name)) {
            UnnamedVariable v = {
                .type = scope->variables[i].type
            };

            if (scope->variables[i].type == NODE_NUMBER) v.number = scope->variables[i].number;
            else if (scope->variables[i].type == NODE_STRING) {
                v.string.string = scope->variables[i].string.string;
                v.string.is_svets = scope->variables[i].string.is_svets;
            }
            else if (scope->variables[i].type == NODE_LIST) {
                v.list.len = scope->variables[i].list.len;
                v.list.list = scope->variables[i].list.list;
            }

            return v;
        }
        
    }
    return (UnnamedVariable){.type = TERMINATOR};

}