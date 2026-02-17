#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

int VAR_AMOUNT = 64;
#define KEYWORD_COUNT 10

double get_var_value(char *variables_names, double *variables_values, int* global_var_index, char var_name)
{
    for (int i = 0; i < *global_var_index; i++)
    {
        if (variables_names[i] == var_name)
            return variables_values[i];
    }
    printf("fog:~$ ERR: Kunde inte ta fram ett numeriskt variabelvärde: %c\n", var_name);
    exit(-1);
}

#include "math_ops.c"

void create_var(char* variables_names, double *variables_values, int* global_var_index, char var_name, double var_value){

    for (int i = 0; i < *global_var_index; i++){
        if (variables_names[i] == var_name){
            variables_values[i] = var_value;
            return;
        }
    }



    variables_names[*global_var_index] = var_name;
    variables_values[*global_var_index] = var_value;
    (*global_var_index)++;


}





int loop_start_stack[32];
char loop_id_stack[32];
int loop_stack_top = 0;

char break_id;

int call_start_index;

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



void get_str_var_value(char *str_variables_names, char str_variables_values[][128], int* str_global_var_index, char var_name, char* dest_str, size_t dest_size){
    for (int i = 0; i < *str_global_var_index; i++)
    {
        if (str_variables_names[i] == var_name){
            strncpy(dest_str, str_variables_values[i], dest_size - 1);
            dest_str[dest_size - 1] = '\0';
            return;
        }
            
    }
    printf("fog:~$ ERR: Kunde inte ta fram en sträng-variabel\n");
    exit(-1);
}


void foug(char* variables_names, double* variables_values, char* current_instruction, char* str_variables_names, char str_variables_values[][128], int* str_global_var_index, int* global_var_index, int debug, int* program_counter){
    if (debug >= 2) printf("%d      FOUG\n", *program_counter);

    int len = strlen(current_instruction);
    int i;
    int print_var_num = 0;
    int print_var_str = 0;
    
    for (i = 0; i < len; i++){
        if (current_instruction[i] == '!'){

            for (int j = 0; j < VAR_AMOUNT; j++){
                if (variables_names[j] == current_instruction[i+1]) {
                    print_var_num = 1;
                    break;
                }
                if (str_variables_names[j] == current_instruction[i+1]) {
                    print_var_str = 1;
                    break;
                }
            }
            break;
            
        }
    }
    
    char args1[64];
    char args2[64];
    printf("fog:~$ ");
    if (print_var_num || print_var_str){
        int j;
        for (j = 5; j < i; j++){
            args1[j-5] = current_instruction[j];
        }

        args1[j-5] = '\0';
        for (j = i+2; j < len; j++){
            args2[j-(i+2)] = current_instruction[j];
        }
        args2[j-(i+2)] = '\0';





        if (print_var_num){
            double var_value = get_var_value(variables_names, variables_values, global_var_index, current_instruction[i+1]);
            
            if (var_value == (int)var_value) var_value = (int)var_value;
            
            printf("%s", args1);
            if (var_value == (int)var_value) printf("%d", (int)var_value);
            else printf("%lf", var_value);
            printf("%s\n", args2);
        }  


        if (print_var_str){
            char str[128];
            get_str_var_value(str_variables_names, str_variables_values, str_global_var_index, current_instruction[i+1], str, sizeof(str));

            printf("%s", args1);
            printf("%s", str);
            printf("%s\n", args2);
            
        }
    } else {
        for (int k = 5; k < len; k++){
            printf("%c", current_instruction[k]);
        }
        printf("\n");
    }

}


void boul(char** variables_names, double** variables_values, int* global_var_index, char* current_instruction, int debug, int* program_counter, char* str_variables_names, char str_variables_values[][128], int* str_global_var_index){
    if (current_instruction[7] != '"'){

        if (debug >= 2) printf("%d      BOUL NUM\n", *program_counter);

        for (int i = 0; i < *global_var_index; i++){
            if (current_instruction[5] == str_variables_names[i]){
                printf("fog:~$ ERR: Kunde inte skapa en numerisk variabel med samma namn som sträng\n");
                exit(-1);
            }
        }



        char var_value_str[64];
        int len = strlen(current_instruction);
        for (int i = 7; i < len+1; i++){
            var_value_str[i-7] = current_instruction[i];
        }
        var_value_str[len-7] = '\0';

        double var_value = str_to_double(var_value_str);


        int create_new_var = 1;
        for (int i = 0; i < *global_var_index; i++){
            if ((*variables_names)[i] == current_instruction[5]){
                (*variables_values)[i] = var_value;
                create_new_var = 0;
                break;
            }
        }

        if (create_new_var){
            if (*global_var_index > VAR_AMOUNT-1){
                char *new_names = realloc((*variables_names), (VAR_AMOUNT+64)*sizeof(char));
                double *new_vals = realloc((*variables_values), (VAR_AMOUNT+64)*sizeof(double));

                if (!new_names || !new_vals){
                    printf("fog:~$ ERR: realloc fail\n");
                    exit(1);
                }

                (*variables_names) = new_names;
                (*variables_values) = new_vals;
                VAR_AMOUNT += 64;
            }

            create_var((*variables_names), (*variables_values), global_var_index, current_instruction[5], var_value);
        }

    } else { // skapa / uppdatera sträng
        if (debug == 2) printf("%d      BOUL STR\n", *program_counter);

        for (int i = 0; i < *global_var_index; i++){
            if (current_instruction[5] == (*variables_names)[i]){
                printf("fog:~$ ERR: Kunde inte skapa en sträng-variabel med samma namn som en numerisk variabel\n");
                exit(-1);
            }
        }

        char str[128];

        int i = 8;
        while (current_instruction[i] != '"'){
            str[i-8] = current_instruction[i];



            i++;
        }
        str[i-8] = '\0';
        


        int create_new_var = 1;
        for (int i = 0; i < *global_var_index; i++){
            if (str_variables_names[i] == current_instruction[5]){

                strncpy(str_variables_values[*str_global_var_index], str, sizeof(str_variables_values[*str_global_var_index])-1);
                str_variables_values[*str_global_var_index][sizeof(str_variables_values[*str_global_var_index])-1] = '\0'; 
                str_variables_names[*str_global_var_index] = current_instruction[5];

                create_new_var = 0;
                break;
            }
        }

        if (create_new_var) {
            str_variables_names[*str_global_var_index] = current_instruction[5];
            strncpy(str_variables_values[*str_global_var_index], str, sizeof(str_variables_values[*str_global_var_index]) - 1);
            str_variables_values[*str_global_var_index][sizeof(str_variables_values[*str_global_var_index]) - 1] = '\0'; 
            (*str_global_var_index)++;
        } 



    }
    
}

void saxx(char* str_variables_names, char str_variables_values[][128], int* str_global_var_index, char* current_instruction, int debug, int* program_counter){
    if (debug >= 2) printf("%d      SAXX\n", *program_counter);
    // hitta argumenten till saxx 
    char args[128];
    int instr_len = strlen(current_instruction);
    for (int i = 5; i < instr_len+1; i++){
        args[i-5] = current_instruction[i];
    }

    int len = strlen(args);
    char first;
    char second;

    if (len > 0 && args[0] == '!') first = args[1]; else first = '\0';

    second = '\0';
    for (int i = 1; i < len; i++){
        if (args[i] == '!' && i + 1 < len){
            second = args[i+1];
            break;
        }
    }
    
    double first_value;
    char first_value_str[128];
    if (first == '\0'){
        int i;
        for (i = 1; i < len; i++){
            if (args[i] == '"') break;
            first_value_str[i] = args[i];
        }
        first_value_str[i] = '\0';

    } else {
        get_str_var_value(str_variables_names, str_variables_values, str_global_var_index, first, first_value_str, sizeof(first_value_str));
    }

    double second_value;
    int start_index = -1;
    char second_value_str[128];
    if (second == '\0'){
        for (int i = 0; i + 2 < len; i++){
            if (args[i] == ' ' && args[i+2] == ' '){
                start_index = i+3;
                break;
            }
        }
        if (start_index == -1){
            for (int i = len - 1; i >= 0; i--){
                if (args[i] == '"'){
                    start_index = i + 1;
                    break;
                }
            }
            if (start_index == -1) start_index = 0;
        }

        int pos = 0;
        int i;
        for (i = start_index; i < len; i++){
            if (args[i] == ' ') break;
            second_value_str[pos++] = args[i];
        }
        second_value_str[pos] = '\0';
        
    } else {
        get_str_var_value(str_variables_names, str_variables_values, str_global_var_index, first, second_value_str, sizeof(second_value_str));
    }

    printf("SAXX FIRST: %s\n", first_value_str);
    printf("SAXX SECOND: %s\n", second_value_str);
    exit(-1);

    char operation;
    char end_var = args[len-1];

    for (int i = 0; i + 2 < len; i++){
        if (args[i] == ' ' && args[i+2] == ' '){
            operation = args[i+1];
            break;
        }
    }

    double result = evaluate(first_value, second_value, operation);
    
    // hitta slutvariabeln
    int create_new_var = 1;
    for (int i = 0; i < VAR_AMOUNT; i++){
        if (str_variables_names[i] == end_var){
            //str_variables_values[i] = result;
            create_new_var = 0;
            break;
        }
    }

    if (create_new_var){
        if (*str_global_var_index > VAR_AMOUNT-1){
            printf("fog:~$ ERR: Kunde inte skapa en ny variabel; Variabelminnet fullt\n");
            exit(-1);
        }
        str_variables_names[*str_global_var_index] = end_var;
        //str_variables_values[*global_var_index] = result;

        (*str_global_var_index)++;
    }

    //printf("END_VAR: %c\n", end_var);
    //printf("RESULT: %f\n", result);
}

void band(char** variables_names, double** variables_values, int* global_var_index, char* current_instruction, int debug, int* program_counter){
    if (debug >= 2) printf("%d      BAND\n", *program_counter);
    // band num x = !a + 2;
    char end_var = current_instruction[9];
    //printf("END_VAR: %c\n", end_var);
    if (current_instruction[5] == 'n' && current_instruction[6] == 'u' && current_instruction[7] == 'm'){
        char args[128];
        int i = 0;
        int len = strlen(current_instruction);

        for (i = 12; i < len; i++){
            args[i-12] = current_instruction[i];
        }
        args[i] = '\0';
        double result = evaluate_expression(args, (*variables_names), (*variables_values), global_var_index); 
        int create_new_var = 1;
        for (int j = 0; j < *global_var_index; j++){
            if ((*variables_names)[j] == end_var){
                create_new_var = 0;
                (*variables_values)[j] = result;
                break;
            }
        }
        if (create_new_var){
            if (*global_var_index > VAR_AMOUNT-1){
                char *new_names = realloc((*variables_names), (VAR_AMOUNT+64)*sizeof(char));
                double *new_vals = realloc((*variables_values), (VAR_AMOUNT+64)*sizeof(double));

                if (!new_names || !new_vals){
                    printf("fog:~$ ERR: realloc fail\n");
                    exit(1);
                }

                (*variables_names) = new_names;
                (*variables_values) = new_vals;
                VAR_AMOUNT += 64;
            }
            create_var((*variables_names), (*variables_values), global_var_index, end_var, result);
        }
    }

}



void giv1(char* variables_names, double* variables_values, int* global_var_index, char* current_instruction, int* program_counter, char instructions[][128], int instr_amount, int debug){
    if (debug >= 2) printf("%d      GIV1\n", *program_counter);
    int len = strlen(current_instruction);
    // giv1 2+1 > !a+1 (1);

    char args1[128];
    char args2[128];
    int q;
    int stop = 0;
    char operation;
    for (q = 5; q < len; q++){
        if (current_instruction[q] == '!' && current_instruction[q-1] == ' ' && current_instruction[q+1] == ' ') {
            stop = 1;
            operation = current_instruction[q];
        } 
        if (current_instruction[q] == '=' || current_instruction[q] == '<' || current_instruction[q] == '>') {
            stop = 1;
            operation = current_instruction[q];
        }
        
        
        
        if(stop) break;
        args1[q-5] = current_instruction[q];
    }
    args1[q-5] = '\0';
    q++;
    int k;
    for (k = q; k < len-3; k++){
        args2[k-(q)] = current_instruction[k];
    }
    args2[k-(q)] = '\0';

    

    char giv1_id = current_instruction[len-2];
    // printf("GIV1_ID: %c\n", giv1_id);
    //printf("ARGS1: %s\n", args1);
    //printf("ARGS2: %s\n", args2);

    double first_value = evaluate_expression(args1, variables_names, variables_values, global_var_index);
    double second_value = evaluate_expression(args2, variables_names, variables_values, global_var_index);

    if (!compare(first_value, second_value, operation)){
        for (int i = 0; i < instr_amount; i++){
            for (int j = 0; j < 5; j++){
                //printf("%c%c%c\n", instructions[(*program_counter)+i+1][j], instructions[(*program_counter)+i+1][j+1], instructions[(*program_counter)+i+1][j+2]);
                if (instructions[(*program_counter)+i+1][j] == '(' && 
                    instructions[(*program_counter)+i+1][j+2] == ')' && 
                    instructions[(*program_counter)+i+1][j+1] == giv1_id){
                    (*program_counter) = (*program_counter)+i;
                    break;
                }
            }
            
        }
    } 

}


void naer(char* variables_names, double* variables_values, int* global_var_index, char* current_instruction, int* program_counter, char instructions[][128], int instr_amount, int debug){
    // hitta argumenten till naer 
    if (debug >= 2) printf("%d      NAER\n", *program_counter);
    
    int len = strlen(current_instruction);

    char args1[128];
    char args2[128];
    int q;
    int stop = 0;
    char operation;
    for (q = 5; q < len; q++){
        if (current_instruction[q] == '!' && current_instruction[q-1] == ' ' && current_instruction[q+1] == ' ') {
            stop = 1;
            operation = current_instruction[q];
        } 
        if (current_instruction[q] == '=' || current_instruction[q] == '<' || current_instruction[q] == '>') {
            stop = 1;
            operation = current_instruction[q];
        }
        
        
        
        if(stop) break;
        args1[q-5] = current_instruction[q];
    }
    args1[q-5] = '\0';
    q++;
    int k;
    for (k = q; k < len-3; k++){
        args2[k-(q)] = current_instruction[k];
    }
    args2[k-(q)] = '\0';

    

    char naer_id = current_instruction[len-2];
    // printf("NAER_ID: %c\n", naer_id);
    //printf("        ARGS1: %s\n", args1);
    //printf("        ARGS2: %s\n", args2);

    double first_value = evaluate_expression(args1, variables_names, variables_values, global_var_index);
    double second_value = evaluate_expression(args2, variables_names, variables_values, global_var_index);

    //printf("            FIRST_VALUE: %lf\n", first_value);
    //printf("            SECOND_VALUE: %lf\n", second_value);

    if (operation == '<') second_value -= 1; // annars kör den en extra gång (lite mongo)



    if (compare(first_value, second_value, operation)){
        // kör loop
        int loop_exists = 0;
        for (int k = 0; k < loop_stack_top; k++){
            if (loop_id_stack[k] == naer_id){
                loop_exists = 1;
                break;
            }
        }
        if (!loop_exists){
            loop_id_stack[loop_stack_top] = naer_id;
            loop_start_stack[loop_stack_top] = *program_counter;
            loop_stack_top++;
        }
    } else {
        // kolla om det är första iterationen
        int loop_exists = 0;
        for (int k = 0; k < loop_stack_top; k++){
            if (loop_id_stack[k] == naer_id){
                loop_exists = 1;
                break;
            }
        }
        if (loop_exists){
            // om den redan kördes: glöm bort loopen
            loop_stack_top--;
        } else {
            // om den aldrig kördes: hoppa till slutet
            
            for (int i = 1; i < instr_amount; i++){
                for (int j = 0; j < 126; j++){
                    if (instructions[*program_counter + i][j] == '(' && 
                        instructions[*program_counter + i][j+2] == ')' && 
                        instructions[*program_counter + i][j+1] == naer_id){
                        *program_counter = *program_counter + 1; // OM LOOPLOGIKEN GICK SÖNDER: ÄNDRA 1 TILL i !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                        break;
                    }
                }
            }
        
        }
    }

}


void grip(char* variables_names, double* variables_values, int* global_var_index, char* current_instruction, int debug, int* program_counter){
    if (debug >= 2) printf("%d      GRIP\n", *program_counter);
    printf("> ");
    double result;
    int len = strlen(current_instruction);
    char end_var = current_instruction[len-1];
    scanf("%lf", &result);


    int create_new_var = 1;
    for (int i = 0; i < *global_var_index; i++){
        if (variables_names[i] == end_var){
            variables_values[i] = result;
            create_new_var = 0;
            break;
        }
    }

    if (create_new_var){
        if (*global_var_index > VAR_AMOUNT-1){
            printf("fog:~$ ERR: Kunde inte skapa en ny variabel; Variabelminnet fullt\n");
            exit(-1);
        }
        variables_names[*global_var_index] = end_var;
        variables_values[*global_var_index] = result;

        (*global_var_index)++;
    }


}


void tpos(char* current_instruction, int debug, int* program_counter) {
    if (debug >= 2) printf("%d      TPOS\n", *program_counter);
    char args[128];
    int len = strlen(current_instruction);
    int i;
    for (i = 5; i < len; i++){
        args[i-5] = current_instruction[i];
    }
    args[i-5] = '\0';

    system(args);
}


void stop(int debug, int* program_counter, char instructions[][128], int instr_amount){
    if (debug >= 2) printf("%d      SLUT\n", *program_counter);
    

    for (int i = *program_counter+1; i < instr_amount-1; i++){

        for (int j = 0; j < 128; j++){
            if (instructions[i][j] == '(' && instructions[i][j+1] == break_id){
                *program_counter = i;
            }
        }
    }
}

void call(int debug, int* program_counter, char instructions[][128], int instr_amount, char* current_instruction){
    if (debug >= 2) printf("%d      CALL\n", *program_counter);
    char func_id = current_instruction[5];
    call_start_index = (*program_counter)+1;
    for (int i = 0; i < instr_amount; i++){
        if (instructions[i][5] == '{' && instructions[i][6] == func_id && instructions[i][7] == '}'){
            (*program_counter) = i;
            break;
        }
    }

}

int main(int argc, char *argv[]){

    if (argc < 2) {
        printf("fog:~$ För få argument!\n");
        printf("fog:~$ SYNTAX: <./fog fil.fg>\n");
        return -1;
    }

    int debug = 0;
    int time = 0;
    for (int i = 0; i < argc; i++){ 
        if (!strcmp(argv[i], "-d")) {
            debug = 1;
        }
        if (!strcmp(argv[i], "-dd")) {
            debug = 2;
        }
        if (!strcmp(argv[i], "-ddd")) {
            debug = 3;
        }
        if (!strcmp(argv[i], "-t")){
            time = 1;
        }
        

    }
    

    char *buff = read_file(argv[1]);
    int buff_len = strlen(buff);
    

    // hitta antalet instr och skapa en arr med så många platser
    int instr_amount = 0;
    
    for (int i = 0; i < buff_len; i++){
        if (buff[i] == ';'){
            instr_amount++;
        }
    }

    char instructions[instr_amount][128];


    // lägg alla instr i arrayen
    int instr_count = 0;
    int last = 0;

    for (int i = 0; i < buff_len; i++) {
        if (buff[i] == ';') {
            int j = 0;
            for (int k = last; k < i; k++) {
                if (buff[k] == '\t' || buff[k] == '\n' || buff[k] == '\b' || buff[k] == '\r') continue;
                instructions[instr_count][j++] = buff[k];
            }
            instructions[instr_count][j] = '\0';
            instr_count++;
            last = i + 1;
        }
    }

    for (int i = 0; i < instr_amount; i++){ // tillåt indents (flytta varje instruktion till vänster om mellanslag i början)
        while (instructions[i][0] == ' ') {
            memmove(instructions[i], instructions[i] + 1, 63);
            instructions[i][63] = '\0';
        }
    }

    // krascha om man skriver för långa ahh rader
    for (int i = 0; i < instr_amount; i++){
        if (strlen(instructions[i]) > 128){
            printf("fog:~$ ERR: För många tecken i rad %d\n", i);
            exit(-1);
        }
    }

    
    



    if (debug){
        for (int i = 0; i < instr_amount; i++){
            printf("%d  %s\n", i, instructions[i]);
        }
        printf("----------------------\n");
    }
    


    char keywords[KEYWORD_COUNT][5] = {"foug", "boul", "band", "giv1", "naer", "grip", "tpos", "stop", "saxx", "call"};

    char *variables_names = calloc(VAR_AMOUNT, sizeof(char));
    double *variables_values = calloc(VAR_AMOUNT, sizeof(double));
    int global_var_index = 0;

    // <wip strängar>
    char str_variables_names[VAR_AMOUNT];
    char str_variables_values[VAR_AMOUNT][128];
    int str_global_var_index = 0;
    memset(str_variables_names, '\0', sizeof(str_variables_names));
    memset(str_variables_values, '\0', sizeof(str_variables_values));
    // </wip strängar>



    // starta klocka
    clock_t start, end;
    double cpu_time_used;
    if (time) {
        start = clock();
    }
    


    int program_counter;

    for (int i = instr_amount; i > 0; i--){
        if (instructions[i][0] == '{' && instructions[i][2] == '}'){
            program_counter = i+1;
            break;
        }
    }

    for (program_counter; program_counter < instr_amount; program_counter++){
        if (instructions[program_counter][0] == '{' && instructions[program_counter][2] == '}') program_counter = call_start_index;

        for (int i = 0; i < KEYWORD_COUNT; i++){
            if (instructions[program_counter][i] == '(' && instructions[program_counter][i+2] == ')'){
                char label_id = instructions[program_counter][i+1];
                if (loop_stack_top > 0 && loop_id_stack[loop_stack_top-1] == label_id){
                    program_counter = loop_start_stack[loop_stack_top-1];
                    //printf("PROGRAM COUNTER ÄNDRAD\n");
                }
            }
        }



        // skapa nuvarande instruktionen
        char current_instruction[64];

        strcpy(current_instruction, instructions[program_counter]);

        
        // hitta keywordet
        char action[5];

        int j;
        for (j = 0; j < 5; j++){
            if (instructions[program_counter][j] == ' ') continue;
            action[j] = instructions[program_counter][j];
        }
        action[4] = '\0';

        


        int action_index = -1;
        for (j = 0; j < KEYWORD_COUNT; j++){

            if (!strcmp(action, keywords[j])){
                action_index = j;
                break;
            }
        }


        switch(action_index){

            case 0: // foug;
                foug(variables_names, variables_values, current_instruction, str_variables_names, str_variables_values, &str_global_var_index, &global_var_index, debug, &program_counter);
                break;
            case 1: // boul
                boul(&variables_names, &variables_values, &global_var_index, current_instruction, debug, &program_counter, str_variables_names, str_variables_values, &str_global_var_index);
                break;
            case 2: // band
                band(&variables_names, &variables_values, &global_var_index, current_instruction, debug, &program_counter);
                break;
            case 3: // giv1
                giv1(variables_names, variables_values, &global_var_index, current_instruction, &program_counter, instructions, instr_amount, debug);
                break;
            case 4: // naer
                naer(variables_names, variables_values, &global_var_index, current_instruction, &program_counter, instructions, instr_amount, debug);
                break;
            case 5: // grip
                grip(variables_names, variables_values, &global_var_index, current_instruction, debug, &program_counter);
                break;
            case 6: // tpos
                tpos(current_instruction, debug, &program_counter);
                break;
            case 7: // stop
                stop(debug, &program_counter, instructions, instr_amount);
                break;
            case 8: // saxx
                saxx(str_variables_names, str_variables_values, &str_global_var_index, current_instruction, debug, &program_counter);
                break;
            case 9: // call
                call(debug, &program_counter, instructions, instr_amount, current_instruction);
                break;
            
        }

    }
    if (debug >= 3){
        printf("----------------------\n");
        int q = 0;
        for (q = 0; q < VAR_AMOUNT; q++){
            printf("STR_MEM %d: %c = '%s'\n", q, str_variables_names[q], str_variables_values[q]);
        }
        for (q = 0; q < VAR_AMOUNT; q++){
            printf("NUM_MEM %d: %c = %lf\n", q, variables_names[q], variables_values[q]);
        }
    }


    if (time){
        end = clock();
        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
        printf("CPU time elapsed: %f\n", cpu_time_used);
    }
    

}

