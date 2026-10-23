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

#include "foglang_var.c"

void free_scope(Scope* scope) {
    for (int i = 0; i < scope->top; i++) {
        free(scope->variables[i].name);
        if (scope->variables[i].type == NODE_STRING) {
            free(scope->variables[i].string);
        }
    }
    free(scope->variables);
}

void free_node(Node* node) {
    if (!node) return;
    if (node->type == NODE_STRING) {
        free(node->string.string);
    }
    else if (node->type == NODE_BINARY) {
        free_node(node->binary.left);
        free_node(node->binary.right);
    }
    else if (node->type == NODE_BAND) {
        free_node(node->band.value);
    }
    else if (node->type == NODE_GIVET || node->type == NODE_NAER) {
        free_node(node->block.condition);
        for (int i = 0; i < node->block.statement_count; i++) {
            free_node(node->block.block[i]);
        }
        free(node->block.block);
        if (node->block.parameters) {
            for (int i = 0; node->block.parameters[i] != NULL; i++) {
                free(node->block.parameters[i]);
            }
            free(node->block.parameters);
        }
    }
    else if (node->type == NODE_FUNCTION_CALL) {
        for (int i = 0; node->function.arguments[i] != NULL; i++) {
            free_node(node->function.arguments[i]);
        }
        free(node->function.arguments);
    }
    else if (node->type == NODE_RETURN) {
        free_node(node->ret.value);
    }
    free(node);
}

Node* clone_node(const Node* src) {
    Node* dst = calloc(1, sizeof(Node));
    if (!dst) {
        printf("Memory allocation failed for node clone\n");
        exit(1);
    }

    dst->type = src->type;

    switch (src->type) {
        case NODE_NUMBER:
            dst->number.value = src->number.value;
            break;

        case NODE_STRING:
            dst->string.string = strdup(src->string.string);
            dst->string.is_svets = src->string.is_svets;
            break;

        case NODE_BINARY:
            dst->binary.left = clone_node(src->binary.left);
            dst->binary.right = clone_node(src->binary.right);
            dst->binary.op = src->binary.op;
            break;

        case NODE_FUNCTION_CALL:
            dst->function.name = strdup(src->function.name);
            // clone arguments array and each arg
            int arg_count = 0;
            while (src->function.arguments[arg_count] != NULL) {
                arg_count++;
            }
            dst->function.arguments = malloc((arg_count + 1) * sizeof(Node*));
            if (!dst->function.arguments) {
                printf("Memory allocation failed for function arguments clone\n");
                exit(1);
            }
            for (int i = 0; i < arg_count; i++) {
                dst->function.arguments[i] = clone_node(src->function.arguments[i]);
            }
            dst->function.arguments[arg_count] = NULL;

            break;
        case NODE_IDENTIFIER:
            dst->string.string = strdup(src->string.string);
            break;
        default:
            printf("Unhandled node type in clone_node, type: %d (%s)\n", src->type, node_type_to_str(src->type));
            exit(1);
    }

    return dst;
}

void call_function(Node** ast, Node* node, Scope* scope) {
    // push return program counter
    int ret_pc = current;
    stack_push(&return_pc, &ret_pc);

    // find function to call by searching program for function definition


    for (int i = 0; ast[i]->type != FILE_END; i++) {
        if (ast[i]->type == NODE_BOUL && !strcmp(ast[i]->block.function_name, node->function.name)) {
            Scope function_scope = {
                .capacity = 8,
                .top = 0,
                .variables = malloc(8 * sizeof(Variable))
            };
            if (!function_scope.variables) {
                printf("Memory allocation failed\n");
                exit(1);
            }
            Scope* func_scope = &function_scope;

            int parameter_count = 0;
            while (ast[i]->block.parameters[parameter_count] != NULL) {
                parameter_count++;
            }

            int argument_count = 0;
            while (node->function.arguments[argument_count] != NULL) {
                argument_count++;
            }

            if (argument_count != parameter_count) {
                printf("Function '%s' expects %d arguments, got %d\n",
                    node->function.name, parameter_count, argument_count);
                free_scope(func_scope);
                exit(1);
            }

            // assign arguments to parameters
            for (int j = 0; j < argument_count; j++) {
                Node* arg = evaluate(ast, node->function.arguments[j], scope);
                create_variable(arg, ast[i]->block.parameters[j], func_scope);
                free(arg);
            }

            int return_value_depth = return_values.top;
            for (int j = 0; j < ast[i]->block.statement_count; j++) {
                interpret_block(ast, ast[i]->block.block[j], func_scope);
                if (return_values.top > return_value_depth) {
                    break;
                }
            }

            free_scope(func_scope);
            return;

        }
    }

}

Node* evaluate(Node** ast, Node* node, Scope* scope) { // returns node on the heap, free after use!
    // copy node type to avoid changing the original node
    Node* eval_node = clone_node(node);

    NodeType type = node->type;

    if (type == NODE_IDENTIFIER) {
        UnnamedVariable v = get_var_value(scope, node->string.string);
        if (v.type == TERMINATOR) {
            printf("Variable '%s' not found\n", node->string.string);
            print_scope(scope);
            exit(1);
        }
        eval_node->type = v.type;
        type = v.type;
        if (v.type == NODE_NUMBER) eval_node->number.value = v.number;
        else if (v.type == NODE_STRING) eval_node->string.string = strdup(v.string);
        else if (v.type == NODE_LIST) eval_node->list = v.list;

    }

    if (type == NODE_FUNCTION_CALL) {
        call_function(ast, node, scope);

        // pop return value from stack
        Node* ret_val = NULL;
        stack_pop(&return_values, &ret_val);
        free(eval_node);
        if (!ret_val) {
            exit(1);
        }
        return ret_val;
    }

    if (type == NODE_NUMBER || type == NODE_STRING || type == NODE_NULL) {
        if (type == NODE_STRING && eval_node->string.is_svets) {
            char* s_s = create_svets_string(eval_node->string.string, scope);
            free(eval_node->string.string);
            eval_node->string.string = s_s;
            eval_node->string.is_svets = 0;
        }
        return eval_node;
    }
    // bin expr
    Node* left = evaluate(ast, eval_node->binary.left, scope);
    Node* right = evaluate(ast, eval_node->binary.right, scope);
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

                ret = make_str(summed_str, left->string.is_svets || right->string.is_svets);

                if (ret->string.is_svets) {
                    char* s_s = create_svets_string(ret->string.string, scope);
                    free(ret->string.string);
                    ret->string.string = s_s;
                    ret->string.is_svets = 0;
                }
            }
            free_node(left);
            free_node(right);
            free_node(eval_node);

            return ret;
            break;
        

        case OP_DIV:
            ret = make_num(left->number.value/right->number.value);
            free_node(left);
            free_node(right);
            free_node(eval_node);
            return ret;
            break;
        case OP_MOD:
            ret = make_num((int)(left->number.value)%(int)(right->number.value));
            free_node(left);
            free_node(right);
            free_node(eval_node);
            return ret;            
            break;
        case OP_MUL:
            ret = make_num(left->number.value*right->number.value);
            free_node(left);
            free_node(right);
            free_node(eval_node);
            return ret;
            break;
        case OP_SUB:
            ret = make_num(left->number.value-right->number.value);
            free_node(left);
            free_node(right);
            free_node(eval_node);
            return ret;
            break;
        case OP_EXP:
            ret = make_num(pow(left->number.value, right->number.value));
            free_node(left);
            free_node(right);
            free_node(eval_node);
            return ret;
            break;
        case CMP_EQUALS:
            if (eval_type == NODE_NUMBER) {
                ret = make_num(left->number.value == right->number.value);
                free_node(left);
                free_node(right);
                free_node(eval_node);
                return ret;
            }
            ret = make_num(!strcmp(left->string.string, right->string.string));
            if (ret->string.is_svets) {
                char* s_s = create_svets_string(ret->string.string, scope);
                free(ret->string.string);
                ret->string.string = s_s;
                ret->string.is_svets = 0;
            }
            free_node(left);
            free_node(right);
            free_node(eval_node);
            return ret;
            break;
        case CMP_NOT_EQUALS:
            if (eval_type == NODE_NUMBER) {
                ret = make_num(left->number.value != right->number.value);
                free_node(left);
                free_node(right);
                free_node(eval_node);
                return ret;
            }

            ret = make_num(strcmp(left->string.string, right->string.string));
            free_node(left);
            free_node(right);
            free_node(eval_node);

            if (ret->string.is_svets) {
                char* s_s = create_svets_string(ret->string.string, scope);
                free(ret->string.string);
                ret->string.string = s_s;
                ret->string.is_svets = 0;
            }
            return ret;
            break;
        case CMP_GREATER_THAN:
            if (eval_type == NODE_NUMBER) {
                ret = make_num(left->number.value > right->number.value);
                free_node(left);
                free_node(right);
                free_node(eval_node);
                return ret;
            }
            break;
        case CMP_LESS_THAN:
            if (eval_type == NODE_NUMBER) {
                ret = make_num(left->number.value < right->number.value);
                free_node(left);
                free_node(right);
                free_node(eval_node);
                return ret;
            }
            break;
    }
    return NULL;
    malloc_error:
        printf("Could not allocate memory\n");
        exit(1);

}

char* create_svets_string(char* str, Scope* scope) {

    int base_len = strlen(str);
    char* new_str = malloc(base_len + 1);
    if (!new_str) {
        printf("Memory allocation failed\n");
        exit(1);
    }

    int w = 0;
    for (int i = 0; i < base_len; i++){
        if (str[i] == '\\') {
            i++;
            continue;
        }
        if (str[i] == '%') {
            // find len of expression
            int svets_len = 0;
            int depth = 0;
            for (int j = i+1; j < base_len; j++){
                if (str[j] == '\\') {
                    j++; 
                    continue;
                }
                if (str[j] == '"' && !depth) depth = 1;
                else if (str[j] == '"' && depth) depth = 0;
                if (!depth && str[j] == '%') {
                    i++; 
                    break;
                }
                svets_len++;


            }

            // copy svets string and tokenize + build ast
            char* svets_part = malloc(svets_len+1);
            if (!svets_part) {
                printf("Memory allocation failed\n");
                exit(1);
            }
            memcpy(svets_part, str+i, svets_len);
            svets_part[svets_len] = '\0';

            int tok_amount = 0;
            Token* svets_tokens = tokenize(svets_part, &tok_amount, 0);
            int saved = current;
            current = 0;
            Node* svets_ast = parse_statement(svets_tokens, tok_amount, 0);
            current = saved;

            Node* complete_svets_string = evaluate(0, svets_ast, scope);

            // append complete_svets_string to new_str
            if (complete_svets_string->type == NODE_STRING) {
                int complete_svets_string_len = strlen(complete_svets_string->string.string);
                new_str = realloc(new_str, w + complete_svets_string_len + 1);
                if (!new_str) {
                    printf("Memory allocation failed\n");
                    exit(1);
                }
                memcpy(new_str + w, complete_svets_string->string.string, complete_svets_string_len);
                w += complete_svets_string_len;
            } else if (complete_svets_string->type == NODE_NUMBER) {
                char num_str[32];
                snprintf(num_str, sizeof(num_str), "%g", complete_svets_string->number.value);
                int num_str_len = strlen(num_str);
                new_str = realloc(new_str, w + num_str_len + 1);
                if (!new_str) {
                    printf("Memory allocation failed\n");
                    exit(1);
                }
                memcpy(new_str + w, num_str, num_str_len);
                w += num_str_len;
            }

            free_node(svets_ast);
            // free tokenized strings
            for (int k = 0; k < tok_amount; k++){
                if (svets_tokens[k].type == STRING) {
                    free(svets_tokens[k].string.string);
                }
            }
            free(svets_tokens);


            i+=svets_len+1;
        }
        new_str[w++] = str[i];
    }

    new_str[w] = '\0';
    
    return new_str;
}

void foug(Node** ast, Node* node, Scope* scope){
    Node* str = evaluate(ast, node->foug.string, scope);

    NodeType type = str->type;
    if (type == NODE_STRING) {
        printf("%s\n", str->string.string);
    } else if (type == NODE_NUMBER) {
        if (str->number.value == (int)str->number.value) printf("%d\n", (int)str->number.value);
        else printf("%lf\n", str->number.value);
    }
    free_node(str);
}


void band(Node** ast, Node* node, Scope* scope){
    Node* value = evaluate(ast, node->band.value, scope);

    // figure out if a new var is to be created
    NodeType t = get_var_value(scope, node->band.name).type;
    if (t == TERMINATOR) {
        create_variable(value, node->band.name, scope);
        free_node(value);
        return;
    }  
    change_var_value(scope, node->band.name, value);
    free_node(value);
    // update instead

}




void interpret_block(Node** ast, Node* block, Scope* scope) {
    switch (block->type) {
        case NODE_FOUG:
            foug(ast, block, scope);
            break;
        case NODE_TPOS:
                
            break;
        case NODE_BAND:
            band(ast, block, scope);
            break;
        case NODE_GIVET:
            Node* condition = evaluate(ast, block->block.condition, scope);
            if (condition->type == NODE_NUMBER && condition->number.value) {
                for (int i = 0; i < block->block.statement_count; i++)
                    interpret_block(ast, block->block.block[i], scope);
            }
            free_node(condition);
            break;
        case NODE_NAER:
            condition = evaluate(ast, block->block.condition, scope);
            while (condition->number.value) {
                for (int i = 0; i < block->block.statement_count; i++)
                    interpret_block(ast, block->block.block[i], scope);

                free_node(condition);
                condition = evaluate(ast, block->block.condition, scope);
            }
            free_node(condition);
            break;
        case NODE_RETURN:
            // get return value
            Node* ret = evaluate(ast, block->ret.value, scope);
            stack_push(&return_values, &ret);
            //free_node(ret);

            // set program counter to return pc
            int ret_pc;
            stack_pop(&return_pc, &ret_pc);
            current = ret_pc;
            break;
        case NODE_FUNCTION_CALL:
            call_function(ast, block, scope);
            break;
    }

    // debug print active scope
    //print_scope(scope);

}

void interpret_ast(Node** ast, int ast_size, Scope* main_scope){
    for (int i = 0; i < ast_size; i++) {
        Node* block = ast[i];
        interpret_block(ast, block, main_scope);
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
    int debug_level = 0;     // -d --debug
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
        else if ((strncmp(argv[i], "-d", 2) == 0 )|| (strncmp(argv[i], "-D", 2) == 0) || (strncmp(argv[i], "--debug", 7) == 0))
        {
            debug_level = 9;
            int argv_len = strlen(argv[i]);

            if (argv_len > 2 && strncmp(argv[i], "--", 2) != 0) {
                debug_level = atoi(argv[i]+2);
            } else if (argv_len > 7 && !strncmp(argv[i], "--debug", 7)){
                debug_level = atoi(argv[i]+7);
            }
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
    Token* tokens = tokenize(buff, &tok_count, debug_level);
    if (debug_level > 1) print_tokens(tokens, tok_count);
    
    int ast_size;
    Node** ast = build_ast(tokens, tok_count, &ast_size, debug_level);
    if (debug_level > 1) print_ast(ast, "", 0, ast_size);

    // init variable stack
    stack_init(&scopes, sizeof(Scope), 8);
    // create main scope
    create_scope();
    Scope main_scope;
    stack_pop(&scopes, &main_scope);

    // init return value stack
    stack_init(&return_values, sizeof(Node*), 8);

    // init return pc stack
    stack_init(&return_pc, sizeof(int), 8);


    interpret_ast(ast, ast_size, &main_scope);


    return 0;

    malloc_error:
        printf("Could not read file '%s'\n", argv[1]);
        exit(1);

}