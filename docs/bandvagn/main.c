/*
Bandvagn package manager for Foglang

    Syntax:
        vagn <command> <option> 

        ex:
        vagn install package_name
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>


#include "bandvagn.h"

#include "http.c"

#define PACKAGES_PATH "https://raw.githubusercontent.com/simonballerina/foglang/refs/heads/main/docs/bandvagn/packages.fgpkg"

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
    Token* tokens = malloc((len+1)*sizeof(Token));

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

char* get_lib_path_unix(char* base, char* name) {
    // add .fg file extention to name
    int name_len = strlen(name);
    char* name_with_ext = malloc(name_len + 3 + 1);
    if (!name_with_ext) {
        fprintf(stderr, "Could not allocate memory for name_with_ext\n");
        exit(1);
    }
    memcpy(name_with_ext, name, name_len);
    memcpy(name_with_ext + name_len, ".fg", 3);
    name_with_ext[name_len+3] = '\0';

    const char *home = getenv("HOME");
    if (!home) {
        fprintf(stderr, "HOME environment variable not set, can't determine install path\n");
        exit(1);
    }
    int base_len = strlen(home) + strlen(base);
    name_len = strlen(name_with_ext);
    char *lib_path = malloc(base_len + name_len + 1);
    if (!lib_path) {
        fprintf(stderr, "Could not allocate memory for lib_path\n");
        exit(1);
    }

    memcpy(lib_path, home, strlen(home));
    memcpy(lib_path + strlen(home), base, strlen(base));
    lib_path[base_len] = '\0';
    strcat(lib_path, name_with_ext);
    lib_path[base_len+name_len] = '\0';

    return lib_path;

}

int check_and_create_dir(char* path) {
    char* dir_path = malloc(strlen(path) + 1);
    if (!dir_path) {
        fprintf(stderr, "Failed to allocate memory for dir_path\n");
        return 1;
    }
    memcpy(dir_path, path, strlen(path));
    char* last_slash = strrchr(dir_path, '/');
    if (last_slash) {
        *last_slash = '\0';
        if (access(dir_path, F_OK) == -1) {
            if (mkdir(dir_path, 0755) != 0) {
                fprintf(stderr, "Failed to create directory '%s'\n", dir_path);
                free(dir_path);
                return 1;
            }
        }
    }
    free(dir_path);

    return 0;
}


int install_package(char* package_name) {
    printf("Locating package '%s'...\n", package_name);
    int EXIT_CODE = 0;
    // Find packages file in Foglang github
    char* packages = NULL;
    if (http_get(PACKAGES_PATH, &packages) != 0) {
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



    #ifdef _WIN32
    #elif __APPLE__
        char* base "/Library/";
    #elif __linux__ || __unix__ || __posix__
        char* base = "/.local/lib/foglang2/packages/";
        char* lib_path = get_lib_path_unix(base, found_packages.tokens[found_index].name);
        // check if directory exists, if not create it
        if (check_and_create_dir(lib_path) != 0) {
            EXIT_CODE = 1;
            goto exit_program;
        }

    #endif
    
    if (http_download(found_packages.tokens[found_index].url, lib_path) == 0) {
        printf("Package download successful!\n");
    } else {
        printf("Package download failed.\n");
        // delete file if it exists
        remove(lib_path);
        EXIT_CODE = 1;
        goto exit_program;
    }

    exit_program:

    free(packages);
    for (size_t i = 0; i < found_packages.size; i++) 
        free(found_packages.tokens[i].url);
    
    free(found_packages.tokens);




    return EXIT_CODE;
}

int remove_package(char* package_name) {
    printf("Removing package '%s'...\n", package_name);
    #ifdef _WIN32
    #elif __APPLE__
        char* base "/Library/";
        char* lib_path = get_lib_path_unix(base, package_name);
    #elif __linux__ || __unix__ || __posix__
        char* base = "/.local/lib/foglang2/packages/";
        char* lib_path = get_lib_path_unix(base, package_name);
    #endif

    if (remove(lib_path) == 0) {
        printf("Package '%s' removed successfully!\n", package_name);
    } else {
        fprintf(stderr, "Failed to remove package '%s'. Is it installed?\n", package_name);
        return 1;
    }

    return 0;
}

char** read_ls(char* path) {
    DIR *d;
    struct dirent *dir;

    d = opendir(path);
    if (d == NULL) {
        perror("opendir");
    }
    int file_amount = 0;
    int ret_cap = 8;
    int ret_top = 0;
    char** ret = calloc(ret_cap, sizeof(char*));
    if (!ret) goto malloc_error;

    while ((dir = readdir(d)) != NULL) {
        if ((strcmp(dir->d_name, "..") == 0) || (strcmp(dir->d_name, ".") == 0)) continue;

        int name_len = strlen(dir->d_name);
        char* str = malloc((name_len+1)*sizeof(char));
        if (!str) goto malloc_error;
        memcpy(str, dir->d_name, name_len);
        str[name_len] = '\0';

        if (ret_top >= ret_cap) { 
            ret = realloc(ret, (ret_cap + 8) * sizeof(char*)); 
            ret_cap += 8; 
            if (!ret) goto malloc_error; 
        }
        ret[ret_top++] = str;
    }
    ret[ret_top] = NULL;

    closedir(d);
    return ret;


    malloc_error:
        fprintf(stderr, "Could not allocate memory\n");
        exit(1);

}

int update_packages() {
    printf("Locating packages...\n");
    int EXIT_CODE = 0;
    // Find packages file in Foglang github
    char* packages = NULL;
    if (http_get(PACKAGES_PATH, &packages) != 0) {
        fprintf(stderr, "Failed to fetch packages\n");
        return 1;
    }
    Token_List found_packages = parse_packages(packages);

    // Find installed packages
    
    #ifdef _WIN32

    #elif __linux__ || __unix__ || __posix__ || __APPLE__

        char* name = getenv("HOME");
        #ifdef __APPLE__
            char* path_suffix = "/Library";
        #else
            char* path_suffix = "/.local/lib/foglang2/packages";
        #endif
        int name_len = strlen(name);
        int suffix_len = strlen(path_suffix);
        int len = name_len+suffix_len;
        char path[len+1];
        memcpy(path, name, name_len);
        memcpy(path+name_len, path_suffix, suffix_len);
        path[len] = '\0';

        char** ls_ret = read_ls(path);

        // ta bort fg file extentions
        for (int i = 0; ls_ret[i]; i++) { 
            int pack_len = strlen(ls_ret[i]); 
            if (pack_len > 3 && strcmp(ls_ret[i] + pack_len - 3, ".fg") == 0) {
                ls_ret[i][pack_len - 3] = '\0'; 
            }
        }

        for (int i = 0; i < sizeof(ls_ret) && ls_ret[i]; i++) {
            int pack_len = strlen(ls_ret[i]);
            if (strncmp(ls_ret[i]+(pack_len-3), ".fg", 3) == 0) {
                ls_ret[i][pack_len-1] = '\0';
                ls_ret[i][pack_len-2] = '\0';
                ls_ret[i][pack_len-3] = '\0';
            }
        }

        for (int i = 0; i < found_packages.size; i++) {
            for (int j = 0; ls_ret[j]; j++) { 
                if (strcmp(found_packages.tokens[i].name, ls_ret[j]) == 0) 
                install_package(ls_ret[j]); 
            }
        }
        printf("Update successful\n");

    for (int i = 0; ls_ret[i]; i++) free(ls_ret[i]); 
    free(ls_ret);
    #endif
    
    exit: 

    return EXIT_CODE;
}



int main(int argc, char *argv[]) {
    int EXIT_CODE = 0;

    int do_install = 0;
    int do_remove = 0;
    int do_update = 0;
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
        } else {
            printf("Unknown command '%s'\n", argv[1]);
            return -1;
        }
    } else {
        printf("No command provided\n");
        // todo: help

        return -1;
    }

    if (do_install) {
        EXIT_CODE = install_package(package_to_modify);
    } else if (do_remove) {
        EXIT_CODE = remove_package(package_to_modify);
    } else if (do_update) {
        EXIT_CODE = update_packages();
    }

    return EXIT_CODE;
}