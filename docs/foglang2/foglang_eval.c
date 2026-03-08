

void cleanup_args(Token* args, int args_amount, Token (*instructions)[128], int instruction_amount, Scope *scope){
    for (int i = 0; i < args_amount; i++){
        if (args[i].type == VARIABLE && args[i].var.type != VAR_FUNCTION){
            // kolla om det är en indexering av en variabel
            if (i+1 < args_amount && args[i+1].type == LEFT_BRACKET){
                int index_len = 0;
                int j = i+2;
                while (j < args_amount && args[j].type != RIGHT_BRACKET){ index_len++; j++; }
                if (j >= args_amount){
                    printf("ERR: Slutklammer saknas i indexering\n");
                    exit(-1);
                }
                cleanup_args(args + i + 2, index_len, instructions, instruction_amount, scope);
                int index = (int)evaluate_expression(args + i + 2, index_len, instructions, instruction_amount, scope);

                Get_var_return var = get_var_value(args[i].var.name, args[i].var.name_len, VAR_LIST, index, scope);
                
                for (int k = i+1; k <= j && k < args_amount; k++){
                    args[k].type = NONE;
                }

                if (var.type == VAR_STRING){
                    args[i].type = STRING;
                    args[i].var.name = var.string;
                    args[i].var.name_len = var.str_len;
                } else if (var.type == VAR_NUMBER){
                    args[i].type = NUMBER;
                    args[i].value = var.value;
                }
            } else {
                Get_var_return var = get_var_value(args[i].var.name, args[i].var.name_len, 0, 0, scope);
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
            Get_var_return value = call_function(args[i].var.name, args[i].var.name_len, program_counter, instructions, instruction_amount);
            if (value.type == VAR_NUMBER){
                args[i].type = NUMBER;
                args[i].value = value.value;
            } else if (value.type == VAR_STRING){
                args[i].type = STRING;
                args[i].var.name = value.string;
                args[i].var.name_len = value.str_len;
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


    while (1)
    {
        // räkna mängden tokens med värden
        int valid_token_count = 0;
        for (int i = 0; i < args_amount; i++)
        {
            if (args[i].type != NONE)
                valid_token_count++;
        }
        // printf("VALID_TOKEN_COUNT: %d\n", valid_token_count);
        if (valid_token_count <= 2) // pga terminator räknas som en
        {
            for (int i = 0; i < args_amount; i++)
            {
                if (args[i].type == NUMBER)
                {
                    // printf("RETURNADE %lf\n", args[i].value);
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
                while (args[i].type != RIGHT_PAR)
                {
                    if (i == args_amount)
                    {
                        printf("ERR: Slutparantes hittades inte\n");
                        exit(-1);
                    }
                    i++;
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

        // ta bort paranteserna
        args[start_par_index].type = NONE;
        args[stop_par_index].type = NONE;

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
            if (args[i].type == MULTIPLIED || args[i].type == DIVIDED)
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
                else
                    args[i].value = first_arg / second_arg;
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
            Get_var_return var_ret = get_var_value(args[i].var.name, args[i].var.name_len, 0, 0, scope);
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


Get_var_return dynamic_eval(Token *args_old, int args_amount, Token (*instructions)[128], int instruction_amount, Scope *scope){

    Token args[args_amount];
    memcpy(args, args_old, args_amount * sizeof(Token)); // av någon skum anledning måste den ha en lokal kopia
    
    cleanup_args(args, args_amount, instructions, instruction_amount, scope);


    int type = VAR_STRING;

    for (int i = 0; args[i].type != TERMINATOR; i++){
        if (args[i].type == NUMBER) {
            type = VAR_NUMBER; // finns ett nummer -> använd eval_expr
            break;
        }

        if (args[i].type == VARIABLE){
            Get_var_return ret = get_var_value(args[i].var.name, args[i].var.name_len, 0, 0, scope);
            if (ret.type == VAR_STRING) type = VAR_STRING;
        }
    }
    String str_ret;
    double num_ret;

    Get_var_return ret;


    if (type == VAR_STRING)
    {
        str_ret = evaluate_str_expression(args, args_amount, instructions, instruction_amount, scope);
        ret.str_len = str_ret.len;
        ret.string = str_ret.string;
        ret.type = VAR_STRING;
        ret.value = 0;
        return ret;
    } 
    else if (type == VAR_NUMBER)
    {
        num_ret = evaluate_expression(args, args_amount, instructions, instruction_amount, scope);
        ret.str_len = 0;
        ret.string = 0;
        ret.type = VAR_NUMBER;
        ret.value = num_ret;
        return ret;
    }


}