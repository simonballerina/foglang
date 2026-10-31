#pragma once

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