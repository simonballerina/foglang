
Get_var_return get_var_value(char *name, int length, int type, double index, Scope *scope){
    Get_var_return ret_value;
    /*for (int i = 0; i < length; i++){
        printf("%c", name[i]);
    }
    printf("    length: %d, type: %d, index: %lf\n", length, type, index);*/
    for (int i = 0; i < (*scope).index; i++){
        if (length == (*scope).variables[i].name_len && strncmp(name, (*scope).variables[i].name, length) == 0){ // hittat en variabel
            if (type == VAR_LIST){
                int ret_type = VAR_STRING;
                if (index >= (*scope).variables[i].len || index < 0){
                    printf("ERR: Ogiltig indexing av lista\n");
                    exit(-1);
                }
                Variable found_var = (*scope).variables[i+(int)index+1];
                
                if (found_var.type == VAR_NUMBER) ret_type = VAR_NUMBER;

                if (ret_type == VAR_STRING){
                    char* ret_str = found_var.str_ptr;

                    ret_value.string = ret_str;
                    
                    ret_value.value = 0;
                    ret_value.str_len = found_var.len;
                } else if (ret_type == VAR_NUMBER){
                    ret_value.str_len = 0;
                    ret_value.string = 0;
                    ret_value.value = found_var.value;
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
                ret_value.type = VAR_STRING;
                if (!ret_value.string) ret_value.type = VAR_NUMBER;
                return ret_value;
            }
        }

    }
    printf("ERR: Kunde inte framställa ett variabelvärde\n");
    exit(-1);
}


void create_list_var(char *name, int name_len, Token *values, Token (*instructions)[128], int instruction_amount, Scope *scope)
{
    // räkna ut hur många element den ska ha i listan
    int len = 0;


    for (int i = 0; values[i].type != TERMINATOR; i++){
        if (values[i].type == COMMA) len++;;
    }
    len++;

    int args_amount = 0;
    for (int i = 0; values[i].type != TERMINATOR; i++){
        args_amount++;
    }

    // todo: ta bort variablerna först
    

    size_t var_size = 1 + len;   // grundvariabeln + strängen/listan

    Variable var = { // init var
        .len = len,
        .name = name,
        .name_len = name_len,
        .type = VAR_LIST,
        .value = 0,
        .str_ptr = 0
    };

    if ((*scope).index >= (*scope).capacity)
    { // kolla att strl är ok
        (*scope).variables = realloc((*scope).variables, sizeof(Variable)*((*scope).capacity + var_size + 64));
        (*scope).capacity += var_size + 64;
        if ((*scope).variables == NULL)
        {
            printf("ERR: Minnesallokering misslyckades\n");
            exit(1);
        }
    }
    (*scope).variables[(*scope).index++] = var;


    // lägg till lista
    for (int i = 0; i < args_amount; i++)
    {
        if (values[i].type == COMMA || values[i].type == RIGHT_BRACKET)
            continue;

        Variable list_var = {
            .name = 0,
            .name_len = 0,
            .type = VAR_NONE,
            .value = 0,
            .len = 0,
            .str_ptr = 0
        };

        // räkna längden till nästa kommatecken eller högerbracket
        int item_len = 0;
        for (int j = i; j < args_amount+1; j++){
                
            if (values[j].type == COMMA || values[j].type == RIGHT_BRACKET) break;
            item_len++;
        }


        if (values[i].type == NUMBER)
        {

            list_var.value = evaluate_expression(values+i, item_len, instructions, instruction_amount, scope);
            list_var.type = VAR_LIST_NUMBER;
            (*scope).variables[(*scope).index++] = list_var;
            i+=item_len;
        }
        else if (values[i].type == STRING)
        {
            String list_str = evaluate_str_expression(values+i, item_len, instructions, instruction_amount, scope);
            list_var.len = list_str.len;
            if (list_str.string == NULL)
            {
                printf("ERR: Minnesallokering misslyckades\n");
                exit(1);
            }
            list_var.str_ptr = list_str.string;
            list_var.type = VAR_LIST_STRING;
            (*scope).variables[(*scope).index++] = list_var;
            i+=item_len;
        } else if (values[i].type == VARIABLE){

            int var_type = VAR_NONE;
            Get_var_return test_var = get_var_value(values[i].var.name, values[i].var.name_len, 0, 0, scope);
            var_type = test_var.type;


            if (var_type == VAR_STRING) {
                String var = evaluate_str_expression(values+i, item_len, instructions, instruction_amount, scope);
                list_var.type = VAR_LIST_STRING;
                list_var.str_ptr = var.string;
                list_var.len = var.len;
                (*scope).variables[(*scope).index++] = list_var;
                i+=item_len;
            }
            else if (var_type == VAR_NUMBER) {
                double var_value = evaluate_expression(values+i, item_len, instructions, instruction_amount, scope);

                list_var.value = var_value;
                list_var.type = VAR_LIST_NUMBER;
                (*scope).variables[(*scope).index++] = list_var;
                i+=item_len;
            }
            

            
            

        } else {
            printf("ERR: Okänd datatyp i lista\n");
            exit(-1);
        }
    }
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
            if (index < (*scope).variables[i].len){
                // free gamla strängar
                if ((*scope).variables[i+1+index].type == VAR_LIST_STRING) free((*scope).variables[i+1+index].str_ptr);

                // kopiera över nya variabeln
                (*scope).variables[i+1+index] = new_var;
            } else { // skapa nytt item
                if ((*scope).index >= (*scope).capacity){
                    (*scope).variables = realloc((*scope).variables, sizeof(Variable)*((*scope).capacity + 1 + 64));
                    (*scope).capacity += 64+1;
                    if ((*scope).variables == NULL){
                        printf("ERR: Minnesallokering misslyckades\n");
                        exit(1);
                    }
                }
                memmove(
                    (*scope).variables + i + index + 2,
                    (*scope).variables + i + index + 1,
                    ((*scope).index - (i + index + 1)) * sizeof(Variable)
                );                
                (*scope).variables[i+1+index] = new_var;
                (*scope).index++;
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