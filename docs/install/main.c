#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>
#include <sys/stat.h>
#include <pwd.h>

#include "install.h"
#include "../bandvagn/http.c"
#include "install_utils.c"

#if defined(_WIN32)
    #include <hlobj_core.h>
    #define INSTALL_PATH "C:\\Program Files\\foglang2\\build\\foglang2.exe"
    #define PACK_PATH "C:\\Program Files\\foglang2\\packages\\"

    #define LIB_PATH ""

#elif defined(__APPLE__)
    #define BIN_LINK "https://github.com/simonballerina/foglang-test/raw/refs/heads/main/foglang2-macos-arm64"
    #define INSTALL_PATH "build/foglang2"

    #define PACK_PATH_SUFFIX "/Library/Application Support/foglang4/packages/"
    #define LIB_PATH "/usr/local/lib/foglang2/"
#elif defined(__linux__)

    #define INSTALL_PATH "/usr/local/bin/foglang2"

    #define PACK_PATH_SUFFIX "/.local/share/foglang2/packages/"
    #define LIB_PATH "/usr/local/lib/foglang2/"

#endif

#define BIN_LINK "https://github.com/simonballerina/foglang-test/raw/refs/heads/main/foglang2-macos-arm64"


int is_admin() {
    #ifdef _WIN32
        return IsUserAnAdmin();
    #endif

    return !geteuid();
}


void mkdir_p_chown(char* path, int permission, char* new_owner){

    #ifdef _WIN32
        #define SLASH '\\'
    #else
        #define SLASH '/'
    #endif

    int len = strlen(path);

    struct passwd *pw = getpwnam(new_owner);


    for (int i = 0; i < len; i++) {
        if (path[i] == SLASH) {

            for (int j = i+1; j < len; j++) {
                if (path[j] == SLASH) {
                    path[j] = '\0';
                    printf("%s\n", path);
                    chdir(path);
                    if (mkdir(path, permission) == 0) {
                        chown(path, pw->pw_uid, (gid_t)-1);
                    }
                    

                    path[j] = SLASH;
                    i+=(j-i);
                }
            }

        }
    }

}


int create_dirs(){

    char* sudo_user = getenv("SUDO_USER");
    if (sudo_user == NULL) {
        printf("SUDO_USER environment variable not set\n");
        return 1;
    }
    char* home = getenv("HOME");
    if (home == NULL) {
        printf("HOME environment variable not set\n");
        return 1;
    }

    int home_len = strlen(home);
    int suffix_len = strlen(PACK_PATH_SUFFIX);

    char* pack_path = malloc(home_len+suffix_len+1);

    if (!pack_path) goto malloc_error;

    memcpy(pack_path, home, home_len);
    memcpy(pack_path+home_len, PACK_PATH_SUFFIX, suffix_len);
    pack_path[home_len+suffix_len] = '\0';

    mkdir_p_chown(pack_path, 0777, sudo_user);
    free(pack_path);

    char* lib_path = strdup(LIB_PATH);
    mkdir_p_chown(lib_path, 0777, "root");
    free(lib_path);


    return 0;

    malloc_error:
        printf("Memory allocation failed\n");
        return 1;
}


int download_github_folder(const char* link) {
    
    char* list;
    
    if (http_get(link, &list) != 0){
        printf("Could not resolve list of files. Aborting...\n");
        return -1;
    }
    int len = strlen(list);

    for (int i = 0; i < len; i++){
        if (i+7 < len && !strncmp("\"name\":", list+i, 7)) {
            char* name;
        }
    }
    free(list);

    return 0;

}


int main() {
    if (!is_admin()) {
        printf("You need to run the installation as root/administrator to install Foglang!\n");
        return -1;
    }

    printf("Downloading Foglang2 from '%s' to '%s'...\n", BIN_LINK, INSTALL_PATH);

    if (http_download(BIN_LINK, INSTALL_PATH) == 0) {
        printf("Download successful!\n");
    } else {
        printf("Download unsuccessful. Exiting install...\n");
        return -1;
    }

    printf("Creating Library & Bandvagn package directory...\n");
    if (create_dirs() == 0) {
        printf("Successfully created Library & Bandvagn package directory!\n");
    } else {
        printf("Could not create Library & Bandvagn package directory. Exiting install...\n");
    }
    

    download_folder("https://api.github.com/repos/simonballerina/foglang/contents/docs/foglang2/lib?ref=main");
    


    return 0;
}