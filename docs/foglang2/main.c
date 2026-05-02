#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>

#ifdef __APPLE__
    #include <sys/syslimits.h>
#endif

#ifdef _WIN32
    #include <windows.h>
#endif
#include "foglang.h"

// konstanter och globala variabler

// program counter
int program_counter = -1;
int* pc_to_line;

// function stack
int *function_origin_program_counter_stack;
Dynamic_Var *function_return_stack;
int function_stack_top = 0;
int function_stack_capacity = 128;

#ifdef PATH_MAX
    char path_diff[PATH_MAX];
#else
    #define PATH_MAX 1024
    char path_diff[PATH_MAX];
#endif

// håller värde för storlek på varje rad
int* row_lengths;
int *loop_links;

#include "foglang_utils.c"
#include "foglang_eval.c" 
#include "foglang_var.c"



Bult_Ret bult(char* file_name, char* user){

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
    int import_line_count = 0;

    #ifdef __APPLE__
        char pre_user[] = "/Users/";
        char post_user[] = "/Library/foglang2/";
        char lib[strlen(pre_user)+strlen(user)+strlen(post_user)];
        sprintf(lib, "%s%s%s", pre_user, user, post_user);
    #endif
    
    #ifdef _WIN32
        char lib[] = "C:\\Program Files\\foglang2\\lib\\";
    #endif

    #ifdef __linux__
        char lib[] = "/usr/local/lib/foglang/";
    #endif

    while (search){
        found = 0;
        for (int i = 0; i < len; i++){

            if (!strncmp(buff+i, "#*", 2)) {
                while (strncmp(buff+i, "*#", 2)) {
                    i++;
                }
            } else if (!strncmp(buff+i, "#", 1)) {
                while (strncmp(buff+i, "\n", 1)) {
                    i++;
                }
            }
            
            if (i + 5 < len && !strncmp(buff+i, "bult ", 5)) {

                int is_sax = 0;
                char origin_wd[PATH_MAX];
                if (i + 9 < len && !strncmp(buff+i+5, "sax ", 4))
                {
                    is_sax = 1;
                    i += 4;
                    //move to relative position
                    getcwd(origin_wd, PATH_MAX);
                    chdir(path_diff);
                }
                int name_len = 0;
                // hitta längden på importnamnet
                name_len = i+5;
                while (name_len < len && buff[name_len] != ';')
                    name_len++;
                name_len-=(i+5);
                char* import_file_name = malloc((name_len+1+5+4*is_sax+strlen(lib))*sizeof(char));
                if (import_file_name == NULL) goto malloc_error;
                buff[i + name_len + 5] = '\0';
                if (is_sax) {
                    memcpy(import_file_name, buff+i+5, name_len*sizeof(char));
                } else {
                    sprintf(import_file_name, "%s%s.fg", lib, buff+i+5);
                }
                int is_dupe = 1;
                char import_file_name_prefix[name_len+7*!is_sax+strlen(lib)*(!is_sax)+1];
                strcpy(import_file_name_prefix, "#");
                import_file_name[name_len+7*!is_sax+strlen(lib)*(!is_sax)] = '\0';
                strcat(import_file_name_prefix, import_file_name);
                char* import_buff = read_file(import_file_name);
                if (!import_buff) {
                    printf("ERR: Kunde inte öppna importfil\n");
                    exit(1);
                }
                // räkna antalet rader i importfilen
                for (int k = 0; k < strlen(import_buff); k++) {
                    if (import_buff[k] == '\n') {
                        import_line_count++;
                    }
                }

                if (find_substring(imports, import_file_name_prefix) == -1) {
                    is_dupe = 0;
                    imports_capacity += name_len+((3+strlen(lib))*(!is_sax))+1;
                    imports = realloc(imports, imports_capacity);
                    strcat(imports, import_file_name);
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

                //move back
                if (is_sax) {
                    chdir(origin_wd);
                }

                break;
            }
        }
        if (found)
            search = 1;
        else search = 0;

    }

    free(imports);
    imports = NULL;

    Bult_Ret ret = {.buff = buff, .import_line_count = import_line_count};

    return ret;

    malloc_error:
        throw_error(ERR_MALLOC, (String){"Memory allocation failed", strlen("Memory allocation failed")}, NULL);
        
}


Program tokenize(char* buff, int debug)
{

    int buff_len = strlen(buff);
    if (debug) printf("BUFF: -----------------------------------------\n%s\n-----------------------------------------------\n", buff);

    // räkna antal instr
    int instruction_amount = 0;
    for (int i = 0; i < buff_len; i++)
    {
        if (buff[i] == ';' || buff[i] == '{' || buff[i] == '}')
            instruction_amount++;
    }
    if (debug) printf("[DEBUG] instruction_amount: %d\n", instruction_amount);

    // räkna ut storleken på pc_to_line

    pc_to_line = malloc(instruction_amount*sizeof(int));
    if (!pc_to_line) goto malloc_error;

    // fyll pc_to_line
    int line = 1;
    int instruction_index = 0;
    for (int i = 0; i < buff_len; i++)
    {
        if (buff[i] == '\n')
            line++;
        if (buff[i] == ';' || buff[i] == '{' || buff[i] == '}')
            pc_to_line[instruction_index++] = line;
    }
    if (debug) {
        printf("[DEBUG] pc_to_line: ");
        for (int i = 0; i < instruction_amount; i++)
            printf("%d\n", pc_to_line[i]);
    }

    // skapa instruktionsarray
    Token **instructions = malloc(instruction_amount * sizeof(Token *));
    if (instructions == NULL) goto malloc_error;
    // grischallokera varje instruktions rad
    for (int i = 0; i < instruction_amount; i++) { // i framtiden bör räkna ut storleken istället för hardcodade 128
        instructions[i] = malloc(128 * sizeof(Token));
        if (instructions[i] == NULL) goto malloc_error;
    }

  
    if (debug) printf("[DEBUG] instructions ptr: %p\n", instructions);

    int i = 0;
    int instructions_OUTER_arr_index = 0;
    int instructions_INNER_arr_index = 0;

    int loop_type = 1;

    //initialize stack
    Stack loops;
    (&loops)->top = -1;
    (&loops)->size = 1;
    (&loops)->max_size = 16;
    (&loops)->arr = malloc((&loops)->max_size*sizeof(int));
    if ((&loops)->arr == NULL) goto malloc_error;

    //loop links
    loop_links = malloc(instruction_amount*sizeof(int));
    if (loop_links == NULL) goto malloc_error;

    while (i < buff_len)
    {
        while (i < buff_len && (buff[i] == ' ' || buff[i] == '\n' || buff[i] == '\r' || buff[i] == '\t'))
            i++;

        // kommentar
        if (buff[i] == '#' && i+1 < buff_len && buff[i+1] != '*'){
            while (i < buff_len && buff[i] != '\n') {
                i++;
                if (buff[i] == ';' || buff[i] == '{' || buff[i] == '}') {
                    instruction_amount--;
                    instructions = realloc(instructions, instruction_amount*sizeof(*instructions));
                    if (instructions == NULL) goto malloc_error;
                }
            }      
            i++;
            continue;      
        }
        

        if (buff[i] == '#' && i+1 < buff_len && buff[i+1] == '*') {
            i += 2;
            while (i+1 < buff_len && !(buff[i] == '*' && buff[i+1] == '#')) {
                if (buff[i] == ';' || buff[i] == '{' || buff[i] == '}') {
                    instruction_amount--;
                    instructions = realloc(instructions, instruction_amount*sizeof(*instructions));
                    if (instructions == NULL) goto malloc_error;
                }
                i++;
            }
            i += 2;
            continue;
        }

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
        else if (strncmp(&buff[i], "junk ", 5) == 0)
        {
            tok.type = JUNK;
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
        else if (strncmp(&buff[i], "slip ", 5) == 0)
        {
            tok.type = SLIP;
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
        else if (strncmp(&buff[i], "och ", 4) == 0)
        {
            tok.type = OCH;
            i += 4;
        }
        else if (strncmp(&buff[i], "eller ", 6) == 0)
        {
            tok.type = ELLER;
            i += 6;
        }
        else if (strncmp(&buff[i], "inte ", 5) == 0)
        {
            tok.type = INTE;
            i += 5;
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
        else if (strncmp(&buff[i], "tpos ", 5) == 0)
        {
            tok.type = TPOS;
            i += 5;
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
        else if (strncmp(&buff[i], "grip", 4) == 0)
        {
            tok.type = GRIP;
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
            //push stack
            if ((&loops)->size >= (&loops)->max_size)
            {
                (&loops)->arr = realloc((&loops)->arr, ((&loops)->max_size + 64)*sizeof(int));
                (&loops)->max_size += 64;
                if ((&loops)->arr == NULL) goto malloc_error;
            }
            (&loops)->arr[++(&loops)->top] = instructions_OUTER_arr_index*loop_type;
            (&loops)->size++;

            if (debug) printf("[DEBUG] Found OPEN_LOOP: _ at instructions[%d][%d]\n", instructions_OUTER_arr_index, instructions_INNER_arr_index);
            //add terminator after
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
            //pop stack
            int other = (&loops)->arr[(&loops)->top];
            (&loops)->top--;
            (&loops)->size--;
            tok.type = CLOSE_LOOP;
            if (other > 0) {
                loop_links[instructions_OUTER_arr_index] = other;
            } else {
                other = abs(other);
                loop_links[instructions_OUTER_arr_index] = instructions_OUTER_arr_index+1;
            }
            loop_links[other]=instructions_OUTER_arr_index;
            if (debug) printf("[DEBUG] Found CLOSE_LOOP: %d at instructions[%d][%d]\n", loop_links[instructions_OUTER_arr_index], instructions_OUTER_arr_index, instructions_INNER_arr_index);
            
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
            if (debug) printf("[DEBUG] TERMINATOR at instructions[%d][%d]\n", instructions_OUTER_arr_index, instructions_INNER_arr_index - 1);
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
            if (debug) printf("[DEBUG] Added token type %d at instructions[%d][%d]\n", tok.type, instructions_OUTER_arr_index, instructions_INNER_arr_index - 1);
        }
    }

    // sätt funktionerna till funktioner & hitta loop id skit
    for (int i = 0; i < instruction_amount; i++)
    {
        if (instructions[i][0].type == FUNCTION)
        {
            instructions[i][1].var.type = VAR_FUNCTION;
            if (debug) printf("[DEBUG] Found token FUNCTION at instructions[%d][0]\n", i);
            // sätt funktionsflaggan på faktiska funktionsanrop (variabel följt av parantes)
            for (int j = i + 1; j < instruction_amount; j++)
            { // rad-loop
                for (int k = 0; instructions[j][k].type != TERMINATOR; k++)
                { // token-loop
                    if (instructions[j][k].type == VARIABLE)
                    {
                        if (instructions[j][k].var.name_len == instructions[i][1].var.name_len
                            && !strncmp(instructions[j][k].var.name, instructions[i][1].var.name, instructions[j][k].var.name_len)
                            && instructions[j][k+1].type == LEFT_PAR)
                        {
                            instructions[j][k].var.type = VAR_FUNCTION;
                        }
                    }
                }
            }
        }
    }

    Program program = {instructions, instruction_amount};
    if (debug) printf("[DEBUG] Tokenize finished. Program.data: %p, instruction_amount: %d\n", program.data, program.instruction_amount);

    // free the grabb
    free((&loops)->arr);
    (&loops)->arr = NULL;
    
    return program;

    malloc_error:
        throw_error(ERR_MALLOC, (String){"Memory allocation failed", strlen("Memory allocation failed")}, NULL);
        exit(1);
        
}

void check_syntax(Program* program){ 
    Token **instructions = program->data;
    int instruction_amount = program->instruction_amount; 

    //checking bracket count
    int openers = 0;
    int closers = 0;
    int opens_loop = 0;

    for (int i = 0; i < instruction_amount; i++){
        
        switch (instructions[i][0].type){
            
            case NAER: ;
                int j = 1;
                int comp_amount = 0;
                int left_args = 0;
                int right_args = 0;
                opens_loop = 0;

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
                        if (instructions[i][j-1].type == NUMBER || 
                            instructions[i][j-1].type == VARIABLE || 
                            instructions[i][j-1].type == STRING || 
                            instructions[i][j-1].type == FUNCTION || 
                            instructions[i][j+1].type == RIGHT_PAR || 
                            instructions[i][j-1].type == RIGHT_BRACKET)
                        {
                            left_args = 1;
                        }

                        comp_amount++;
                    }

                    if (tok == OPEN_LOOP){
                        opens_loop = 1;
                        openers++;
                    }

                    j++;
                }
                if (comp_amount < 1){
                    printf("[NAER]: ERR: Syntax error, instruktion %d, hittade %d jämförelseoperationer när det ska vara minst 1\n", i, comp_amount);
                    exit(-1);
                }
                if (!left_args || !right_args){
                    printf("RIGHT: %d, left: %d\n", right_args, left_args);
                    printf("[NAER]: ERR: Syntax error, instruktion %d, hittade inga värden att jämföra\n", i);
                    exit(-1);
                }
                if (!opens_loop){
                    printf("[NAER]: ERR: Syntax error, instruktion %d, Öppnade ingen loop vid naer\n", i);
                    exit(-1);
                }
                break;

            case GIVET: ;
                j = 1;
                comp_amount = 0;
                left_args = 0;
                right_args = 0;
                int att_exists = 0;
                opens_loop = 0;

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
                        if (instructions[i][j-1].type == NUMBER || 
                            instructions[i][j-1].type == VARIABLE || 
                            instructions[i][j-1].type == STRING || 
                            instructions[i][j-1].type == FUNCTION || 
                            instructions[i][j+1].type == RIGHT_PAR || 
                            instructions[i][j-1].type == RIGHT_BRACKET)
                        {
                            left_args = 1;
                        }

                        comp_amount++;
                    }

                    if (tok == OPEN_LOOP){
                        opens_loop = 1;
                        openers++;
                    }


                    j++;
                }
                
                if (comp_amount < 1){
                    printf("[GIVET]: ERR: Syntax error, instruktion %d, hittade %d jämförelseoperationer när det ska vara minst 1\n", i, comp_amount);
                    exit(-1);
                }
                if (!left_args || !right_args){
                    printf("[GIVET]: ERR: Syntax error, instruktion %d, hittade inga värden att jämföra\n", i);
                    exit(-1);
                }
                if (!opens_loop){
                    printf("[GIVET]: ERR: Syntax error, instruktion %d, Öppnade ingen loop vid givet\n", i);
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
            case FUNCTION: ;
                j = 1;
                opens_loop = 0;
                int found_return = 0;

                int func_argument_count = 0;
                int sig_paren = 0;

                // Count only variables inside the function signature parentheses
                for (j = 2; instructions[i][j].type != TERMINATOR; j++){
                    if (instructions[i][j].type == LEFT_PAR){
                        sig_paren = j;
                        break;
                    }
                }
                if (!sig_paren){
                    printf("[BOUL]: ERR: Syntax error, instruktion %d, funktionsdeklarationen saknar '('\n", i);
                    exit(-1);
                }
                for (int k = sig_paren + 1; instructions[i][k].type != TERMINATOR; k++){
                    if (instructions[i][k].type == RIGHT_PAR) break;
                    if (instructions[i][k].type == VARIABLE) func_argument_count++;
                }

                int func_stop;
                while (instructions[i][j-1].type != TERMINATOR){
                    if (instructions[i][j].type == TERMINATOR){
                        if (j >= 4) break;
                        printf("[BOUL]: ERR: Syntax error, instruktion %d\n", i);
                        exit(-1);
                    }
                    if (instructions[i][j].type == OPEN_LOOP) {
                        opens_loop = 1;
                        openers++;
                    }
                    j++;
                }
                
                // räkna antal argument för ALLA anrop av funktionen
                for (int a = 0; a < instruction_amount; a++){
                    if (instructions[a][0].type == FUNCTION) continue; // skippa funktionsdefinitioner
                    for (int b = 0; instructions[a][b].type != TERMINATOR; b++){
                        Token tok = instructions[a][b];
                        if (tok.type == VARIABLE){
                            if (!strncmp(tok.var.name, instructions[i][1].var.name, tok.var.name_len) && tok.var.name_len == instructions[i][1].var.name_len) {
                                int counted_args = 0;
                                int saw_arg = 0;
                                int depth = 0;
                                int c = b + 1;
                                int list_depth = 0;
                                while (instructions[a][c].type != TERMINATOR && instructions[a][c].type != LEFT_PAR) c++;
                                if (instructions[a][c].type != LEFT_PAR) continue;
                                depth = 1;
                                c++;
                                while (instructions[a][c].type != TERMINATOR && depth > 0){
                                    if (instructions[a][c].type == LEFT_PAR){
                                        depth++;
                                    } else if (instructions[a][c].type == RIGHT_PAR){
                                        depth--;
                                        if (depth == 0) break;
                                    } else if (instructions[a][c].type == LEFT_BRACKET) {
                                        list_depth++;
                                    } else if (instructions[a][c].type == RIGHT_BRACKET) {
                                        list_depth--;
                                    } else if (depth == 1){
                                        if (instructions[a][c].type == COMMA && list_depth == 0){
                                            counted_args++;
                                            saw_arg = 0;
                                        } else {
                                            saw_arg = 1;
                                        }
                                    }
                                    c++;
                                }
                                if (depth != 0){
                                    printf("[BOUL]: ERR: Syntax error, instruktion %d, förväntade ')' efter funktionsanropet '%.*s'\n", a, instructions[i][1].var.name_len, instructions[i][1].var.name);
                                    exit(-1);
                                }
                                if (saw_arg || counted_args > 0) counted_args++;

                                if (counted_args != func_argument_count){
                                    printf("[BOUL]: ERR: Syntax error, instruktion %d, funktionsanropet har %d argument när funktionen '%.*s' kräver %d\n", a, counted_args, instructions[i][1].var.name_len, instructions[i][1].var.name, func_argument_count);
                                    exit(-1);
                                }

                            }
                        }
                    }
                }
                
                if (!opens_loop){
                    printf("[BOUL]: ERR: Syntax error, instruktion %d, Öppnade ingen loop vid funktion\n", i);
                    exit(-1);
                }
                //TODO check not implemented
                /*if (!found_return)){
                    printf("[BOUL]: ERR: Syntax error, instruktion %d, kunde inte hitta RETURN token\n", i);
                    exit(-1);
                }*/
                break;
            case CLOSE_LOOP:
                closers++;
                break;
        }
        if (openers < closers) {
        printf("[CLOSE]: ERR: Syntax error, instruktion %d, ensamt stängande bracket\n", i);
        exit(-1);
        }
    }

    if (openers != closers) {
        printf("ERR: Syntax error, ostängda bracket, öppnar x%d men stänger x%d\n", openers, closers);
        exit(-1);
    }
}

void throw_error(int type, String err_str, Token *instruction){
    char tick = '\'';  
    char colon = ':';
    print_red("Error at line ", strlen("Error at line "), 0);

    if (pc_to_line && program_counter >= 0) {
        int number_of_digits = floor(log10(abs(pc_to_line[program_counter]))) + 1;
        char str[number_of_digits];
        sprintf(str, "%d", pc_to_line[program_counter]);
        print_red(str, number_of_digits, 0);
    }
    
    print_red(&colon, 1, 1);
    if (instruction != NULL)
        print_token_row(instruction);

    switch (type) {


        case ERR_MALLOC:

            print_red("ERR_MALLOC: Could not allocate memory\n", strlen("ERR_MALLOC: Could not allocate memory\n"), 0);
            print_red(err_str.string, err_str.len, 1);     // err_str is a description of the malloc error

            break;
        case ERR_INDEX:
            print_red("ERR_INDEX: Index out of range\n", strlen("ERR_INDEX: Index out of range\n"), 0);
            print_red("Could not index item '", strlen("Could not index item '"), 0);
            print_red(err_str.string, err_str.len, 0);     // err_str is the name of the variable that was attempted to be indexed
            print_red(&tick, 1, 1);

            break;
        case ERR_MATH:
            print_red("ERR_MATH: Invalid math operation\n", strlen("ERR_MATH: Invalid math operation\n"), 0);
            print_red(err_str.string, err_str.len, 1);     // err_str is a description of the math error

            break;
        case ERR_NAME:
            print_red("ERR_NAME: Variable not recognized:\n", strlen("ERR_NAME: Variable not recognized:\n"), 0);
            print_red("Name: '", strlen("Name: '"), 0);
            print_red(err_str.string, err_str.len, 0);     // err_str is the name of the variable that was not recognized
            print_red(&tick, 1, 1);
            break;
        case ERR_SYNTAX:
            print_red("ERR_SYNTAX: Syntax error\n", strlen("ERR_SYNTAX: Syntax error\n"), 0);
            print_red(err_str.string, err_str.len, 1);     // err_str is a description of the syntax error
            break;
        case ERR_TYPE:
            print_red("ERR_TYPE: Invalid type\n", strlen("ERR_TYPE: Invalid type\n"), 0);
            print_red(err_str.string, err_str.len, 1);     // err_str is a description of the type error
            break;
    }
    exit(type);
}

void band(Token *instruction, Token **instructions, int instruction_amount, Scope *scope)
{
    int is_grip = 0;
    int is_slip = 0;
    if (instruction[1].type == SLIP) is_slip = 1;
    else if (instruction[1].type == GRIP) is_grip = 1;
    Token end_var = instruction[1+is_slip+is_grip];
    // ta reda på vilken typ av variabler som används
    
    int start_eval = 0;
    while (instruction[start_eval].type != EQUALS &&
        instruction[start_eval].type != TERMINATOR)
        start_eval++;

    start_eval++; // hoppa över =

    // räkna korrekt antal tokens
    int args_count = 0;
    while (instruction[start_eval + args_count].type != TERMINATOR)
        args_count++;

    Dynamic_Var eval_result = dynamic_eval(instruction+start_eval, args_count, instructions, instruction_amount, scope);

    int type = eval_result.type;


    //printf("BAND_TYPE: %d\n", type);


    // kolla om slutvariabeln finns sparad
    int create_new = 1;
    for (int i = 0; i < (*scope).index; i++)
    {
        // skippa list elements som inte har namn
        if ((*scope).variables[i].name == NULL)
            continue;
        if (end_var.var.name_len == (*scope).variables[i].name_len && !strncmp(end_var.var.name, (*scope).variables[i].name, end_var.var.name_len))
        {
            create_new = 0;
        }
    }
    int index = 0;
    int modify_list_item = 0;
    int* indicies = NULL;
    int depth = 0;
    int index_amount = 0;
    if (instruction[2+is_grip+is_slip].type == LEFT_BRACKET) {
        modify_list_item = 1;
        for (int i = 2+is_grip+is_slip+1; instruction[i].type != TERMINATOR; i++) {
            // hitta alla index som används i list indexering
            if (instruction[i].type == EQUALS) break;

            if (instruction[i].type == LEFT_BRACKET) {
                depth++;
            } else if (instruction[i].type == RIGHT_BRACKET) {
                depth--;
            } else if (depth == 0) {
                index_amount++;
            }
            

        }
        indicies = malloc(index_amount*sizeof(int));
        if (indicies == NULL) goto malloc_error;
        // lägg till alla index (int) i indices, med eval expr
        int index_top = 0;
        depth = 0;
        for (int i = 2+is_grip+is_slip+1; instruction[i].type != TERMINATOR; i++) {
            if (instruction[i].type == EQUALS) break;
            if (instruction[i].type == LEFT_BRACKET) {
                depth++;
            } else if (instruction[i].type == RIGHT_BRACKET) {
                depth--;
            } else if (depth == 0) {
                Dynamic_Var index_eval = dynamic_eval(&instruction[i], 1, instructions, instruction_amount, scope);
                if (index_eval.type != VAR_NUMBER)
                    throw_error(ERR_TYPE, (String){.string = "List index must be a number", .len = strlen("List index must be a number")}, instruction);
                
                indicies[index_top++] = index_eval.value;
            }
            
        }
    }


    if (create_new)
    {
        if (type == VAR_NUMBER)
        {
            create_num_var(end_var.var.name, end_var.var.name_len, eval_result.value, scope);
        }
        else if (type == VAR_LIST)
        {
            create_list_var(end_var.var.name, end_var.var.name_len, eval_result, scope);
        }
        else if (type == VAR_STRING && is_slip == 0)
        {
            create_str_var(end_var.var.name, end_var.var.name_len, eval_result.str_len, eval_result.string, scope);
        } 
        else if (is_slip) 
        {
            // kopiera eval_result.string for att kunna null terminata den för read_file:s skull
            char eval_c_str[eval_result.str_len+1];
            memcpy(eval_c_str, eval_result.string, eval_result.str_len);
            eval_c_str[eval_result.str_len] = '\0';

            char* slip_string = read_file(eval_c_str);
            if (slip_string == NULL) goto malloc_error;
            create_str_var(end_var.var.name, end_var.var.name_len, strlen(slip_string), slip_string, scope);
            free(slip_string);
        }
        
        else 
        {
            throw_error(ERR_TYPE, (String){.string = "Invalid type for variable creation", .len = strlen("Invalid type for variable creation")}, instruction);
            exit(-1);
        }

    } else { // uppdatera istället
        if (type == VAR_NUMBER && !modify_list_item){
            for (int i = 0; i < (*scope).index; i++){
                if ((*scope).variables[i].name == NULL) // hoppa över de som inte har namn!!!
                    continue;
                if ((*scope).variables[i].name_len == end_var.var.name_len && !strncmp(end_var.var.name, (*scope).variables[i].name, end_var.var.name_len)){
                    (*scope).variables[i].value = eval_result.value;
                    (*scope).variables[i].type = VAR_NUMBER;
                    (*scope).variables[i].str_ptr = 0;
                    (*scope).variables[i].len = 0;
                }
            }
        }
        else if (type == VAR_STRING && is_slip == 0 && !modify_list_item){
            for (int i = 0; i < (*scope).index; i++){
                if ((*scope).variables[i].name == NULL)
                    continue;
                if (end_var.var.name_len == (*scope).variables[i].name_len && !strncmp(end_var.var.name, (*scope).variables[i].name, end_var.var.name_len)){
                    free((*scope).variables[i].str_ptr);
                    (*scope).variables[i].value = 0;
                    (*scope).variables[i].str_ptr = eval_result.string;
                    (*scope).variables[i].len = eval_result.str_len;
                    (*scope).variables[i].type = VAR_STRING;
                }
            }
        } else if (modify_list_item){
            Variable new_list_item = {
                .len = eval_result.str_len,
                .name = eval_result.string,
                .name_len = eval_result.str_len,
                .str_ptr = eval_result.string,
                .type = eval_result.type,
                .value = eval_result.value,
                .list_ptr = eval_result.list_ptr
            };

            change_list_item(end_var.var.name, end_var.var.name_len, indicies, new_list_item, scope, index_amount);
  
        } else if (type == VAR_LIST){
            for (int i = 0; i < (*scope).index; i++){
                if ((*scope).variables[i].name == NULL)
                    continue;
                if (end_var.var.name_len == (*scope).variables[i].name_len && !strncmp(end_var.var.name, (*scope).variables[i].name, end_var.var.name_len)){
                    (*scope).variables[i].type = VAR_LIST;
                    (*scope).variables[i].list_ptr = eval_result.list_ptr;
                    (*scope).variables[i].len = eval_result.str_len;
                    (*scope).variables[i].str_ptr = NULL;
                    (*scope).variables[i].value = 0;
                }
            }
        } else if (is_slip){
            for (int i = 0; i < (*scope).index; i++){
                if ((*scope).variables[i].name == NULL)
                    continue;
                if (end_var.var.name_len == (*scope).variables[i].name_len && !strncmp(end_var.var.name, (*scope).variables[i].name, end_var.var.name_len)){
                    free((*scope).variables[i].str_ptr);
                    (*scope).variables[i].value = 0;
                    // kopiera eval_result.string for att kunna null terminata den för read_file:s skull
                    char eval_c_str[eval_result.str_len+1];
                    memcpy(eval_c_str, eval_result.string, eval_result.str_len);
                    eval_c_str[eval_result.str_len] = '\0';

                    char* slip_string = read_file(eval_c_str);
                    if (slip_string == NULL) goto malloc_error;
                    
                    (*scope).variables[i].str_ptr = slip_string;
                    (*scope).variables[i].len = strlen(slip_string);

                    (*scope).variables[i].type = VAR_STRING;
                }
            }

        }
    }

    return;

    malloc_error:
        throw_error(ERR_MALLOC, (String){"Memory allocation failed", strlen("Memory allocation failed")}, NULL);

}

void foug(Token *instruction, Scope *scope) {
    int is_svets = 0;
    int is_junk = 0;
    for (int i = 0; instruction[i].type != TERMINATOR; i++){
        if (instruction[i].type == SVETS) is_svets = 1;
        else if (instruction[i].type == JUNK) is_junk = 1;
    }
    
    // printf("FOUG KALLAD PÅ\n");
    if (!is_svets)
    {
        if (instruction[1+is_junk].type == STRING)
        {
            // printf("STRING I FOUG\n");
            for (int i = 0; i < instruction[1+is_junk].var.name_len; i++)
            {
                if (instruction[1+is_junk].var.name[i] == '\\' && i+1 < instruction[1+is_junk].var.name_len && instruction[1+is_junk].var.name[i + 1] == 'n')
                {
                    printf("\n");
                    i += 2;
                }
                if (i < instruction[1+is_junk].var.name_len) {
                    if (is_junk) 
                        print_red(&instruction[1+is_junk].var.name[i], 1, 0);
                    else 
                        printf("%c", instruction[1+is_junk].var.name[i]);
                }        
            }
        }
        else if (instruction[1+is_junk].type == VARIABLE)
        {
            // printf("VARIABLE I FOUG\n");
            // print variable info before lookup:
            Dynamic_Var value = get_var_value(instruction[1+is_junk].var.name, instruction[1+is_junk].var.name_len, 0, 0, scope);
            print_variable(value, is_junk);
            printf("\n");
        }
        else
        {
            throw_error(ERR_TYPE, (String){"Invalid type for output", strlen("Invalid type for output")}, instruction);
        }
    }
    else
    { // svets-string
        for (int i = 0; i < instruction[2+is_junk].var.name_len; i++)
        {
            if (instruction[2+is_junk].var.name[i] == '\\' && instruction[2+is_junk].var.name[i + 1] == 'n') // printa \n
            {
                printf("\n");
                i += 2;
            }
            if (instruction[2+is_junk].var.name[i] == '\\' && instruction[2+is_junk].var.name[i + 1] == '%') // printa %
            {
                if (is_junk)
                    print_red("%", 1, 0);
                else
                    printf("%%");
                i += 2;
            }

            if (instruction[2+is_junk].var.name[i] == '%')
            {
                // kolla längden på den
                int len = 0;
                for (int j = i + 1; j < instruction[2+is_junk].var.name_len; j++)
                {
                    if (instruction[2+is_junk].var.name[j] == '%')
                        break;
                    len++;
                }
                Dynamic_Var value_svets = get_var_value(instruction[2+is_junk].var.name+i+1, len, 0, 0, scope);
                print_variable(value_svets, is_junk);
                i += len + 1;
            }
            else
            {
                if (i < instruction[2+is_junk].var.name_len) {
                    if (is_junk) 
                        print_red(&instruction[2+is_junk].var.name[i], 1, 0);
                    else 
                        printf("%c", instruction[2+is_junk].var.name[i]);
                }
            }
        }
    }
}

void loop(Token *instruction, Program program, Scope *scope, int keyword_count){
    int len = 0;
    while (instruction[len].type != OPEN_LOOP) len++;
    len -=keyword_count;
    int do_statement = logic_eval(instruction+keyword_count, len, program.data, program.instruction_amount, scope);
    //printf("loop do statement: %d\n", do_statement);
    
    if (!do_statement)
    {
        program_counter = loop_links[program_counter];
        return;
    }
}

void tpos(Token *instruction, Scope *scope)
{
    // allocation: reserve space for NUL
    int call_len = instruction[1 + (instruction[1].type == SVETS)].var.name_len + 1;
    char *call = malloc(call_len * sizeof(char));
    if (call == NULL)
    {
        throw_error(ERR_MALLOC, (String){"Memory allocation failed", strlen("Memory allocation failed")}, NULL);
    }
    call[0] = '\0'; // fogligt sätt att nullterminera sträng direkt
    int writer = 0;
    
    if (instruction[1].type != SVETS && instruction[2].type != SVETS) {
        if (instruction[1].type == STRING) {
            for (int i = 0; i < instruction[1].var.name_len; i++) {
                if (instruction[1].var.name[i] == '\\' && instruction[1].var.name[i + 1] == 'n') {
                    i += 2;
                }
                if (i < instruction[1].var.name_len) {
                    sprintf(call + writer, "%c", instruction[1].var.name[i]);
                    writer++;
                }  
            }
        }
        else if (instruction[1].type == VARIABLE || instruction[2].type == VARIABLE)
        {
            // printf("VARIABLE I TPOS\n");
            Dynamic_Var value = get_var_value(instruction[1].var.name, instruction[1].var.name_len, 0, 0, scope);
                if (value.type == VAR_NUMBER){
                    if ((int)(value.value) == (value.value)) {
                        call_len += 32; // extra biffigt utrymme
                        char *tmp = realloc(call, call_len);
                        if (!tmp) { 
                            free(call); 
                            throw_error(ERR_MALLOC, (String){"Memory allocation failed", strlen("Memory allocation failed")}, NULL); 
                        }
                        call = tmp;
                        int n = snprintf(call + writer, call_len-writer, "%d", (int)value.value);
                        if (n > 0) writer += n;
                    } else {
                        call_len += 64;
                        char *tmp = realloc(call, call_len);
                        if (!tmp) { 
                            free(call); 
                            throw_error(ERR_MALLOC, (String){"Memory allocation failed", strlen("Memory allocation failed")}, NULL); 
                        }
                        call = tmp;
                        int n = snprintf(call + writer, call_len - writer, "%lf", (double)value.value);
                        if (n > 0) writer += n;
                    }
                } else if (value.type == VAR_STRING) {
                    call_len += value.str_len + 1;
                    char *tmp = realloc(call, call_len);
                    if (!tmp) { 
                        free(call); 
                        throw_error(ERR_MALLOC, (String){"Memory allocation failed", strlen("Memory allocation failed")}, NULL); 
                    }
                    call = tmp;
                    int n = snprintf(call + writer, call_len - writer, "%.*s", value.str_len, value.string);
                    if (n > 0) writer += n;
                }
            else
            {
                throw_error(ERR_TYPE, (String){"Invalid type for output", strlen("Invalid type for output")}, instruction);
            }
                
        }
        else
        {
            throw_error(ERR_SYNTAX, (String){"Expected string or variable", strlen("Expected string or variable")}, instruction);
            exit(-1);
        }
    } else { // svets-string
        for (int i = 0; i < instruction[2].var.name_len; i++){
            if (instruction[2].var.name[i] == '\\' && instruction[2].var.name[i + 1] == 'n') // printa \n
            {
                if (writer + 1 >= call_len) {
                    call_len += 16;
                    char *tmp = realloc(call, call_len);
                    if (!tmp) { 
                        free(call); 
                        throw_error(ERR_MALLOC, (String){"Memory allocation failed", strlen("Memory allocation failed")}, NULL); 
                    }
                    call = tmp;
                }
                call[writer++] = '\n';
                call[writer] = '\0';
                i += 2;
            }
            if (instruction[2].var.name[i] == '\\' && instruction[2].var.name[i + 1] == '%') // printa %
            {
                if (writer + 2 >= call_len) {
                    call_len += 16;
                    char *tmp = realloc(call, call_len);
                    if (!tmp) { 
                        free(call); 
                        throw_error(ERR_MALLOC, (String){"Memory allocation failed", strlen("Memory allocation failed")}, NULL); 
                    }
                    call = tmp;
                }
                call[writer++] = '%';
                call[writer++] = '%';
                call[writer] = '\0';
                i += 2;
            }

            if (instruction[2].var.name[i] == '%'){
                // kolla längden på den
                int len = 0;
                for (int j = i+1; j < instruction[2].var.name_len; j++){
                    if (instruction[2].var.name[j] == '%') break;
                    len++;
                }
                Dynamic_Var value = get_var_value(instruction[2].var.name + i + 1, len, 0, 0, scope);
                if (value.type == VAR_NUMBER){
                    if ((int)(value.value) == (value.value)) {
                        call_len += 32;
                        char *tmp = realloc(call, call_len);
                        if (!tmp) { 
                            free(call); 
                            throw_error(ERR_MALLOC, (String){"Memory allocation failed", strlen("Memory allocation failed")}, NULL); 
                        }
                        call = tmp;
                        int n = snprintf(call + writer, call_len - writer, "%d", (int)value.value);
                        if (n > 0) writer += n;
                    } else {
                        call_len += 64;
                        char *tmp = realloc(call, call_len);
                        if (!tmp) { 
                            free(call); 
                            throw_error(ERR_MALLOC, (String){"Memory allocation failed", strlen("Memory allocation failed")}, NULL); 
                        }
                        call = tmp;
                        int n = snprintf(call + writer, call_len - writer, "%lf", (double)value.value);
                        if (n > 0) writer += n;
                    }
                } else if (value.type == VAR_STRING) {
                    call_len += value.str_len + 1;
                    char *tmp = realloc(call, call_len);
                    if (!tmp) { 
                        free(call); 
                        throw_error(ERR_MALLOC, (String){"Memory allocation failed", strlen("Memory allocation failed")}, NULL); 
                    }
                    call = tmp;
                    int n = snprintf(call + writer, call_len - writer, "%.*s", value.str_len, value.string);
                    if (n > 0) writer += n;
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

Dynamic_Var call_function(char *name, int name_len, int origin_program_counter, Token **instructions, int instruction_amount, Token* instruction, Scope* old_scope)
{
    // börja med att hitta argument
    // städa upp instruction

    Scope scope = {
        .index = 0,
        .capacity = 128,
        .variables = malloc(128 * sizeof(Variable)),
    };
    
    // räkna antal formella argument
    int amount_of_args = 1;
    int paren_depth = 0;
    int bracket_depth = 0;
    for (int i = 2; instruction[i].type != TERMINATOR; i++){
        if (instruction[i].type == LEFT_PAR) paren_depth++;
        else if (instruction[i].type == RIGHT_PAR) paren_depth--;
        else if (instruction[i].type == LEFT_BRACKET) bracket_depth++;
        else if (instruction[i].type == RIGHT_BRACKET) bracket_depth--;
        // only count commas that are not inside parentheses or brackets
        if (instruction[i].type == COMMA && paren_depth == 0 && bracket_depth == 0) amount_of_args++;
    }

    typedef struct {
        int start_index;
        int len;
        Dynamic_Var info;
    } Arg;

    int arg_count = 0;
    int paren_depth2 = 0;
    int bracket_depth2 = 0;
    int start = 2; // första token efter (

    Arg arg_info[amount_of_args];

    int arg_tokens_len = 0;
    for (int i = 2; instruction[i].type != TERMINATOR; i++) {
        if (instruction[i].type == LEFT_PAR) paren_depth2++;
        else if (instruction[i].type == RIGHT_PAR) {
            if (paren_depth2 == 0) {
                // sista argumentet
                arg_info[arg_count].start_index = start;
                arg_info[arg_count].len = i - start;
                arg_count++;
                arg_tokens_len = i - 2 + 1;
                break;
            }
            paren_depth2--;
        } else if (instruction[i].type == LEFT_BRACKET) bracket_depth2++;
        else if (instruction[i].type == RIGHT_BRACKET) bracket_depth2--;

        // comma separerar argument om den inte är inuti parenteser eller brackets
        if (instruction[i].type == COMMA && paren_depth2 == 0 && bracket_depth2 == 0) {
            arg_info[arg_count].start_index = start;
            arg_info[arg_count].len = i - start;
            arg_count++;
            start = i + 1;
        }
    }

    if (arg_tokens_len == 0) {
        throw_error(ERR_SYNTAX, (String){"Syntax error in function call: missing closing parenthesis", strlen("Syntax error in function call: missing closing parenthesis")}, instruction);
    }
    cleanup_args(instruction+2, arg_tokens_len, instructions, instruction_amount, old_scope);
    // skapa Dynamic_Var för varje värde
    for (int i = 0; i < arg_count; i++){
        
        Dynamic_Var eval_ret = dynamic_eval(instruction+arg_info[i].start_index, arg_info[i].len, instructions, instruction_amount, old_scope);
        
        //printf("==================================\n");
        //print_token_row(instruction);
        //printf("\n\n\n");
        arg_info[i].info = eval_ret;
    }
    // hitta index dit den ska hoppa
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
        throw_error(ERR_SYNTAX, (String){"Function not found", strlen("Function not found")}, instruction);
    }

    // skapa variabler
    int arg_number = 0;
    for (int i = 3; instructions[func_index][i].type != TERMINATOR; i++){
        Token current = instructions[func_index][i];
        if (current.type == VARIABLE){
            int type = arg_info[arg_number].info.type;

            if (type == VAR_NUMBER){
                double value = arg_info[arg_number].info.value;
                create_num_var(current.var.name, current.var.name_len, value, &scope);

            } else if (type == VAR_STRING){
                int str_len = arg_info[arg_number].info.str_len;
                char* string = arg_info[arg_number].info.string;
                create_str_var(current.var.name, current.var.name_len, str_len, string, &scope);
            } else if (type == VAR_LIST){
                create_list_var(current.var.name, current.var.name_len, arg_info[arg_number].info, &scope);
            }

            arg_number++;
        }
    }

    //print_variables(&scope);
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

    // skapa argumenten/parametermaxxing

    while (function_stack_top > call_stack_level)
    {
        if (program_counter >= instruction_amount)
        {
            throw_error(ERR_SYNTAX, (String){"Function reached end of file without return", strlen("Function reached end of file without return")}, NULL);
        }
        Token *current = instructions[program_counter];
        interpret_instruction(current, instructions, instruction_amount, &scope);
        if (function_stack_top <= call_stack_level)
            break;
        program_counter++;
    }
    //free up
    free(scope.variables);
    scope.variables = NULL;

    Dynamic_Var ret = function_return_stack[call_stack_level];
    return ret;

    malloc_error:
        throw_error(ERR_MALLOC, (String){"Memory allocation failed", strlen("Memory allocation failed")}, NULL);
        exit(1);
        
}

void interpret_instruction(Token *current, Token **instructions, int instruction_amount, Scope *scope)
{
    switch (current[0].type)
    {
    case FOUG:
        foug(current, scope);
        break;

    case BAND:
        band(current, instructions, instruction_amount, scope);
        break;

    case GIVET:
        loop(current, (Program){instructions, instruction_amount}, scope, 2);
        break;

    case NAER:
        loop(current, (Program){instructions, instruction_amount}, scope, 1);
        break;

    case TPOS:
        tpos(current, scope);
        break;
    case CLOSE_LOOP:
        program_counter = loop_links[program_counter]-1;
        break;

    case RETURN:
    {
        Dynamic_Var return_value = { 0 };

        // räkna hur lång eval strängen blir
        int len = 0;
        while(current[len].type != TERMINATOR) len++;
        len--;

        return_value = dynamic_eval(current+1, len, instructions, instruction_amount, scope);

        function_return_stack[function_stack_top - 1] = return_value;
        program_counter = function_origin_program_counter_stack[function_stack_top - 1];
        function_stack_top--;

        break;
    }

    case VARIABLE: // anta att det är en funktion
        if (current[0].var.type == VAR_FUNCTION) {
            call_function(current[0].var.name, current[0].var.name_len, program_counter, instructions, instruction_amount, current, scope);
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

    // kolla om -d finns

    int debug = 0;
    if (argc > 2){
        if (!strncmp(argv[2], "-d", 2)){
            debug = 1;
        }
    }
    
    //define the difference in path of file and cwd
    strcpy(path_diff, argv[1]);
    char* path_ptr = &path_diff[0];
    while (strchr(path_ptr, '/') != NULL)
    {
        path_ptr++;
    }
    path_ptr[-1] = '\0';
    char* user = malloc(PATH_MAX);
    user = getenv("SUDO_USER") ? getenv("SUDO_USER") : getenv("USER");

    // skapa konstantarrays
    // variabler
    Scope scope = {
        .index = 0,
        .capacity = 128,
        .variables = malloc(128 * sizeof(Variable))
    };

    // function stack
    function_origin_program_counter_stack = malloc(128 * sizeof(int));
    function_return_stack = malloc(128 * sizeof(Dynamic_Var));

    // row stack
    row_lengths = malloc(128 * sizeof(int));
    // fyll row_lengths med 128 i varje position
    for (int i = 0; i < 128; i++) row_lengths[i] = 128;





    if (scope.variables == NULL ||
        function_origin_program_counter_stack == NULL ||
        function_return_stack == NULL || row_lengths == NULL)
        goto malloc_error;


    Bult_Ret bult_ret = bult(argv[1], user);
    char* buff = bult_ret.buff;
    int line_offset = bult_ret.import_line_count;


    
    Program program = tokenize(buff, debug);
    
    Token **instructions = program.data;
    int instruction_amount = program.instruction_amount;
    check_syntax(&program);
    if (debug) print_tokens(instructions, instruction_amount);

    // lägg på offset
    for (int i = 0; i < instruction_amount; i++) {
        pc_to_line[i] -= line_offset;
    }

    // hitta entry point (main)
    for (int i = 0; i < instruction_amount; i++)
    {
        if (instructions[i][0].type == MAIN)
            program_counter = i;
    }
    if (program_counter == -1)
    {
        throw_error(ERR_SYNTAX, (String){"Main token not found", strlen("Main token not found")}, NULL);
    }


    while (program_counter < instruction_amount)
    {
        Token *current = instructions[program_counter];

        interpret_instruction(current, instructions, instruction_amount, &scope);

        program_counter++;
    }

    if (debug) print_variables(&scope);
    return 0;

    malloc_error:
        throw_error(ERR_MALLOC, (String){"Memory allocation failed", strlen("Memory allocation failed")}, NULL);
        exit(1);
}


