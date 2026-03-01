#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "math_ops.c"
char current_loop_id;

char *read_file(const char *filename)
{
    FILE *f = fopen(filename, "rb");
    if (!f)
        return NULL;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    char *buffer = malloc(size + 1);
    if (!buffer)
        return NULL;

    fread(buffer, 1, size, f);
    buffer[size] = '\0'; // null-terminate

    fclose(f);
    return buffer;
}

float get_var_value(char *variables_names, float *variables_values, char var_name)
{
    for (int i = 0; i < 30; i++)
    {
        if (variables_names[i] == var_name)
            return variables_values[i];
    }
    return -1;
}

void foug(char *instruction, char *variables_names, float *variables_values)
{
    // printf("FOUG\n");
    int instr_len = strlen(instruction);

    char args[256];
    int l;
    int m = 0;

    for (l = 5; l < instr_len; l++)
    {
        args[m++] = instruction[l];
    }
    args[m] = '\0';

    if (args[0] == '*')
    {

        float var_value = get_var_value(variables_names, variables_values, args[1]);

        printf("fog:~$ %f\n", var_value);
    }
    else
    {
        printf("fog:~$ %s\n", args);
    }
}

void boul(char *instruction, char *variables_names, float *variables_values, int *global_var_index)
{
    // printf("BOUL\n");
    char var_value_str[256];
    char var_name = instruction[5];

    int p = 0;
    for (int n = 7; instruction[n] != '\0'; n++)
    { // kopiera variabelvärdet från hela instruktionen
        var_value_str[p++] = instruction[n];
    }
    var_value_str[p] = '\0';

    float var_value = str_to_float(var_value_str);

    // printf("%f\n", var_value);

    variables_names[*global_var_index] = var_name;
    variables_values[*global_var_index] = var_value;
    (*global_var_index)++;
}

void band(char *instruction, char *variables_names, float *variables_values, int *global_var_index)
{
    // printf("BAND\n");
    int instr_len = strlen(instruction);
    float band_value1;
    float band_value2;

    if (instruction[5] == '*')
    { // om det är en variabel hämtar den variabelns värde
        char var1 = instruction[6];
        band_value1 = get_var_value(variables_names, variables_values, var1);
        // printf("BAND: VALUE1: VAR: %f\n", band_value1);
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

        band_value1 = str_to_float(value1_str);
        // printf("BAND: VALUE1: FLT: %f\n", band_value1);
    }

    if (instruction[instr_len - 4] == '*')
    { // om det är en variabel hämtar den variabelns värde
        char var2 = instruction[instr_len - 3];

        band_value2 = get_var_value(variables_names, variables_values, var2);
        // printf("BAND: VALUE2: VAR: %f\n", band_value2);
    }
    else
    { // om det är ett tal hittar den vad talet är

        char value2_str[128];
        int t;

        int value2_startindex;
        for (t = instr_len - 3; t >= 0; t--)
        { // hitta var talet börjar
            if (instruction[t] == ' ')
            {
                value2_startindex = t + 1;
                break;
            }
        }

        int s = 0;
        for (t = value2_startindex; t < instr_len - 2; t++)
        {
            value2_str[s] = instruction[t];
            s++;
        }
        value2_str[s] = '\0';

        band_value2 = str_to_float(value2_str);
        // printf("BAND: VALUE2: FLT: %f\n", band_value2);
    }

    char operation;

    for (int t = 5; t < instr_len; t++)
    { // hitta vilket räknesätt som ska användas
        if (instruction[t] == ' ')
        {
            operation = instruction[t + 1];
            break;
        }
    }

    char result_var = instruction[strlen(instruction) - 1];

    float result = evaluate(band_value1, band_value2, operation);

    int create_new_var = 1; // ta reda på om en variabel ska skapas eller uppdateras
    for (int q = 0; q < 30; q++)
    {
        if (variables_names[q] == result_var)
        {
            variables_values[q] = result;
            create_new_var = 0;
            break;
        }
    }
    if (create_new_var)
    {
        variables_names[*global_var_index] = result_var;
        variables_values[*global_var_index] = result;
        *(global_var_index)++;
    }
    /*
    for (int a = 0; a < 30; a++){
        printf("%c ", variables_names[a]); 
    }
    printf("\n");
    for (int a = 0; a < 30; a++){
        printf("%f ", variables_values[a]); 
    }
    printf("\n");
    */
}


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







void naer(char *instruction, char *variables_names, float *variables_values, int *global_var_index, 
    char* buff, int start_index, int* i, int* do_loop, int* loop_start_index, int* first_id_index)
{   

    // printf("NAER\n");
    
    int instr_len = strlen(instruction);
    float giv1_value1;
    float giv1_value2;

    if (instruction[5] == '*')
    { // om det är en variabel hämtar den variabelns värde

        char var1 = instruction[6];

        giv1_value1 = get_var_value(variables_names, variables_values, var1);
        // printf("NAER: VALUE1: VAR: %f\n", giv1_value1);
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

        // printf("NAER: VALUE1: FLT: %f\n", giv1_value1);
    }

    if (instruction[instr_len - 2] == '*')
    { // om det är en variabel hämtar den variabelns värde
        char var2 = instruction[instr_len - 1];

        giv1_value2 = get_var_value(variables_names, variables_values, var2);
        // printf("NAER: VALUE2: VAR: %f\n", giv1_value2);
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
        // printf("NAER: VALUE2: FLT: %f\n", giv1_value2);
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

    
    int buff_len = strlen(buff);



    char loop_id = '\0';
    int t;
    for (t = start_index; t < *i+5; t++){
        if (buff[t] == '(' && buff[t+2] == ')'){
            current_loop_id = buff[t+1];
            *first_id_index = t;
            //*loop_start_index = t+3;
            *loop_start_index = start_index;
            printf("hittade (1) nr 1: %c\n", buff[start_index-1]);
            //printf("FIRST LOOP_ID: %c FIRST LOOP INDEX: %d\n", loop_id, t+1);
            break;
        }
    }
    t += 2;
    if (compare(giv1_value1, giv1_value2, compare_operation)){
        *do_loop = 1;
        printf("                genomför loop\n");        
    } else {
        printf("                avslutar loopen\n");
        for (t; t < buff_len; t++){
            if (buff[t] == '(' && buff[t+2] == ')' && buff[t+1] == current_loop_id){
                //printf("SECOND LOOP_ID: %c SECOND LOOP INDEX: %d\n", buff[t+1], t+2);
                
                *do_loop = 0;
                break;
            }
        }
        *i = t+2;   
        
    }
}

int main(int argc, char *argv[])
{

    if (argc != 2)
    {
        printf("fog:~$ För få argument!\n");
        printf("fog:~$ SYNTAX: <./fog fil.fg>\n");
        return -1;
    }

    char *buff = read_file(argv[1]);

    char keywords[5][5] = {"foug", "boul", "band", "giv1", "naer"};

    char variables_names[30];
    float variables_values[30];
    int global_var_index = 0;

    char action[5];
    int i = 0;

    int len = strlen(buff);

    int prev = -1;

    int do_loop = 0;
    int loop_start_index;
    int first_id_index;

    for (i = 0; i < len; i++)
    {
        // printf("%s\n", buff);
        printf("%d BUFF: %c%c%c LOOP_ID: %c DO_LOOP: %d FIRST_ID_INDEX: %d\n", i, buff[i], buff[i+1], buff[i+2], current_loop_id, do_loop, first_id_index);
        if (buff[i] == '(' &&
            buff[i+2] == ')' &&
            buff[i+1] == current_loop_id &&
            do_loop &&
            i != first_id_index)
        {
            printf("HOPPADE\n");
            i = loop_start_index;
        }
        if (buff[i] == ';')
        {
            if (prev != -1 && i - prev > 1)
            {
                int start_index = prev + 1;
                int end_index = i - 1;

                // printf("START: %d\nEND: %d\n", start_index, end_index);



                int k = 0;
                char instruction[128];
                for (int j = start_index; j <= end_index; j++)
                {
                    if (buff[j] == '\n')
                        continue;
                    instruction[k] = buff[j];
                    k++;
                }
                instruction[k] = '\0';

                //printf("INSTR %d: %s\n", i, instruction);

                char action[5];
                int j;
                for (j = 0; j < 4; j++)
                {
                    action[j] = instruction[j];
                }
                action[j] = '\0';

                int o;
                for (o = 0; o < sizeof(keywords) / (5 * sizeof(char)); o++)
                {

                    if (!strcmp(action, keywords[o]))
                    {

                        // printf("%s\n", keywords[o]);  // identifierade handlingen

                        break;
                    }
                }

                int instr_len = strlen(instruction);

                switch (o)
                {
                case 0: // foug (print) foug str; foug *var; (foug Hello, world!;     foug *a;)

                    foug(instruction, variables_names, variables_values);
                    break;

                case 1: // boul (deklarera variabel) boul var value; (boul a 14;)

                    boul(instruction, variables_names, variables_values, &global_var_index);
                    break;

                case 2: // band: räkna matte med variabler och spara resultat     band var1 + var2 result_var;  (band a + b c;)

                    band(instruction, variables_names, variables_values, &global_var_index);
                    break;

                case 3: // giv1 *a = *b;

                    giv1(instruction, variables_names, variables_values, &global_var_index, buff, start_index, &i);
                    break;

                case 4: // loop? kopiera strängar x antal ggr in i buff?

                    naer(instruction, variables_names, variables_values, &global_var_index, buff, start_index, &i, &do_loop, &loop_start_index, &first_id_index);
                    break;
                }
            }
            prev = i;
        }
    }

    return 0;
}
