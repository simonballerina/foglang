#include "foglang.h"

void create_variable(Node* value, char* name, Scope* scope){

    int len = strlen(name);
    char* new_name = malloc(len+1);
    if (!new_name) goto malloc_error;
    memcpy(new_name, name, len+1);

    Variable v = {
        .name = new_name,
        .value = clone_node(value)
    };

    if (value->type == NODE_NUMBER) v.value->number.value = value->number.value;
    else if (value->type == NODE_STRING) {

        int len = strlen(value->string.string);
        char* new_str = malloc(len+1);
        if (!new_str) goto malloc_error;
        memcpy(new_str, value->string.string, len+1);
        
        v.value->string.string = new_str;
        v.value->string.is_svets = value->string.is_svets;
    }
    else if (value->type == NODE_LIST) {
        v.value->list.len = value->list.len;
        v.value->list.list = value->list.list;
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

void change_var_value(Scope* scope, char* name, Node* new_value, Node** indicies, int index_count){
    for (int i = 0; i < scope->top; i++){
        if (!strcmp(scope->variables[i].name, name)) {
            
            Variable* var_to_be_changed = &scope->variables[i];
            // change var_to_be_changed based on indicies
            if (indicies != NULL && index_count > 0) {
                Node* target = var_to_be_changed->value;

                for (int idx = 0; idx < index_count; idx++) {
                    if (target->type != NODE_LIST && target->type != NODE_STRING) {
                        // ERROR:
                        printf("Cannot index number\n");
                        exit(1);
                    }

                    Node* index_node = evaluate(NULL, indicies[idx], scope);
                    if (index_node->type != NODE_NUMBER) {
                        // ERROR: index must be a number
                        printf("Index must be number\n");
                        exit(1);
                    }

                    int index = (int)index_node->number.value;
                    if (target->type == NODE_LIST) {
                        if (index < 0) index = target->list.len + index;
                        if (index < 0 || index >= target->list.len) {
                            // ERROR:
                            printf("Index out of bounds\n");
                            exit(1);
                        }

                        if (idx == index_count - 1) {
                            free_node(target->list.list[index]);
                            target->list.list[index] = clone_node(new_value);
                            free_node(index_node);
                            return;
                        }

                        target = target->list.list[index];
                        free_node(index_node);
                    } else if (target->type == NODE_STRING) {
                        int len = strlen(target->string.string);
                        if (index < 0) index = len + index;
                        if (index < 0 || index >= len) {
                            // ERROR:
                            printf("Index out of bounds\n");
                            exit(1);
                        }

                        if (idx != index_count - 1) {
                            // ERROR:
                            printf("Cannot index into a character\n");
                            exit(1);
                        }

                        if (new_value->type != NODE_STRING || strlen(new_value->string.string) == 0) {
                            printf("String index assignment requires a single-character string\n");
                            exit(1);
                        }

                        target->string.string[index] = new_value->string.string[0];
                        free_node(index_node);
                        return;
                    }
                }
            }

            NodeType old_type = var_to_be_changed->value->type;
            if (old_type == NODE_STRING && indicies == NULL){ // free old string
                free(var_to_be_changed->value->string.string);
            } else if (old_type == NODE_LIST && indicies == NULL) { // free old list if no indicies are provided
                for (int j = 0; j < var_to_be_changed->value->list.len; j++) {
                    free_node(var_to_be_changed->value->list.list[j]);
                }
                free(var_to_be_changed->value->list.list);
            }

            NodeType type = new_value->type;
            if (type == NODE_STRING) {
                int len = strlen(new_value->string.string);
                char* new_str = malloc(len+1);
                if (!new_str) goto malloc_error;
                memcpy(new_str, new_value->string.string, len+1);
                
                var_to_be_changed->value->string.string = new_str;
                var_to_be_changed->value->string.is_svets = new_value->string.is_svets;
            }

            else if (type == NODE_NUMBER) var_to_be_changed->value->number.value = new_value->number.value;
            else if (type == NODE_LIST) {
                var_to_be_changed->value->list.len = new_value->list.len;
                var_to_be_changed->value->list.list = clone_node(new_value)->list.list; // clone the list to avoid issues with freeing
            }

            if (indicies == NULL)
                var_to_be_changed->value->type = type;
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
                .type = scope->variables[i].value->type
            };

            if (scope->variables[i].value->type == NODE_NUMBER) v.number = scope->variables[i].value->number.value;
            else if (scope->variables[i].value->type == NODE_STRING) {
                v.string.string = scope->variables[i].value->string.string;
                v.string.is_svets = scope->variables[i].value->string.is_svets;
            }
            else if (scope->variables[i].value->type == NODE_LIST) {
                v.list.len = scope->variables[i].value->list.len;
                v.list.list = scope->variables[i].value->list.list;
            }

            return v;
        }
        
    }
    return (UnnamedVariable){.type = TERMINATOR};

}