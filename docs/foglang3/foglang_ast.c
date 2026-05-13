#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "foglang.h"

int current = 0;

Node* make_num(double number){
    Node* ret = malloc(sizeof(Node));
    if (!ret) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    ret->number.value = number;
    ret->type = NODE_NUMBER;

    return ret;
}

Node* make_binary(Node* left, TokType op, Node* right){
    Node* ret = malloc(sizeof(Node));
    if (!ret) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    ret->type = NODE_BINARY;

    ret->binary.left = left;
    ret->binary.right = right;
    ret->binary.op = op;

    return ret;
}

Node* parse_factor(Token* tokens, int tok_count) {

    if (tokens[current].type == NUMBER) {
        Node* ret = make_num(tokens[current].value);
        current++;

        return ret;
    }
    if(tokens[current].type == LEFT_PAR) {

        current++;

        Node* expr = parse_expression(tokens, tok_count);

        if(tokens[current].type != RIGHT_PAR) {
            printf("Expected )\n");
            exit(1);
        }

        current++;

        return expr;
    }
    printf("Expected number, got ");
    print_tokens(tokens+current, 1);
    exit(1);
}

Node* parse_term(Token* tokens, int tok_count){

    Node* left = parse_factor(tokens, tok_count);


    while (current < tok_count &&
        (tokens[current].type == OP_MUL ||
            tokens[current].type == OP_DIV ||
            tokens[current].type == OP_EXP ||
            tokens[current].type == OP_MOD || 
            tokens[current].type == CMP_EQUALS)) 
    {
        TokType op = tokens[current].type;

        current++;

        Node* right = parse_factor(tokens, tok_count);

        left = make_binary(left, op, right);
    }

    return left;
}

Node* parse_expression(Token* tokens, int tok_count){

    Node* left = parse_term(tokens, tok_count);

    while (current < tok_count && (tokens[current].type == OP_ADD || tokens[current].type == OP_SUB)) 
    {
        TokType op = tokens[current].type;
        current++;

        Node* right = parse_term(tokens, tok_count);

        left = make_binary(left, op, right);

    }

    return left;

}

Node* parse_givet(Token* tokens, int tok_count) {
    current++; // Hoppa över GIVET
    Node* ret = malloc(sizeof(Node));

    Node* condition = parse_expression(tokens, tok_count);

    current++; // Hoppa över {

    // Count statement/block size
    int len = 0;
    for (int i = 0; i < tok_count; i++) {
        if (tokens[i].type == TERMINATOR || tokens[i].type == OPEN_BLOCK) len++;
    }
    
    Node** block = malloc(sizeof(Node*)*len);
    if (!block) goto malloc_error;

    int statement_count = 0;

    while (tokens[current].type != CLOSE_BLOCK && tokens[current].type != FILE_END) {
        block[statement_count++] = parse_statement(tokens, tok_count);
    }
    current++; // Hoppa över }

    ret->type = NODE_GIVET;
    ret->block.condition = condition;
    ret->block.block = block;
    ret->block.statement_count = statement_count;

    return ret;
    malloc_error:
        printf("Memory allocation failed\n");
        exit(1);
}

Node* parse_band(Token* tokens, int tok_count) {
    current++;
    Node* ret = malloc(sizeof(Node));
    if (!ret) goto malloc_error;

    ret->band.name = tokens[current].string;
    current+=2;
    Node* value = parse_expression(tokens, tok_count);
    ret->band.value = value;
    ret->type = NODE_BAND;

    if (tokens[current].type != TERMINATOR){
        printf("Expected ;, got ");
        print_tokens(tokens+current, 1);
        exit(1);
    }

    current++;

    return ret;

    malloc_error:
        printf("Memory allocation failed\n");
        exit(1);
}

Node* parse_statement(Token* tokens, int tok_count) {

    if (tokens[current].type == BAND)
        return parse_band(tokens, tok_count);

    if (tokens[current].type == GIVET)
        return parse_givet(tokens, tok_count);

    return NULL;
}



Node** build_ast(Token* tokens, int tok_count, int* ast_size){
    printf("Building AST...\n");
    // Count semicolons and blocks
    int len = 0;
    for (int i = 0; i < tok_count; i++) {
        if (tokens[i].type == TERMINATOR || tokens[i].type == OPEN_BLOCK) len++;
    }
    
    Node** ast = malloc(sizeof(Node*)*len);
    if (!ast) goto malloc_error;

    int ast_top = 0;

    while (tokens[current].type != FILE_END) {
        Node* statement = parse_statement(tokens, tok_count);
        ast[ast_top++] = statement;
    }
    *ast_size = ast_top;

    printf("AST generation complete\n\n");

    return ast;
    malloc_error:
        printf("Memory allocation failed\n");
        exit(1);

}


Token* tokenize(char* buff, int* tok_amount){
    int tok_top = 0;
    Token* tokens = malloc(4096*sizeof(Token));
    if (!tokens) goto malloc_error;

    int buff_len = strlen(buff);
    for (int i = 0; i < buff_len; i++){

        char letter = buff[i];
        if (letter == ' ' || letter == '\n') continue;
        if (isdigit(letter)){
            // find how many chars the number is
            int len = 0; 
            while (i+len < buff_len && (isdigit(buff[i+len]) || buff[i+len] == '.')) len++;

            sscanf(buff+i, "%lf", &tokens[tok_top].value);
            tokens[tok_top++].type = NUMBER;
            i += len-1;
            continue;
        }

        int letter_token_flag = 0;
        switch (letter) {
            case '(':
                tokens[tok_top++].type = LEFT_PAR;
                letter_token_flag = 1;
                break;
            case ')':
                tokens[tok_top++].type = RIGHT_PAR;
                letter_token_flag = 1;
                break;
            case '+':
                tokens[tok_top++].type = OP_ADD;
                letter_token_flag = 1;
                break;
            case '-':
                tokens[tok_top++].type = OP_SUB;
                letter_token_flag = 1;
                break;
            case '*':
                tokens[tok_top++].type = OP_MUL;
                letter_token_flag = 1;
                break;
            case '/':
                tokens[tok_top++].type = OP_DIV;
                letter_token_flag = 1;
                break;
            case '%':
                tokens[tok_top++].type = OP_MOD;
                letter_token_flag = 1;
                break;
            case '^':
                tokens[tok_top++].type = OP_EXP;
                letter_token_flag = 1;
                break;
            case '=':
                tokens[tok_top++].type = CMP_EQUALS;
                letter_token_flag = 1;
                break;
            case '>':
                tokens[tok_top++].type = CMP_GREATER_THAN;
                letter_token_flag = 1;
                break;
            case '<':
                tokens[tok_top++].type = CMP_LESS_THAN;
                letter_token_flag = 1;
                break;
            case ';':
                tokens[tok_top++].type = TERMINATOR;
                letter_token_flag = 1;
                break;
            case '{':
                tokens[tok_top++].type = OPEN_BLOCK;
                letter_token_flag = 1;
                break;
            case '}':
                tokens[tok_top++].type = CLOSE_BLOCK;
                letter_token_flag = 1;
                break;
        }
        if (strncmp(buff+i, "band ", 5) == 0){
            tokens[tok_top++].type = BAND;
            i+=4;
        } else if (strncmp(buff+i, "foug ", 5) == 0){
            tokens[tok_top++].type = FOUG;
            i+=4;
        } else if (strncmp(buff+i, "givet att ", 10) == 0) {
            i+=9;
            tokens[tok_top++].type = GIVET;
        } else if (strncmp(buff+i, "!=", 2) == 0) {
            i+=2;
            tokens[tok_top++].type = CMP_NOT_EQUALS;
        } else if (!letter_token_flag) {
            // anta variabel
            int j = i;
            while (j < buff_len && ((buff[j] >= 'a' && buff[j] <= 'z') || (buff[j] >= 'A' && buff[j] <= 'Z') || (buff[j] >= '0' && buff[j] <= '9') || buff[j] == '_')) j++;
            j -= i;
            
            tokens[tok_top].type = IDENTIFIER;
            tokens[tok_top].string = malloc(j+1);
            if (!tokens[tok_top].string) goto malloc_error;
            strncpy(tokens[tok_top].string, buff+i, j);
            tokens[tok_top].string[j] = '\0';

            tok_top++;
            i+=j-1;
        }
        
        
    }

    tokens[tok_top].type = FILE_END;

    *tok_amount = tok_top;

    return tokens;

    malloc_error:
        printf("Memory allocation failed\n");
        exit(1);
}

double interpret(Node* ast) {

    if (ast->type == NODE_NUMBER) return ast->number.value;

    // bin expr
    double left = interpret(ast->binary.left);
    double right = interpret(ast->binary.right);

    switch (ast->binary.op) {
        case OP_ADD:
            return left + right;
            break;
        case OP_DIV:
            return left / right;
            break;
        case OP_MOD:
            return (int)left % (int)right;
            break;
        case OP_MUL:
            return left * right;
            break;
        case OP_SUB:
            return left - right;
            break;
        case CMP_EQUALS:
            return left == right;
            break;
    }
}

