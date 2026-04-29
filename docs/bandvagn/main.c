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

#include "bandvagn.h"

#include "http.c"

#define PACKAGES_PATH "https://raw.githubusercontent.com/simonballerina/foglang/refs/heads/main/docs/bandvagn/packages.fgpkg"

Token* parse_packages(char* data) {
    int data_len = strlen(data);

    // make token lines like this:
    // package_name version url
    // split by space and store in Token struct
    // store in array of Token structs

    for (int i = 0; i < data_len; i++){ 
        if (data[i] == '\n') {
            // split line by space
            char* line = strtok(data, "\n");
            while (line != NULL) {
                char* name = strtok(line, " ");
                char* version = strtok(NULL, " ");
                char* url = strtok(NULL, " ");

                printf("name: %s, version: %s, url: %s\n", name, version, url);

                line = strtok(NULL, "\n");
            }
        }


    }
}

int main(int argc, char *argv[]) {

    int install = 0;
    int remove = 0;
    char* package_to_modify = NULL;

    if (argc > 1) {
        if (strcmp(argv[1], "install") == 0) {
            install = 1;
            if (argc < 3) {
                printf("No package name provided for installation\n");
                return 1;
            }
            package_to_modify = argv[2];
            printf("Installing package: %s\n", argv[2]);
        } else if (strcmp(argv[1], "remove") == 0) {
            remove = 1;
            if (argc < 3) {
                printf("No package name provided for removal\n");
                return 1;
            }
            package_to_modify = argv[2];
            printf("Removing package: %s\n", argv[2]);
        } else {
            printf("Unknown command: %s\n", argv[1]);
            return -1;
        }
    } else {
        printf("No command provided\n");
        // todo: help

        return -1;
    }


    printf("Locating package '%s'...\n", package_to_modify);

    // Find packages json in Foglang github
    char* packages = http_get(PACKAGES_PATH_test);
    printf("packages: %s\n", packages);
    parse_packages(packages);

    free(packages);
    return 0;
}