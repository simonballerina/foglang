

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

typedef struct Variable Variable;
typedef struct Dynamic_Var Dynamic_Var;

typedef struct List
{
    int len;
    Dynamic_Var* items;
} List;

typedef struct Variable
{
    int type;
    char *name;
    int name_len;
    double value;
    int len;     // längden på listan / strängen
    char *str_ptr;
    Dynamic_Var* list_ptr;
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
    List* list_ptr;
} Token;

typedef struct
{
    char* string;
    int len;
} String;

typedef struct Dynamic_Var
{
    char* string;
    int str_len;
    double value;
    int type;
    struct Dynamic_Var* list_ptr;
} Dynamic_Var;

typedef struct
{
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
    SLIP,          // 14
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
    OPEN_LOOP,     // 32
    CLOSE_LOOP,    // 33
    GRIP,          // 34
    OCH,           // 35
    ELLER,         // 36
    INTE,          // 37
    JUNK,          // 38
    LIST           // 39    
};

typedef struct
{
    Token** data;
    int instruction_amount;
} Program;

typedef struct {
    int* arr;
    int size;
    int top;
    int max_size;        
} Stack;

// utils, ex hjälpfunktioner
double str_to_double(char *num);
char *read_file(const char *filename);
void print_tokens(Token **instructions, int instruction_amount);
void debug_print_var(char *name, int len);
void print_variables(Scope *scope);
int find_substring(char *txt, char *pat);
void print_red(char* str, int len, int print_backslash);
static void print_dynamic_items(Dynamic_Var *items, int len, int indent);
void print_token_row(Token* args);
static int is_top_level_list_literal(Token *args, int args_amount);

// Evals
double evaluate_expression(Token *args_old, int args_amount, Token **instructions, int instruction_amount, Scope *scope);
String evaluate_str_expression(Token *args_old, int args_amount, Token **instructions, int instruction_amount, Scope *scope);
void cleanup_args(Token* args, int args_amount, Token **instructions, int instruction_amount, Scope *scope);
Dynamic_Var dynamic_eval(Token *args, int args_amount, Token **instructions, int instruction_amount, Scope *scope);
List evaluate_list_expression(Token *args_old, int args_amount, Token **instructions, int instruction_amount, Scope *scope);
int logic_eval(Token* args_old, int args_amount, Token **instructions, int instruction_amount, Scope *scope);

void create_str_var(char *name, int name_len, int len, char *string, Scope *scope);
void create_num_var(char *name, int name_len, double value, Scope *scope);
void create_list_var(char *name, int name_len, Dynamic_Var value, Scope *scope);

/*
name: char* till variabelnamn. 
length: längden på namnsträngen. 
type: VAR_LIST om du indexerar en lista, annars 0. 
index: indexeringen på listan, annars 0
*/
Dynamic_Var get_var_value(char *name, int length, int type, double index, Scope *scope);
void change_list_item(char* name, int name_len, int* indices, Variable new_var, Scope *scope, int index_amount);

Program tokenize(char* buff, int debug);
void check_syntax(Program* program);


void band(Token *instruction, Token **instructions, int instruction_amount, Scope *scope);
Dynamic_Var call_function(char *name, int name_len, int origin_program_counter, Token **instructions, int instruction_amount, Token* instruction, Scope* old_scope);
void interpret_instruction(Token *current, Token **instructions, int instruction_amount, Scope *scope);
void foug(Token *instruction, Scope *scope);
char* bult(char* file_name);
void loop(Token *instruction, Program program, Scope *scope, int keyword_count);
void tpos(Token *instruction, Scope *scope);