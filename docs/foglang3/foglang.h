#pragma once

typedef enum {
    NODE_NUMBER,
    NODE_STRING,
    NODE_BINARY,
    NODE_BAND,
    NODE_GIVET,
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

    TERMINATOR, // 15
    BAND,       // 16
    GIVET,      // 17
    FOUG,       // 18

    OPEN_BLOCK,
    CLOSE_BLOCK,

    FILE_END,


} TokType;

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

    };
};

typedef struct {
    TokType type;
    union {
        double value;
        char* string;
    };

} Token;

char *read_file(const char *filename);
void help(int argc, char **argv);

Node* make_num(double number);
Node* make_str(char* str);

Node* make_binary(Node* left, TokType op, Node* right);
Node* parse_factor(Token* tokens, int tok_count);
Node* parse_term(Token* tokens, int tok_count);
Node* parse_expression(Token* tokens, int tok_count);
Node* parse_statement(Token* tokens, int tok_count);

Node* parse_givet(Token* tokens, int tok_count);
Node* parse_band(Token* tokens, int tok_count);

Node** build_ast(Token* tokens, int tok_count, int* ast_size);
Token* tokenize(char* buff, int* tok_amount);