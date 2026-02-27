#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

// konstanter och globala variabler

// program counter
int program_counter = -1;

// foglangvariabler
char variables_names[128][128];
int global_var_index = 0;
double variables_values[128];

// loop stack
char loop_id_stack[128]; // hard-coded till 128 pga antalet utf-8-chars som är en byte är 128 st
int loop_program_counter_stack[128];
int loop_stack_top_id = 0;

// function stack
char function_id_stack[128]; // hard-coded till 128 pga antalet utf-8-chars som är en byte är 128 st
int function_origin_program_counter_stack[128];
double function_return_stack[128];
int function_stack_top = 0;



void create_variable(char *name, double value)
{   
    strncpy(variables_names[global_var_index], name, sizeof(variables_names[global_var_index]));
    variables_values[global_var_index] = value;
    global_var_index++;
}

double get_var_value(char* name, int length){

    int found = 0;
    for (int i = 0; i < global_var_index; i++){
        for (int j = 0; j < length; j++){
            if (name[j] == variables_names[i][j] && strlen(variables_names[i]) == length){
                found = 1;
            } else found = 0;
        }
        if (found) return variables_values[i];
    }
    
    printf("ERR: Kunde inte framställa ett variabelvärde\n");
    exit(-1);
}

double str_to_double(char *num)
{
    int len = strlen(num);
    // kolla om '-' eller '.' finns
    int negative = 0;
    int j;
    int power = len - 1;
    for (int i = 0; i < len; i++)
    {
        if (num[i] == '-')
        {
            negative = 1;
            power--;
        }
        if (num[i] == '.')
        {
            power = -1 - negative;
            for (j = 0; j < len; j++)
            {
                if (num[j] == '.')
                    break;
                power++;
            }
        }
    }

    double sum = 0;

    for (int i = negative; i < len; i++)
    {
        if (num[i] == '.' || num[i] == ' ')
            continue;
        sum += (num[i] - '0') * pow(10, power);

        power--;
    }

    if (negative)
        sum = sum * -1;

    return sum;
}
enum tok_type    // (num+(num*num)\0
{                // (num+§§num§§)\0
    NONE,        // 0
    TERMINATOR,  // 1
    FOUG,        // 2
    BAND,        // 3
    GIVET,       // 4
    ATT,         // 5
    NAER,         // 6
    NUM,         // 7
    STRING,      // 8
    RIGHT_PAR,   // 9
    LEFT_PAR,    // 10
    VARIABLE,    // 11
    EQUALS,      // 12
    NUMBER,      // 13
    LOOP_MARKER, // 14
    PLUS,        // 15
    MINUS,       // 16
    MULTIPLIED,  // 17
    DIVIDED,     // 18
    EXPONENT,    // 19
    MODULO,      // 20
    GREATER_THAN,// 21
    LESS_THAN,   // 22
    NOT_EQUAL_TO,// 23
    FUNCTION,    // 24
    RETURN,      // 25
    MAIN         // 26
};

typedef struct
{
    int is_variable;
    int is_function;
    char *name;
    int name_len;

} Variable;

typedef struct
{
    double value;
    int type;
    char loop_id;
    Variable var;
} Token;

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

void print_tokens(Token instructions[][128], int instruction_amount)
{
    for (int i = 0; i < instruction_amount; i++)
    {
        printf("%d:     ", i);
        for (int j = 0; instructions[i][j-1].type != TERMINATOR; j++)
        {
            printf("%d  ", instructions[i][j].type);
        }
        printf("\n");
    }
    printf("\n");
    for (int i = 0; i < instruction_amount; i++)
    {
        printf("%d:     ", i);
        for (int j = 0; instructions[i][j-1].type != TERMINATOR; j++)
        {
            switch (instructions[i][j].type)
            {
            case FOUG:
                printf("'FOUG'    ");
                break;
            case BAND:
                printf("'BAND'    ");
                break;
            case GIVET:
                printf("'GIVET'    ");
                break;
            case ATT:
                printf("'ATT'    ");
                break;
            case NAER:
                printf("'NAER'    ");
                break;
            case RIGHT_PAR:
                printf("')'    ");
                break;
            case LEFT_PAR:
                printf("'('    ");
                break;
            case LOOP_MARKER:
                printf("'{%c}'    ", instructions[i][j].loop_id);
                break;
            case PLUS:
                printf("'+'    ");
                break;
            case MINUS:
                printf("'-'    ");
                break;
            case MULTIPLIED:
                printf("'*'    ");
                break;
            case DIVIDED:
                printf("'/'    ");
                break;
            case EXPONENT:
                printf("'^'    ");
                break;
            case MODULO:
                printf("'%%'    ");
                break;
            case VARIABLE:
                printf("'");
                if (instructions[i][j].var.is_function) printf("f ");
                for (int k = 0; k < instructions[i][j].var.name_len; k++)
                {
                    printf("%c", *(instructions[i][j].var.name + k));
                }
                printf("'    ");
                break;
            case STRING:
                printf("'");
                for (int k = 0; k < instructions[i][j].var.name_len; k++)
                {
                    printf("%c", *(instructions[i][j].var.name + k));
                }
                printf("'    ");
                break;
            case EQUALS:
                printf("'='    ");
                break;
            case NOT_EQUAL_TO:
                printf("'!='    ");
                break;
            case GREATER_THAN:
                printf("'>'    ");
                break;
            case LESS_THAN:
                printf("'<'    ");
                break;
            case NUMBER:
                printf("'%lf'    ", instructions[i][j].value);
                break;
            case TERMINATOR:
                printf("'\\0'    ");
                break;
            case FUNCTION:
                printf("'FUNC'    ");
                break;
            case RETURN:
                printf("'RETURN'    ");
                break;
            case MAIN:
                printf("'MAIN'    ");
                break;
            }
        }
        printf("\n");
    }
    printf("\n");
}

typedef struct
{
    Token (*data)[128];
    int instruction_amount;
} Program;

Program tokenize(char *file_name)
{
    char *buff = read_file(file_name);
    int buff_len = strlen(buff);
    printf("BUFF: -----------------------------------------------\n%s\n-----------------------------------------------\n", buff);

    // räkna antal instr
    int instruction_amount = 0;
    for (int i = 0; i < buff_len; i++)
    {
        if (buff[i] == ';')
            instruction_amount++;
    }
    printf("[DEBUG] instruction_amount: %d\n", instruction_amount);

    // skapa instruktionsarray
    Token(*instructions)[128] = calloc(instruction_amount, sizeof(*instructions));
    if (instructions == NULL) {
        printf("Malloc failed\n");
        exit(-1);
    }
    printf("[DEBUG] instructions ptr: %p\n", instructions);

    int i = 0;
    int instructions_OUTER_arr_index = 0;
    int instructions_INNER_arr_index = 0;

    while (i < buff_len)
    {
        while (i < buff_len && (buff[i] == ' ' || buff[i] == '\n' || buff[i] == '\r' || buff[i] == '\t'))
            i++;

        if (i >= buff_len)
            break;

        Token tok;
        tok.type = 0;

        // ord-tokens (FOUG, BAND, etc.)
        if (strncmp(&buff[i], "foug", 4) == 0) { tok.type = FOUG; i += 4; }
        else if (strncmp(&buff[i], "band", 4) == 0) { tok.type = BAND; i += 4; }
        else if (strncmp(&buff[i], "givet", 5) == 0) { tok.type = GIVET; i += 5; }
        else if (strncmp(&buff[i], "att", 3) == 0) { tok.type = ATT; i += 3; }
        else if (strncmp(&buff[i], "naer", 4) == 0) { tok.type = NAER; i += 4; }
        else if (strncmp(&buff[i], "func", 4) == 0) { tok.type = FUNCTION; i += 4; }
        else if (strncmp(&buff[i], "return", 6) == 0) { tok.type = RETURN; i += 6; }
        else if (strncmp(&buff[i], "main", 4) == 0) { tok.type = MAIN; i += 4; }
        else if (buff[i] == '"') {
            i++;
            int start = i;
            while (i < buff_len && buff[i] != '"') i++;
            tok.type = STRING;
            tok.var.name = &buff[start];
            tok.var.name_len = i - start;
            i++;
        }
        else if (buff[i] == '(') { tok.type = LEFT_PAR; i++; }
        else if (buff[i] == ')') { tok.type = RIGHT_PAR; i++; }
        else if (i + 2 < buff_len && buff[i] == '{' && buff[i+2] == '}') {
            tok.type = LOOP_MARKER;
            tok.loop_id = buff[i+1];

            printf("[DEBUG] Found LOOP_MARKER: {%c} at instructions[%d][%d]\n", tok.loop_id, instructions_OUTER_arr_index, instructions_INNER_arr_index);
            i+=3;
        }
        else if (buff[i] == '=') { tok.type = EQUALS; i++; }
        else if (buff[i] == '>') { tok.type = GREATER_THAN; i++; }
        else if (buff[i] == '<') { tok.type = LESS_THAN; i++; }
        else if (buff[i] == '!' && buff[i+1] == '=') { tok.type = NOT_EQUAL_TO; i+=2; }
        else if (buff[i] == '+') { tok.type = PLUS; i++; }
        else if (buff[i] == '-') { tok.type = MINUS; i++; }
        else if (buff[i] == '*') { tok.type = MULTIPLIED; i++; }
        else if (buff[i] == '/') { tok.type = DIVIDED; i++; }
        else if (buff[i] == '^') { tok.type = EXPONENT; i++; }
        else if (buff[i] == '%') { tok.type = MODULO; i++; }
        else if (buff[i] == ';') {
            tok.type = TERMINATOR;
            instructions[instructions_OUTER_arr_index][instructions_INNER_arr_index++] = tok;
            printf("[DEBUG] TERMINATOR at instructions[%d][%d]\n", instructions_OUTER_arr_index, instructions_INNER_arr_index-1);
            i++;
            instructions_INNER_arr_index = 0;
            instructions_OUTER_arr_index++;
            continue;
        }
        else {
            if (buff[i] >= '0' && buff[i] <= '9') {
                int start = i;
                while (i < buff_len && ((buff[i] >= '0' && buff[i] <= '9') || buff[i] == '.')) i++;
                char args[i-start+1];
                for (int k=start; k<i; k++) args[k-start] = buff[k];
                args[i-start] = '\0';
                tok.type = NUMBER;
                tok.value = str_to_double(args);
            } else {
                tok.type = VARIABLE;
                int start = i;
                while (i < buff_len && ((buff[i] >= 'a' && buff[i] <= 'z') || (buff[i] >= 'A' && buff[i] <= 'Z') || (buff[i] >= '0' && buff[i] <= '9') || buff[i] == '_'))
                    i++;
                Variable variable_info = {1, 0, &buff[start], i-start};
                tok.var = variable_info;
            }
        }

        if (tok.type != TERMINATOR) {
            instructions[instructions_OUTER_arr_index][instructions_INNER_arr_index++] = tok;
            printf("[DEBUG] Added token type %d at instructions[%d][%d]\n", tok.type, instructions_OUTER_arr_index, instructions_INNER_arr_index-1);
        }
    }

    // sätt funktionerna till funktioner
    for (int i = 0; i < instruction_amount; i++){
        if (instructions[i][0].type == FUNCTION) {
            instructions[i][1].var.is_function = 1;
            printf("[DEBUG] Found token FUNCTION at instructions[%d][0]\n", i);
            // sätt funktionsflaggan på alla med samma namn
            for (int j = i+1; j < instruction_amount; j++){ // rad-loop
                for (int k = 0; instructions[j][k].type != TERMINATOR; k++){ // token-loop
                    if (instructions[j][k].type == VARIABLE){
                        int size;
                        if (instructions[j][k].var.name_len >= instructions[i][1].var.name_len) size = instructions[j][k].var.name_len;
                        else size = instructions[i][1].var.name_len;
                        if (!strncmp(instructions[j][k].var.name, instructions[i][1].var.name, size)) {
                            instructions[j][k].var.is_function = 1;
                        }
                    }
                }
            }

        }
    }

    Program program = { instructions, instruction_amount };
    printf("[DEBUG] Tokenize finished. Program.data: %p, instruction_amount: %d\n", program.data, program.instruction_amount);
    return program;
}

void interpret_instruction(Token* current, Token(*instructions)[128], int instruction_amount);

double call_function(char* name, int name_len, int origin_program_counter, Token(*instructions)[128], int instruction_amount);

double evaluate_expression(Token *args_old, int args_amount, Token(*instructions)[128], int instruction_amount)
{

    /*for (int i = 0; i < args_amount; i++)
    {
        printf("TYPE: %d,    ", args[i].type, args[i].value);
    }
    printf("\n");

    for (int i = 0; i < args_amount; i++)
    {
        if (args[i].type == NUMBER)
            printf("VALUE: %lf,    ", args[i].value);
    }
    printf("\n");
    printf("\n");*/

    Token args[args_amount];
    memcpy(args, args_old, args_amount*sizeof(Token)); // av någon skum anledning måste den ha en lokal kopia

    // ta bort variablerna
    for (int i = 0; i < args_amount; i++){
        if (args[i].type == VARIABLE && !args[i].var.is_function){
            double val = get_var_value(args[i].var.name, args[i].var.name_len);
            args[i].value = val;
            args[i].type = NUMBER;
        }
    }

    // byt ut funktioner mot värden
    for (int i = 0; i < args_amount; i++){
        if (args[i].type == VARIABLE && args[i].var.is_function){
            double value = call_function(args[i].var.name, args[i].var.name_len, program_counter, instructions, instruction_amount);
            args[i].type = NUMBER;
            args[i].value = value;
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
        //printf("VALID_TOKEN_COUNT: %d\n", valid_token_count);
        if (valid_token_count <= 2) // pga terminator räknas som en
        {
            for (int i = 0; i < args_amount; i++)
            {
                if (args[i].type == NUMBER){
                    //printf("RETURNADE %lf\n", args[i].value);
                    return args[i].value;
                }
                    
            }
        }

        // hitta paranteser
        int start_par_index = -1;
        int stop_par_index = -1;
        for (int i = args_amount-1; i >= 0; i--)
        {
            if (args[i].type == LEFT_PAR)
            {
                start_par_index = i;
                while (args[i].type != RIGHT_PAR){
                    if (i == args_amount) {
                        printf("ERR: Slutparantes hittades inte\n");
                        exit(-1);
                    }
                    i++;
                }
                    
                stop_par_index = i;
                break;
            }
        }
        //printf("START: %d, STOP: %d\n", start_par_index, stop_par_index);

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

                //printf("FIRST: %lf, SECOND: %lf\n", first_arg, second_arg);
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

                //printf("FIRST: %lf, SECOND: %lf         %d\n", first_arg, second_arg, args[i].type);

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

void band(Token* instruction, Token(*instructions)[128], int instruction_amount){
    // hitta slutvariabeln
    Token end_var = instruction[1];

    char end_var_name[end_var.var.name_len + 1];
    memcpy(end_var_name, end_var.var.name, end_var.var.name_len);
    end_var_name[end_var.var.name_len] = '\0';


    // hitta hur många tokens den ska evaluata
    int length = 0;
    for (int i = 3; instruction[i].type != TERMINATOR; i++)
        length++;
    //printf("length: %d\n", length);

    double value;
    Token args[length]; // skapa en array med argumenten
    for (int i = 0; i < length; i++){
        args[i] = instruction[i+3];
    }
    /*for (int i = 0; i < length; i++){
        printf("%d  ", args[i].type);
    }
    printf("\n");*/
    if (instruction[2].type == EQUALS){
        value = evaluate_expression(args, length, instructions, instruction_amount);
    } else {
        printf("ERR: Syntax error, band\n");
        exit(-1);
    }
    
    int var_exists = 0;
    for (int i = 0; i < global_var_index; i++){
        //printf("end_var: %s, var_names[i]: %s\n", end_var_name, variables_names[i]);
        if (!strcmp(end_var_name, variables_names[i])){
            //printf("HITTADE: '%s'\n", variables_names[i]);
            variables_values[i] = value;
            var_exists = 1;
        }
    }
    if (!var_exists){
        create_variable(end_var_name, value);
    } 
}

void foug(Token* instruction){
    //printf("FOUG KALLAD PÅ\n");
    if (instruction[1].type == STRING){
        //printf("STRING I FOUG\n");
        for (int i = 0; i < instruction[1].var.name_len; i++){
            if (instruction[1].var.name[i] == '\\' && instruction[1].var.name[i+1] == 'n') {
                printf("\n");
                i+=2;
            }
            if (i < instruction[1].var.name_len)
            printf("%c", instruction[1].var.name[i]);
        }
    } else if (instruction[1].type == VARIABLE){
        //printf("VARIABLE I FOUG\n");
        double value = get_var_value(instruction[1].var.name, instruction[1].var.name_len);
        if ((int)value == value)
            printf("%d", (int)value);
        else 
            printf("%lf", value);
    } else {
        printf("ERR: Foug: Syntax error\n");
        exit(-1);
    }
}

void givet(Token* instruction, Program program){
    
    // hitta argumenten
    int i = 2;
    while (instruction[i].type != EQUALS && instruction[i].type != GREATER_THAN && instruction[i].type != LESS_THAN && instruction[i].type != NOT_EQUAL_TO)
        i++;
    int operation = instruction[i].type;
    
    Token left_args[i-2];
    int left_args_length = i-2;
    int j = i+1;
    while (instruction[i].type != LOOP_MARKER) 
        i++;

    Token right_args[i-j-1];
    int right_args_length = i-j-1;
    
    // kopiera argumenten
    // vänster
    for (int k = 2; k < left_args_length+2; k++){
        left_args[k-2] = instruction[k];
    }
    // höger
    for (int k = j; k < right_args_length+j; k++){
        right_args[k-j] = instruction[k];
    }

    double left_value = evaluate_expression(left_args, left_args_length, program.data, program.instruction_amount);
    double right_value = evaluate_expression(right_args, right_args_length+1, program.data, program.instruction_amount);
    int do_statement = 0;

    switch (operation){
        case EQUALS:
            if (left_value == right_value) do_statement = 1;
            break;

        case GREATER_THAN:
            if (left_value > right_value) do_statement = 1;
            break;

        case LESS_THAN:
            if (left_value < right_value) do_statement = 1;
            break;

        case NOT_EQUAL_TO:
            if (left_value != right_value) do_statement = 1;
            break;
    }
    if (!do_statement){
        int k = 0;
        while (instruction[k].type != TERMINATOR)
            k++;
        char givet_id = instruction[k-1].loop_id;

        for (int k = program_counter; k < program.instruction_amount; k++){ // kolla varje rad
            for (int l = 0; program.data[k][l].type != TERMINATOR; l++){ // kolla varje token i raden
                if (program.data[k][l].type == LOOP_MARKER && program.data[k][l].loop_id == givet_id){
                    program_counter = k+1;
                    return;
                }
            }
            
        }
        
    }   

}

void naer(Token* instruction, Token(*instructions)[128], int instruction_amount){
    //printf("[DEBUG] Entered NAER, program_counter: %d\n", program_counter);
    int i = 1;
    while (instruction[i].type != EQUALS && instruction[i].type != GREATER_THAN && instruction[i].type != LESS_THAN && instruction[i].type != NOT_EQUAL_TO && i < 128)
        i++;
    int operation = instruction[i].type;
    i++;

    int left_args_length = i-2;
    int j = i;
    while (instruction[i].type != LOOP_MARKER && i < 128) i++;
    int right_args_length = i-j;

    //printf("[DEBUG] left_args_length: %d, right_args_length: %d\n", left_args_length, right_args_length);

    Token left_args[left_args_length];
    Token right_args[right_args_length];

    if (left_args_length > 0) memcpy(left_args, instruction+1, left_args_length*sizeof(Token));
    if (right_args_length > 0) memcpy(right_args, instruction+j, right_args_length*sizeof(Token));

    //for (int k=0; k<left_args_length; k++) printf("[DEBUG] LEFT %d: %d\n", k, left_args[k].type);
    //for (int k=0; k<right_args_length; k++) printf("[DEBUG] RIGHT %d: %d\n", k, right_args[k].type);

    double left_value = evaluate_expression(left_args, left_args_length, instructions, instruction_amount);
    double right_value = evaluate_expression(right_args, right_args_length, instructions, instruction_amount);

    //printf("[DEBUG] NAER left: %lf, right: %lf\n", left_value, right_value);

    int do_statement = 0;
    switch (operation){
        case EQUALS: if(left_value==right_value) do_statement=1; break;
        case GREATER_THAN: if(left_value>right_value) do_statement=1; break;
        case LESS_THAN: if(left_value<right_value) do_statement=1; break;
        case NOT_EQUAL_TO: if(left_value!=right_value) do_statement=1; break;
    }

    int k = 0;
    while (instruction[k].type != TERMINATOR && k<128) k++;
    char loop_id = 0;
    if (k>0 && instruction[k-1].type==LOOP_MARKER) loop_id = instruction[k-1].loop_id;
    //printf("[DEBUG] NAER loop_id: %c\n", loop_id);

    if (!do_statement) {
        //printf("[DEBUG] Condition false, jumping\n");
        //printf("[DEBUG] Looking for loop_id: %c\n", loop_id);

        for (k = program_counter + 1; k < instruction_amount; k++) {
            int l = 0;
            while (instructions[k][l].type != TERMINATOR) {
                if (instructions[k][l].type == LOOP_MARKER) {
                    if (instructions[k][l].loop_id == loop_id) {
                        //printf(" -> MATCH! Jumping to instruction %d\n", k);
                        program_counter = k;
                        return;
                    }
                }
                //printf("\n");
                l++;
            }
        }
        //printf("[DEBUG] Did not find matching loop_id!\n");
    } else {
        //printf("[DEBUG] Condition true, pushing loop\n");
        int loop_already_exists = 0;
        for (int l=0; l<loop_stack_top_id; l++) if(loop_id_stack[l]==loop_id) loop_already_exists=1;
        if(!loop_already_exists){
            loop_id_stack[loop_stack_top_id]=loop_id;
            loop_program_counter_stack[loop_stack_top_id++]=program_counter;
        }
    }
}


double call_function(char* name, int name_len, int origin_program_counter, Token(*instructions)[128], int instruction_amount){
    int func_index = -1;
    for (int i = 0; i < instruction_amount; i++){
        if (instructions[i][0].type == FUNCTION){
            size_t size = (instructions[i][1].var.name_len >= name_len) ? instructions[i][1].var.name_len : name_len;
            if (!strncmp(instructions[i][1].var.name, name, size)){
                func_index = i;
                break;
            }
        }
    }
    if (func_index == -1){
        printf("ERR: Ofärdig funktion\n");
        exit(-1);
    }

    int call_stack_level = function_stack_top;
    function_origin_program_counter_stack[function_stack_top] = origin_program_counter;
    function_return_stack[function_stack_top] = 0;
    function_stack_top++;

    program_counter = func_index + 1; // börja precis efter "func"

    
    while (function_stack_top > call_stack_level) {
        if (program_counter >= instruction_amount) {
            printf("ERR: Funktion nådde filslut utan return\n");
            exit(-1);
        }
        Token* current = instructions[program_counter];
        interpret_instruction(current, instructions, instruction_amount);
        program_counter++;
    }
    program_counter--; // program_counter inkrementeras 2 ggr annars

    double ret = function_return_stack[call_stack_level];
    return ret;
}

// spara längden av funktionen (antal rader)
// interpreta den mängden instruktioner
// hoppa tillbaka till origin

void interpret_instruction(Token* current, Token(*instructions)[128], int instruction_amount)
{
    switch (current[0].type)
    {
        case FOUG:
            foug(current);
            break;

        case BAND:
            band(current, instructions, instruction_amount);
            break;

        case GIVET:
            givet(current, (Program){instructions, instruction_amount});
            break;

        case NAER:
            naer(current, instructions, instruction_amount);
            break;

        case LOOP_MARKER:
            for (int i = 0; i < loop_stack_top_id; i++) {
                if (loop_id_stack[i] == current[0].loop_id) {
                    program_counter = loop_program_counter_stack[i]-1;
                    break;
                }
            }
            break;

        case RETURN:
        {
            double return_value = 0;

            if (current[1].type == NUMBER)
                return_value = current[1].value;
            else if (current[1].type == VARIABLE)
                return_value = get_var_value(current[1].var.name, current[1].var.name_len);
            
                //printf("RETURN: %lf\n", return_value);

            function_return_stack[function_stack_top - 1] = return_value;

            program_counter = function_origin_program_counter_stack[function_stack_top - 1];

            function_stack_top--;

            break;
        }

        case FUNCTION:
            // gör inget när man bara passerar definitionen
            break;

        case MAIN:
            break;
    }
}

int main(int argc, char** argv)
{

    if (argc < 2) {
        printf("fog:~$ För få argument!\n");
        printf("fog:~$ SYNTAX: <./fog fil.fg>\n");
        return -1;
    }


    Program program = tokenize(argv[1]);
    Token(*instructions)[128] = program.data;
    int instruction_amount = program.instruction_amount;
    print_tokens(instructions, instruction_amount);

    // hitta entry point (main)
    for (int i = 0; i < instruction_amount; i++){
        if (instructions[i][0].type == MAIN) program_counter = i;
    }
    if (program_counter == -1) {
        printf("ERR: main inte hittad\n");
        exit(-1);
    }

    while (program_counter < instruction_amount)
    {
        Token* current = instructions[program_counter];

        interpret_instruction(current, instructions, instruction_amount);

        program_counter++;
    }    
    return 0;
}