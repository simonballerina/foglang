
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
                    printf("ERR: Försöker indexera en icke-list-variabel\n");
                    exit(-1);
                }
                if (index < 0) index = list_var.len+index;
                if (index >= list_var.len || index < 0){
                    printf("ERR: Ogiltig indexing av lista\n");
                    exit(-1);
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
                    printf("ERR: Kunde inte framställe ett variabelvärde för en ogiltig datatyp\n");
                    exit(-1);
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
    printf("ERR: Kunde inte framställa ett variabelvärde\n");
    exit(-1);
}


void create_list_var(char *name, int name_len, Token *values, Token (*instructions)[128], int instruction_amount, Scope *scope)
{
    // räkna ut hur många element den ska ha i listan (notera att man inte kan räkna kommatecken för listor kan finnas i andra listor)
    int len = 0;
    int depth = 0;

    for (int i = 0; values[i].type != RIGHT_BRACKET || depth != 0; i++){
        if (values[i].type == LEFT_BRACKET) depth++;
        if (values[i].type == RIGHT_BRACKET) depth--;
        if (values[i].type == COMMA && depth == 0) len++;
    }
    len++; // antal element är antal kommatecken + 1

    int args_amount = 0;
    for (int i = 0; values[i].type != TERMINATOR; i++){
        args_amount++;
    }

    if ((*scope).index >= (*scope).capacity)
    {
        (*scope).variables = realloc((*scope).variables, sizeof(Variable)*((*scope).capacity + 1 + 64));
        (*scope).capacity += 1 + 64;
        if ((*scope).variables == NULL)
        {
            printf("ERR: Minnesallokering misslyckades\n");
            exit(1);
        }
    }

    Dynamic_Var *items = malloc(len * sizeof(Dynamic_Var));
    if (items == NULL)
    {
        printf("ERR: Minnesallokering misslyckades\n");
        exit(1);
    }

    int start_index = 0;
    if (args_amount > 0 && values[0].type == LEFT_BRACKET) {
        start_index = 1;
    }

    int i = start_index;
    int item_index = 0;
    while (i < args_amount && values[i].type != RIGHT_BRACKET) {
        if (values[i].type == COMMA) {
            i++;
            continue;
        }

        int item_len = 0;
        int item_depth = 0;
        for (int j = i; j < args_amount; j++) {
            if (values[j].type == LEFT_BRACKET) {
                item_depth++;
                item_len++;
            } else if (values[j].type == RIGHT_BRACKET) {
                if (item_depth == 0) break;
                item_depth--;
                item_len++;
            } else if (values[j].type == COMMA && item_depth == 0) {
                break;
            } else {
                item_len++;
            }
        }

        Dynamic_Var value = dynamic_eval(values+i, item_len, instructions, instruction_amount, scope);
        Dynamic_Var list_item = {
            .string = NULL,
            .str_len = 0,
            .value = 0,
            .type = VAR_NONE,
            .list_ptr = NULL
        };

        if (value.type == VAR_NUMBER) {
            list_item.type = VAR_NUMBER;
            list_item.value = value.value;
        } else if (value.type == VAR_STRING) {
            list_item.type = VAR_STRING;
            list_item.str_len = value.str_len;
            list_item.string = value.string;
        } else if (value.type == VAR_LIST) {
            list_item.type = VAR_LIST;
            list_item.str_len = value.str_len;
            list_item.list_ptr = value.list_ptr;
        } else {
            printf("ERR: Okänd datatyp i lista\n");
            exit(1);
        }

        items[item_index++] = list_item;
        i += item_len;
        if (i < args_amount && values[i].type == COMMA) {
            i++;
        }
    }

    if (item_index != len) {
        printf("ERR: Felaktig lista, antal element stämmer inte\n");
        exit(1);
    }

    Variable list_var = {
        .name = name,
        .name_len = name_len,
        .type = VAR_LIST,
        .value = 0,
        .len = len,
        .str_ptr = 0,
        .list_ptr = items
    };

    (*scope).variables[(*scope).index++] = list_var;
}


void create_num_var(char *name, int name_len, double value, Scope *scope)
{
    Variable var = {// init var
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
            printf("ERR: Minnesallokering misslyckades\n");
            exit(1);
        }
    }
    (*scope).variables[(*scope).index++] = var;
}


void change_list_item(char* name, int name_len, int index, Variable new_var, Scope *scope){

    for (int i = 0; i < (*scope).index; i++){
        if ((*scope).variables[i].name_len == name_len && !strncmp(name, (*scope).variables[i].name, name_len)){
            if ((*scope).variables[i].type != VAR_LIST){
                printf("ERR: Försöker ändra listitem i icke-list-variabel\n");
                exit(1);
            }

            Variable *parent = &(*scope).variables[i];
            if (index < 0) index = parent->len + index;

            if (index < parent->len){
                if (parent->list_ptr[index].type == VAR_STRING) free(parent->list_ptr[index].string);
                Dynamic_Var new_item = {
                    .string = NULL,
                    .str_len = 0,
                    .value = 0,
                    .type = VAR_NONE,
                    .list_ptr = NULL
                };
                if (new_var.type == VAR_STRING){
                    new_item.type = VAR_STRING;
                    new_item.string = new_var.str_ptr;
                    new_item.str_len = new_var.len;
                } else if (new_var.type == VAR_NUMBER){
                    new_item.type = VAR_NUMBER;
                    new_item.value = new_var.value;
                } else if (new_var.type == VAR_LIST){
                    new_item.type = VAR_LIST;
                    new_item.str_len = new_var.len;
                    new_item.list_ptr = new_var.list_ptr;
                } else {
                    printf("ERR: Ogiltig lista-uppdateringstyp\n");
                    exit(1);
                }
                parent->list_ptr[index] = new_item;
            } else if (index == parent->len){
                Dynamic_Var *new_ptr = realloc(parent->list_ptr, sizeof(Dynamic_Var) * (parent->len + 1));
                if (new_ptr == NULL){
                    printf("ERR: Minnesallokering misslyckades\n");
                    exit(1);
                }
                parent->list_ptr = new_ptr;
                Dynamic_Var append_item = {
                    .string = NULL,
                    .str_len = 0,
                    .value = 0,
                    .type = VAR_NONE,
                    .list_ptr = NULL
                };
                if (new_var.type == VAR_STRING){
                    append_item.type = VAR_STRING;
                    append_item.string = new_var.str_ptr;
                    append_item.str_len = new_var.len;
                } else if (new_var.type == VAR_NUMBER){
                    append_item.type = VAR_NUMBER;
                    append_item.value = new_var.value;
                } else if (new_var.type == VAR_LIST){
                    append_item.type = VAR_LIST;
                    append_item.str_len = new_var.len;
                    append_item.list_ptr = new_var.list_ptr;
                } else {
                    printf("ERR: Ogiltig lista-uppdateringstyp\n");
                    exit(1);
                }
                parent->list_ptr[parent->len] = append_item;
                parent->len++;
            } else {
                printf("ERR: Ogiltig indexering vid list-uppdatering\n");
                exit(1);
            }
        }
    }
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
        printf("ERR: Minnesallokering misslyckades\n");
        exit(1);
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
            printf("ERR: Minnesallokering misslyckades\n");
            exit(1);
        }
    }
    (*scope).variables[(*scope).index++] = var;
}