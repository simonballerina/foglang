#define _XOPEN_SOURCE 700           // for nftw() on Linux

/*
Bandvagn package manager for Foglang

    Syntax:
        vagn <command> <option> 

        ex:
        vagn install package_name
        vagn highlight
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <ftw.h>

#ifdef _WIN32
#include <io.h>
#include <direct.h>
#endif

#include "bandvagn.h"

#include "http.c"

#include "bandvagn_utils.c"

#define PACKAGES_LIST_PATH "https://raw.githubusercontent.com/simonballerina/foglang-packages/refs/heads/main/packages.fgpkg"
#define HIGHLIGHT_PATH "https://github.com/handej08/foglanghighlight/releases/latest/download/foglanghighlight.vsix"

#if defined(_WIN32)
    #define PACK_PATH "C:\\Program Files\\foglang2\\packages\\"
#elif defined(__APPLE__)
    #define PACK_PATH_SUFFIX "/Library/Application Support/foglang2/packages/"
#elif defined(__linux__)
    #define PACK_PATH_SUFFIX "/.local/share/foglang2/packages/"

#endif

#ifndef PACKPATH
    #define PACKPATH ""
#endif

Token_List parse_packages(char* data) {
    int data_len = strlen(data);
    char divider = ' ';
    // make token lines like this:
    // package_name version url
    // split by space and store in Token struct
    // store in array of Token structs
    int len = 0;
    int i = 0;
    while (data[i]) if (data[i++] == '\n') len++;
    Token* tokens = calloc((len+1), sizeof(Token));

    int top_tok = 0;
    for (int i = 0; i < data_len; i++){ 

        if (data[i] == '\n' || i == 0) {
            if (i != 0) {
                data[i] = '\0';
                tokens[top_tok].name = data+i+1;
            } else {
                tokens[top_tok].name = data;
            }

            for (int j = i+1; j < data_len; j++){
                if (data[j] == divider) {
                    data[j] = '\0';
                    tokens[top_tok].version = data+j+1;
                    for (int k = j+1; k < data_len; k++) {
                        if (data[k] == divider) {
                            data[k] = '\0';
                            // find length of url string
                            int url_len = 0;
                            for (int a = k+1; a < data_len; a++) {
                                if (data[a] == '\n' || data[a] == '\0') break;
                                url_len++;
                            }
                            char *url_heap = malloc(url_len + 1);
                            if (!url_heap) {
                                fprintf(stderr, "Failed to allocate memory for URL\n");
                                exit(1);
                            } else {
                                memcpy(url_heap, data+k+1, url_len);
                                url_heap[url_len] = '\0';
                            }

                            tokens[top_tok].url = url_heap;

                            
                            break;
                        }
                    }
                    break;
                }
            
            }
            //printf("Package: %s, Version: %s, URL: %s\n", tokens[top_tok].name, tokens[top_tok].version, tokens[top_tok].url);
            top_tok++;
        }
    }
    Token_List ret = {tokens, top_tok};
    return ret;

}


char* get_pack_path(const char* pack_name){

    #if defined(_WIN32)
        #define SLASH '\\'
        int pack_path_len = strlen(PACK_PATH);
    #else
        #define SLASH '/'
        int pack_path_len = strlen(PACK_PATH_SUFFIX);
    #endif

    char* pack_path;
    int name_len = strlen(pack_name);

    #ifndef _WIN32
        // lägg på HOME
        char* home = getenv("HOME");
        int home_len = strlen(home);

        pack_path = malloc(home_len+pack_path_len+name_len+1);
        if (!pack_path) goto malloc_error;

        memcpy(pack_path, home, home_len);
        memcpy(pack_path+home_len, PACK_PATH_SUFFIX, pack_path_len);
        memcpy(pack_path+home_len+pack_path_len, pack_name, name_len);
        pack_path[home_len+pack_path_len+name_len] = '\0';

    #else
        // lägg inte på HOME (windows)
        pack_path = malloc(pack_path_len+name_len+1);
        if (!pack_path) goto malloc_error;

        memcpy(pack_path, PACK_PATH, pack_path_len);
        memcpy(pack_path+pack_path_len, pack_name, name_len);
        pack_path[pack_path_len+name_len] = '\0';

    #endif
    return pack_path;

    malloc_error:
        printf("Memory allocation failed\n");
        exit(1);
}


int check_and_create_dir(char* path) {
    char* dir_path = malloc(strlen(path) + 1);
    if (!dir_path) {
        fprintf(stderr, "Failed to allocate memory for dir_path\n");
        return 1;
    }
    size_t path_len = strlen(path);
    memcpy(dir_path, path, path_len);
    dir_path[path_len] = '\0';

    char* last_slash = strrchr(dir_path, '/');
    char* last_backslash = strrchr(dir_path, '\\');
    char* last_sep = NULL;
    if (last_slash && last_backslash) {
        last_sep = (last_slash > last_backslash) ? last_slash : last_backslash;
    } else {
        last_sep = last_slash ? last_slash : last_backslash;
    }

    if (last_sep) {
        *last_sep = '\0';
        #ifdef _WIN32
        if (_access(dir_path, 0) == -1) {
            if (_mkdir(dir_path) != 0) {
                fprintf(stderr, "Failed to create directory '%s'\n", dir_path);
                free(dir_path);
                return 1;
            }
        }
        #else
        if (access(dir_path, F_OK) == -1) {
            if (mkdir(dir_path, 0755) != 0) {
                fprintf(stderr, "Failed to create directory '%s'\n", dir_path);
                free(dir_path);
                return 1;
            }
        }
        #endif
    }
    free(dir_path);

    return 0;
}


char** read_dir(char* path){
    DIR *dir = opendir(path);
    struct dirent *entry;

    int cap = 8;
    char** items = malloc(cap*sizeof(char*));
    int top = 0;


    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            if (top >= cap) {
                char** tmp = realloc(items, cap*2);
                if (!tmp) goto malloc_error;
                items = tmp;
                cap*=2;
            }

            items[top++] = entry->d_name;

        }
    }

    items[top] = 0;


    return items;

    malloc_error:
        printf("Memory allocation failed\n");
        exit(1);
}


int list_installed_packages(){
    printf("Installed packages:\n");
    char* pack_path = get_pack_path("");

    char** items = read_dir(pack_path);

    for (int i = 0; items[i]; i++){
        printf("    %s\n", items[i]);
    }

    free(pack_path);
    free(items);

    return 0;
}

int install_package(char* package_name) {
    printf("Locating package '%s'...\n", package_name);
    int EXIT_CODE = 0;
    // Find packages file in Foglang github
    char* packages = NULL;
    if (http_get(PACKAGES_LIST_PATH, &packages) != 0) {
        fprintf(stderr, "Failed to fetch packages\n");
        return 1;
    }
    Token_List found_packages = parse_packages(packages);

    // match package
    int found_index = -1;
    for (size_t i = 0; i < found_packages.size; i++) {
        if (strcmp(found_packages.tokens[i].name, package_name) == 0) {
            printf("Package '%s' found! Version: %s, URL: %s\n", found_packages.tokens[i].name, found_packages.tokens[i].version, found_packages.tokens[i].url);
            found_index = i;
            break;
        }
    }
    if (found_index == -1) {
        printf("Package '%s' not found in registry\n", package_name);
        EXIT_CODE = 1;
        goto exit_program;
    }

    char* lib_path = get_pack_path(found_packages.tokens[found_index].name);
    // check if directory exists, if not create it
    if (check_and_create_dir(lib_path) != 0) {
        EXIT_CODE = 1;
        goto exit_program;
    }
    mkdir(lib_path, 0755);
    
    if (download_github_folder(found_packages.tokens[found_index].url, lib_path, NULL) == 0) {
        printf("Package download successful!\n");
    } else {
        printf("Package download failed.\n");
        EXIT_CODE = 1;
        goto exit_program;
    }
    free(lib_path);

    exit_program:
    for (size_t i = 0; i < found_packages.size; i++) 
        free(found_packages.tokens[i].url);
    
    free(found_packages.tokens);
    free(packages);



    return EXIT_CODE;
}


int remove_package(char* package_name) {

    char* lib_path = get_pack_path(package_name);

    if (nftw(lib_path, ftw_rm, 64, FTW_DEPTH | FTW_PHYS) == 0) {
        printf("Package '%s' removed successfully!\n", package_name);
    } else {
        fprintf(stderr, "Failed to remove package '%s'. Is it installed?\n", package_name);
        return 1;
    }

    free(lib_path);

    return 0;
}


int update_packages() {
    printf("Updating packages...\n");
    int EXIT_CODE = 0;

    // Find packages file in Foglang github
    char* packages = NULL;
    if (http_get(PACKAGES_LIST_PATH, &packages) != 0) {
        fprintf(stderr, "Failed to fetch packages\n");
        return 1;
    }
    Token_List found_packages = parse_packages(packages);

    char* pack_path = get_pack_path("");
    char** installed_packages = read_dir(pack_path);
    free(pack_path);

    for (int i = 0; i < found_packages.size; i++) {
        for (int j = 0; installed_packages[j]; j++) { 
            if (strcmp(found_packages.tokens[i].name, installed_packages[j]) == 0) 
                install_package(installed_packages[j]); 
        }
    }

    free(installed_packages);
    printf("Update successful\n");

    return EXIT_CODE;
}



int get_highlighter() {
    int EXIT_CODE = 0;
    printf("Installing Foglanghighlight for Visual Studio Code...\n");
    #ifdef _WIN32
        system("powershell -Command curl -O foglanghighlight.vsix "HIGHLIGHT_PATH" && code.cmd --install-extension foglanghighlight.vsix && powershell -Command del -r foglanghighlight.vsix");
    #elif __APPLE__
        if (http_download(HIGHLIGHT_PATH, "foglanghighlight.vsix") == 0) {
            printf("Download successful!\n");
        } else {
            printf("Download failed, exiting...!\n");
            rmdir("foglanghighlight.vsix");
            exit(1);
        }
        system("'/Applications/Visual Studio Code.app/Contents/Resources/app/bin/code' --install-extension foglanghighlight.vsix && rm foglanghighlight.vsix");

    #elif __linux__ || __unix__ || __posix__
        if (http_download(HIGHLIGHT_PATH, "foglanghighlight.vsix") == 0){
            printf("Download successful!\n");
        } else {
            printf("Download failed, exiting...!\n");
            rmdir("foglanghighlight.vsix");
            exit(1);
        }
        system("code --install-extension foglanghighlight.vsix && rm foglanghighlight.vsix");
        
        
    #endif
    return EXIT_CODE;
}

int help() {
    int EXIT_CODE = 0;
    printf("\
usage: vagn [--help] \n\
            install <package_name>\n\
            remove <package_name>\n\
            update <package_name>\n\
            highlight\n\
            list\n\
\n\
Install primtalsletaren: vagn install primtalsletaren\n\
\n\
Links:\n\
- Documentation: https://foglang.readthedocs.io\n\
- Github: https://github.com/simonballerina/foglang\n");
    return EXIT_CODE;
}


int main(int argc, char *argv[]) {
    int EXIT_CODE = 0;

    int do_install = 0;
    int do_remove = 0;
    int do_update = 0;
    int do_highlight = 0;
    int do_list = 0;
    int flag_help = 0;
    char* package_to_modify = NULL;

    if (argc > 1) {
        if (strcmp(argv[1], "install") == 0) {
            do_install = 1;
            if (argc < 3) {
                printf("No package name provided for installation\n");
                return 1;
            }
            package_to_modify = argv[2];
            printf("Installing package '%s'\n", argv[2]);
        } else if (strcmp(argv[1], "remove") == 0) {
            do_remove = 1;
            if (argc < 3) {
                printf("No package name provided for removal\n");
                return 1;
            }
            package_to_modify = argv[2];
            printf("Removing package '%s'\n", argv[2]);
        } else if (strcmp(argv[1], "update") == 0) {
            do_update = 1;
        } else if (strcmp(argv[1], "highlight") == 0) {
            do_highlight = 1;
        } else if (strcmp(argv[1], "list") == 0) {
            do_list = 1;
        } else if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
            flag_help = 1;
        } else {
            printf("Unknown command '%s'\n", argv[1]);
            return -1;
        }
    } else {
        flag_help = 1;
    }

    if (do_install) {
        EXIT_CODE = install_package(package_to_modify);
    } else if (do_remove) {
        EXIT_CODE = remove_package(package_to_modify);
    } else if (do_update) {
        EXIT_CODE = update_packages();
    } else if (do_highlight) {
        EXIT_CODE = get_highlighter();
    } else if (flag_help) {
        EXIT_CODE = help();
    } else if (do_list) {
        EXIT_CODE = list_installed_packages();
    }

    return EXIT_CODE;
}