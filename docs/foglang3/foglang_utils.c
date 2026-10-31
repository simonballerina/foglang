#pragma once

void free_scope(Scope* scope) {
    for (int i = 0; i < scope->top; i++) {
        free(scope->variables[i].name);
        if (scope->variables[i].type == NODE_STRING) {
            free(scope->variables[i].string.string);
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
        case NODE_LIST:
            dst->list.len = src->list.len;
            dst->list.list = malloc(src->list.len * sizeof(Node*));
            if (!dst->list.list) {
                printf("Memory allocation failed for list clone\n");
                exit(1);
            }
            for (int i = 0; i < src->list.len; i++) {
                dst->list.list[i] = clone_node(src->list.list[i]);
            }
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

int compare_lists(Node* list_1, Node* list_2){
    if (list_1->list.len != list_2->list.len) return 0;

    int len = list_1->list.len;
    for (int i = 0; i < len; i++){
        NodeType eval_type_1 = list_1->list.list[i]->type;
        NodeType eval_type_2 = list_2->list.list[i]->type;
        if (eval_type_1 == eval_type_2) {
            switch (eval_type_1){
                case NODE_NUMBER:
                    if (!(list_1->list.list[i]->number.value == list_2->list.list[i]->number.value)) return 0;
                    break;
                case NODE_STRING:
                    if (strcmp(list_1->list.list[i]->string.string, list_2->list.list[i]->string.string) != 0) return 0;
                    break;
                case NODE_LIST:
                    if (compare_lists(list_1->list.list[i], list_2->list.list[i]) == 0) return 0;
                    break;
            }
        } else return 0;
    }

    return 1;
}

void help(int argc, char **argv) {
    printf("\
usage: foglang3 [--version] [--help] \n\
                <filepath> [--debug] [--unchecked]\n\
                <command> [<args>]\n\
\n\
Interpret a Foglang3 program: foglang3 <filepath>\n");
}