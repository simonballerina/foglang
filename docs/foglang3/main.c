#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#include "foglang.h"

#include "foglang_utils.c"
#include "foglang_debug.c"

#include "foglang_ast.c"
#include "foglang_stack.c"

Node* evaluate(Node* node, Scope* scope) {
    // todo, skapa modiferabar kopia av node
    NodeType type = node->type;
    if (type == NODE_IDENTIFIER) {
        UnnamedVariable v = get_var_value(scope, node->string.string);
        node->type = v.type;
        type = v.type;
        if (v.type == NODE_NUMBER) node->number.value = v.number;
        else if (v.type == NODE_STRING) node->string.string = v.string;
        else if (v.type == NODE_LIST) node->list = v.list;

    }
    if (type == NODE_NUMBER || type == NODE_STRING) {
        return node;
    }
    // bin expr
    Node* left = evaluate(node->binary.left, scope);
    Node* right = evaluate(node->binary.right, scope);
    NodeType eval_type = left->type;

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
        case OP_EXP:
            return make_num(pow(left->number.value, right->number.value));
            break;
        case CMP_EQUALS:
            if (eval_type == NODE_NUMBER) {
                return make_num(left->number.value == right->number.value);
            }
            return make_num(!strcmp(left->string.string, right->string.string));
            break;
        case CMP_NOT_EQUALS:
            if (eval_type == NODE_NUMBER) {
                return make_num(left->number.value != right->number.value);
            }
            break;
    }
    return NULL;
    malloc_error:
        printf("Could not allocate memory\n");
        exit(1);

}

void foug(Node* node, Scope* scope){
    Node* str = evaluate(node->foug.string, scope);
    NodeType type = str->type;
    if (type == NODE_STRING) {
        printf("%s\n", str->string.string);
    }
    else if (type == NODE_NUMBER) {
        if (str->number.value == (int)str->number.value) printf("%d\n", (int)str->number.value);
        else printf("%lf", str->number.value);
    }
    free(str);
}

void create_variable(Node* value, char* name, Scope* scope){

    int len = strlen(name);
    char* new_name = malloc(len+1);
    if (!new_name) goto malloc_error;
    memcpy(new_name, name, len+1);

    Variable v = {
        .name = new_name,
        .type = value->type
    };

    if (value->type == NODE_NUMBER) v.number = value->number.value;
    else if (value->type == NODE_STRING) {

        int len = strlen(value->string.string);
        char* new_str = malloc(len+1);
        if (!new_str) goto malloc_error;
        memcpy(new_str, value->string.string, len+1);
        
        v.string = new_str;
    }
    else if (value->type == NODE_LIST) v.list = value->list;

    if (scope->top >= scope->capacity) {
        Variable* tmp = realloc(scope->variables, (scope->capacity*2)*sizeof(Variable));
        if (!tmp) goto malloc_error;
        scope->variables = tmp;
        scope->capacity *= 2;
    }
    scope->variables[scope->top++] = v;
    
    return;

    malloc_error:
        printf("Memory allocation failed\n");
        exit(1);
}

void change_var_value(Scope* scope, char* name, Node* new_value){
    for (int i = 0; i < scope->top; i++){
        if (!strcmp(scope->variables[i].name, name)) {

            NodeType old_type = scope->variables[i].type;
            if (old_type == NODE_STRING){ // när listor implementeras: free:a dem!
                free(scope->variables[i].string);
            }

            NodeType type = new_value->type;
            if (type == NODE_STRING) {
                int len = strlen(new_value->string.string);
                char* new_str = malloc(len+1);
                if (!new_str) goto malloc_error;
                memcpy(new_str, new_value->string.string, len+1);
                
                scope->variables[i].string = new_str;
            }

            else if (type == NODE_NUMBER) scope->variables[i].number = new_value->number.value;
            else if (type == NODE_LIST) scope->variables[i].list = new_value->list;

            scope->variables[i].type = type;
        }
    }

    return;
    malloc_error:
        printf("Memory allocation failed\n");
        exit(1);
}

UnnamedVariable get_var_value(Scope* scope, char* name){
    for (int i = 0; i < scope->top; i++){
        if (!strcmp(scope->variables[i].name, name)) {
            UnnamedVariable v = {
                .type = scope->variables[i].type
            };

            if (scope->variables[i].type == NODE_NUMBER) v.number = scope->variables[i].number;
            else if (scope->variables[i].type == NODE_STRING) v.string = scope->variables[i].string;
            else if (scope->variables[i].type == NODE_LIST) v.list = scope->variables[i].list;

            return v;
        }
        
    }
    return (UnnamedVariable){.type = NODE_NULL};

}


void band(Node* node, Scope* scope){
    Node* value = evaluate(node->band.value, scope);

    // ta reda på om en ny variabel ska skapas
    NodeType t = get_var_value(scope, node->band.name).type;
    if (t == NODE_NULL) {
        create_variable(value, node->band.name, scope);
        return;
    }  
    change_var_value(scope, node->band.name, value);
    // uppdatera istället

    

}

void interpret_block(Node* block, Scope* scope) {
    switch (block->type) {
        case NODE_FOUG:
            foug(block, scope);
            break;
        case NODE_TPOS:
                
            break;
        case NODE_BAND:
            band(block, scope);
            break;
        case NODE_GIVET:

            break;
        case NODE_NAER:

            break;
    }

}

void interpret_ast(Node** ast, int ast_size, Scope* main_scope){
    for (int i = 0; i < ast_size; i++) {
        Node* block = ast[i];
        interpret_block(block, main_scope);
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

    Scope main_scope;
    stack_pop(&scopes, &main_scope);

    interpret_ast(ast, ast_size, &main_scope);


    return 0;

    malloc_error:
        printf("Could not read file '%s'\n", argv[1]);
        exit(1);

}