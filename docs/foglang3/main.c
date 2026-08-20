#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "foglang.h"

#include "foglang_utils.c"
#include "foglang_debug.c"

#include "foglang_ast.c"
#include "foglang_stack.c"

Node* evaluate(Node* node) {
    NodeType type = node->type;
    if (type == NODE_NUMBER || type == NODE_STRING) {
        return node;
    }
    // bin expr
    Node* left = evaluate(node->binary.left);
    Node* right = evaluate(node->binary.right);
    
    TokType eval_type = left->type;

    Node* ret;

    switch (node->binary.op) {

        case OP_ADD:
            
            if (eval_type == NODE_NUMBER) {
                ret = make_num(left->number.value+right->number.value);
            } else if (eval_type == NODE_STRING) {
                int l_len = strlen(left->string.string);
                int r_len = strlen(right->string.string);

                char* summed_str = malloc(l_len+r_len+1);
                if (!summed_str) goto malloc_error;
                
                memcpy(summed_str, left->string.string, l_len);
                memcpy(summed_str+l_len, right->string.string, r_len);
                summed_str[l_len+r_len] = '\0';

                ret = make_str(summed_str);
            }

            return ret;
            break;
        case OP_DIV:
            return make_num(left->number.value/right->number.value);
            break;
        case OP_MOD:
            return make_num((int)(left->number.value)%(int)(right->number.value));
            break;
        case OP_MUL:
            return make_num(left->number.value*right->number.value);
            break;
        case OP_SUB:
            return make_num(left->number.value-right->number.value);
            break;
        case CMP_EQUALS:
            if (eval_type == NODE_NUMBER) {
                return make_num(left->number.value == right->number.value);
            }
            return make_num(!strcmp(left->string.string, right->string.string));
            break;
    }

    malloc_error:
        printf("Could not allocate memory\n");
        exit(1);

}

void foug(Node* node){
    Node* str = evaluate(node->foug.string);
    TokType type = str->type;

    if (type == NODE_STRING) {
        printf("%s\n", str->string.string);
        free(str->string.string);
    }
    else if (type == NODE_NUMBER) {
        if (str->number.value == (int)str->number.value) printf("%d\n", (int)str->number.value);
        else printf("%lf", str->number.value);
    }
    free(str);
}

void create_variable(Node* value, char* name, Scope* scope){

    Variable v = {
        .list = NULL,
        .name = name,
        .number = value->number.value,
        .string = value->string.string,
        .type = value->type
    };


}

UnnamedVariable get_var_value(Scope* scope, char* name){

    for (int i = 0; i < scope->top; i++){
        printf("name: %s\n", scope->variables[i].name);
        if (!strcmp(scope->variables[i].name, name)) 
            return (UnnamedVariable){
                .list = scope->variables[i].list,
                .number = scope->variables[i].number,
                .string = scope->variables[i].string,
                .type = scope->variables[i].type
            };
        
    }

}


void band(Node* node){
    Node* value = evaluate(node->band.value);


}

void interpret_ast(Node** ast, int ast_size){
    for (int i = 0; i < ast_size; i++) {
        Node* block = ast[i];
        switch (block->type) {
            case NODE_FOUG:
                foug(block);
                break;
            case NODE_TPOS:
                
                break;
            case NODE_BAND:
                break;
            case NODE_GIVET:

                break;
            case NODE_NAER:

                break;
        }
    }
}


void create_scope() {

    Scope s = {
        .capacity = 8,
        .top = 0,
        .variables = malloc(8*sizeof(Variable))
    };
    if (!s.variables) goto malloc_error;

    stack_push(&scopes, &s);

    return;

    malloc_error:
        printf("Memory allocation failed\n");
        exit(1);
        
}

int main(int argc, char **argv){
    //check for flags
    int flag_help = 0;      // -h --help
    int flag_version = 0;   // -v --version --ver
    int flag_debug = 0;     // -d --debug
    int flag_unchecked = 0; // -u --unchecked

    for (int i = 0; i < argc; i++)
    {
        if ((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "-H") == 0) || (strcmp(argv[i], "--help") == 0) || argc < 2)
        {
            flag_help = 1;
        }
        else if ((strcmp(argv[i], "-v") == 0 )|| (strcmp(argv[i], "-V") == 0) || (strcmp(argv[i], "--version") == 0) || (strcmp(argv[i], "--ver") == 0))
        {
            flag_version = 1;
        }
        else if ((strcmp(argv[i], "-d") == 0 )|| (strcmp(argv[i], "-D") == 0) || (strcmp(argv[i], "--debug") == 0))
        {
            flag_debug = 1;
        }
        else if ((strcmp(argv[i], "-u") == 0) || (strcmp(argv[i], "-U") == 0) || (strcmp(argv[i], "--unchecked") == 0))
        {
            flag_unchecked = 1;
        }
    }

    if (flag_version) {
        printf("Foglang version: %s\n", "Unknown");
        exit(0);
    }
    
    if (flag_help) {
        help(argc, argv);
        exit(0);
    }

    char* buff;
    if (argv[1]){
        buff = read_file(argv[1]);
        if (!buff) goto malloc_error; 
    }


    int tok_count;
    Token* tokens = tokenize(buff, &tok_count);
    print_tokens(tokens, tok_count);
    
    int ast_size;
    Node** ast = build_ast(tokens, tok_count, &ast_size);
    print_ast(ast, "", 0, ast_size);

    // setup variable stack
    
    stack_init(&scopes, sizeof(Scope), 8);
    // create main scope
    create_scope();


    interpret_ast(ast, ast_size);


    return 0;

    malloc_error:
        printf("Could not read file '%s'\n", argv[1]);
        exit(1);

}