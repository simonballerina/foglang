

void cleanup_args(Token* args, int args_amount, Token (*instructions)[128], int instruction_amount, Scope *scope){
    
    for (int i = 0; i < args_amount; i++){

        if (args[i].type == VARIABLE && args[i].var.type != VAR_FUNCTION){
            // kolla om det är en indexering av en variabel
            if (i+1 < args_amount && args[i+1].type == LEFT_BRACKET){
                
                int index_len = 0;
                int j = i+2;
                while (j < args_amount && args[j].type != RIGHT_BRACKET){ 
                    index_len++; 
                    j++; 
                }
                if (j >= args_amount){
                    printf("ERR: Slutklammer saknas i indexering\n");
                    exit(-1);
                }
                cleanup_args(args + i + 2, index_len, instructions, instruction_amount, scope);
                int index = (int)evaluate_expression(args + i + 2, index_len, instructions, instruction_amount, scope);

                //printf("----§§§§§§----\nNu ska jag hitta en variabel, info: \nNamn: ");
                //printf("%.*s\nScope:\n", args[i].var.name_len, args[i].var.name);
                //print_variables(scope);
                //printf("----§§§§§§----\n");
                
                // ta reda på om det är en lista som indexeras eller en sträng som indexeras
                Dynamic_Var str_var = get_var_value(args[i].var.name, args[i].var.name_len, 0, 0, scope);
                if (str_var.type == VAR_STRING){

                    for (int k = i+1; k <= j && k < args_amount; k++){
                        args[k].type = NONE;
                    }
                    
                    args[i].type = STRING;
                    // är index ok?
                    if (index < 0) index = str_var.str_len+index;
                    if (index >= str_var.str_len || index < 0){
                        printf("ERR: Ogiltig indexing av lista\n");
                        exit(-1);
                    } 
                    args[i].var.name = str_var.string+index;
                    args[i].var.name_len = 1;
                } else {
                    Dynamic_Var list_var = get_var_value(args[i].var.name, args[i].var.name_len, VAR_LIST, index, scope);

                    for (int k = i+1; k <= j && k < args_amount; k++){
                        args[k].type = NONE;
                    }

                    if (list_var.type == VAR_STRING){
                        args[i].type = STRING;
                        args[i].var.name = list_var.string;
                        args[i].var.name_len = list_var.str_len;
                    } else if (list_var.type == VAR_NUMBER){
                        args[i].type = NUMBER;
                        args[i].value = list_var.value;
                    }

                }

                
            } else {
                //printf("----§§§§§§----\nNu ska jag hitta en variabel, info: \nNamn: ");
                //printf("%.*s\nScope:\n", args[i].var.name_len, args[i].var.name);
                //print_variables(scope);
                //printf("----§§§§§§----\n");
                Dynamic_Var var = get_var_value(args[i].var.name, args[i].var.name_len, 0, 0, scope);
                args[i].type = NONE;
                if (var.type == VAR_STRING) {
                    args[i].type = STRING;
                    args[i].var.name = var.string;
                    args[i].var.name_len = var.str_len;
                } else if (var.type == VAR_NUMBER){
                    args[i].type = NUMBER;
                    args[i].value = var.value;
                }
            }


        

        }


        if (args[i].type == VARIABLE && args[i].var.type == VAR_FUNCTION)
        {
            

            int start = i;
            int depth = 0;
            int end = i;

            // hitta första (
            while (end < args_amount && args[end].type != LEFT_PAR)
                end++;

            if (end == args_amount) {
                printf("ERR: expected (\n");
                exit(1);
            }

            depth = 1;
            end++; // gå in i parentes

            while (end < args_amount && depth > 0) {
                if (args[end].type == LEFT_PAR) depth++;
                else if (args[end].type == RIGHT_PAR) depth--;
                end++;
            }

            end--; // backa till sista ')'


            
            int saved_pc = program_counter;
            Dynamic_Var value = call_function(args[i].var.name, args[i].var.name_len, program_counter, instructions, instruction_amount, args+start, scope);
            program_counter = saved_pc;

            // ersätt hela token-strängen med returvärdet
            args[i].type = (value.type == VAR_NUMBER) ? NUMBER : STRING;
            if (value.type == VAR_NUMBER) {
                args[i].value = value.value;
            } else {
                args[i].var.name = value.string;
                args[i].var.name_len = value.str_len;
            }

            // sätt resten till NONE
            for (int j = i+1; j <= end && j < args_amount; j++) {
                args[j].type = NONE;
            }
        }


    }

}


double evaluate_expression(Token *args_old, int args_amount, Token (*instructions)[128], int instruction_amount, Scope *scope)
{
    /*for (int i = 0; i < args_amount; i++)
    {
        printf("TYPE: %d,    ", args_old[i].type, args_old[i].value);
    }
    printf("\n");

    for (int i = 0; i < args_amount; i++)
    {
        if (args_old[i].type == NUMBER)
            printf("VALUE: %lf,    ", args_old[i].value);
    }
    printf("\n");
    printf("\n");*/

    Token args[args_amount];
    memcpy(args, args_old, args_amount * sizeof(Token)); // av någon skum anledning måste den ha en lokal kopia

    cleanup_args(args, args_amount, instructions, instruction_amount, scope);

    // kolla efter negativa tal
    for (int i = 1; i < args_amount; i++){ // i = 1 för att inte läsa utanför buffer
        if (args[i].type == NUMBER && args[i-1].type == MINUS){
            if (i <= 2 && args[i-2].type == NUMBER) continue;

            args[i-1].type = NONE;
            args[i].value = args[i].value*(-1);
        }
    }


    while (1)
    {

        // räkna mängden tokens med värden
        int valid_token_count = 0;
        for (int i = 0; i < args_amount; i++)
        {
            if (args[i].type != NONE)
                valid_token_count++;
        }
        if (valid_token_count == 1)
        {
            for (int i = 0; i < args_amount; i++)
            {
                if (args[i].type == NUMBER)
                {
                    return args[i].value;
                }
            }
        }

        // hitta paranteser
        int start_par_index = -1;
        int stop_par_index = -1;
        for (int i = args_amount - 1; i >= 0; i--)
        {
            if (args[i].type == LEFT_PAR)
            {
                start_par_index = i;
                i++;
                while (i < args_amount && args[i].type != RIGHT_PAR)
                {
                    i++;
                }
                if (i >= args_amount)
                {
                    printf("ERR: Slutparantes hittades inte\n");
                    exit(-1);
                }

                stop_par_index = i;
                break;
            }
        }
        // printf("START: %d, STOP: %d\n", start_par_index, stop_par_index);

        if (start_par_index == -1)
        {
            stop_par_index = args_amount;
        }
        else
        {
            // ta bort paranteserna
            args[start_par_index].type = NONE;
            args[stop_par_index].type = NONE;
        }

        double first_arg, second_arg;
        for (int i = start_par_index + 1; i < stop_par_index; i++)
        { // hitta exponenter
            if (args[i].type == EXPONENT)
            {
                for (int j = i + 1; j < args_amount; j++)
                {
                    if (args[j].type == NUMBER)
                    {
                        second_arg = args[j].value;
                        args[j].type = NONE;
                        break;
                    }
                }
                for (int j = i - 1; j >= 0; j--)
                {
                    if (args[j].type == NUMBER)
                    {
                        first_arg = args[j].value;
                        args[j].type = NONE;
                        break;
                    }
                }

                // printf("FIRST: %lf, SECOND: %lf\n", first_arg, second_arg);
                args[i].type = NUMBER;
                args[i].value = pow(first_arg, second_arg);
                break;
            }
        }
        for (int i = start_par_index + 1; i < stop_par_index; i++)
        { // hitta multiplikationer/divisioner
            if (args[i].type == MULTIPLIED || args[i].type == DIVIDED || args[i].type == MODULO)
            {
                for (int j = i + 1; j < args_amount; j++)
                {
                    if (args[j].type == NUMBER)
                    {
                        second_arg = args[j].value;
                        args[j].type = NONE;
                        break;
                    }
                }
                for (int j = i - 1; j >= 0; j--)
                {
                    if (args[j].type == NUMBER)
                    {
                        first_arg = args[j].value;
                        args[j].type = NONE;
                        break;
                    }
                }

                // printf("FIRST: %lf, SECOND: %lf         %d\n", first_arg, second_arg, args[i].type);

                if (args[i].type == MULTIPLIED)
                    args[i].value = first_arg * second_arg;
                else if (args[i].type == DIVIDED) {
                    if (second_arg == 0) {
                        printf("ERR: Division med 0\n");
                        exit(-1);
                    }
                    args[i].value = first_arg / second_arg;
                }
                    
                else if (args[i].type == MODULO) {
                    if (second_arg == 0) {
                        printf("ERR: Division med 0\n");
                        exit(-1);
                    }
                    args[i].value = fmod(first_arg, second_arg);
                }
                args[i].type = NUMBER;
                break;
            }
        }
        for (int i = start_par_index + 1; i < stop_par_index; i++)
        { // hitta +-
            if (args[i].type == PLUS || args[i].type == MINUS)
            {
                for (int j = i + 1; j < args_amount; j++)
                {
                    if (args[j].type == NUMBER)
                    {
                        second_arg = args[j].value;
                        args[j].type = NONE;
                        break;
                    }
                }
                for (int j = i - 1; j >= 0; j--)
                {
                    if (args[j].type == NUMBER)
                    {
                        first_arg = args[j].value;
                        args[j].type = NONE;
                        break;
                    }
                }

                //printf("FIRST: %lf, SECOND: %lf\n", first_arg, second_arg);
                if (args[i].type == PLUS)
                    args[i].value = first_arg + second_arg;
                else
                    args[i].value = first_arg - second_arg;
                args[i].type = NUMBER;
                break;
            }
        }
        for (int i = start_par_index + 1; i < stop_par_index; i++)
        { // hitta bool
            if (args[i].type == OCH) // AND högst prio
            {
                for (int j = i + 1; j < args_amount; j++)
                {
                    if (args[j].type == NUMBER)
                    {
                        second_arg = args[j].value;
                        args[j].type = NONE;
                        break;
                    }
                }
                for (int j = i - 1; j >= 0; j--)
                {
                    if (args[j].type == NUMBER)
                    {
                        first_arg = args[j].value;
                        args[j].type = NONE;
                        break;
                    }
                }

                //printf("FIRST: %lf, SECOND: %lf\n", first_arg, second_arg);
                if (args[i].type == OCH)
                    args[i].value = first_arg && second_arg;
                args[i].type = NUMBER;
                break;
            }
        }
        for (int i = start_par_index + 1; i < stop_par_index; i++)
        { // hitta ELLER 
            if (args[i].type == ELLER)
            {
                for (int j = i + 1; j < args_amount; j++)
                {
                    if (args[j].type == NUMBER)
                    {
                        second_arg = args[j].value;
                        args[j].type = NONE;
                        break;
                    }
                }
                for (int j = i - 1; j >= 0; j--)
                {
                    if (args[j].type == NUMBER)
                    {
                        first_arg = args[j].value;
                        args[j].type = NONE;
                        break;
                    }
                }

                args[i].value = first_arg || second_arg;
                args[i].type = NUMBER;
                break;
            }
        }
        for (int i = start_par_index + 1; i < stop_par_index; i++)
        { // hitta INTE 
            if (args[i].type == INTE)
            {
                for (int j = i + 1; j < args_amount; j++)
                {
                    if (args[j].type == NUMBER)
                    {
                        first_arg = args[j].value;
                        args[j].type = NONE;
                        break;
                    }
                }

                args[i].value = !first_arg;
                args[i].type = NUMBER;
                break;
            }
        }



        /*for (int i = 0; i < args_amount; i++)
        {
            printf("TYPE: %d,    ", args[i].type, args[i].value);
        }
        printf("\n");

        for (int i = 0; i < args_amount; i++)
        {
            if (1)
                printf("VALUE: %lf,    ", args[i].value);
        }
        printf("\n");*/
    }

}


String evaluate_str_expression(Token *args_old, int args_amount, Token (*instructions)[128], int instruction_amount, Scope *scope){

    // konkatenera strängar. free:a gamla strängar!
    Token args[args_amount];
    memcpy(args, args_old, args_amount * sizeof(Token)); // av någon skum anledning måste den ha en lokal kopia

    cleanup_args(args, args_amount, instructions, instruction_amount, scope);


    // räkna ut den totala längden som krävs
    int final_len = 0;

    for (int i = 0; i < args_amount; i++){
        if (args[i].type == STRING){
            final_len += args[i].var.name_len;
        }
        if (args[i].type == VARIABLE){
            int var_len = get_var_value(args[i].var.name, args[i].var.name_len, 0, 0, scope).str_len;
            //printf("VAR_LEN: %d\n", var_len);
            final_len += var_len;
        }
    }


    char* result_str = malloc(final_len*sizeof(char));
    int copied_chars = 0;

    // konkatenera samtliga strängar
    for (int i = 0; i < args_amount; i++){
        if (args[i].type == STRING){
            memcpy(result_str+copied_chars, args[i].var.name, args[i].var.name_len*sizeof(char));
            copied_chars += args[i].var.name_len;
        } else if (args[i].type == VARIABLE) {
            Dynamic_Var var_ret = get_var_value(args[i].var.name, args[i].var.name_len, 0, 0, scope);
            memcpy(result_str+copied_chars, var_ret.string, var_ret.str_len*sizeof(char));
            copied_chars += var_ret.str_len;
        }
    }

    String ret = {
        .len = copied_chars,
        .string = result_str
    };

    return ret;
}


Dynamic_Var dynamic_eval(Token *args_old, int args_amount, Token (*instructions)[128], int instruction_amount, Scope *scope){
    Token *args = malloc((args_amount+1) * sizeof(Token));
    if (!args) goto malloc_error;

    memcpy(args, args_old, (args_amount+1) * sizeof(Token)); // av någon skum anledning måste den ha en lokal kopia

    cleanup_args(args, args_amount, instructions, instruction_amount, scope);

    /*printf("EFTER CLEANUP:\n");
    for (int i = 0; i < args_amount; i++) {
        printf("TYPE: %d ", args[i].type);
        if (args[i].type == NUMBER)
            printf("VAL: %lf", args[i].value);
        printf("\n");
    }
    printf("\n");*/

    int type = VAR_STRING;

    for (int i = 0; i < args_amount; i++){
        if (args[i].type == NUMBER) {
            type = VAR_NUMBER; // finns ett nummer -> använd eval_expr
            break;
        }

        if (args[i].type == VARIABLE){
            Dynamic_Var ret = get_var_value(args[i].var.name, args[i].var.name_len, 0, 0, scope);
            if (ret.type == VAR_STRING) type = VAR_STRING;
        }
    }
    String str_ret;
    double num_ret;

    Dynamic_Var ret;

    if (type == VAR_STRING)
    {
        str_ret = evaluate_str_expression(args, args_amount, instructions, instruction_amount, scope);
        ret.str_len = str_ret.len;
        ret.string = str_ret.string;
        ret.type = VAR_STRING;
        ret.value = 0;
        free(args);
        return ret;
    } 
    else if (type == VAR_NUMBER)
    {
        num_ret = evaluate_expression(args, args_amount, instructions, instruction_amount, scope);
        ret.str_len = 0;
        ret.string = 0;
        ret.type = VAR_NUMBER;
        ret.value = num_ret;
        free(args);
        return ret;
    }
    free(args);
    return ret;
    malloc_error:
        printf("ERR: Minnesallokering misslyckades\n");
        exit(1);
}


int logic_eval(Token* args_old, int args_amount, Token (*instructions)[128], int instruction_amount, Scope *scope){
    // x*2 < 8 och x+1 = 2
    //printf("LOGIC EVAL, ARGS AMOUNT :%d\n", args_amount);
    Token* args = malloc((args_amount+1) * sizeof(Token));
    if (!args) goto malloc_error;

    memcpy(args, args_old, (args_amount+1) * sizeof(Token)); // lokal kopia
    cleanup_args(args, args_amount, instructions, instruction_amount, scope);

    int arr_tok_count = 6;
    Token* bool_eval_arr = malloc(arr_tok_count*sizeof(Token));
    int bool_eval_top = 0;

    if (!bool_eval_arr) goto malloc_error;

    int i = 0;
    while (i < args_amount){
        int negate = 0;
        if (args[i].type == INTE){
            negate = 1;
            i++;
            if (i >= args_amount){
                printf("ERR: INTE saknar operand\n");
                free(args);
                free(bool_eval_arr);
                exit(1);
            }
        }

        int eval_args_amount = i;
        while (eval_args_amount < args_amount && args[eval_args_amount].type != OCH && args[eval_args_amount].type != ELLER) eval_args_amount++;
        eval_args_amount -= i;
        //printf("ARGSAMOUNT: %d\n", eval_args_amount);
        
        // räkna ut jämförelsen
        int op_type = -1;
        int op_index = -1;
        for (int j = i; j < i+eval_args_amount; j++) {
            switch (args[j].type){
                case EQUALS:
                    op_type = EQUALS;
                    op_index = j;
                    break;
                case GREATER_THAN:
                    op_type = GREATER_THAN;
                    op_index = j;
                    break;
                case NOT_EQUAL_TO:
                    op_type = NOT_EQUAL_TO;
                    op_index = j;
                    break;
                case LESS_THAN:
                    op_type = LESS_THAN;
                    op_index = j;
                    break;
            }
        }

        if (op_index == -1) {
            printf("ERR: Operatör saknas i logic_eval\n");
            free(args);
            free(bool_eval_arr);
            exit(1);
        }
        
        Dynamic_Var result_left = dynamic_eval(args+i, op_index-i, instructions, instruction_amount, scope);
        
        Dynamic_Var result_right = dynamic_eval(args+op_index+1, eval_args_amount-op_index+i-1, instructions, instruction_amount, scope);
        
        int current_eval_result;

        if (result_left.type == VAR_NUMBER && result_right.type == VAR_NUMBER){
            switch (op_type){
                case EQUALS:
                    current_eval_result = result_left.value == result_right.value;
                    break;
                case GREATER_THAN:
                    current_eval_result = result_left.value > result_right.value;
                    break;
                case NOT_EQUAL_TO:
                    current_eval_result = result_left.value != result_right.value;
                    break;
                case LESS_THAN:
                    current_eval_result = result_left.value < result_right.value;
                    break;       
            }
        } else if (result_left.type == VAR_STRING && result_right.type == VAR_STRING){
            switch (op_type){
                case EQUALS:
                    current_eval_result = (!strncmp(result_left.string, result_right.string, result_left.str_len)) && (result_left.str_len == result_right.str_len);
                    break;
                case NOT_EQUAL_TO:
                    current_eval_result = (strncmp(result_left.string, result_right.string, result_left.str_len));
                    break;
            }
        } else {
            printf("ERR: Kan inte jämföra två olika datatyper\n");
            free(args);
            free(bool_eval_arr);
            exit(1);
        }

        if (negate) current_eval_result = !current_eval_result;

        if (bool_eval_top-1 >= arr_tok_count){
            arr_tok_count *= 2;
            bool_eval_arr = realloc(bool_eval_arr, arr_tok_count*sizeof(Token));
            if (!bool_eval_arr) goto malloc_error;
        }
        bool_eval_arr[bool_eval_top].value = current_eval_result;
        bool_eval_arr[bool_eval_top].type = NUMBER;
        int next_op = 0;
        for (int j = i+eval_args_amount; j < args_amount; j++){
            if (args[j].type == OCH || args[j].type == ELLER){
                next_op = args[j].type;
                break;
            }
        }
        bool_eval_arr[bool_eval_top+1].type = next_op;
        bool_eval_top+=2;

        i += eval_args_amount+1;
    }

    // printa bool_eval_arr
    //printf("BOOL EVAL ARR:\n");
    //for (int i = 0; i < bool_eval_top; i++){
    //    printf("TYPE: %d, VALUE: %lf\n", bool_eval_arr[i].type, bool_eval_arr[i].value);
    //}
    //printf("\n\n");

    // räkna ut hela bool_eval_arr
    double res = evaluate_expression(bool_eval_arr, bool_eval_top, instructions, instruction_amount, scope);
    //printf("RES: %lf\n", res);
    res = (int)res;
    free(args);
    free(bool_eval_arr);
    return res;

    malloc_error:
        printf("ERR: Minnesallokering misslyckades\n");
        exit(1);
}
