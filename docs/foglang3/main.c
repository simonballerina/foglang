#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "foglang.h"

#include "foglang_utils.c"
#include "foglang_debug.c"

#include "foglang_ast.c"

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



    return 0;

    malloc_error:
        printf("Could not read file '%s'\n", argv[1]);
        exit(1);

}