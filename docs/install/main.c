#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>

#include "install.h"
#include "../bandvagn/http.c"

#if defined(_WIN32)
    #include <hlobj_core.h>
    #define INSTALL_PATH "C:\\Program Files\\foglang2\\build\\foglang2.exe"

#elif defined(__APPLE__)
    #define BIN_LINK "https://github.com/simonballerina/foglang-test/raw/refs/heads/main/foglang2-macos-arm64"
    #define INSTALL_PATH "build/foglang2"

#elif defined(__linux__)

    #define INSTALL_PATH "/usr/local/bin/foglang2"

#endif

#define BIN_LINK "https://github.com/simonballerina/foglang-test/raw/refs/heads/main/foglang2-macos-arm64"


int is_admin() {
    #ifdef _WIN32
        return IsUserAnAdmin();
    #endif

    return !geteuid();
}

int main() {
    if (!is_admin()) {
        printf("You need to run the installation as root/administrator to install Foglang!\n");
        return -1;
    }

    printf("Downloading Foglang2 from '%s' to '%s'\n", BIN_LINK, INSTALL_PATH);

    if (http_download(BIN_LINK, INSTALL_PATH) == 0) {
        printf("Download successful!\n");
    }
    

    


    return 0;
}