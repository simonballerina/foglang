#include "foglang.h"

char* builtin_names[] = {"len", "exit"}; 
Dynamic_Var (*builtins[])(Arg*, int, Token*) = {foglang_len, foglang_exit};

const int builtin_amount = sizeof(builtins)/sizeof(builtins[0]);

/*
Dynamic_Var example_function(Arg* args, int arg_count, Token* instruction){ // NEEDS to have these parameters

    if (arg_count != x) { // begin with checking argc
        throw_error(ERR_TYPE, (String){"example_function() takes x argument(s)", strlen("example_function() takes x argument(s)")}, instruction);
    }

    // do stuff

    return (Dynamic_Var){}; // NEEDS to return a Dynamic_Var, can be an empty Dynamic_Var though.
}
*/

Dynamic_Var foglang_len(Arg* args, int arg_count, Token* instruction) {

    if (arg_count != 1) {
        throw_error(ERR_TYPE, (String){"len() takes one argument", strlen("len() takes one argument")}, instruction);
    }

    Dynamic_Var arg = args[0].info;
    if (arg.type == VAR_STRING || arg.type == VAR_LIST) {
        return (Dynamic_Var){.value = arg.str_len, .type = VAR_NUMBER};
    } else {
        throw_error(ERR_TYPE, (String){"len() argument must be a string or list", strlen("len() argument must be a string or list")}, instruction);
    }
}

Dynamic_Var foglang_exit(Arg* args, int arg_count, Token* instruction) {
    if (arg_count > 1) {
        throw_error(ERR_TYPE, (String){"exit() takes at most one argument", strlen("exit() takes at most one argument")}, instruction);
    }

    int exit_code = 0;
    if (arg_count == 1) {
        Dynamic_Var arg = args[0].info;
        if (arg.type != VAR_NUMBER) {
            throw_error(ERR_TYPE, (String){"exit() argument must be a number", strlen("exit() argument must be a number")}, instruction);
        }
        exit_code = (int)arg.value;
    }

    exit(exit_code);

    return (Dynamic_Var){};
}




