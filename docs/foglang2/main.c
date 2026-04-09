#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "foglang.h"

// konstanter och globala variabler

// program counter
int program_counter = -1;

// function stack
int *function_origin_program_counter_stack;
Dynamic_Var *function_return_stack;
int function_stack_top = 0;
int function_stack_capacity = 128;

int loop_links[48];

#include "foglang_eval.c" 
#include "foglang_var.c"
#include "stack.c"


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
    printf("Printing tokens...\n");
    printf("Instruction amount: %d\n", instruction_amount);
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
            case TPOS:
                printf("'TPOS'    ");
                break;
            case COMMA:
                printf("','    ");
                break;
            case OPEN_LOOP:
                printf("'OPEN'    ");
                break;
            case CLOSE_LOOP:
                printf("'CLOSE'    ");
                break;
            }
        }
        printf("\n");
    }
    printf("\n");
}

void print_variables(Scope *scope)
{
    for (int i = 0; i < (*scope).index; i++)
    {
        printf("%i: Type: %d    Name: ", i, (*scope).variables[i].type);
        // printa namn
        if ((*scope).variables[i].name != NULL)
        {
            for (int j = 0; j < (*scope).variables[i].name_len; j++)
            {
                printf("%c", (*scope).variables[i].name[j]);
            }
        }
        printf("    Value: %lf    List/String_len: %d   String: '", (*scope).variables[i].value, (*scope).variables[i].len);

        if ((*scope).variables[i].str_ptr != 0)
        {
            for (int j = 0; j < (*scope).variables[i].len; j++)
            {
                printf("%c", (*scope).variables[i].str_ptr[j]);
            }
        }

        printf("'\n");
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

char* bult(char* file_name){

    char *buff = read_file(file_name);
    int imports_capacity = 32;
    char *imports = malloc(imports_capacity);
    if (!buff) {
        printf("ERR: Kunde inte öppna fil\n");
        exit(1);
    }
    int len = strlen(buff);
    int found;
    int search = 1;
    while (search){
        found = 0;
        for (int i = 0; i < len; i++){
            
            if (i + 5 < len && !strncmp(buff+i, "bult ", 5)) {

                int is_sax = 0;
                if (i + 9 < len && !strncmp(buff+i+5, "sax ", 4))
                {
                    is_sax = 1;
                    i += 4;
                }
                int name_len = 0;
                // hitta längden på importnamnet
                name_len = i+5;
                while (name_len < len && buff[name_len] != ';')
                    name_len++;
                name_len-=(i+5);

                char* import_file_name = malloc((name_len+1+5+4*is_sax)*sizeof(char));
                if (import_file_name == NULL) goto malloc_error;
                buff[i + name_len + 5] = '\0';
                if (is_sax) {
                    memcpy(import_file_name, buff+i+5, name_len*sizeof(char));
                } else {
                    sprintf(import_file_name, "lib/%s.fg", buff+i+5);
                }
                
                int is_dupe = 1;
                char* import_buff = read_file(import_file_name);
                if (find_substring(imports, import_file_name) == -1) {
                    is_dupe = 0;
                    imports_capacity += name_len+7*is_sax;
                    imports = realloc(imports, imports_capacity);
                    strcat(imports, import_file_name);
                }
                if (!import_buff && !is_dupe) {
                    printf("ERR: Kunde inte öppna importfil\n");
                    exit(1);
                }
                free(import_file_name);
                int import_end = i + 5 + name_len + 1;
                int left_side_len = i - 4*is_sax;
                int right_side_len = len - import_end;
                int import_buff_len = strlen(import_buff);
                // skapa ny sträng
                char* new_buff = malloc(left_side_len + import_buff_len + right_side_len + 1);
                if (new_buff == NULL) goto malloc_error;

                memcpy(new_buff, buff, left_side_len*sizeof(char));
                memcpy(new_buff+left_side_len, import_buff, import_buff_len*sizeof(char));
                memcpy(new_buff + left_side_len + (import_buff_len)*!is_dupe, buff + import_end, right_side_len);

                new_buff[left_side_len + import_buff_len + right_side_len] = '\0';
                char *old_buff = buff;
                buff = new_buff;
                len = left_side_len + import_buff_len + right_side_len;
                found = 1;

                free(old_buff);
                free(import_buff);

                break;
            }
        }
        if (found)
            search = 1;
        else search = 0;

    }

    free(imports);
    imports = NULL;

    return buff; 


    malloc_error:
        printf("[BULT] ERR: Minnesallokering misslyckades\n");
        exit(1);
}



Program tokenize(char* buff)
{
    int buff_len = strlen(buff);
    printf("BUFF: -----------------------------------------------\n%s\n-----------------------------------------------\n", buff);

    // räkna antal instr
    int instruction_amount = 0;
    for (int i = 0; i < buff_len; i++)
    {
        if (buff[i] == ';' || buff[i] == '{' || buff[i] == '}')
            instruction_amount++;
    }
    printf("[DEBUG] instruction_amount: %d\n", instruction_amount);

    // skapa instruktionsarray
    Token(*instructions)[128] = calloc(instruction_amount, sizeof(*instructions));
    if (instructions == NULL) goto malloc_error;
  
    printf("[DEBUG] instructions ptr: %p\n", instructions);

    int i = 0;
    int instructions_OUTER_arr_index = 0;
    int instructions_INNER_arr_index = 0;

    int loop_type = 1;

    Stack loops;
    initialize(&loops); 

    while (i < buff_len)
    {
        while (i < buff_len && (buff[i] == ' ' || buff[i] == '\n' || buff[i] == '\r' || buff[i] == '\t'))
            i++;

        if (i >= buff_len)
            break;

        Token tok;
        tok.type = 0;

        // ord-tokens
        if (strncmp(&buff[i], "foug ", 5) == 0)
        {
            tok.type = FOUG;
            i += 5;
        }
        else if (strncmp(&buff[i], "svets ", 6) == 0)
        {
            tok.type = SVETS;
            i += 6;
        }
        else if (strncmp(&buff[i], "band ", 5) == 0)
        {
            tok.type = BAND;
            i += 5;
        }
        else if (strncmp(&buff[i], "givet ", 6) == 0)
        {
            tok.type = GIVET;
            i += 6;
            loop_type = -1;
        }
        else if (strncmp(&buff[i], "att ", 4) == 0)
        {
            tok.type = ATT;
            i += 4;
        }
        else if (strncmp(&buff[i], "naer ", 5) == 0)
        {
            tok.type = NAER;
            i += 5;
            loop_type = 1;
        }
        else if (strncmp(&buff[i], "boul ", 5) == 0)
        {
            tok.type = FUNCTION;
            i += 5;
        }
        else if (strncmp(&buff[i], "tpos", 4) == 0)
        {
            tok.type = TPOS;
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
        else if (buff[i] == '{')
        {
            tok.type = OPEN_LOOP;
            push(&loops, instructions_OUTER_arr_index*loop_type);

            printf("[DEBUG] Found OPEN_LOOP: _ at instructions[%d][%d]\n", instructions_OUTER_arr_index, instructions_INNER_arr_index);
            Token next;
            next.type = TERMINATOR;
            instructions[instructions_OUTER_arr_index][instructions_INNER_arr_index++] = tok;
            instructions[instructions_OUTER_arr_index][instructions_INNER_arr_index++] = next;
            i += 1;
            instructions_INNER_arr_index = 0;
            instructions_OUTER_arr_index++;
            continue;
        }
        else if (buff[i] == '}')
        {
            int other = pop(&loops);
            tok.type = CLOSE_LOOP;
            if (other > 0) {
                loop_links[instructions_OUTER_arr_index] = other;
            } else {
                other = abs(other);
                loop_links[instructions_OUTER_arr_index] = instructions_OUTER_arr_index+1;
            }
            loop_links[other]=instructions_OUTER_arr_index;
            printf("[DEBUG] Found CLOSE_LOOP: %d at instructions[%d][%d]\n", loop_links[instructions_OUTER_arr_index], instructions_OUTER_arr_index, instructions_INNER_arr_index);
            
            Token next;
            next.type = TERMINATOR;
            instructions[instructions_OUTER_arr_index][instructions_INNER_arr_index++] = tok;
            instructions[instructions_OUTER_arr_index][instructions_INNER_arr_index++] = next;
            i += 1;
            instructions_INNER_arr_index = 0;
            instructions_OUTER_arr_index++;
            continue;
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

    malloc_error:
        printf("[LEXER] ERR: Minnesallokering misslyckades\n");
        exit(1);
        
}

void check_syntax(Program* program){
    Token(*instructions)[128] = program->data;
    int instruction_amount = program->instruction_amount; 

    for (int i = 0; i < instruction_amount; i++){
        
        switch (instructions[i][0].type){
            
            case NAER: 
                /*
                naer 14*2 = 10+18 {1};
                    // gör något
                {1};
                */
                int j = 1;
                int comp_amount = 0;
                int left_args = 0;
                int right_args = 0;
                char loop_id = 0;
                int found_loop_id = 0;

                while (instructions[i][j-1].type != TERMINATOR){
                    if (instructions[i][j].type == TERMINATOR){
                        if (j >= 4) break;
                        printf("[NAER]: ERR: Syntax error, instruktion %d\n", i);
                        exit(-1);
                    }

                    int tok = instructions[i][j].type;



                    if (tok == EQUALS || tok == NOT_EQUAL_TO || tok == GREATER_THAN || tok == LESS_THAN) {
                        if (instructions[i][j+1].type == NUMBER || instructions[i][j+1].type == VARIABLE || instructions[i][j+1].type == STRING || instructions[i][j+1].type == FUNCTION || instructions[i][j+1].type == LEFT_PAR){
                            right_args = 1;
                        }
                        if (instructions[i][j-1].type == NUMBER || instructions[i][j-1].type == VARIABLE || instructions[i][j-1].type == STRING || instructions[i][j-1].type == FUNCTION || instructions[i][j+1].type == RIGHT_PAR){
                            left_args = 1;
                        }

                        comp_amount++;
                    }

                    if (tok == LOOP_MARKER){
                        loop_id = instructions[i][j].loop_id;
                    }
                    // kolla om den hittar en matchande loop marker
                    for (int k = i; k < instruction_amount; k++){
                        if (instructions[k][0].type == LOOP_MARKER && instructions[k][0].loop_id == loop_id) {
                            found_loop_id = 1;
                            break;
                        }
                    }

                    j++;
                }
                if (comp_amount != 1){
                    printf("[NAER]: ERR: Syntax error, instruktion %d, hittade %d jämförelseoperationer när det ska vara 1\n", i, comp_amount);
                    exit(-1);
                }
                if (!left_args || !right_args){
                    printf("[NAER]: ERR: Syntax error, instruktion %d, hittade inga värden att jämföra\n", i);
                    exit(-1);
                }
                if (!found_loop_id || !loop_id){
                    printf("[NAER]: ERR: Syntax error, instruktion %d, kunde inte hitta första LOOP_MARKER token eller sista LOOP_MARKER token\n", i);
                    exit(-1);
                }
                break;

            case GIVET: 
                /*
                givet att 14*2 = 10+18 {1};
                    // gör något
                {1};
                */
                j = 1;
                comp_amount = 0;
                left_args = 0;
                right_args = 0;
                int att_exists = 0;
                loop_id = 0;
                found_loop_id = 0;

                if (instructions[i][1].type == ATT) att_exists = 1;

                while (instructions[i][j-1].type != TERMINATOR){
                    if (instructions[i][j].type == TERMINATOR){
                        if (j >= 5) break;
                        printf("[GIVET]: ERR: Syntax error, instruktion %d\n", i);
                        exit(-1);
                    }
                    int tok = instructions[i][j].type;
                    


                    if (tok == EQUALS || tok == NOT_EQUAL_TO || tok == GREATER_THAN || tok == LESS_THAN) {
                        if (instructions[i][j+1].type == NUMBER || instructions[i][j+1].type == VARIABLE || instructions[i][j+1].type == STRING || instructions[i][j+1].type == FUNCTION || instructions[i][j+1].type == LEFT_PAR){
                            right_args = 1;
                        }
                        if (instructions[i][j-1].type == NUMBER || instructions[i][j-1].type == VARIABLE || instructions[i][j-1].type == STRING || instructions[i][j-1].type == FUNCTION || instructions[i][j+1].type == RIGHT_PAR){
                            left_args = 1;
                        }

                        comp_amount++;
                    }

                    if (tok == LOOP_MARKER){
                        loop_id = instructions[i][j].loop_id;
                        // kolla om den hittar en matchande loop marker
                        for (int k = i; k < instruction_amount; k++){

                            if (instructions[k][0].type == LOOP_MARKER && instructions[k][0].loop_id == loop_id) {
                                found_loop_id = 1;
                                break;
                            }
                        }
                    }


                    j++;
                }
                
                if (comp_amount != 1){
                    printf("[GIVET]: ERR: Syntax error, instruktion %d, hittade %d jämförelseoperationer när det ska vara 1\n", i, comp_amount);
                    exit(-1);
                }
                if (!left_args || !right_args){
                    printf("[GIVET]: ERR: Syntax error, instruktion %d, hittade inga värden att jämföra\n", i);
                    exit(-1);
                }
                if (!found_loop_id || !loop_id){
                    printf("[GIVET]: ERR: Syntax error, instruktion %d, kunde inte hitta första LOOP_MARKER token eller sista LOOP_MARKER token\n", i);
                    exit(-1);
                }
                if (!att_exists){
                    printf("[GIVET]: ERR: Syntax error, instruktion %d, ATT token saknas\n", i);
                    exit(-1);
                }

                break;

            case FOUG:
                break;
            case BAND:
                break;
            case FUNCTION:
                /*
                boul func_name(a, b) {1};
                    // gör skit
                {1};
                */
                j = 1;
                found_loop_id = 0;
                loop_id = 0;
                int found_return = 0;

                while (instructions[i][j-1].type != TERMINATOR){
                    if (instructions[i][j].type == TERMINATOR){
                        if (j >= 4) break;
                        printf("[BOUL]: ERR: Syntax error, instruktion %d\n", i);
                        exit(-1);
                    }
                    int tok = instructions[i][j].type;

                    int func_stop;
                    if (instructions[i][j].type == LOOP_MARKER) {
                        loop_id = instructions[i][j].loop_id;
                        // hitta loop marker
                        for (int k = i; k < instruction_amount; k++){

                            if (instructions[k][0].type == LOOP_MARKER && instructions[k][0].loop_id == loop_id) {
                                found_loop_id = 1;
                                func_stop = k;
                                break;
                            }
                        }
                        // hitta ret
                        if (!found_return){
                            for (int k = i; k < func_stop; k++){
                                if (instructions[k][0].type == RETURN){
                                    found_return = 1;
                                    break;
                                }
                            }
                        }
                    }
                    

                    j++;
                }
                
                if (!found_loop_id || !loop_id){
                    printf("[BOUL]: ERR: Syntax error, instruktion %d, kunde inte hitta första LOOP_MARKER token eller sista LOOP_MARKER token\n", i);
                    exit(-1);
                }
                if (!found_return){
                    printf("[BOUL]: ERR: Syntax error, instruktion %d, kunde inte hitta RETURN token\n", i);
                    exit(-1);
                }
                break;
        }
    }

}

void band(Token *instruction, Token (*instructions)[128], int instruction_amount, Scope *scope)
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

    int start_eval = 0;
    while (instruction[start_eval].type != EQUALS && instruction[start_eval].type != TERMINATOR)
        start_eval++;
    start_eval++;

    // kolla om en lista skapas
    int type = VAR_NONE;
    if (instruction[3].type == LEFT_BRACKET){
        type = VAR_LIST;
    }

    Dynamic_Var eval_result;
    if (type != VAR_LIST){
        eval_result = dynamic_eval(instruction+start_eval, args_count, instructions, instruction_amount, scope);
        type = eval_result.type;
    }
        

    

    // kolla om en lista ska uppdateras istället
    if (instruction[2].type == LEFT_BRACKET && type == VAR_STRING)
        type = VAR_LIST_STRING;
    else if (instruction[2].type == LEFT_BRACKET && type == VAR_NUMBER)
        type = VAR_LIST_NUMBER;
    


    //printf("BAND_TYPE: %d\n", type);


    // kolla om slutvariabeln finns sparad
    int create_new = 1;
    for (int i = 0; i < (*scope).index; i++)
    {
        // skippa list elements som inte har namn
        if ((*scope).variables[i].name == NULL)
            continue;
        if (!strncmp(end_var.var.name, (*scope).variables[i].name, end_var.var.name_len) && end_var.var.name_len == (*scope).variables[i].name_len)
        {
            create_new = 0;
        }
    }
    
    int index = 0;

    args_count -= 3;
    

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
        
        index = evaluate_expression(instruction + 3, index_args_count, instructions, instruction_amount, scope);
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
        index = evaluate_expression(instruction + 3, index_args_count, instructions, instruction_amount, scope);
    }
    

    if (create_new)
    {
        if (type == VAR_NUMBER)
        {
            create_num_var(end_var.var.name, end_var.var.name_len, eval_result.value, scope);
        }
        else if (type == VAR_LIST)
        {
            create_list_var(end_var.var.name, end_var.var.name_len, instruction + 4, instructions, instruction_amount, scope);
        }
        else if (type == VAR_STRING)
        {
            create_str_var(end_var.var.name, end_var.var.name_len, eval_result.str_len, eval_result.string, scope);
        } else 
        {
            printf("ERR: Felaktig variabeltyp\n");
            exit(-1);
        }

    } else { // uppdatera istället
        if (type == VAR_NUMBER){
            for (int i = 0; i < (*scope).index; i++){
                if ((*scope).variables[i].name == NULL) // hoppa över de som inte har namn!!!
                    continue;
                if (!strncmp(end_var.var.name, (*scope).variables[i].name, (*scope).variables[i].name_len) && (*scope).variables[i].name_len == end_var.var.name_len){
                    (*scope).variables[i].value = eval_result.value;
                    (*scope).variables[i].type = VAR_NUMBER;
                    (*scope).variables[i].str_ptr = 0;
                    (*scope).variables[i].len = 0;
                }
            }
        }
        else if (type == VAR_STRING){
            for (int i = 0; i < (*scope).index; i++){
                if ((*scope).variables[i].name == NULL)
                    continue;
                if (!strncmp(end_var.var.name, (*scope).variables[i].name, end_var.var.name_len) && end_var.var.name_len == (*scope).variables[i].name_len){
                    free((*scope).variables[i].str_ptr);
                    (*scope).variables[i].value = 0;
                    (*scope).variables[i].str_ptr = eval_result.string;
                    (*scope).variables[i].len = eval_result.str_len;
                    (*scope).variables[i].type = VAR_STRING;
                }
            }
        } else if (type == VAR_LIST_NUMBER){
            Variable new_list_item = {
                .len = 0,
                .name = 0,
                .name_len = 0,
                .str_ptr = 0,
                .type = VAR_LIST_NUMBER,
                .value = eval_result.value
            };
            change_list_item(end_var.var.name, end_var.var.name_len, index, new_list_item, scope);

        } else if (type == VAR_LIST_STRING){
            Variable new_list_item = {
                .len = eval_result.str_len,
                .name = 0,
                .name_len = 0,
                .str_ptr = eval_result.string,
                .type = VAR_LIST_STRING,
                .value = 0
            };
            change_list_item(end_var.var.name, end_var.var.name_len, index, new_list_item, scope);
        }
    }

}

void foug(Token *instruction, Scope *scope)
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
            double value = get_var_value(instruction[1].var.name, instruction[1].var.name_len, 0, 0, scope).value;
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
                double value = get_var_value(instruction[2].var.name + i + 1, len, 0, 0, scope).value;
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

void loop(Token *instruction, Token (*instructions)[128], int instruction_amount, Scope *scope)
{
    // printf("[DEBUG] Entered NAER, program_counter: %d\n", program_counter);
    int i = 1;
    while (instruction[i].type != EQUALS && instruction[i].type != GREATER_THAN && instruction[i].type != LESS_THAN && instruction[i].type != NOT_EQUAL_TO && i < 128)
        i++;
    int operation = instruction[i].type;
    i++;
  
    int left_args_length = i - 2;
    int j = i;
    while (instruction[i].type != OPEN_LOOP && i < 128)
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

    Dynamic_Var left_value = dynamic_eval(left_args, left_args_length, instructions, instruction_amount, scope);
    Dynamic_Var right_value = dynamic_eval(right_args, right_args_length, instructions, instruction_amount, scope);

    int type = VAR_NONE;
    if (left_value.type == VAR_NUMBER && right_value.type == VAR_NUMBER) type = VAR_NUMBER;
    else if (left_value.type == VAR_STRING && right_value.type == VAR_STRING) type = VAR_STRING;
    if (type == VAR_NONE){
        printf("ERR: Kan inte jämföra två olika datatyper\n");
        exit(1);
    }

    //printf("NAER LEFT: %lf, NAER RIGHT %lf\n", left_value.value, right_value.value);

    int do_statement = 0;

    if (type == VAR_NUMBER){
        switch (operation)
        {
        case EQUALS:
            if (left_value.value == right_value.value)
                do_statement = 1;
            break;

        case GREATER_THAN:
            if (left_value.value > right_value.value)
                do_statement = 1;
            break;

        case LESS_THAN:
            if (left_value.value < right_value.value)
                do_statement = 1;
            break;

        case NOT_EQUAL_TO:
            if (left_value.value != right_value.value)
                do_statement = 1;
            break;
        }
    } else if (type == VAR_STRING){
        switch (operation)
        {
        case EQUALS:
            if (!strncmp(left_value.string, right_value.string, left_value.str_len) && left_value.str_len == right_value.str_len)
                do_statement = 1;
            break;

        case GREATER_THAN:
            printf("ERR: Ogiltig jämförelse mellan strängar\n");
            exit(1);
            break;

        case LESS_THAN:
            printf("ERR: Ogiltig jämförelse mellan strängar\n");
            exit(1);
            break;

        case NOT_EQUAL_TO:
            if (strncmp(left_value.string, right_value.string, left_value.str_len) || left_value.str_len != right_value.str_len)
                do_statement = 1;
            break;
        }

    }

    if (!do_statement)
    {
        program_counter = loop_links[program_counter];
        return;
    }
    //printf("moving\n");
    return;

    malloc_error:
        printf("[NAER] ERR: Minnesallokering misslyckades\n");
        exit(1);
}

void tpos(Token *instruction, Scope *scope)
{
    //allocation
    int call_len = sizeof(char)*instruction[1 + (instruction[1].type == SVETS)].var.name_len;
    char *call = malloc(call_len);
    strcpy(call, "");
    if (call == NULL)
    {
        printf("ERR: Minnesallokering misslyckades\n");
        exit(1);
    }
    int writer = 0;
    
    if (instruction[1].type != SVETS)
    {
        if (instruction[1].type == STRING)
        {
            for (int i = 0; i < instruction[1].var.name_len; i++)
            {
                if (instruction[1].var.name[i] == '\\' && instruction[1].var.name[i + 1] == 'n')
                {
                    i += 2;
                }
                if (i < instruction[1].var.name_len) {
                    sprintf(call + writer, "%c", instruction[1].var.name[i]);
                    writer++;
                }  
            }
        }
        else if (instruction[1].type == VARIABLE)
        {
            // printf("VARIABLE I TPOS\n");
            double value = get_var_value(instruction[1].var.name, instruction[1].var.name_len, 0, 0, scope).value;
            if ((int)value == value){
                call_len += sizeof(int);
                call = realloc(call, call_len);
                sprintf(call + writer, "%d", (int)value);
                writer += strlen(call + writer);
            } else {
                call_len += sizeof(double);
                call = realloc(call, call_len);
                sprintf(call + writer, "%lf", (double)value);
                writer += strlen(call + writer);
            }
                
        }
        else
        {
            printf("ERR: Tpos: Syntax error\n");
            exit(-1);
        }
    } else { // svets-string
        for (int i = 0; i < instruction[2].var.name_len; i++){
            if (instruction[2].var.name[i] == '\\' && instruction[2].var.name[i + 1] == 'n') // printa \n
            {
                strcat(call, "\n");
                i += 2;
            }
            if (instruction[2].var.name[i] == '\\' && instruction[2].var.name[i + 1] == '%') // printa %
            {
                strcat(call, "%%");
                i += 2;
            }

            if (instruction[2].var.name[i] == '%'){
                // kolla längden på den
                int len = 0;
                for (int j = i+1; j < instruction[2].var.name_len; j++){
                    if (instruction[2].var.name[j] == '%') break;
                    len++;
                }
                double value = get_var_value(instruction[2].var.name+i+1, len, 0, 0, scope).value;
                if ((int)value == value) {
                    call_len += sizeof(int);
                    call = realloc(call, call_len);
                    sprintf(call + writer, "%d", (int)value);
                    writer += strlen(call + writer);
                } else {
                    call_len += sizeof(double);
                    call = realloc(call, call_len);
                    sprintf(call + writer, "%lf", (double)value);
                    writer += strlen(call + writer);
                }
                i+=len+1;    
            } else if (i < instruction[2].var.name_len) {
                sprintf(call + writer, "%c", instruction[2].var.name[i]);
                writer++;
                
            }
        }
        
    }
    system(call);
    //free my boy
    free(call);
    call = NULL;
    
}

Dynamic_Var call_function(char *name, int name_len, int origin_program_counter, Token (*instructions)[128], int instruction_amount)
{
    Scope scope = {
                .index = 0,
                .capacity = 128,
                .variables = malloc(128 * sizeof(Variable))
            };
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
        function_origin_program_counter_stack = realloc(function_origin_program_counter_stack, (function_stack_capacity + 64)*sizeof(int));
        function_return_stack = realloc(function_return_stack, (function_stack_capacity + 64)*sizeof(Dynamic_Var));
        function_stack_capacity += 64;
        if (function_origin_program_counter_stack == NULL || function_return_stack == NULL) goto malloc_error;
    }
    function_origin_program_counter_stack[function_stack_top] = origin_program_counter;
    Dynamic_Var zero_var = {
        .str_len = 0,
        .string = 0,
        .type = 0,
        .value = 0
    };
    function_return_stack[function_stack_top] = zero_var;
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
        interpret_instruction(current, instructions, instruction_amount, &scope);
        program_counter++;
    }
    program_counter--; // program_counter inkrementeras 2 ggr annars

    //free up
    free(scope.variables);
    scope.variables = NULL;

    Dynamic_Var ret = function_return_stack[call_stack_level];
    return ret;

    malloc_error:
        printf("[FUNCTION CALL] ERR: Minnesallokering misslyckades\n");
        exit(1);
}

void interpret_instruction(Token *current, Token (*instructions)[128], int instruction_amount, Scope *scope)
{
    //printf("intep %d\n", program_counter);
    switch (current[0].type)
    {
    case FOUG:
        foug(current, scope);
        break;

    case BAND:
        band(current, instructions, instruction_amount, scope);
        break;

    case GIVET:
        loop(&(current[1]), &(instructions[1]), instruction_amount, scope);
        break;

    case NAER:
        loop(current, instructions, instruction_amount, scope);
        break;

    case TPOS:
        tpos(current, scope);
        break;
    case CLOSE_LOOP:
        program_counter = loop_links[program_counter]-1;
        break;

    case RETURN:
    {
        Dynamic_Var return_value;
        if (current[1].type == NUMBER) {
            return_value.value = current[1].value;
            return_value.type = VAR_NUMBER;
            return_value.string = 0;
            return_value.str_len = 0;
        } else if (current[1].type == VARIABLE) {
            return_value = get_var_value(current[1].var.name, current[1].var.name_len, 0, 0, scope);
        } else if (current[1].type == STRING) {
            return_value.string = current[1].var.name;
            return_value.str_len = current[1].var.name_len;
            return_value.type = VAR_STRING;
            return_value.value = 0;
        }

        // printf("RETURN: %lf\n", return_value);

        function_return_stack[function_stack_top - 1] = return_value;

        program_counter = function_origin_program_counter_stack[function_stack_top - 1];

        function_stack_top--;

        break;
    }

    case VARIABLE: // anta att det är en funktion
        if (current[0].var.type == FUNCTION) {
            call_function(current[0].var.name, current[0].var.name_len, program_counter, instructions, instruction_amount);
        }
            
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
    Scope scope = {
        .index = 0,
        .capacity = 128,
        .variables = malloc(128 * sizeof(Variable))
    };

    // function stack
    function_origin_program_counter_stack = malloc(128 * sizeof(int));
    function_return_stack = malloc(128 * sizeof(double));

    if (scope.variables == NULL ||
        function_origin_program_counter_stack == NULL ||
        function_return_stack == NULL)
        goto malloc_error;

    

    char* buff = bult(argv[1]);
    Program program = tokenize(buff);
    
    Token(*instructions)[128] = program.data;
    int instruction_amount = program.instruction_amount;
    print_tokens(instructions, instruction_amount);
    //check_syntax(&program);

    // hitta entry point (main)
    for (int i = 0; i < instruction_amount; i++)
    {
        if (instructions[i][0].type == MAIN)
            program_counter = i;
    }
    if (program_counter == -1)
    {
        printf("[MAIN] ERR: main inte hittad\n");
        exit(-1);
    }

    while (program_counter < instruction_amount)
    {
        Token *current = instructions[program_counter];

        interpret_instruction(current, instructions, instruction_amount, &scope);

        program_counter++;
    }

    print_variables(&scope);
    printf("it is");
    for (int i = 0; i < instruction_amount; i++)
    {
        printf("%d-", loop_links[i]);
    }
    printf("\n");
    
    return 0;

    malloc_error:
        printf("[MAIN] ERR: Minnesallokering misslyckades\n");
        exit(1);
}

int find_substring(char *txt, char *pat) {
    int n = strlen(txt);
    int m = strlen(pat);
    for (int i = 0; i <= n - m; i++) {
        int j;
        for (j = 0; j < m; j++) {
            if (txt[i + j] != pat[j]) {
                break;
            }
        }
        if (j == m) {
            return i;
        }
    }
    return -1;
}