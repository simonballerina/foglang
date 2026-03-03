#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

// konstanter och globala variabler

    // program counter
    int program_counter = -1;

    

    // foglangvariabler
    enum var_type {
        VAR_NONE,
        VAR_NUMBER,
        VAR_STRING,
        VAR_LIST,
        VAR_FUNCTION,
        VAR_LIST_NUMBER,
        VAR_LIST_STRING
    };

    typedef struct {
        int type;
        char* name;
        int name_len;
        double value;
        int mem_ptr;  // adress i variables arr
        int len;  // längden på listan / strängen
        char* str_ptr;
    } Variable;

    Variable* variables;
    int var_index = 0;

    char (*variables_names)[128];
    double* variables_values;
    int global_var_index = 0;
    int variables_capacity = 128;

    // loop stack
    char* loop_id_stack;
    int* loop_program_counter_stack;
    int loop_stack_top_id = 0;
    int loop_stack_capacity = 128;

    // function stack
    int* function_origin_program_counter_stack;
    double* function_return_stack;
    int function_stack_top = 0;
    int function_stack_capacity = 128;

typedef struct // 12 bytes?
{
    char *name;
    int type;
    int name_len;

} Tok_Variable;

typedef struct
{
    char loop_id;
    double value;
    int type;
    Tok_Variable var;
} Token;

enum tok_type     
{                 
    NONE,         // 0
    TERMINATOR,   // 1
    FOUG,         // 2
    BAND,         // 3
    GIVET,        // 4
    ATT,          // 5
    NAER,         // 6
    NUM,          // 7
    STRING,       // 8
    RIGHT_PAR,    // 9
    LEFT_PAR,     // 10
    VARIABLE,     // 11
    EQUALS,       // 12
    NUMBER,       // 13
    LOOP_MARKER,  // 14
    PLUS,         // 15
    MINUS,        // 16
    MULTIPLIED,   // 17
    DIVIDED,      // 18
    EXPONENT,     // 19
    MODULO,       // 20
    GREATER_THAN, // 21
    LESS_THAN,    // 22
    NOT_EQUAL_TO, // 23
    FUNCTION,     // 24
    RETURN,       // 25
    MAIN,         // 26
    SVETS,        // 27
    LEFT_BRACKET, // 28
    RIGHT_BRACKET,// 29
    COMMA         // 30
};

void create_variable(int type, char* name, int name_len, double value, Token* values, char* string, int list_len){
    size_t var_size;
    int mem_ptr;

    printf("\nSkapar variabel: \nType: %d\nName: %c\nName_len: %d\nValue: %lf\nLen: %d\nString: '", type, name[0], name_len, value, list_len);
    for (int i = 0; i < list_len; i++){
        printf("%c", string[i]);
    }
    printf("'\n\n");

    Variable var = {
        .name = name,
        .name_len = name_len,
        .value = 0,
        .type = VAR_NONE,
        .len = list_len,
        .mem_ptr = 0,
        .str_ptr = 0
    };
    if (type == VAR_NUMBER){
        var_size = 1;

        var.type = VAR_NUMBER;
        var.value = value;

        if (var_index >= variables_capacity){ // kolla att strl är ok
            variables = realloc(variables, variables_capacity+var_size+64);
            variables_capacity+=var_size+64;
            if (variables == NULL){
                printf("ERR: Minnesallokering misslyckades\n");
                exit(1);
            }
        }
        variables[var_index++] = var;

    } else if (type == VAR_LIST) {
        printf("VAR_LIST\n");
        var_size = 1+list_len; // grundvariabeln + strängen/listan
        mem_ptr = var_index+1; // strängen/listan kommer efter grundvariabeln
        var.mem_ptr = mem_ptr;
        var.len = list_len;
        if (type == VAR_STRING)
            var.type = VAR_STRING;
        else 
            var.type = VAR_LIST;

        if (var_index >= variables_capacity){ // kolla att strl är ok
            variables = realloc(variables, variables_capacity+var_size+64);
            variables_capacity+=var_size+64;
            if (variables == NULL){
                printf("ERR: Minnesallokering misslyckades\n");
                exit(1);
            }
        }
        variables[var_index++] = var;
        // lägg till sträng/lista 
        for (int i = 0; i < list_len; i++){
            if (values[i].type == COMMA || values[i].type == RIGHT_BRACKET) continue;
            
            Variable list_var = {
                .name = 0,
                .name_len = 0,
                .type = VAR_NONE,
                .value = 0,
                .len = 0,
                .mem_ptr = 0,
                .str_ptr = 0
            };

            if (values[i].type == NUMBER) {

                list_var.value = values[i].value;
                list_var.type = VAR_LIST_NUMBER;
                variables[var_index++] = list_var;

            } else if (values[i].type == STRING){

                list_var.len = values[i].var.name_len;
                char* list_str_ptr = malloc(values[i].var.name_len*sizeof(char));
                if (list_str_ptr == NULL){
                    printf("ERR: Minnesallokering misslyckades\n");
                    exit(1);
                }
                list_var.str_ptr = list_str_ptr;
                memcpy(list_str_ptr, values[i].var.name, values[i].var.name_len*sizeof(char));
                list_var.type = VAR_LIST_STRING;
                variables[var_index++] = list_var;
            }

        } 
   
    
    } else if (type == VAR_STRING){
        // reservera minne till strängen
        char* string_ptr = malloc(list_len*sizeof(char)); 
        if (string_ptr == NULL){
            printf("string_ptr == NULL\n");
            printf("ERR: Minnesallokering misslyckades\n");
            exit(1);
        }
        printf("str_ptr: 0x%x\n", string_ptr);

        // spara inputsträngen i nyreserverade området
        memcpy(string_ptr, string, list_len*sizeof(char));

        var.str_ptr = string_ptr;
        var.type = type;
        if (var_index >= variables_capacity){ // kolla att strl är ok
            variables = realloc(variables, variables_capacity+var_size+64);
            variables_capacity+=var_size+64;
            if (variables == NULL){
                printf("ERR: Minnesallokering misslyckades\n");
                exit(1);
            }
        }
        variables[var_index++] = var;

    } else {
        printf("ERR: Felaktig variabeltyp\n");
        exit(-1);
    }


}

void create_variable_old(char *name, double value)
{
    if (global_var_index >= variables_capacity) {
        variables_values = realloc(variables_values, variables_capacity+64);
        variables_names = realloc(variables_names, (128+64)*sizeof*variables_names);
        variables_capacity += 64;
        if (variables_values == NULL || variables_names == NULL){
            printf("ERR: Minnesallokering misslyckades\n");
            exit(1);
        }
    }
    strncpy(variables_names[global_var_index], name, sizeof(variables_names[global_var_index]));
    variables_values[global_var_index] = value;
    global_var_index++;
}

double get_var_value(char *name, int length)
{

    int found = 0;
    for (int i = 0; i < global_var_index; i++)
    {
        for (int j = 0; j < length; j++)
        {
            if (name[j] == variables_names[i][j] && strlen(variables_names[i]) == length)
            {
                found = 1;
            }
            else
                found = 0;
        }
        if (found)
            return variables_values[i];
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
        for (int j = 0; instructions[i][j].type != TERMINATOR; j++)
        {
            printf("%d  ", instructions[i][j].type);
        }
        printf("\n");
    }
    printf("\n");
    for (int i = 0; i < instruction_amount; i++)
    {
        printf("%d:     ", i);
        for (int j = 0; instructions[i][j].type != TERMINATOR; j++)
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
            case RIGHT_BRACKET:
                printf("']'    ");
                break;
            case LEFT_BRACKET:
                printf("'['    ");
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
                if (instructions[i][j].var.type == VAR_FUNCTION)
                    printf("f ");
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
                printf("'BOUL'    ");
                break;
            case RETURN:
                printf("'RETURN'    ");
                break;
            case MAIN:
                printf("'MAIN'    ");
                break;
            case SVETS:
                printf("'SVETS'    ");
                break;
            case COMMA:
                printf("','    ");
                break;
            }
        }
        printf("\n");
    }
    printf("\n");
}

void print_variables(){
    for (int i = 0; i < var_index; i++){
        printf("%i: Type: %d    Name: ", i, variables[i].type);
        // printa namn
        for (int j = 0; j < variables[i].name_len; j++){
            printf("%c", variables[i].name[j]);
        }
        printf("    Value: %lf    List_size: %d    List_begin: %d   String: '", variables[i].value, variables[i].len, variables[i].mem_ptr);

        if (variables[i].str_ptr != 0){
            for (int j = 0; j < variables[i].len; j++){
                printf("%c", variables[i].str_ptr[j]);
            }
        }

        printf("'\n");
    }
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
    if (instructions == NULL)
    {
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

        // ord-tokens 
        if (strncmp(&buff[i], "foug", 4) == 0)
        {
            tok.type = FOUG;
            i += 4;
        }
        else if (strncmp(&buff[i], "svets", 5) == 0)
        {
            tok.type = SVETS;
            i += 5;
        }
        else if (strncmp(&buff[i], "band", 4) == 0)
        {
            tok.type = BAND;
            i += 4;
        }
        else if (strncmp(&buff[i], "givet", 5) == 0)
        {
            tok.type = GIVET;
            i += 5;
        }
        else if (strncmp(&buff[i], "att", 3) == 0)
        {
            tok.type = ATT;
            i += 3;
        }
        else if (strncmp(&buff[i], "naer", 4) == 0)
        {
            tok.type = NAER;
            i += 4;
        }
        else if (strncmp(&buff[i], "boul", 4) == 0)
        {
            tok.type = FUNCTION;
            i += 4;
        }
        else if (strncmp(&buff[i], "return", 6) == 0)
        {
            tok.type = RETURN;
            i += 6;
        }
        else if (strncmp(&buff[i], "main", 4) == 0)
        {
            tok.type = MAIN;
            i += 4;
        }
        else if (buff[i] == '"')
        {
            i++;
            int start = i;
            while (i < buff_len && buff[i] != '"')
                i++;
            tok.type = STRING;
            tok.var.name = &buff[start];
            tok.var.name_len = i - start;
            i++;
        }
        else if (buff[i] == '(')
        {
            tok.type = LEFT_PAR;
            i++;
        }
        else if (buff[i] == ')')
        {
            tok.type = RIGHT_PAR;
            i++;
        }
        else if (buff[i] == '[')
        {
            tok.type = LEFT_BRACKET;
            i++;
        }
        else if (buff[i] == ']')
        {
            tok.type = RIGHT_BRACKET;
            i++;
        }
        else if (buff[i] == ',')
        {
            tok.type = COMMA;
            i++;
        }
        else if (i + 2 < buff_len && buff[i] == '{' && buff[i + 2] == '}')
        {
            tok.type = LOOP_MARKER;
            tok.loop_id = buff[i + 1];

            printf("[DEBUG] Found LOOP_MARKER: {%c} at instructions[%d][%d]\n", tok.loop_id, instructions_OUTER_arr_index, instructions_INNER_arr_index);
            i += 3;
        }
        else if (buff[i] == '=')
        {
            tok.type = EQUALS;
            i++;
        }
        else if (buff[i] == '>')
        {
            tok.type = GREATER_THAN;
            i++;
        }
        else if (buff[i] == '<')
        {
            tok.type = LESS_THAN;
            i++;
        }
        else if (buff[i] == '!' && buff[i + 1] == '=')
        {
            tok.type = NOT_EQUAL_TO;
            i += 2;
        }
        else if (buff[i] == '+')
        {
            tok.type = PLUS;
            i++;
        }
        else if (buff[i] == '-')
        {
            tok.type = MINUS;
            i++;
        }
        else if (buff[i] == '*')
        {
            tok.type = MULTIPLIED;
            i++;
        }
        else if (buff[i] == '/')
        {
            tok.type = DIVIDED;
            i++;
        }
        else if (buff[i] == '^')
        {
            tok.type = EXPONENT;
            i++;
        }
        else if (buff[i] == '%')
        {
            tok.type = MODULO;
            i++;
        }
        else if (buff[i] == ';')
        {
            tok.type = TERMINATOR;
            instructions[instructions_OUTER_arr_index][instructions_INNER_arr_index++] = tok;
            printf("[DEBUG] TERMINATOR at instructions[%d][%d]\n", instructions_OUTER_arr_index, instructions_INNER_arr_index - 1);
            i++;
            instructions_INNER_arr_index = 0;
            instructions_OUTER_arr_index++;
            continue;
        }
        else
        {
            if (buff[i] >= '0' && buff[i] <= '9')
            {
                int start = i;
                while (i < buff_len && ((buff[i] >= '0' && buff[i] <= '9') || buff[i] == '.'))
                    i++;
                char args[i - start + 1];
                for (int k = start; k < i; k++)
                    args[k - start] = buff[k];
                args[i - start] = '\0';
                tok.type = NUMBER;
                tok.value = str_to_double(args);
            }
            else
            {
                tok.type = VARIABLE;
                int start = i;
                while (i < buff_len && ((buff[i] >= 'a' && buff[i] <= 'z') || (buff[i] >= 'A' && buff[i] <= 'Z') || (buff[i] >= '0' && buff[i] <= '9') || buff[i] == '_'))
                    i++;
                Tok_Variable variable_info = {&buff[start], VAR_NONE, i-start};
                tok.var = variable_info;
            }
        }



        if (tok.type != TERMINATOR)
        {
            instructions[instructions_OUTER_arr_index][instructions_INNER_arr_index++] = tok;
            printf("[DEBUG] Added token type %d at instructions[%d][%d]\n", tok.type, instructions_OUTER_arr_index, instructions_INNER_arr_index - 1);
        }
    }

    // sätt funktionerna till funktioner
    for (int i = 0; i < instruction_amount; i++)
    {
        if (instructions[i][0].type == FUNCTION)
        {
            instructions[i][1].var.type = VAR_FUNCTION;
            printf("[DEBUG] Found token FUNCTION at instructions[%d][0]\n", i);
            // sätt funktionsflaggan på alla med samma namn
            for (int j = i + 1; j < instruction_amount; j++)
            { // rad-loop
                for (int k = 0; instructions[j][k].type != TERMINATOR; k++)
                { // token-loop
                    if (instructions[j][k].type == VARIABLE)
                    {
                        int size;
                        if (instructions[j][k].var.name_len >= instructions[i][1].var.name_len)
                            size = instructions[j][k].var.name_len;
                        else
                            size = instructions[i][1].var.name_len;
                        if (!strncmp(instructions[j][k].var.name, instructions[i][1].var.name, size))
                        {
                            instructions[j][k].var.type = VAR_FUNCTION;
                        }
                    }
                }
            }
        }
    }

    Program program = {instructions, instruction_amount};
    printf("[DEBUG] Tokenize finished. Program.data: %p, instruction_amount: %d\n", program.data, program.instruction_amount);
    return program;
}

void interpret_instruction(Token *current, Token (*instructions)[128], int instruction_amount);

double call_function(char *name, int name_len, int origin_program_counter, Token (*instructions)[128], int instruction_amount);

double evaluate_expression(Token *args_old, int args_amount, Token (*instructions)[128], int instruction_amount)
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
    memcpy(args, args_old, args_amount * sizeof(Token)); // av någon skum anledning måste den ha en lokal kopia

    // ta bort variablerna
    for (int i = 0; i < args_amount; i++)
    {
        if (args[i].type == VARIABLE && args[i].var.type != VAR_FUNCTION)
        {
            double val = get_var_value(args[i].var.name, args[i].var.name_len);
            args[i].value = val;
            args[i].type = NUMBER;
        }
    }

    // byt ut funktioner mot värden
    for (int i = 0; i < args_amount; i++)
    {
        if (args[i].type == VARIABLE && args[i].var.type == VAR_FUNCTION)
        {
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

                // printf("FIRST: %lf, SECOND: %lf\n", first_arg, second_arg);
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

void band2(Token *instruction, Token (*instructions)[128], int instruction_amount){
    Token end_var = instruction[1];
    // ta reda på vilken typ av variabler som används

    int type = VAR_NUMBER;

    // kolla om det är en sträng
    for (int i = 0; instruction[i].type != TERMINATOR; i++){ // leta hårdkodade strängar
        if (instruction[i].type == STRING){
            type = VAR_STRING;
        }
    }
    if (instruction[3].type == LEFT_BRACKET){
        type = VAR_LIST;
    }
    if (type != VAR_STRING){ // hitta om variablerna är strängar
        for (int i = 3; instruction[i].type != TERMINATOR; i++) // kolla efter varje variabel
        {
            if (instruction[i].type == VARIABLE){ // kolla om det är en sträng
                for (int j = 0; j < var_index; j++){
                    if (!strncmp(instruction[i].var.name, variables[j].name, fmax(instruction[i].var.name_len, variables[j].name_len))){
                        type = VAR_STRING;
                        break;
                    }
                }
            }
            if (type == VAR_STRING) break;
        }
    }

    // kolla om slutvariabeln finns sparad
    int create_new = 1;
    for (int i = 0; i < var_index; i++){
        if (!strncmp(end_var.var.name, variables[i].name, fmax(end_var.var.name_len, variables[i].name_len))){
            create_new = 0;
        }
    }

    double value;
    // räkna hur många args
    int args_count = 3;
    while (instruction[args_count].type != TERMINATOR) 
        args_count++;
    
    args_count-=3;
    if (type == VAR_NUMBER){
        value = evaluate_expression(instruction+3, args_count, instructions, instruction_amount);
    } else value = 0;

    // todo: evaluate expression för strängar

    char* string = instruction[3].var.name; // med denna logik kan man inte plussa ihop strängar
    int str_len = instruction[3].var.name_len;

    if (create_new){
        create_variable(
            type, 
            end_var.var.name, 
            end_var.var.name_len,
            value, 
            instruction+4, // values
            string,
            str_len
        );
    }


}

void band(Token *instruction, Token (*instructions)[128], int instruction_amount)
{
    // hitta slutvariabeln
    Token end_var = instruction[1];

    char end_var_name[end_var.var.name_len + 1];
    memcpy(end_var_name, end_var.var.name, end_var.var.name_len);
    end_var_name[end_var.var.name_len] = '\0';

    // hitta hur många tokens den ska evaluata
    int length = 0;
    for (int i = 3; instruction[i].type != TERMINATOR; i++)
        length++;
    // printf("length: %d\n", length);

    double value;
    Token args[length]; // skapa en array med argumenten
    for (int i = 0; i < length; i++)
    {
        args[i] = instruction[i + 3];
    }
    /*for (int i = 0; i < length; i++){
        printf("%d  ", args[i].type);
    }
    printf("\n");*/
    if (instruction[2].type == EQUALS)
    {
        value = evaluate_expression(args, length, instructions, instruction_amount);
    }
    else
    {
        printf("ERR: Syntax error, band\n");
        exit(-1);
    }

    int var_exists = 0;
    for (int i = 0; i < global_var_index; i++)
    {
        // printf("end_var: %s, var_names[i]: %s\n", end_var_name, variables_names[i]);
        if (!strcmp(end_var_name, variables_names[i]))
        {
            // printf("HITTADE: '%s'\n", variables_names[i]);
            variables_values[i] = value;
            var_exists = 1;
        }
    }
    if (!var_exists)
    {
        create_variable(VAR_NUMBER, end_var.var.name, end_var.var.name_len, value, 0, 0, 0);
    }
}

void debug_print_var(char* name, int len){
    printf("§");
    for (int i = 0; i < len; i++)
    {
        printf("%c", name[i]);
    }
    printf("§\n");
}

void foug(Token *instruction)
{
    // printf("FOUG KALLAD PÅ\n");
    if (instruction[1].type != SVETS)
    {
        if (instruction[1].type == STRING)
        {
            // printf("STRING I FOUG\n");
            for (int i = 0; i < instruction[1].var.name_len; i++)
            {
                if (instruction[1].var.name[i] == '\\' && instruction[1].var.name[i + 1] == 'n')
                {
                    printf("\n");
                    i += 2;
                }
                if (i < instruction[1].var.name_len)
                    printf("%c", instruction[1].var.name[i]);
            }
        }
        else if (instruction[1].type == VARIABLE)
        {
            // printf("VARIABLE I FOUG\n");
            double value = get_var_value(instruction[1].var.name, instruction[1].var.name_len);
            if ((int)value == value)
                printf("%d", (int)value);
            else
                printf("%lf", value);
        }
        else
        {
            printf("ERR: Foug: Syntax error\n");
            exit(-1);
        }
    } else { // svets-string
        for (int i = 0; i < instruction[2].var.name_len; i++){
            if (instruction[2].var.name[i] == '\\' && instruction[2].var.name[i + 1] == 'n') // printa \n
            {
                printf("\n");
                i += 2;
            }
            if (instruction[2].var.name[i] == '\\' && instruction[2].var.name[i + 1] == '%') // printa %
            {
                printf("%%");
                i += 2;
            }

            if (instruction[2].var.name[i] == '%'){
                // kolla längden på den
                int len = 0;
                for (int j = i+1; j < instruction[2].var.name_len; j++){
                    if (instruction[2].var.name[j] == '%') break;
                    len++;
                }
                double value = get_var_value(instruction[2].var.name+i+1, len);
                if ((int)value == value)
                    printf("%d", (int)value);
                else 
                    printf("%lf", value);
                i+=len+1;
            } else {
                if (i < instruction[2].var.name_len)
                    printf("%c", instruction[2].var.name[i]);
            }
        }
        
    }
}

void givet(Token *instruction, Program program){
    // hitta ptr till argumenten
    
    int i = 2;
    Token* left_args = &instruction[i];
    while (instruction[i].type != EQUALS && instruction[i].type != GREATER_THAN && instruction[i].type != LESS_THAN && instruction[i].type != NOT_EQUAL_TO)
        i++;
    int operation = instruction[i].type;
    int left_len = i-2;
    i++;
    Token* right_args = &instruction[i];
    while (instruction[i].type != TERMINATOR) i++;
    int right_len = i-left_len-4;
 

    double left_value = evaluate_expression(left_args, left_len, program.data, program.instruction_amount);
    double right_value = evaluate_expression(right_args, right_len, program.data, program.instruction_amount);


    int do_statement = 0;

    switch (operation)
    {
    case EQUALS:
        if (left_value == right_value)
            do_statement = 1;
        break;

    case GREATER_THAN:
        if (left_value > right_value)
            do_statement = 1;
        break;

    case LESS_THAN:
        if (left_value < right_value)
            do_statement = 1;
        break;

    case NOT_EQUAL_TO:
        if (left_value != right_value)
            do_statement = 1;
        break;
    }

    if (!do_statement)
    {
        int k = 0;
        while (instruction[k].type != TERMINATOR)
            k++;
        char givet_id = instruction[k - 1].loop_id;

        for (int k = program_counter; k < program.instruction_amount; k++)
        { // kolla varje rad
            for (int l = 0; program.data[k][l].type != TERMINATOR; l++)
            { // kolla varje token i raden
                if (program.data[k][l].type == LOOP_MARKER && program.data[k][l].loop_id == givet_id)
                {
                    program_counter = k + 1;
                    return;
                }
            }
        }
    }
}


void naer(Token *instruction, Token (*instructions)[128], int instruction_amount)
{
    // printf("[DEBUG] Entered NAER, program_counter: %d\n", program_counter);
    int i = 1;
    while (instruction[i].type != EQUALS && instruction[i].type != GREATER_THAN && instruction[i].type != LESS_THAN && instruction[i].type != NOT_EQUAL_TO && i < 128)
        i++;
    int operation = instruction[i].type;
    i++;

    int left_args_length = i - 2;
    int j = i;
    while (instruction[i].type != LOOP_MARKER && i < 128)
        i++;
    int right_args_length = i - j;

    // printf("[DEBUG] left_args_length: %d, right_args_length: %d\n", left_args_length, right_args_length);

    Token left_args[left_args_length];
    Token right_args[right_args_length];

    if (left_args_length > 0)
        memcpy(left_args, instruction + 1, left_args_length * sizeof(Token));
    if (right_args_length > 0)
        memcpy(right_args, instruction + j, right_args_length * sizeof(Token));

    // for (int k=0; k<left_args_length; k++) printf("[DEBUG] LEFT %d: %d\n", k, left_args[k].type);
    // for (int k=0; k<right_args_length; k++) printf("[DEBUG] RIGHT %d: %d\n", k, right_args[k].type);

    double left_value = evaluate_expression(left_args, left_args_length, instructions, instruction_amount);
    double right_value = evaluate_expression(right_args, right_args_length, instructions, instruction_amount);

    // printf("[DEBUG] NAER left: %lf, right: %lf\n", left_value, right_value);

    int do_statement = 0;
    switch (operation)
    {
    case EQUALS:
        if (left_value == right_value)
            do_statement = 1;
        break;
    case GREATER_THAN:
        if (left_value > right_value)
            do_statement = 1;
        break;
    case LESS_THAN:
        if (left_value < right_value)
            do_statement = 1;
        break;
    case NOT_EQUAL_TO:
        if (left_value != right_value)
            do_statement = 1;
        break;
    }

    int k = 0;
    while (instruction[k].type != TERMINATOR && k < 128)
        k++;
    char loop_id = 0;
    if (k > 0 && instruction[k - 1].type == LOOP_MARKER)
        loop_id = instruction[k - 1].loop_id;
    // printf("[DEBUG] NAER loop_id: %c\n", loop_id);

    if (!do_statement)
    {
        // printf("[DEBUG] Condition false, jumping\n");
        // printf("[DEBUG] Looking for loop_id: %c\n", loop_id);

        for (k = program_counter + 1; k < instruction_amount; k++)
        {
            int l = 0;
            while (instructions[k][l].type != TERMINATOR)
            {
                if (instructions[k][l].type == LOOP_MARKER)
                {
                    if (instructions[k][l].loop_id == loop_id)
                    {
                        // printf(" -> MATCH! Jumping to instruction %d\n", k);
                        program_counter = k;
                        return;
                    }
                }
                // printf("\n");
                l++;
            }
        }
        // printf("[DEBUG] Did not find matching loop_id!\n");
    }
    else
    {
        // printf("[DEBUG] Condition true, pushing loop\n");
        int loop_already_exists = 0;
        for (int l = 0; l < loop_stack_top_id; l++)
            if (loop_id_stack[l] == loop_id)
                loop_already_exists = 1;
        if (!loop_already_exists)
        {
            if (loop_stack_top_id >= loop_stack_capacity){
                loop_id_stack = realloc(loop_id_stack, loop_stack_capacity+64);
                loop_program_counter_stack = realloc(loop_program_counter_stack, loop_stack_capacity+64);
                loop_stack_capacity += 64;
                if (loop_id_stack == NULL || loop_program_counter_stack == NULL){
                    printf("ERR: Minnesallokering misslyckades\n");
                    exit(1);
                }
            }
            loop_id_stack[loop_stack_top_id] = loop_id;
            loop_program_counter_stack[loop_stack_top_id++] = program_counter;
        }
    }
}

double call_function(char *name, int name_len, int origin_program_counter, Token (*instructions)[128], int instruction_amount)
{
    int func_index = -1;
    for (int i = 0; i < instruction_amount; i++)
    {
        if (instructions[i][0].type == FUNCTION)
        {
            size_t size = (instructions[i][1].var.name_len >= name_len) ? instructions[i][1].var.name_len : name_len;
            if (!strncmp(instructions[i][1].var.name, name, size))
            {
                func_index = i;
                break;
            }
        }
    }
    if (func_index == -1)
    {
        printf("ERR: Ofärdig funktion\n");
        exit(-1);
    }

    int call_stack_level = function_stack_top;
    if (function_stack_top >= function_stack_capacity){
        function_origin_program_counter_stack = realloc(function_origin_program_counter_stack, function_stack_capacity+64);
        function_return_stack = realloc(function_return_stack, function_stack_capacity+64);
        function_stack_capacity += 64;
        if (function_origin_program_counter_stack == NULL || function_return_stack == NULL){
            printf("ERR: Minnesallokering misslyckades\n");
            exit(1);
        }
    }
    function_origin_program_counter_stack[function_stack_top] = origin_program_counter;
    function_return_stack[function_stack_top] = 0;
    function_stack_top++;

    program_counter = func_index + 1; // börja precis efter "boul"

    while (function_stack_top > call_stack_level)
    {
        if (program_counter >= instruction_amount)
        {
            printf("ERR: Funktion nådde filslut utan return\n");
            exit(-1);
        }
        Token *current = instructions[program_counter];
        interpret_instruction(current, instructions, instruction_amount);
        program_counter++;
    }
    program_counter--; // program_counter inkrementeras 2 ggr annars

    double ret = function_return_stack[call_stack_level];
    return ret;
}


void interpret_instruction(Token *current, Token (*instructions)[128], int instruction_amount)
{
    switch (current[0].type)
    {
    case FOUG:
        foug(current);
        break;

    case BAND:
        band2(current, instructions, instruction_amount);
        break;

    case GIVET:
        givet(current, (Program){instructions, instruction_amount});
        break;

    case NAER:
        naer(current, instructions, instruction_amount);
        break;

    case LOOP_MARKER:
        for (int i = 0; i < loop_stack_top_id; i++)
        {
            if (loop_id_stack[i] == current[0].loop_id)
            {
                program_counter = loop_program_counter_stack[i] - 1;
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

        // printf("RETURN: %lf\n", return_value);

        function_return_stack[function_stack_top - 1] = return_value;

        program_counter = function_origin_program_counter_stack[function_stack_top - 1];

        function_stack_top--;

        break;
    }

    case VARIABLE: // anta att det är en funktion
        if (current[0].var.type == FUNCTION)
        call_function(current[0].var.name, current[0].var.name_len, program_counter, instructions, instruction_amount);
        break;
    }
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("fog:~$ För få argument!\n");
        printf("fog:~$ SYNTAX: <./fog fil.fg>\n");
        return -1;
    }

    // skapa konstantarrays
        // variabler
    variables = malloc(128*sizeof(Variable)); 

    variables_values = malloc(128*sizeof(double));
    variables_names = malloc(128*sizeof(*variables_names));
        // loopstack
    loop_id_stack = malloc(128*sizeof(char));
    loop_program_counter_stack = malloc(128*sizeof(int));
        // function stack
    function_origin_program_counter_stack = malloc(128*sizeof(int));
    function_return_stack = malloc(128*sizeof(double));


    if (variables_values == NULL || 
        variables_names == NULL || 
        loop_id_stack == NULL || 
        loop_program_counter_stack == NULL || 
        function_origin_program_counter_stack == NULL || 
        function_return_stack == NULL){
        printf("ERR: Minnesallokering misslyckades\n");
        exit(1);
    }

    Program program = tokenize(argv[1]);
    Token(*instructions)[128] = program.data;
    int instruction_amount = program.instruction_amount;
    print_tokens(instructions, instruction_amount);

    // hitta entry point (main)
    for (int i = 0; i < instruction_amount; i++)
    {
        if (instructions[i][0].type == MAIN)
            program_counter = i;
    }
    if (program_counter == -1)
    {
        printf("ERR: main inte hittad\n");
        exit(-1);
    }

    while (program_counter < instruction_amount)
    {
        Token *current = instructions[program_counter];

        interpret_instruction(current, instructions, instruction_amount);

        program_counter++;
    }
    print_variables();
    return 0;
}
