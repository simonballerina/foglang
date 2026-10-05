#pragma once

char* tok_type_to_string(TokType type){
    
    switch(type){
        case LEFT_PAR: return "LEFT_PAR";
        case RIGHT_PAR: return "RIGHT_PAR";
        case OP_ADD: return "OP_ADD";
        case OP_SUB: return "OP_SUB";
        case OP_MUL: return "OP_MUL";
        case OP_DIV: return "OP_DIV";
        case OP_MOD: return "OP_MOD";
        case OP_EXP: return "OP_EXP";
        case NUMBER: return "NUMBER";
        case STRING: return "STRING";
        case COMMA: return "COMMA";

        case CMP_EQUALS: return "CMP_EQUALS";
        case CMP_NOT_EQUALS: return "CMP_NOT_EQUALS";
        case CMP_GREATER_THAN: return "CMP_GREATER_THAN";
        case CMP_LESS_THAN: return "CMP_LESS_THAN";

        case IDENTIFIER: return "IDENTIFIER";

        case TERMINATOR: return "TERMINATOR";
        case BAND: return "BAND";
        case GIVET: return "GIVET ATT";
        case FOUG: return "FOUG";
        case NAER: return "NAER";
        case SVETS: return "SVETS";
        case JUNK: return "JUNK";
        case TPOS: return "TPOS";
        case BOUL: return "BOUL";
        case RETURN: return "RETURN";

        default:
            printf("Unknown token type '%d'\n", type);
            exit(1);
    }
}

char* node_type_to_str(NodeType type){
    switch(type) {
        case NODE_NULL:
            return "NODE_NULL";
        case NODE_NUMBER:
            return "NODE_NUMBER";
        case NODE_STRING:
            return "NODE_STRING";
        case NODE_BINARY:
            return "NODE_BINARY";
        case NODE_BAND:
            return "NODE_BAND";
        case NODE_GIVET:        
            return "NODE_GIVET";
        case NODE_IDENTIFIER:
            return "NODE_IDENTIFIER";
        case NODE_NAER:
            return "NODE_NAER";
        case NODE_FOUG:
            return "NODE_FOUG";
        case NODE_TPOS:
            return "NODE_TPOS";
        case NODE_LIST:
            return "NODE_LIST";
        case NODE_BOUL: 
            return "NODE_BOUL";
        case NODE_FUNCTION_CALL:
            return "NODE_FUNCTION_CALL";
        case NODE_RETURN:
            return "NODE_RETURN";
        default:
            return "[?]";
    }
}

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
        case CMP_NOT_EQUALS: return "!=";
        case CMP_GREATER_THAN: return ">";
        case CMP_LESS_THAN: return "<";

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

            printf("s'");
            for (int i = 0; i < strlen(node->string.string); i++) {
                if (node->string.string[i] == '\n') printf("\\n");
                else printf("%c", node->string.string[i]);
            }
            printf("'\n");

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
        case NODE_FOUG: {

            printf("FOUG ");
            if (node->foug.is_junk) printf("JUNK ");

            if (node->foug.is_svets) printf("SVETS");
            printf("\n");
            char new_prefix[256];

            snprintf(
                new_prefix,
                sizeof(new_prefix),
                "%s%s",
                prefix,
                is_left ? "│   " : "    "
            );

            print_ast_statement(
                node->foug.string,
                new_prefix,
                0
            );

            break;
        }
        case NODE_TPOS: {

            printf("TPOS ");
            if (node->tpos.is_svets) printf("SVETS");
            printf("\n");
            char new_prefix[256];

            snprintf(
                new_prefix,
                sizeof(new_prefix),
                "%s%s",
                prefix,
                is_left ? "│   " : "    "
            );

            print_ast_statement(
                node->tpos.string,
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

        case NODE_FUNCTION_CALL: {

            printf("CALL %s\n", node->function.name);

            char new_prefix[256];
            snprintf(
                new_prefix,
                sizeof(new_prefix),
                "%s%s",
                prefix,
                is_left ? "│   " : "    "
            );

            if (node->function.arguments == NULL) {
                break;
            }

            for (int i = 0; node->function.arguments[i] != NULL; i++) {
                int is_last = node->function.arguments[i + 1] == NULL;
                print_ast_statement(node->function.arguments[i], new_prefix, !is_last);
            }

            break;
        }
        case NODE_RETURN: {

            printf("RETURN\n");

            char new_prefix[256];

            snprintf(
                new_prefix,
                sizeof(new_prefix),
                "%s%s",
                prefix,
                is_left ? "│   " : "    "
            );

            print_ast_statement(
                node->ret.value,
                new_prefix,
                0
            );

            break;
        }
        case NODE_BOUL: {

            printf("BOUL (%s)\n", node->block.function_name);

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

            printf("%s├── PARAMETERS\n", new_prefix);

            char cond_prefix[256];

            snprintf(cond_prefix, sizeof(cond_prefix), "%s│   ", new_prefix);

            for (int i = 0; i < node->block.parameters[i] != 0; i++) {
                printf("%s%s%s\n", cond_prefix, node->block.parameters[i+1] != 0 ? "├── " : "└── ", node->block.parameters[i]); 
                if (node->block.parameters[i+1] == 0) break;
            }

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

void print_scope(Scope* scope) {
    printf("--------\nScope: \n");
    for (int i = 0; i < scope->top; i++) {
        Variable v = scope->variables[i];
        printf("  %s: ", v.name);
        if (v.type == NODE_NUMBER) printf("%lf\n", v.number);
        else if (v.type == NODE_STRING) printf("s'%s'\n", v.string);
        else if (v.type == NODE_LIST) printf("List\n");
    }
    printf("--------\n");
}


void print_tokens(Token* instructions, int instruction_amount)
{
    int indent = 0;
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
        case SVETS:
            printf("'SVETS'    ");
            break;
        case JUNK:
            printf("'JUNK'    ");
            break;
        case TPOS:
            printf("'TPOS'    ");
            break;
        case RETURN:
            printf("'RETURN'    ");
            break;
        case COMMA:
            printf("',' ");
            break;
        case BOUL:
            printf("'BOUL'    ");
            break;
        case RIGHT_PAR:
            printf("')' ");
            break;
        case LEFT_PAR:
            printf("'(' ");
            break;
        case OPEN_BLOCK:
            printf("'{' \n");
            indent++;
            for (int j = 0; j < indent; j++)
                printf("    ");
            break;
        case CLOSE_BLOCK:
            printf("'}' \n");
            indent--;
            int j = 0;
            if (i+1 < instruction_amount && instructions[i+1].type == CLOSE_BLOCK) {
                j++;
            }
            for (j; j < indent; j++)
                printf("    ");
            break;
        case OP_ADD:
            printf("'+' ");
            break;
        case OP_SUB:
            printf("'-' ");
            break;
        case OP_MUL:
            printf("'*' ");
            break;
        case OP_DIV:
            printf("'/' ");
            break;
        case OP_EXP:
            printf("'^' ");
            break;
        case OP_MOD:
            printf("'%%' ");
            break;
        case IDENTIFIER:
            printf("'%.*s' ", (int)strlen(instructions[i].string), instructions[i].string);
            break;
        case CMP_EQUALS:
            printf("'=' ");
            break;
        case CMP_NOT_EQUALS:
            printf("'!=' ");
            break;
        case CMP_GREATER_THAN:
            printf("'>' ");
            break;
        case CMP_LESS_THAN:
            printf("'<' ");
            break;
        case NUMBER:
            printf("'%lf' ", instructions[i].value);
            break;
        case STRING:
            printf("s'");
            for (int j = 0; j < strlen(instructions[i].string); j++){
                char c = instructions[i].string[j];
                if (c == '\n') printf("\\n");
                else printf("%c", c);
            }
            printf("' ");

            break;
        case TERMINATOR:
            printf(";\n");
            int k = 0;
            if (i+1 < instruction_amount && instructions[i+1].type == CLOSE_BLOCK) {
                k++;
            }
            for (k; k < indent; k++)
                printf("    ");
            break;
        default:
            printf("'TKN(%d)' ", instructions[i].type);
        }
    }
    printf("\n");
}
