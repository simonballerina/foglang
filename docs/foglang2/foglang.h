

enum Var_type
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
} Dynamic_Var;

typedef struct Scope {
    Variable *variables;
    int index;
    int capacity;
} Scope;

enum Tok_type
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
    COMMA,         // 30
    TPOS,          // 31
    SLIP,          // 32
    GRIP,          // 33
    OCH,           // 34
    ELLER,         // 35
    INTE           // 36
};

typedef struct
{
    Token (*data)[128];
    int instruction_amount;
} Program;

// utils
double str_to_double(char *num);
char *read_file(const char *filename);
void print_tokens(Token instructions[][128], int instruction_amount);
void debug_print_var(char *name, int len);
void print_variables(Scope *scope);
int find_substring(char *txt, char *pat);

double evaluate_expression(Token *args_old, int args_amount, Token (*instructions)[128], int instruction_amount, Scope *scope);
String evaluate_str_expression(Token *args_old, int args_amount, Token (*instructions)[128], int instruction_amount, Scope *scope);
void cleanup_args(Token* args, int args_amount, Token (*instructions)[128], int instruction_amount, Scope *scope);
Dynamic_Var dynamic_eval(Token *args, int args_amount, Token (*instructions)[128], int instruction_amount, Scope *scope);



void create_str_var(char *name, int name_len, int len, char *string, Scope *scope);
void create_num_var(char *name, int name_len, double value, Scope *scope);
void create_list_var(char *name, int name_len, Token *values, Token (*instructions)[128], int instruction_amount, Scope *scope);

/*
name: char* till variabelnamn. 
length: längden på namnsträngen. 
type: VAR_LIST om du indexerar en lista, annars 0. 
index: indexeringen på listan, annars 0
*/
Dynamic_Var get_var_value(char *name, int length, int type, double index, Scope *scope);
void change_list_item(char* name, int name_len, int index, Variable new_var, Scope *scope);

// declarations som inte är i foglang_var.c eller foglang_eval.c
Dynamic_Var call_function(char *name, int name_len, int origin_program_counter, Token (*instructions)[128], int instruction_amount, Token* instruction, Scope* old_scope);
void interpret_instruction(Token *current, Token (*instructions)[128], int instruction_amount, Scope *scope);

