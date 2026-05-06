
#include "foglang.h"

Dynamic_Var get_var_value(char *name, int length, int type, double index, Scope *scope){
    Dynamic_Var ret_value;
    
    //printf("[DEBUG get_var_value] Looking for var, name len: %d, scope ptr: %p, scope->index: %d\n", length, (void*)scope, scope->index);
    //for (int i = 0; i < length && i < 10; i++) printf("%c", name[i]);
    //printf("\n");
    
    for (int i = 0; i < (*scope).index; i++){
        if (length == (*scope).variables[i].name_len && strncmp(name, (*scope).variables[i].name, length) == 0){ // hittat en variabel
            if (type == VAR_LIST){
                int ret_type = VAR_STRING;
                Variable list_var = (*scope).variables[i];
                if (list_var.type != VAR_LIST){
                    throw_error(ERR_TYPE, (String){"Trying to index a non-list variable", strlen("Trying to index a non-list variable")}, NULL);
                }
                if (index < 0) index = list_var.len+index;
                if (index >= list_var.len || index < 0){
                    throw_error(ERR_INDEX, (String){list_var.name, list_var.name_len}, NULL);
                }
                Dynamic_Var found_var = list_var.list_ptr[(int)index];
                
                if (found_var.type == VAR_NUMBER) {
                    ret_type = VAR_NUMBER;
                } else if (found_var.type == VAR_LIST) {
                    ret_type = VAR_LIST;
                }

                if (ret_type == VAR_STRING){
                    char* ret_str = found_var.string;

                    ret_value.string = ret_str;
                    
                    ret_value.value = 0;
                    ret_value.str_len = found_var.str_len;
                    ret_value.type = VAR_STRING;
                } else if (ret_type == VAR_NUMBER){
                    ret_value.str_len = 0;
                    ret_value.string = 0;
                    ret_value.value = found_var.value;
                    ret_value.type = VAR_NUMBER;
                } else if (ret_type == VAR_LIST) {
                    ret_value.str_len = found_var.str_len;
                    ret_value.string = NULL;
                    ret_value.value = 0;
                    ret_value.type = VAR_LIST;
                    ret_value.list_ptr = found_var.list_ptr;
                } else {
                    throw_error(ERR_TYPE, (String){"Unknown variable type", strlen("Unknown variable type")}, NULL);
                }

                return ret_value;
            }
            else {
                ret_value.value = (*scope).variables[i].value;
                ret_value.string = (*scope).variables[i].str_ptr;
                ret_value.str_len = (*scope).variables[i].len;
                ret_value.type = (*scope).variables[i].type;
                ret_value.list_ptr = (*scope).variables[i].list_ptr;
                return ret_value;
            }
        }

    }
    throw_error(ERR_NAME, (String){name, length}, NULL);
    return ret_value;
}


int get_var_type(char* name, int length, Scope *scope){

    for (int i = 0; i < (*scope).index; i++){
        if (length == (*scope).variables[i].name_len && strncmp(name, (*scope).variables[i].name, length) == 0){ // hittat en variabel

            return (*scope).variables[i].type;
        }
    }
    throw_error(ERR_NAME, (String){name, length}, NULL);
    return -1;
}

void create_list_var(char *name, int name_len, Dynamic_Var value, Scope *scope)
{
    Variable var = {
        .len = value.str_len,
        .name = name,
        .name_len = name_len,
        .type = VAR_LIST,
        .value = 0,
        .str_ptr = 0,
        .list_ptr = value.list_ptr
    };

    if ((*scope).index >= (*scope).capacity)
    {
        (*scope).variables = realloc((*scope).variables, sizeof(Variable)*((*scope).capacity + 1 + 64));
        (*scope).capacity += 1 + 64;
        if ((*scope).variables == NULL)
        {
            throw_error(ERR_MALLOC, (String){"Memory allocation failed", strlen("Memory allocation failed")}, NULL);
        }
    }
    (*scope).variables[(*scope).index++] = var;
}


void create_num_var(char *name, int name_len, double value, Scope *scope)
{
    Variable var = { // init var
                    .len = 0,
                    .name = name,
                    .name_len = name_len,
                    .type = VAR_NUMBER,
                    .value = value,
                    .str_ptr = 0};

    if ((*scope).index >= (*scope).capacity)
    { // kolla att strl är ok
        (*scope).variables = realloc((*scope).variables, (*scope).capacity + 1 + 64);
        (*scope).capacity += 1 + 64;
        if ((*scope).variables == NULL)
        {
            throw_error(ERR_MALLOC, (String){"Memory allocation failed", strlen("Memory allocation failed")}, NULL);
        }
    }
    (*scope).variables[(*scope).index++] = var;
}



void create_str_var(char *name, int name_len, int len, char *string, Scope *scope)
{
    // reservera minne till strängen

    Variable var = {
        // init var
        .len = len,
        .name = name,
        .name_len = name_len,
        .type = VAR_STRING,
        .value = 0,
    };

    char *string_ptr = malloc(len * sizeof(char));
    if (string_ptr == NULL)
    {
        throw_error(ERR_MALLOC, (String){"Memory allocation failed", strlen("Memory allocation failed")}, NULL);    
    }

    // spara inputsträngen i nyreserverade området
    memcpy(string_ptr, string, len * sizeof(char));

    var.str_ptr = string_ptr;
    var.type = VAR_STRING;
    if ((*scope).index >= (*scope).capacity)
    { // kolla att strl är ok
        (*scope).variables = realloc((*scope).variables, (*scope).capacity + 1 + 64);
        (*scope).capacity += 1 + 64;
        if ((*scope).variables == NULL)
        {
            throw_error(ERR_MALLOC, (String){"Memory allocation failed", strlen("Memory allocation failed")}, NULL);
        }
    }
    (*scope).variables[(*scope).index++] = var;
}


void change_str_char(char* var_name, int name_len, int index, char new_char, Scope* scope) {
    for (int i = 0; i < (*scope).index; i++){
        if ((*scope).variables[i].name_len == name_len && !strncmp(var_name, (*scope).variables[i].name, name_len)){
            Variable *parent = &(*scope).variables[i];

            if (index < 0) index = parent->len + index; 
            if (index == parent->len){
                // append
                char* new_str = malloc(parent->len+1);
                if (!new_str) {
                    throw_error(ERR_MALLOC, (String){"Memory allocation failed", strlen("Memory allocation failed")}, NULL);
                    exit(1);
                }

                memcpy(new_str, parent->str_ptr, parent->len);
                new_str[parent->len] = new_char;
                free(parent->str_ptr);
                parent->len++;

                parent->str_ptr = new_str;
            } 
            else if (index < 0 || index > parent->len) throw_error(ERR_INDEX, (String){var_name, name_len}, NULL);
            else { // byt ut bokstav
                parent->str_ptr[index] = new_char;
            }
        }
    }  

}

void change_list_item(char* name, int name_len, int* indices, Variable new_var, Scope *scope, int index_amount){
    for (int i = 0; i < (*scope).index; i++){
        if ((*scope).variables[i].name_len == name_len && !strncmp(name, (*scope).variables[i].name, name_len)){
            Variable var = (*scope).variables[i];
            if (var.type != VAR_LIST){
                throw_error(ERR_TYPE, (String){"Cannot change list item in non-list variable", strlen("Cannot change list item in non-list variable")}, NULL);
            }

            Variable *parent = &(*scope).variables[i];
            int index = indices[0];
            if (index < 0) index = parent->len + index;

            if (index == parent->len){
                // append
                Dynamic_Var *new_ptr = realloc(parent->list_ptr, sizeof(Dynamic_Var) * (parent->len + 1));
                if (new_ptr == NULL){
                    throw_error(ERR_MALLOC, (String){"Memory allocation failed", strlen("Memory allocation failed")}, NULL);
                    exit(1);
                }
                parent->list_ptr = new_ptr;
                //init
                parent->list_ptr[parent->len].string = NULL;
                parent->list_ptr[parent->len].str_len = 0;
                parent->list_ptr[parent->len].value = 0;
                parent->list_ptr[parent->len].type = VAR_NONE;
                parent->list_ptr[parent->len].list_ptr = NULL;
                parent->len++;

            }
            else if (index > parent->len){
                throw_error(ERR_INDEX, (String){"Invalid indexing for list update", strlen("Invalid indexing for list update")}, NULL);
                exit(1);
            }

            int do_str_index = 0;
            Dynamic_Var *current_item = &parent->list_ptr[index];
            for (int j = 1; j < index_amount; j++){
                if (current_item->type != VAR_LIST && current_item->type != VAR_STRING){
                    throw_error(ERR_TYPE, (String){"Cannot index into non-list variable", strlen("Cannot index into non-list variable")}, NULL);
                    exit(1);
                }

                index = indices[j];
                if (index < 0) index = current_item->str_len + index;
                if (index > current_item->str_len || index < 0 || (index == current_item->str_len && j+1 != index_amount)){
                    throw_error(ERR_INDEX, (String){"Invalid indexing for list update", strlen("Invalid indexing for list update")}, NULL);
                    exit(1);
                }
                if (index == current_item->str_len) { // append
                    if (current_item->type == VAR_LIST) {
                        Dynamic_Var *new_ptr = realloc(current_item->list_ptr, sizeof(Dynamic_Var)*(current_item->str_len+1));
                        if (new_ptr == NULL) {
                            throw_error(ERR_MALLOC, (String){"Memory allocation failed", strlen("Memory allocation failed")}, NULL);
                            exit(1);
                        }
                        current_item->list_ptr = new_ptr;
                        // init maxxa
                        current_item->list_ptr[current_item->str_len].string = NULL;
                        current_item->list_ptr[current_item->str_len].str_len = 0;
                        current_item->list_ptr[current_item->str_len].value = 0;
                        current_item->list_ptr[current_item->str_len].type = VAR_NONE;
                        current_item->list_ptr[current_item->str_len].list_ptr = NULL;
                        current_item->str_len++;
                    } else if (current_item->type == VAR_STRING) {
                        char *new_str = realloc(current_item->string, (current_item->str_len+1)*sizeof(char));
                        if (!new_str) {
                            throw_error(ERR_MALLOC, (String){"Memory allocation failed", strlen("Memory allocation failed")}, NULL);
                            exit(1);
                        }
                        current_item->string = new_str;
                        // charändring fixas senare i koden
                        current_item->str_len++;
                    }
                }
                if (current_item->type == VAR_STRING) 
                    do_str_index = 1;
                else
                    current_item = &current_item->list_ptr[index];
            }
            if (current_item->type == VAR_STRING && !do_str_index) free(current_item->string);
            Dynamic_Var new_item = {
                .string = NULL,
                .str_len = 0,
                .value = 0,
                .type = VAR_NONE,
                .list_ptr = NULL
            };
                    
            if (new_var.type == VAR_STRING && !do_str_index){
                    
                new_item.type = VAR_STRING;
                new_item.string = new_var.str_ptr;
                new_item.str_len = new_var.len;

            } else if (new_var.type == VAR_STRING && do_str_index) {
                new_item.type = VAR_STRING;
                new_item.string = current_item->string;
                new_item.string[index] = new_var.str_ptr[0];
                new_item.str_len = current_item->str_len;
            }
            
            else if (new_var.type == VAR_NUMBER){
                new_item.type = VAR_NUMBER;
                new_item.value = new_var.value;
            } else if (new_var.type == VAR_LIST){
                new_item.type = VAR_LIST;
                new_item.str_len = new_var.len;
                new_item.list_ptr = new_var.list_ptr;
            } else {
                throw_error(ERR_TYPE, (String){"Unknown variable type", strlen("Unknown variable type")}, NULL);
                exit(1);
            }

            *current_item = new_item;
        }
    }
}