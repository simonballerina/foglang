#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>


#include "foglang.h"
#include "foglang_utils.c"

#ifdef PATH_MAX
    char path_diff[PATH_MAX];
#else
    #define PATH_MAX 1024
    char path_diff[PATH_MAX];
#endif

#ifndef LIBPATH
    #ifdef __APPLE__
        #define LIBPATH "/usr/local/lib/foglang2/"
    #endif

    #ifdef _WIN32
        #define LIBPATH "C:\\Program Files\\foglang2\\lib\\"
    #endif

    #ifdef __linux__
        #define LIBPATH "/usr/local/lib/foglang2/"
    #endif
#endif

#ifndef PACKPATH
    #ifdef __APPLE__
        #define PACKPATH "~/Library/Application Support/foglang2/packages/"
    #endif

    #ifdef _WIN32
        #define PACKPATH "C:\\Program Files\\foglang2\\packages\\"
    #endif

    #ifdef __linux__
        #define PACKPATH "~/.local/share/foglang2/packages/"
    #endif
#endif


void throw_error(int type, String err_str, Token *instruction){
    char tick = '\'';  
    char colon = ':';
    print_red("Error at line ", strlen("Error at line "), 0);


    
    print_red(&colon, 1, 1);
    if (instruction != NULL)
        print_token_row(instruction);

    switch (type) {


        case ERR_MALLOC:

            print_red("ERR_MALLOC: Could not allocate memory\n", strlen("ERR_MALLOC: Could not allocate memory\n"), 0);
            print_red(err_str.string, err_str.len, 1);     // err_str is a description of the malloc error

            break;
        case ERR_INDEX:
            print_red("ERR_INDEX: Index out of range\n", strlen("ERR_INDEX: Index out of range\n"), 0);
            print_red("Could not index item '", strlen("Could not index item '"), 0);
            print_red(err_str.string, err_str.len, 0);     // err_str is the name of the variable that was attempted to be indexed
            print_red(&tick, 1, 1);

            break;
        case ERR_MATH:
            print_red("ERR_MATH: Invalid math operation\n", strlen("ERR_MATH: Invalid math operation\n"), 0);
            print_red(err_str.string, err_str.len, 1);     // err_str is a description of the math error

            break;
        case ERR_NAME:
            print_red("ERR_NAME: Variable not recognized:\n", strlen("ERR_NAME: Variable not recognized:\n"), 0);
            print_red("Name: '", strlen("Name: '"), 0);
            print_red(err_str.string, err_str.len, 0);     // err_str is the name of the variable that was not recognized
            print_red(&tick, 1, 1);
            break;
        case ERR_SYNTAX:
            print_red("ERR_SYNTAX: Syntax error\n", strlen("ERR_SYNTAX: Syntax error\n"), 0);
            print_red(err_str.string, err_str.len, 1);     // err_str is a description of the syntax error
            break;
        case ERR_TYPE:
            print_red("ERR_TYPE: Invalid type\n", strlen("ERR_TYPE: Invalid type\n"), 0);
            print_red(err_str.string, err_str.len, 1);     // err_str is a description of the type error
            break;
        case ERR_FILE:
            print_red("ERR_FILE: Could not read file\n", strlen("ERR_FILE: Could not read file\n"), 0);
            print_red(err_str.string, err_str.len, 1);
            break;
    }
    exit(type);
}


typedef struct Bult_file_types {
    struct {
        char** arr;
        int cap;
        int top;
    } sax;
    struct {
        char** arr;
        int cap;
        int top;
    } gung;
    struct {
        char** arr;
        int cap;
        int top;
    } lib;
} Bult_File_Types;

void bult_rec(char* file_name, Bult_File_Types* visited_files, String* buff) {

    /* 
        FÖR SAX: LÄGG TILL PWD OUTPUT I BÖRJAN AV NAMNEN

        ex: 
            bult sax fog/svets.fg;
        blir:
            /home/simon/programming/foglang/docs/foglang2/fog/svets.fg



        NÄR MAN ÖPPNAR EN NY FIL, CHDIR TILL FILEN OM DET BEHÖVS.

    */

    // här cddir:ar man till den nya filen

    int file_name_len = strlen(file_name);

    for (int i = file_name_len-1; i >= 0; i--) {

        if (file_name[i] == '/') {
            char* file_path = malloc(i+1);
            if (!file_path) goto malloc_error;

            memcpy(file_path, file_name, i);
            file_path[i+1] = '\0';
            
            printf("        byter till %s\n", file_path);
            chdir(file_path);

            free(file_path);
            break;
        }
    }

    char* text = read_file(file_name);
    if (!text) throw_error(ERR_FILE, (String){.len = strlen("Could not find imported file"), .string = "Could not find imported file"}, NULL);


    int text_len = strlen(text);

    for (int i = 0; i < text_len; i++) {


        if (i+5 < text_len && !strncmp(text+i, "bult ", 5)) {
            
            printf("bult ");
            i+=5;
            // hitta om det är sax eller gung
            int is_sax = 0, is_gung = 0;

            if (i+4 < text_len && !strncmp(text+i, "sax ", 4)) {
                is_sax = 1;
                i+=4;
                printf("sax ");
            } else if (i+5 < text_len && !strncmp(text+i, "gung ", 5)) {
                is_gung = 1;
                i+=5;
                printf("gung ");
            }
            printf("hittad, \"");

            char*** v;
            int* cap;
            int* top;

            if (is_sax) {
                v = &visited_files->sax.arr;
                cap = &visited_files->sax.cap;
                top = &visited_files->sax.top;
            } else if (is_gung) {
                v = &visited_files->gung.arr;
                cap = &visited_files->gung.cap;
                top = &visited_files->gung.top;
            } else {
                v = &visited_files->lib.arr;
                cap = &visited_files->lib.cap;
                top = &visited_files->lib.top;
            }


            // hitta importnamnet
            int pack_len = 0;
            for (int j = i; j < text_len; j++) {
                if (text[j] == ';') break;
                pack_len++;
            }

            // kopiera namn
            char* pack_name = malloc(pack_len+1); // free:a senare!!
            if (!pack_name) goto malloc_error;
            memcpy(pack_name, text+i, pack_len);
            pack_name[pack_len] = '\0';

            printf("%s\"\n", pack_name);
            
            int cont = 0;
            for (int j = 0; j < *top; j++) { // kolla om filen redan finns i importlistan
                if (!strcmp(pack_name, (*v)[j])) {
                    cont = 1;
                }
            }
            if (cont) continue;

            // lägg till i listan
            if (*top >= *cap) {
                char** new = realloc(*v, ((*cap)*2)*sizeof(char*));
                if (!new) goto malloc_error;

                *cap *= 2;
                *v = new;
            }

            // givet sax, lägg till getcwd

            char wd[PATH_MAX];
            getcwd(wd, PATH_MAX);

            if (is_sax && pack_name[0] != '/') { // absoluta sökvägar behöver inte få getcwd
                // lägg på getcwd på namn
                int wd_len = strlen(wd);

                char* new_name = malloc(pack_len+wd_len+1);
                if (!new_name) goto malloc_error;

                int new_name_len = wd_len + pack_len + 2;
                memcpy(new_name, wd, wd_len);
                new_name[wd_len] = '/';
                memcpy(new_name + wd_len + 1, pack_name, pack_len);
                new_name[wd_len + 1 + pack_len] = '\0';
                
                free(pack_name);
                pack_name = new_name;
            } else if (!is_sax){
                char* prefix = LIBPATH;
                if (is_gung) prefix = PACKPATH;

                int prefix_len = strlen(prefix);
                // prefixlen + packlen + / + strlen(main.fg)
                int new_name_len = prefix_len+pack_len+1+7;
                char* new_name = malloc(new_name_len+1);
                if (!new_name) goto malloc_error;

                memcpy(new_name, prefix, prefix_len);

                memcpy(new_name+prefix_len, pack_name, pack_len);

                new_name[prefix_len+pack_len] = '/';

                memcpy(new_name+prefix_len+1+pack_len, "main.fg", 7);
                new_name[new_name_len] = '\0';

                free(pack_name);
                pack_name = new_name;

            }


            (*v)[*top] = pack_name;
            (*top)++;

            
            bult_rec(pack_name, visited_files, buff);

        }
        
    }

    // lägg till filinnehållet i buff
    // hitta var programmet börjar utan import statements
    char* cleaned_text = text;
    int new_text_len = text_len;

    for (int i = 0; i < text_len; i++){
        if (!strncmp(text+i, "bult ", 5)) {
            while (text[i] != ';') i++;
            cleaned_text = text+i+1; // +1 tar bort ;
        }
    }
    new_text_len = text_len-(cleaned_text-text);

    // allokera och kopiera
    if (!(buff->string)) {
        buff->string = malloc(new_text_len+1);
        if (!(buff->string)) goto malloc_error;
        buff->len = new_text_len;
        
        memcpy(buff->string, cleaned_text, new_text_len);
        buff->string[new_text_len] = '\0';

    } else {
        buff->string = realloc(buff->string, buff->len+new_text_len+2);
        if (!(buff->string)) {
            goto malloc_error;
        }

        memcpy(buff->string+buff->len+1, cleaned_text, new_text_len);
        buff->string[buff->len] = '\n';
        buff->len = buff->len+new_text_len+1;
        buff->string[buff->len] = '\0';
    }
    

    free(text);

    return;

    malloc_error:
        throw_error(ERR_MALLOC, (String){"Memory allocation failed", strlen("Memory allocation failed")}, NULL);
}



Bult_Ret bult(char* file_name){

    int visited_capacity = 16;

    Bult_File_Types visited_files = {

        .sax.arr = malloc(visited_capacity*sizeof(char*)), // realloca vid behov
        .gung.arr = malloc(visited_capacity*sizeof(char*)), 
        .lib.arr = malloc(visited_capacity*sizeof(char*)),

        .sax.cap = visited_capacity,
        .gung.cap = visited_capacity,
        .lib.cap = visited_capacity,

        .sax.top = 0,
        .gung.top = 0,
        .lib.top = 0
    };

    String buff = {
        .len = 0,
        .string = 0
    };

    bult_rec(file_name, &visited_files, &buff);


    printf("\nvisited:\nsax:");
    for (int q = 0; q < visited_files.sax.top; q++){
        printf("    %s", (visited_files.sax.arr)[q]);
    }
    printf("\ngung:");
    for (int q = 0; q < visited_files.gung.top; q++){
        printf("    %s", (visited_files.gung.arr)[q]);
    }
    printf("\nlib:");
    for (int q = 0; q < visited_files.lib.top; q++){
        printf("    %s", (visited_files.lib.arr)[q]);
    }
    printf("\n\n");

    printf("RESULTAT: \n%.*s\n", buff.len, buff.string);

    return (Bult_Ret){.buff = 0, .import_line_count = 0};
}

int main(){
    Bult_Ret b = bult("bult-test.fg");

    return 0;
}