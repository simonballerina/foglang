#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

double get_var_value(char *variables_names, double *variables_values, int* global_var_index, char var_name);

double str_to_double(char* num){
    int len = strlen(num);
    // kolla om '-' eller '.' finns
    int negative = 0;
    int j;
    int power = len-1;
    for (int i = 0; i < len; i++){
        if (num[i] == '-'){
            negative = 1;
            power--;
            
        }
        if (num[i] == '.'){
            power = -1-negative;
            for (j = 0; j < len; j++){
                if (num[j] == '.') break;
                power++;
            }
        }
    }

    double sum = 0;    
   
    
    for (int i = negative; i < len; i++){
        if (num[i] == '.' || num[i] == ' ') continue;
        sum += (num[i] - '0') * pow(10, power);
        
        power--;
    }

    if (negative) sum = sum*-1;
    
    return sum;


}


double evaluate(double var1, double var2, char operation){
    if (operation == '+') return (var1+var2);
    if (operation == '-') return (var1-var2);
    if (operation == 'x') return (var1*var2);
    if (operation == '^') return (pow(var1, var2));
    if (operation == '/') {
        if (var2 == 0) {
            printf("fog:~$ ERR: Division med noll (/)\n");
            exit(-1);
        }
        return (var1/var2);
    }
    if (operation == '%') {
        if ((int)var2 == 0) {
            printf("fog:~$ ERR: Division med noll (%%)\n");
            exit(-1);
        }
        return ((int)var1%(int)var2);
    }
    return -1;
}

int compare(double var1, double var2, char operation){
    if (operation == '=') return (var1 == var2);
    if (operation == '!') return (var1 != var2);
    if (operation == '>') return (var1 > var2);
    if (operation == '<') return (var1 < var2);
    return -1;
}


typedef struct
{
    char type;
    double value; // for numbers
    char var;     // for variables
} token;

double evaluate_expression(char* buff, char* variables_names, double* variables_values, int* global_var_index)
{
    token token_list[128];
    int token_list_index = 0;

    int i = 0;
    int len = strlen(buff);
    while (i < len)
    {
        while (buff[i] == ' ')
        {
            i++;
            continue;
        }

        token tok;
        tok.type = '\0';
        tok.value = 0;
        tok.var = '\0';
        switch (buff[i])
        {
        case '(':
            tok.type = '(';
            break;
        case ')':
            tok.type = ')';
            break;
        case '!':
            tok.type = 'v';
            if (i + 1 < len)
            {
                tok.var = buff[i + 1];
                // inkrementera inte: det göers senare
            }
            break;
        case '+':
            tok.type = '+';
            break;
        case '-':
            tok.type = '-';
            break;
        case '*':
            tok.type = '*';
            break;
        case '^':
            tok.type = '^';
            break;
        case '%':
            tok.type = '%';
            break;
        case '/':
            tok.type = '/';
            break;
        default:
            char number[64];
            if (isdigit(buff[i]))
            {
                int j;
                for (j = i; j < len && isdigit(buff[j]); j++)
                {
                    number[j - i] = buff[j];
                }
                number[j - i] = '\0';
                tok.type = 'n';
                tok.value = str_to_double(number);
                i = j - 1;  
            }

            break;
        }

        token_list[token_list_index] = tok;
        token_list_index++;
        
        if (buff[i] == '!') {
            i += 2;  // skippa ! och variabelnamn
        } else {
            i++;
        }

        /*for (int k = 0; k < token_list_index; k++){
            printf("TOKEN: \n   type: %c\n  value: %lf\n    var: %c\n", token_list[k].type, token_list[k].value, token_list[k].var);
        }*/
    }

    //printf("//////////////////////////\n");
    int a = 0;
    while (1)
    {
        /*for (int k = 0; k < token_list_index; k++){
            printf("TOKEN: \n   type: %c\n  value: %lf\n    var: %c\n", token_list[k].type, token_list[k].value, token_list[k].var);
        }*/



        //ta bort paranteserna ifall man har något som (+) eller (v)
        for (int i = 0; i < token_list_index; i++){
            if (token_list[i].type == '('){
                int valid_token_count = 0;
                int j = i + 1;
                for (j; j < token_list_index; j++){
                    if (token_list[j].type == ')') break;
                    if (token_list[j].type != '\0' && token_list[j].type != ')') {
                        valid_token_count++;
                    }
                }
                if (valid_token_count == 1){
                    token_list[i].type = '\0';
                    token_list[j].type = '\0';
                }
            }
        }

        int operational_token_count = 0;

        for (int j = 0; j < token_list_index; j++)
        {
            if (token_list[j].type != '\0')
                operational_token_count++;
        }



        int start_index = -1;
        int end_index = -1;
        for (int j = token_list_index - 1; j >= 0; j--)
        {
            if (token_list[j].type == '(')
            {
                start_index = j;
                break;
            }
        }
        for (int j = 0; j < token_list_index; j++)
        {
            if (token_list[j].type == ')')
            {
                end_index = j;
                break;
            }
        }
        if (start_index == -1)
            start_index = 0;
        if (end_index == -1)
            end_index = token_list_index;
        token args[128];
        
        for (int c = 0; c < 128; c++) {
            args[c].type = '\0';
            args[c].value = 0;
            args[c].var = '\0';
        }

        for (int j = start_index; j < end_index; j++)
        {
            args[j - start_index] = token_list[j];
        }
        for (int j = 0; j < end_index - start_index; j++)
        {
            if (!args[j].type)
                continue;
            if (args[j].type == 'v')
            {
                args[j].value = get_var_value(variables_names, variables_values, global_var_index, args[j].var);
                args[j].type = 'n';
            }
        }



        //printf("VALID_COUNT: %d\n", operational_token_count);
        if (operational_token_count <= 1) {
            for (int a = 0; a < token_list_index; a++){
                if (token_list[a].type == 'n') {
                    //printf("SLUTRESULTAT: %lf\n", token_list[a].value);
                    return token_list[a].value;
                }
            }
            break;
        }
        //printf("--------------------\n");


        for (int j = 0; j < end_index - start_index; j++)
        {
            if (args[j].type == '\0')
                continue;

            if (args[j].type == '*' || args[j].type == '/' || args[j].type == '+' || args[j].type == '-' || args[j].type == '%' || args[j].type == '^')
            {
                double first_arg;
                double second_arg;
                int k;
                int l;
                char op_type = args[j].type;  // spara aritmetisk op innan den ändars
                for (k = 1; j + k < end_index - start_index; k++)
                {
                    if (args[j + k].type == 'n')
                    {
                        first_arg = args[j + k].value;
                        break;
                    }
                }
                for (l = -1; l >= -j; l--)
                {
                    if (args[j + l].type == 'n')
                    {
                        second_arg = args[j + l].value;
                        break;
                    }
                }
                //printf("j+k: %d\n", j+k);
                //printf("j+l: %d\n", j+l);
                //printf("ARGS[j+k]: %lf\n", args[j+k].value);
                //printf("ARGS[j+l]: %lf\n", args[j+l].value);
                //printf("OP_TYPE: %c\n", op_type);

                args[j].type = 'n';
                args[j + k].type = '\0';
                args[j + l].type = '\0';

                //printf("FIRST_ARG: %lf\n", first_arg);
                //printf("SECOND_ARG: %lf\n", second_arg);

                double result;
                switch (op_type) // av någon jävla anledning så funkar bara second_arg <icke-kommutativ aritmetisk operation> first_arg istället för tvärtom
                {
                case '*':
                    result = (first_arg) * (second_arg);
                    break;
                case '^':
                    result = pow(second_arg, first_arg);
                    break;
                case '/':
                    result = (second_arg) / (first_arg);
                    break;
                case '%':
                    result = ((int)second_arg)/((int)first_arg);
                    break;
                case '+':
                    result = (first_arg) + (second_arg);
                    break;
                case '-':
                    result = (second_arg) - (first_arg);
                    break;
                }
                //printf("RESULT: %lf\n", result);
                args[j].value = result;

                // "bortkommentera" delarna av strängen som e fixade
                for (int c = start_index; c <= end_index; c++){
                    token_list[c].type = '\0';
                }
                // spara resultatet i tokenlistan
                token_list[start_index].type = 'n';
                token_list[start_index].value = result;
                token_list[end_index].type = '\0';

            }

        }
    }
}

