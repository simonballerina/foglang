#pragma once

typedef enum {
    NODE_NULL,
    NODE_NUMBER,
    NODE_STRING,
    NODE_BINARY,
    NODE_BAND,
    NODE_GIVET,
    NODE_IDENTIFIER,
    NODE_NAER,
    NODE_FOUG,
    NODE_TPOS,
    NODE_LIST,
} NodeType;


typedef enum {
    LEFT_PAR,           // 0
    RIGHT_PAR,          // 1
    OP_ADD,             // 2
    OP_SUB,             // 3
    OP_MUL,             // 4
    OP_DIV,             // 5
    OP_MOD,             // 6
    OP_EXP,             // 7
    NUMBER,             // 8
    STRING,             // 9

    CMP_EQUALS,         // 10
    CMP_NOT_EQUALS,     // 11
    CMP_GREATER_THAN,   // 12
    CMP_LESS_THAN,      // 13


    IDENTIFIER, // 14

    TERMINATOR, // 14
    BAND,       // 15
    GIVET,      // 16
    FOUG,       // 17
    NAER,       // 18
    SVETS,      // 19
    JUNK,       // 20
    TPOS,       // 21

    OPEN_BLOCK,
    CLOSE_BLOCK,

    FILE_END,


} TokType;

typedef struct UnnamedVariable UnnamedVariable;

typedef struct {
    UnnamedVariable* list;
    int len;
} List;

struct UnnamedVariable {

    NodeType type;

    union {
        List list;
        double number;
        char* string;
    };
};




typedef struct Node Node;

struct Node {

    NodeType type;

    union {
        struct {
            double value;
        } number;

        struct {
            char* string;
        } string;

        List list;

        struct {
            char* name;
            Node* value;
        } band;

        struct {
            Node* condition;
            Node** block;
            int statement_count;
        } block;

        struct {
            Node* left;
            Node* right;
            TokType op;
        } binary;

        struct {
            Node* string;
            int is_svets;
        } tpos;

        struct {
            Node* string;
            int is_svets;
            int is_junk;
        } foug;

    };
};

typedef struct {
    TokType type;
    union {
        double value;
        char* string;
    };

} Token;



typedef struct {
    
    char* name;
    NodeType type;
    
    union {
        List list;
        double number;
        char* string;
    };
} Variable;

typedef struct {
    void *data;
    int element_size;
    int top;
    int capacity;
} Stack;

typedef struct {
    Variable* variables;
    int top;
    int capacity;
} Scope;

Stack scopes;

char *read_file(const char *filename);
void help(int argc, char **argv);

void print_indent(int indent);
char* op_to_str(TokType op);
void print_ast_statement(Node* node, const char* prefix, int is_left);
void print_ast(Node** ast, const char* prefix, int is_left, int ast_size);
void print_tokens(Token* instructions, int instruction_amount);

Node* make_num(double number);
Node* make_identifier(char* name);
Node* make_str(char* str);
Node* make_binary(Node* left, TokType op, Node* right);
Node* parse_exp(Token* tokens, int tok_count);
Node* parse_factor(Token* tokens, int tok_count);
Node* parse_term(Token* tokens, int tok_count);
Node* parse_cmp(Token* tokens, int tok_count);
Node* parse_expression(Token* tokens, int tok_count);
Node* parse_cond_block(Token* tokens, int tok_count, TokType type);
Node* parse_band(Token* tokens, int tok_count);
Node* parse_output_statement(Token* tokens, int tok_count, TokType type);
Node* parse_statement(Token* tokens, int tok_count);
Node** build_ast(Token* tokens, int tok_count, int* ast_size);
Token* tokenize(char* buff, int* tok_amount);

Node* evaluate(Node* node, Scope* scope);
void foug(Node* node, Scope* scope);
void create_variable(Node* value, char* name, Scope* scope);
void change_var_value(Scope* scope, char* name, Node* new_value);
UnnamedVariable get_var_value(Scope* scope, char* name);
void band(Node* node, Scope* scope);
void interpret_block(Node* block, Scope* scope);
void interpret_ast(Node** ast, int ast_size, Scope* main_scope);
void create_scope(void);

void stack_init(Stack* stack, int element_size, int capacity);
void stack_push(Stack* stack, void* value);
void stack_pop(Stack* stack, void* out);