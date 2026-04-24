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


void print_tokens(Token** instructions, int instruction_amount)
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


void debug_print_var(char *name, int len)
{
    printf("§");
    for (int i = 0; i < len; i++)
    {
        printf("%c", name[i]);
    }
    printf("§\n");
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

        if ((*scope).variables[i].type == VAR_LIST)
        {
            printf("    Lista (len: %d):\n", (*scope).variables[i].len);
            print_dynamic_items((*scope).variables[i].list_ptr, (*scope).variables[i].len, 2);
        }
        else
        {
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


void print_red(char* str, int len, int print_backslash) {

    #ifdef _WIN32
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
        fprintf(stderr, "%.*s", len, str);
        if (print_backslash) fprintf(stderr, "\n");
        SetConsoleTextAttribute(hConsole, 7);  // reset
    #else
        fprintf(stderr, "\033[31m%.*s\033[0m", len, str);
        if (print_backslash) fprintf(stderr, "\n");
    #endif

}


static void print_dynamic_items(Dynamic_Var *items, int len, int indent)
{
    for (int j = 0; j < len; j++)
    {
        for (int sp = 0; sp < indent; sp++)
        {
            printf("  ");
        }

        printf("%d: Type: %d    Value: %lf    List/String_len: %d   String: '",
               j,
               items[j].type,
               items[j].value,
               items[j].str_len);

        if (items[j].type == VAR_STRING && items[j].string != NULL)
        {
            for (int k = 0; k < items[j].str_len; k++)
            {
                printf("%c", items[j].string[k]);
            }
        }

        printf("'\n");

        if (items[j].type == VAR_LIST)
        {
            print_dynamic_items(items[j].list_ptr, items[j].str_len, indent + 4);
        }
    }
}


void print_token_row(Token* args){
for (int j = 0; args[j].type != TERMINATOR; j++)
        {
            switch (args[j].type)
            {
            case FOUG:
                printf("'FOUG'    ");
                break;
            case JUNK:
                printf("'JUNK'    ");
                break;
            case BAND:
                printf("'BAND'    ");
                break;
            case SLIP:
                printf("'SLIP'    ");
                break;
            case GRIP:
                printf("'GRIP'    ");
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
                if (args[j].var.type == VAR_FUNCTION)
                    printf("f ");
                for (int k = 0; k < args[j].var.name_len; k++)
                {
                    printf("%c", *(args[j].var.name + k));
                }
                printf("'    ");
                break;
            case STRING:
                printf("'");
                for (int k = 0; k < args[j].var.name_len; k++)
                {
                    printf("%c", *(args[j].var.name + k));
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
                printf("'%lf'    ", args[j].value);
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
            }
        }
        printf("\n");
}

static int is_top_level_list_literal(Token *args, int args_amount)
{
    int start = 0;
    while (start < args_amount && args[start].type == NONE) start++;
    if (start >= args_amount || args[start].type != LEFT_BRACKET) return 0;

    int depth = 0;
    for (int i = start; i < args_amount; i++) {
        if (args[i].type == LEFT_BRACKET) {
            depth++;
        } else if (args[i].type == RIGHT_BRACKET) {
            depth--;
            if (depth == 0) {
                int j = i + 1;
                while (j < args_amount && args[j].type == NONE) j++;
                return j == args_amount;
            }
        }
    }
    return 0;
}






