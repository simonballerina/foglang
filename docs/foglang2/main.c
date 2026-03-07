#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

// konstanter och globala variabler

// program counter
int program_counter = -1;

// foglangvariabler
enum var_type
{
    VAR_NONE,
    VAR_NUMBER,
    VAR_STRING,
    VAR_LIST,
    VAR_FUNCTION,
    VAR_LIST_NUMBER,
    VAR_LIST_STRING
};

typedef struct
{
    int type;
    char *name;
    int name_len;
    double value;
    int len;     // längden på listan / strängen
    char *str_ptr;
} Variable;

Variable *variables;
int var_index = 0;
int variables_capacity = 128;



// loop stack
char *loop_id_stack;
int *loop_program_counter_stack;
int loop_stack_top_id = 0;
int loop_stack_capacity = 128;

// function stack
int *function_origin_program_counter_stack;
double *function_return_stack;
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

typedef struct
{
    char* string;
    int len;
} String;

typedef struct 
{
    char* string;
    int str_len;
    double value;
    int type;
} Get_var_return;

enum tok_type
{
    NONE,          // 0
    TERMINATOR,    // 1
    FOUG,          // 2
    BAND,          // 3
    GIVET,         // 4
    ATT,           // 5
    NAER,          // 6
    NUM,           // 7
    STRING,        // 8
    RIGHT_PAR,     // 9
    LEFT_PAR,      // 10
    VARIABLE,      // 11
    EQUALS,        // 12
    NUMBER,        // 13
    LOOP_MARKER,   // 14
    PLUS,          // 15
    MINUS,         // 16
    MULTIPLIED,    // 17
    DIVIDED,       // 18
    EXPONENT,      // 19
    MODULO,        // 20
    GREATER_THAN,  // 21
    LESS_THAN,     // 22
    NOT_EQUAL_TO,  // 23
    FUNCTION,      // 24
    RETURN,        // 25
    MAIN,          // 26
    SVETS,         // 27
    LEFT_BRACKET,  // 28
    RIGHT_BRACKET, // 29
    COMMA          // 30
};

Get_var_return get_var_value(char *name, int length, int type, double index){
    Get_var_return ret_value;
    for (int i = 0; i < var_index; i++){
        if (length == variables[i].name_len && strncmp(name, variables[i].name, length) == 0){ // hittat en variabel
            if (type == VAR_LIST){
                int ret_type = VAR_STRING;
                if (index >= variables[i].len || index < 0){
                    printf("ERR: Ogiltig indexing av lista\n");
                    exit(-1);
                }
                Variable found_var = variables[i+(int)index+1];
                
                if (found_var.str_ptr == NULL) ret_type = VAR_NUMBER;

                if (ret_type == VAR_STRING){
                    char* ret_str = malloc(found_var.len*sizeof(char));
                    if (ret_str == NULL){
                        printf("ERR: Minnesallokering misslyckades\n");
                        exit(1);
                    }
                    ret_value.string = ret_str;
                    
                    memcpy(ret_value.string, found_var.str_ptr, found_var.len*sizeof(char));
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
            } else if (type == VAR_NUMBER){
                ret_value.value = variables[i].value;
                ret_value.string = 0;
                ret_value.str_len = 0;
                ret_value.type = VAR_NUMBER;
                return ret_value;
            } else if (type == VAR_STRING){
                ret_value.type = VAR_STRING;
                ret_value.value = 0;
                ret_value.string = variables[i].str_ptr;
                ret_value.str_len = variables[i].len;
                //printf("returnerar strämg, struct: str_ptr: %x, str_len: %d, value: %lf\n", value.string, value.str_len, value.value);
                return ret_value;
            }
        }

    }
    printf("ERR: Kunde inte framställa ett variabelvärde\n");
    exit(-1);
}

double evaluate_expression(Token *args_old, int args_amount, Token (*instructions)[128], int instruction_amount);

String evaluate_str_expression(Token *args_old, int args_amount, Token (*instructions)[128], int instruction_amount);

void create_list_var(char *name, int name_len, Token *values, Token (*instructions)[128], int instruction_amount)
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

    if (var_index >= variables_capacity)
    { // kolla att strl är ok
        variables = realloc(variables, sizeof(Variable)*(variables_capacity + var_size + 64));
        variables_capacity += var_size + 64;
        if (variables == NULL)
        {
            printf("ERR: Minnesallokering misslyckades\n");
            exit(1);
        }
    }
    variables[var_index++] = var;


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

            list_var.value = evaluate_expression(values+i, item_len, instructions, instruction_amount);
            list_var.type = VAR_LIST_NUMBER;
            variables[var_index++] = list_var;
            i+=item_len;
        }
        else if (values[i].type == STRING)
        {
            String list_str = evaluate_str_expression(values+i, item_len, instructions, instruction_amount);
            list_var.len = list_str.len;
            if (list_str.string == NULL)
            {
                printf("ERR: Minnesallokering misslyckades\n");
                exit(1);
            }
            list_var.str_ptr = list_str.string;
            list_var.type = VAR_LIST_STRING;
            variables[var_index++] = list_var;
            i+=item_len;
        } else if (values[i].type == VARIABLE){

            int var_type = VAR_NONE;
            Get_var_return test_var = get_var_value(values[i].var.name, values[i].var.name_len, VAR_STRING, 0);
            var_type = test_var.type;


            if (var_type == VAR_STRING) {
                String var = evaluate_str_expression(values+i, item_len, instructions, instruction_amount);
                list_var.type = VAR_LIST_STRING;
                list_var.str_ptr = var.string;
                list_var.len = var.len;
                variables[var_index++] = list_var;
                i+=item_len;
            }
            else if (var_type == VAR_NUMBER) {
                double var_value = evaluate_expression(values+i, item_len, instructions, instruction_amount);

                list_var.value = var_value;
                list_var.type = VAR_LIST_NUMBER;
                variables[var_index++] = list_var;
                i+=item_len;
            }
            

            
            

        } else {
            printf("ERR: Okänd datatyp i lista\n");
            exit(-1);
        }
    }
}

void create_num_var(char *name, int name_len, double value)
{
    Variable var = {// init var
                    .len = 0,
                    .name = name,
                    .name_len = name_len,
                    .type = VAR_NUMBER,
                    .value = value,
                    .str_ptr = 0};

    if (var_index >= variables_capacity)
    { // kolla att strl är ok
        variables = realloc(variables, variables_capacity + 1 + 64);
        variables_capacity += 1 + 64;
        if (variables == NULL)
        {
            printf("ERR: Minnesallokering misslyckades\n");
            exit(1);
        }
    }
    variables[var_index++] = var;
}

void create_str_var(char *name, int name_len, int len, char *string)
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
    if (var_index >= variables_capacity)
    { // kolla att strl är ok
        variables = realloc(variables, variables_capacity + 1 + 64);
        variables_capacity += 1 + 64;
        if (variables == NULL)
        {
            printf("ERR: Minnesallokering misslyckades\n");
            exit(1);
        }
    }
    variables[var_index++] = var;
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

void print_variables()
{
    for (int i = 0; i < var_index; i++)
    {
        printf("%i: Type: %d    Name: ", i, variables[i].type);
        // printa namn
        if (variables[i].name != NULL)
        {
            for (int j = 0; j < variables[i].name_len; j++)
            {
                printf("%c", variables[i].name[j]);
            }
        }
        printf("    Value: %lf    List/String_len: %d   String: '", variables[i].value, variables[i].len);

        if (variables[i].str_ptr != 0)
        {
            for (int j = 0; j < variables[i].len; j++)
            {
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
                Tok_Variable variable_info = {&buff[start], VAR_NONE, i - start};
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

    // ta bort list[index]
    for (int i = 0; i < args_amount; i++)
    {
        if (args[i].type == VARIABLE && (i + 1) < args_amount && args[i + 1].type == LEFT_BRACKET)
        {
            // hitta motsvarande RIGHT_BRACKET
            int start = i + 2; // första token i indexuttrycket
            int stop = start;
            while (stop < args_amount && args[stop].type != RIGHT_BRACKET)
                stop++;
            if (stop >= args_amount)
            {
                printf("ERR: Slutklammer hittades inte i indexering\n");
                exit(-1);
            }

            int index_args_amount = stop - start;

            int index = (int)evaluate_expression(&args[start], index_args_amount, instructions, instruction_amount);
            int ret_type = VAR_STRING;

            Get_var_return val = get_var_value(args[i].var.name, args[i].var.name_len, VAR_LIST, index);
            ret_type = val.type;

            if (ret_type == VAR_STRING){
                printf("ERR: Kan ej utföra aritmetik med blandade datatyper\n");
                exit(1);
            } else if (ret_type == VAR_NUMBER){
                args[i].value = val.value;
                args[i].type = NUMBER;
            }

            // städa upp tokenerna inom hakparentesen inklusive parenteserna
            for (int k = i + 1; k <= stop; k++)
                args[k].type = NONE;
        }
    }

    // ta bort variablerna
    for (int i = 0; i < args_amount; i++)
    {
        if (args[i].type == VARIABLE && args[i].var.type != VAR_FUNCTION)
        {
            double val = get_var_value(args[i].var.name, args[i].var.name_len, VAR_NUMBER, 0).value;
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

String evaluate_str_expression(Token *args_old, int args_amount, Token (*instructions)[128], int instruction_amount){


    // konkatenera strängar. free:a gamla strängar!
    Token args[args_amount];
    memcpy(args, args_old, args_amount * sizeof(Token)); // av någon skum anledning måste den ha en lokal kopia

    for (int i = 0; i < args_amount; i++) // ta bort [index]
    {
        if (args[i].type == VARIABLE && (i + 1) < args_amount && args[i + 1].type == LEFT_BRACKET)
        {
            // hitta motsvarande RIGHT_BRACKET
            int start = i + 2; // första token i indexuttrycket
            int stop = start;
            while (stop < args_amount && args[stop].type != RIGHT_BRACKET)
                stop++;
            if (stop >= args_amount)
            {
                printf("ERR: Slutklammer hittades inte i indexering\n");
                exit(-1);
            }

            int index_args_amount = stop - start;

            int index = (int)evaluate_expression(&args[start], index_args_amount, instructions, instruction_amount);
            int ret_type = VAR_STRING;

            Get_var_return val = get_var_value(args[i].var.name, args[i].var.name_len, VAR_LIST, index);
            ret_type = val.type;

            if (ret_type == VAR_NUMBER){
                printf("ERR: Kan ej utföra aritmetik med blandade datatyper\n");
                exit(1);
            } else if (ret_type == VAR_STRING){
                args[i].type = STRING;
                args[i].var.name = val.string;
                args[i].var.name_len = val.str_len;
            }

            // städa upp tokenerna inom hakparentesen inklusive parenteserna
            for (int k = i + 1; k <= stop; k++)
                args[k].type = NONE;
        }
    }


    // räkna ut den totala längden som krävs
    int final_len = 0;

    for (int i = 0; i < args_amount; i++){
        if (args[i].type == STRING){
            final_len += args[i].var.name_len;
        }
        if (args[i].type == VARIABLE){
            int var_len = get_var_value(args[i].var.name, args[i].var.name_len, VAR_STRING, 0).str_len;
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
            Get_var_return var_ret = get_var_value(args[i].var.name, args[i].var.name_len, VAR_STRING, 0);
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

void change_list_item(char* name, int name_len, int index, Variable new_var){

    for (int i = 0; i < var_index; i++){
        if (variables[i].name_len == name_len && !strncmp(name, variables[i].name, name_len)){
            if (index < variables[i].len){
                // free gamla strängar
                if (variables[i+1+index].type == VAR_LIST_STRING) free(variables[i+1+index].str_ptr);

                // kopiera över nya variabeln
                variables[i+1+index] = new_var;
            } else { // skapa nytt item
                if (var_index >= variables_capacity){
                    variables = realloc(variables, sizeof(Variable)*(variables_capacity + 1 + 64));
                    variables_capacity += 64+1;
                    if (variables == NULL){
                        printf("ERR: Minnesallokering misslyckades\n");
                        exit(1);
                    }
                }
                memmove(
                    variables + i + index + 2,
                    variables + i + index + 1,
                    (var_index - (i + index + 1)) * sizeof(Variable)
                );                
                variables[i+1+index] = new_var;
                var_index++;
            }
        }
    }
}

void band(Token *instruction, Token (*instructions)[128], int instruction_amount)
{
    Token end_var = instruction[1];
    // ta reda på vilken typ av variabler som används
    
    // räkna hur många args
    int args_count = 3;


    for (int i = 0; instruction[i].type != TERMINATOR; i++){
        if (instruction[i].type == EQUALS){
            while (instruction[args_count].type != TERMINATOR)
                args_count++;
            break;
        }
    }

    // ta bort alla [index]
    for (int i = 3; instruction[i].type != TERMINATOR; i++)
    {
        if (instruction[i].type == VARIABLE && (i + 1) < args_count && instruction[i + 1].type == LEFT_BRACKET)
        {
            // hitta motsvarande RIGHT_BRACKET
            int start = i + 2; // första token i indexuttrycket
            int stop = start;
            while (stop < args_count && instruction[stop].type != RIGHT_BRACKET)
                stop++;
            if (stop >= args_count)
            {
                printf("ERR: Slutklammer hittades inte i indexering\n");
                exit(-1);
            }

            int index_args_amount = stop - start;

            int index = (int)evaluate_expression(&instruction[start], index_args_amount, instructions, instruction_amount);
            int ret_type = VAR_STRING;

            Get_var_return val = get_var_value(instruction[i].var.name, instruction[i].var.name_len, VAR_LIST, index);
            ret_type = val.type;

            if (ret_type == VAR_STRING){
                instruction[i].value = 0;
                instruction[i].type = STRING;
                instruction[i].var.name = val.string;
                instruction[i].var.name_len = val.str_len;

            } else if (ret_type == VAR_NUMBER){
                instruction[i].value = val.value;
                instruction[i].type = NUMBER;
            }

            // städa upp tokenerna inom hakparentesen inklusive parenteserna
            for (int k = i + 1; k <= stop; k++)
                instruction[k].type = NONE;
        }
    }


    int type = VAR_NUMBER;

    // kolla om det är en sträng
    for (int i = 0; instruction[i].type != TERMINATOR; i++)
    { // leta hårdkodade strängar
        if (instruction[i].type == STRING)
        {
            type = VAR_STRING;
        }
    }
    if (instruction[3].type == LEFT_BRACKET)
    {
        type = VAR_LIST;
    }
    if (type != VAR_STRING)
    {                                                           // hitta om variablerna är strängar
        for (int i = 3; instruction[i].type != TERMINATOR; i++) // kolla efter varje variabel
        {
            if (instruction[i].type == VARIABLE && instruction[i].var.type == VAR_STRING)
            { // kolla om det är en sträng
                for (int j = 0; j < var_index; j++)
                {
                    if (variables[j].name != NULL && !strncmp(instruction[i].var.name, variables[j].name, instruction[i].var.name_len) && variables[j].name_len == instruction[i].var.name_len)
                    {
                        type = VAR_STRING;
                        break;
                    }
                }
            }
            if (type == VAR_STRING)
                break;
        }
    }

    // kolla om en lista ska uppdateras istället
    if (instruction[2].type == LEFT_BRACKET && type == VAR_STRING)
        type = VAR_LIST_STRING;
    else if (instruction[2].type == LEFT_BRACKET && type == VAR_NUMBER)
        type = VAR_LIST_NUMBER;
    


    //printf("BAND_TYPE: %d\n", type);


    // kolla om slutvariabeln finns sparad
    int create_new = 1;
    for (int i = 0; i < var_index; i++)
    {
        // skippa list elements som inte har namn
        if (variables[i].name == NULL)
            continue;
        if (!strncmp(end_var.var.name, variables[i].name, end_var.var.name_len) && end_var.var.name_len == variables[i].name_len)
        {
            create_new = 0;
        }
    }
    
    double value = 0;
    String string_pack;
    char* string = 0;
    int str_len = 0;
    int index = 0;

    args_count -= 3;
    if (type == VAR_NUMBER)
    {
        //printf("BAND SPARAR ETT VÄRDE I EN NUMMERVARIABEL\n");
        value = evaluate_expression(instruction + 3, args_count, instructions, instruction_amount);
    }
    else if (type == VAR_STRING){
        //printf("BAND SPARAR ETT VÄRDE I EN STRÄNGVARIABEL\n");
        string_pack = evaluate_str_expression(instruction+3, args_count, instructions, instruction_amount);
        string = string_pack.string;
        str_len = string_pack.len;
    }

    if (type == VAR_LIST_NUMBER)
    {
        //printf("BAND SPARAR ETT VÄRDE I EN LISTNUMMERVARIABEL\n");
        // hitta hur mycket som ska evaluatas i indexet
        int index_args_count = 0;
        for (int i = 0; instruction[i].type != TERMINATOR; i++){
            if (instruction[i].type == LEFT_BRACKET){
                for (int j = i+1; instruction[j].type != TERMINATOR; j++){
                    if (instruction[j].type == RIGHT_BRACKET) break;
                    index_args_count++;
                }  
                break;
            }
        }
        value = evaluate_expression(instruction + 6, args_count, instructions, instruction_amount);
        index = evaluate_expression(instruction + 3, index_args_count, instructions, instruction_amount);
    }
    else if (type == VAR_LIST_STRING){

        //printf("BAND SPARAR ETT VÄRDE I EN LISTSTRÄNGVARIABEL\n");
            // hitta hur mycket som ska evaluatas i indexet
        int index_args_count = 0;
        for (int i = 0; instruction[i].type != TERMINATOR; i++){
            if (instruction[i].type == LEFT_BRACKET){
                for (int j = i+1; instruction[j].type != TERMINATOR; j++){
                    if (instruction[j].type == RIGHT_BRACKET) break;
                    index_args_count++;
                }  
                break;
            }
        }
        index = evaluate_expression(instruction + 3, index_args_count, instructions, instruction_amount);
        string_pack = evaluate_str_expression(instruction + 6, args_count, instructions, instruction_amount);
        string = string_pack.string;
        str_len = string_pack.len;
    }
    

    if (create_new)
    {
        if (type == VAR_NUMBER)
        {
            create_num_var(end_var.var.name, end_var.var.name_len, value);
        }
        else if (type == VAR_LIST)
        {
            create_list_var(end_var.var.name, end_var.var.name_len, instruction + 4, instructions, instruction_amount);
        }
        else if (type == VAR_STRING)
        {
            create_str_var(end_var.var.name, end_var.var.name_len, str_len, string);
        } else 
        {
            printf("ERR: Felaktig variabeltyp\n");
            exit(-1);
        }

    } else { // uppdatera istället
        if (type == VAR_NUMBER){
            for (int i = 0; i < var_index; i++){
                if (variables[i].name == NULL) // hoppa över de som inte har namn!!!
                    continue;
                if (!strncmp(end_var.var.name, variables[i].name, variables[i].name_len) && variables[i].name_len == end_var.var.name_len){
                    variables[i].value = value;
                    variables[i].type = VAR_NUMBER;
                    variables[i].str_ptr = 0;
                    variables[i].len = 0;
                }
            }
        }
        else if (type == VAR_STRING){
            for (int i = 0; i < var_index; i++){
                if (variables[i].name == NULL)
                    continue;
                if (!strncmp(end_var.var.name, variables[i].name, end_var.var.name_len) && end_var.var.name_len == variables[i].name_len){
                    free(variables[i].str_ptr);
                    variables[i].value = 0;
                    variables[i].str_ptr = string;
                    variables[i].len = str_len;
                    variables[i].type = VAR_STRING;
                }
            }
        } else if (type == VAR_LIST_NUMBER){
            Variable new_list_item = {
                .len = 0,
                .name = 0,
                .name_len = 0,
                .str_ptr = 0,
                .type = VAR_LIST_NUMBER,
                .value = value
            };
            change_list_item(end_var.var.name, end_var.var.name_len, index, new_list_item);

        } else if (type == VAR_LIST_STRING){
            Variable new_list_item = {
                .len = str_len,
                .name = 0,
                .name_len = 0,
                .str_ptr = string,
                .type = VAR_LIST_STRING,
                .value = 0
            };
            change_list_item(end_var.var.name, end_var.var.name_len, index, new_list_item);
        }
    }

}


void debug_print_var(char *name, int len)
{
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
            double value = get_var_value(instruction[1].var.name, instruction[1].var.name_len, VAR_NUMBER, 0).value;
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
    }
    else
    { // svets-string
        for (int i = 0; i < instruction[2].var.name_len; i++)
        {
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

            if (instruction[2].var.name[i] == '%')
            {
                // kolla längden på den
                int len = 0;
                for (int j = i + 1; j < instruction[2].var.name_len; j++)
                {
                    if (instruction[2].var.name[j] == '%')
                        break;
                    len++;
                }
                double value = get_var_value(instruction[2].var.name + i + 1, len, VAR_NUMBER, 0).value;
                if ((int)value == value)
                    printf("%d", (int)value);
                else
                    printf("%lf", value);
                i += len + 1;
            }
            else
            {
                if (i < instruction[2].var.name_len)
                    printf("%c", instruction[2].var.name[i]);
            }
        }
    }
}

void givet(Token *instruction, Program program)
{
    // hitta ptr till argumenten

    int i = 2;
    Token *left_args = &instruction[i];
    while (instruction[i].type != EQUALS && instruction[i].type != GREATER_THAN && instruction[i].type != LESS_THAN && instruction[i].type != NOT_EQUAL_TO)
        i++;
    int operation = instruction[i].type;
    int left_len = i - 2;
    i++;
    Token *right_args = &instruction[i];
    while (instruction[i].type != TERMINATOR)
        i++;
    int right_len = i - left_len - 4;

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
            if (loop_stack_top_id >= loop_stack_capacity)
            {
                loop_id_stack = realloc(loop_id_stack, loop_stack_capacity + 64);
                loop_program_counter_stack = realloc(loop_program_counter_stack, loop_stack_capacity + 64);
                loop_stack_capacity += 64;
                if (loop_id_stack == NULL || loop_program_counter_stack == NULL)
                {
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
    if (function_stack_top >= function_stack_capacity)
    {
        function_origin_program_counter_stack = realloc(function_origin_program_counter_stack, function_stack_capacity + 64);
        function_return_stack = realloc(function_return_stack, function_stack_capacity + 64);
        function_stack_capacity += 64;
        if (function_origin_program_counter_stack == NULL || function_return_stack == NULL)
        {
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
        band(current, instructions, instruction_amount);
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
            return_value = get_var_value(current[1].var.name, current[1].var.name_len, VAR_NUMBER, 0).value;

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
    variables = malloc(128 * sizeof(Variable));

    // loopstack
    loop_id_stack = malloc(128 * sizeof(char));
    loop_program_counter_stack = malloc(128 * sizeof(int));
    // function stack
    function_origin_program_counter_stack = malloc(128 * sizeof(int));
    function_return_stack = malloc(128 * sizeof(double));

    if (
        loop_id_stack == NULL ||
        loop_program_counter_stack == NULL ||
        function_origin_program_counter_stack == NULL ||
        function_return_stack == NULL)
    {
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
