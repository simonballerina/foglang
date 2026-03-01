void giv1(char *instruction, char *variables_names, float *variables_values, int *global_var_index, char* buff, int start_index, int* i){
    //printf("GIV1\n");
    int len = strlen(buff);
    int instr_len = strlen(instruction);
    float giv1_value1;
    float giv1_value2;

    if (instruction[5] == '*')
    { // om det är en variabel hämtar den variabelns värde

        char var1 = instruction[6];

        giv1_value1 = get_var_value(variables_names, variables_values, var1);
        // printf("giv1: VALUE1: VAR: %f\n", giv1_value1);
    }
    else
    { // om det är ett tal hittar den vad talet är

        char value1_str[128];
        int t = 0;
        while (instruction[t + 5] != ' ')
        {
            value1_str[t] = instruction[t + 5];
            t++;
        }
        value1_str[t] = '\0';

        giv1_value1 = str_to_float(value1_str);

        // printf("giv1: VALUE1: FLT: %f\n", giv1_value1);
    }

    if (instruction[instr_len - 2] == '*')
    { // om det är en variabel hämtar den variabelns värde
        char var2 = instruction[instr_len - 1];

        giv1_value2 = get_var_value(variables_names, variables_values, var2);
        // printf("giv1: VALUE2: VAR: %f\n", giv1_value2);
    }
    else
    { // om det är ett tal hittar den vad talet är

        char value2_str[128];
        int t;

        int value2_startindex;
        for (t = instr_len - 1; t >= 0; t--)
        { // hitta var talet börjar
            if (instruction[t] == ' ')
            {
                value2_startindex = t + 1;
                break;
            }
        }

        int s = 0;
        for (t = value2_startindex; t < instr_len; t++)
        {
            value2_str[s] = instruction[t];
            s++;
        }
        value2_str[s] = '\0';

        giv1_value2 = str_to_float(value2_str);
        // printf("GIV1: VALUE2: FLT: %f\n", giv1_value2);
    }

    char compare_operation;

    for (int t = 5; t < instr_len; t++)
    { // hitta vilket jämförelsemetod som ska användas
        if (instruction[t] == ' ')
        {
            compare_operation = instruction[t + 1];
            break;
        }
    }



    // hitta gränser och skit

    // hitta första id
    char loop_id = '\0';
    int t;
    for (t = start_index; t < *i+5; t++){
        if (buff[t] == '(' && buff[t+2] == ')'){
            loop_id = buff[t+1];
            //printf("FIRST LOOP_ID: %c FIRST LOOP INDEX: %d\n", loop_id, t+1);
            break;
        }
    }
    t += 2;
    if (!compare(giv1_value1, giv1_value2, compare_operation)){
        for (t; t < len; t++){
            if (buff[t] == '(' && buff[t+2] == ')' && buff[t+1] == loop_id){
                //printf("SECOND LOOP_ID: %c SECOND LOOP INDEX: %d\n", buff[t+1], t+2);
                *i = t+2;
                break;
            }
        }   
    }

    






}