#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#define VAR_AMOUNT 64

double get_var_value(char *variables_names, double *variables_values, int* global_var_index, char var_name)
{
    for (int i = 0; i < *global_var_index; i++)
    {
        if (variables_names[i] == var_name)
            return variables_values[i];
    }
    printf("ERR: Kunde inte ta fram ett numeriskt variabelvärde: ");
    printf("%c\n", var_name);
    exit(-1);
}

#include "math_ops.c"

int main(){
    char variables_names[64] = {0};
    double variables_values[64] = {0};
    variables_names[0] = 'i';
    variables_values[0] = 4;
    int global_var_index = 2;
    printf("Testing: (!i)\n");
    double k = evaluate_expression("!i", variables_names, variables_values, &global_var_index);
    printf("Result: %lf\n", k);
    return 0;
}
