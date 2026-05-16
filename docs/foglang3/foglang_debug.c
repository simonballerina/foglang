#pragma once

void print_indent(int indent){

    for(int i = 0; i < indent; i++)
        printf("  ");
}

char* op_to_str(TokType op){

    switch(op){

        case OP_ADD: return "+";
        case OP_SUB: return "-";
        case OP_MUL: return "*";
        case OP_DIV: return "/";
        case OP_MOD: return "%";
        case OP_EXP: return "^";
        case CMP_EQUALS: return "=";

        default: return "?";
    }
}

void print_ast_statement(Node* node, const char* prefix, int is_left){

    if(node == NULL)
        return;

    printf("%s", prefix);

    printf("%s",
        is_left ? "├── " : "└── ");

    switch(node->type){

        case NODE_NUMBER:

            printf("%g\n", node->number.value);

            break;
        case NODE_IDENTIFIER:

            printf("%s\n", node->string.string);

            break;

        case NODE_STRING:

            printf("'%s'\n", node->string.string);

            break;


        case NODE_BINARY: {

            printf("%s\n",
                op_to_str(node->binary.op));

            char new_prefix[256];

            snprintf(
                new_prefix,
                sizeof(new_prefix),
                "%s%s",
                prefix,
                is_left ? "│   " : "    "
            );

            print_ast_statement(
                node->binary.left,
                new_prefix,
                1
            );

            print_ast_statement(
                node->binary.right,
                new_prefix,
                0
            );

            break;
        }

        case NODE_BAND: {

            printf("BAND (%s)\n",
                node->band.name);

            char new_prefix[256];

            snprintf(
                new_prefix,
                sizeof(new_prefix),
                "%s%s",
                prefix,
                is_left ? "│   " : "    "
            );

            print_ast_statement(
                node->band.value,
                new_prefix,
                0
            );

            break;
        }
        case NODE_GIVET: {

            printf("GIVET ATT\n");

            char new_prefix[256];

            snprintf(
                new_prefix,
                sizeof(new_prefix),
                "%s%s",
                prefix,
                is_left ? "│   " : "    "
            );

            /*
                CONDITION
            */

            printf("%s├── CONDITION\n", new_prefix);

            char cond_prefix[256];

            snprintf(cond_prefix, sizeof(cond_prefix), "%s│   ", new_prefix);

            print_ast_statement(node->block.condition, cond_prefix, 0);

            /*
                BLOCK
            */

            printf("%s└── BLOCK\n", new_prefix);

            char block_prefix[256];

            snprintf(block_prefix, sizeof(block_prefix), "%s    ", new_prefix);

            for(int i = 0;
                i < node->block.statement_count;
                i++)
            {
                print_ast_statement(node->block.block[i], block_prefix, i != node->block.statement_count - 1);
            }

            break;
        }   
        case NODE_NAER: {

            printf("NAER\n");

            char new_prefix[256];

            snprintf(
                new_prefix,
                sizeof(new_prefix),
                "%s%s",
                prefix,
                is_left ? "│   " : "    "
            );

            /*
                CONDITION
            */

            printf("%s├── CONDITION\n", new_prefix);

            char cond_prefix[256];

            snprintf(cond_prefix, sizeof(cond_prefix), "%s│   ", new_prefix);

            print_ast_statement(node->block.condition, cond_prefix, 0);

            /*
                BLOCK
            */

            printf("%s└── BLOCK\n", new_prefix);

            char block_prefix[256];

            snprintf(block_prefix, sizeof(block_prefix), "%s    ", new_prefix);

            for(int i = 0;
                i < node->block.statement_count;
                i++)
            {
                print_ast_statement(node->block.block[i], block_prefix, i != node->block.statement_count - 1);
            }

            break;
        }  

    }
}

void print_ast(Node** ast, const char* prefix, int is_left, int ast_size) {
    for (int i = 0; i < ast_size; i++) {
        print_ast_statement(ast[i], prefix, is_left);
    }
}




void print_tokens(Token* instructions, int instruction_amount)
{

    for (int i = 0; i < instruction_amount; i++)
    {
        switch (instructions[i].type)
        {
        case FOUG:
            printf("'FOUG'    ");
            break;
        case BAND:
            printf("'BAND'    ");
            break;
        case NAER:
            printf("'NAER'    ");
            break;
        case GIVET:
            printf("'GIVET ATT'    ");
            break;
        case RIGHT_PAR:
            printf("')'    ");
            break;
        case LEFT_PAR:
            printf("'('    ");
        case OPEN_BLOCK:
            printf("'{'    \n");
            break;
        case CLOSE_BLOCK:
            printf("'}'    ");
            break;
        case OP_ADD:
            printf("'+'    ");
            break;
        case OP_SUB:
            printf("'-'    ");
            break;
        case OP_MUL:
            printf("'*'    ");
            break;
        case OP_DIV:
            printf("'/'    ");
            break;
        case OP_EXP:
            printf("'^'    ");
            break;
        case OP_MOD:
            printf("'%%'    ");
            break;
        case IDENTIFIER:
            printf("'%.*s'    ", (int)strlen(instructions[i].string), instructions[i].string);
            break;
        case CMP_EQUALS:
            printf("'='    ");
            break;
        case CMP_NOT_EQUALS:
            printf("'!='    ");
            break;
        case CMP_GREATER_THAN:
            printf("'>'    ");
            break;
        case CMP_LESS_THAN:
            printf("'<'    ");
            break;
        case NUMBER:
            printf("'%lf'    ", instructions[i].value);
            break;
        case STRING:
            printf("s'%s'    ", instructions[i].string);
            break;
        case TERMINATOR:
            printf(";\n");
            break;
        default:
            printf("'TKN'    ");
        }
    }
    printf("\n");
}
