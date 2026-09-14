#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>
#include <sys/stat.h>
#include <pwd.h>
#include <ftw.h>

#include "install.h"
#include "../bandvagn/http.c"
#include "../bandvagn/bandvagn_utils.c" // get_json_item(), download_github_folder()
#include "install_utils.c"

#if defined(_WIN32)
    #include <hlobj_core.h>

    #define FOGLANG_BIN_LINK "https://github.com/simonballerina/foglang/releases/latest/download/foglang2_windows_x86_64.exe"
    #define BANDVAGN_BIN_LINK "https://github.com/simonballerina/foglang/releases/latest/download/vagn_windows_x86_64.exe"

    #define FOGLANG_INSTALL_PATH "C:\\Program Files\\foglang2\\build\\foglang2.exe"
    #define BANDVAGN_INSTALL_PATH "C:\\Program Files\\bandvagn\\build\\vagn.exe"

    #define PACK_PATH "C:\\Program Files\\foglang2\\packages\\"
    #define LIBPATH "C:\\Program Files\\foglang2\\lib\\"


#elif defined(__APPLE__)

    #define FOGLANG_BIN_LINK "https://github.com/simonballerina/foglang/releases/latest/download/foglang2_macos_arm64"
    #define BANDVAGN_BIN_LINK "https://github.com/simonballerina/foglang/releases/latest/download/vagn_macos_arm64"

    #define FOGLANG_INSTALL_PATH "/usr/local/bin/foglang2"
    #define BANDVAGN_INSTALL_PATH "/usr/local/bin/vagn"

    #define PACK_PATH_SUFFIX "/Library/Application Support/foglang2/packages/"
    #define LIB_PATH "/usr/local/lib/foglang2/"

#elif defined(__linux__)

    #define FOGLANG_BIN_LINK "https://github.com/simonballerina/foglang/releases/latest/download/foglang2_linux_x86_64"
    #define BANDVAGN_BIN_LINK "https://github.com/simonballerina/foglang/releases/latest/download/vagn_linux_x86_64"

    #define FOGLANG_INSTALL_PATH "/usr/local/bin/foglang2"
    #define BANDVAGN_INSTALL_PATH "/usr/local/bin/vagn"

    #define PACK_PATH_SUFFIX "/.local/share/foglang2/packages/"
    #define LIB_PATH "/usr/local/lib/foglang2/"

#endif

#define LIB_LINK "https://api.github.com/repos/simonballerina/foglang/contents/docs/foglang2/lib?ref=main"

#ifndef PATH_MAX
    #define PATH_MAX 1024
#endif



int is_admin() {
    #ifdef _WIN32
        return IsUserAnAdmin();
    #endif

    return !geteuid();
}


void mkdir_p_chown(char* path, int permission, char* new_owner){
    printf("Creating directory: %s\n", path);

    #ifdef _WIN32
        #define SLASH '\\'
    #else
        #define SLASH '/'
    #endif

    int len = strlen(path);


    char origin_dir[PATH_MAX];
    #if defined(__linux__) || defined(__APPLE__)
        getcwd(origin_dir, sizeof(origin_dir));
        struct passwd *pw = getpwnam(new_owner);
    #elif defined (_WIN32)
        GetCurrentDirectoryW(PATH_MAX, origin_dir);
    #endif

    for (int i = 0; i < len; i++) {
        if (path[i] == SLASH) {

            for (int j = i+1; j < len; j++) {
                if (path[j] == SLASH) {
                    path[j] = '\0';
                    #if defined(__linux__) || defined(__APPLE__)
                        chdir(path);
                        if (mkdir(path, permission) == 0) {
                            chown(path, pw->pw_uid, (gid_t)-1);
                        }
                    #elif defined(_WIN32)
                        SetCurrentDirectoryW(path);
                        CreateDirectoryA(path, NULL);
                        
                    #endif
                    

                    path[j] = SLASH;
                    i+=(j-i);
                }
            }

        }
    }
    #if defined(__linux__) || defined(__APPLE__)
        chdir(origin_dir);
    #elif defined (_WIN32)
        SetCurrentDirectory(origin_dir, NULL);
    #endif

}


int create_dirs(){
    #ifdef _WIN32

    mkdir_p_chown(PACK_PATH);
    mkdir_p_chown(LIB_PATH);

    return 0;
    #else
    char* sudo_user = getenv("SUDO_USER");
    if (sudo_user == NULL) {
        printf("SUDO_USER environment variable not set\n");
        return 1;
    }
    struct passwd *pw = getpwnam(sudo_user);

    char* home = pw->pw_dir;
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

    mkdir_p_chown(pack_path, 0755, sudo_user);
    free(pack_path);

    char* lib_path = strdup(LIB_PATH);
    mkdir_p_chown(lib_path, 0755, "root");
    free(lib_path);


    return 0;
    malloc_error:
        printf("Memory allocation failed\n");
        return 1;
    
    #endif

}

int remove_foglang(){
    printf("Removing Foglang2 and Bandvagn package manager. Continue? (y/n) ");
    char in;
    scanf(" %c",&in);

    if (in != 'Y' && in != 'y') {
        printf("Cancelling removal...\n");
        return 0;
    }

    printf("Removing binaries... ");
    if (remove(FOGLANG_INSTALL_PATH) == 0 && remove(BANDVAGN_INSTALL_PATH) == 0) printf("Done\n");
    else {
        printf("Could not remove binaries '%s' and '%s'\n", FOGLANG_INSTALL_PATH, BANDVAGN_INSTALL_PATH);
    }

    printf("Removing Library & Bandvagn package directory...\n");
    if (nftw(LIB_PATH, ftw_rm, 64, FTW_DEPTH | FTW_PHYS) == 0) {
        printf("    Successfully removed Library directory!\n");
    } else {
        printf("    Could not remove Library directory '%s'\n", LIB_PATH);
    }


    char* sudo_user = getenv("SUDO_USER");
    if (sudo_user == NULL) {
        printf("SUDO_USER environment variable not set\n");
        return 1;
    }
    struct passwd *pw = getpwnam(sudo_user);

    char* home = pw->pw_dir;
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


    if (nftw(pack_path, ftw_rm, 64, FTW_DEPTH | FTW_PHYS) == 0) {
        printf("    Successfully removed Package directory!\n");
    } else {
        printf("    Could not remove Package directory '%s'\n", pack_path);
    }
    free(pack_path);


    return 0;

    malloc_error:
        printf("Memory allocation failed\n");
        return 1;
}

int main(int argc, char** argv) {

    if (!is_admin()) {
        printf("You need to run the installation as root/administrator to install Foglang!\n");
        return -1;
    }

    if (argc == 2 && strcmp(argv[1], "remove") == 0) {
        return remove_foglang();
    }

    printf("Installing Foglang2 and Bandvagn package manager. Continue? (y/n) ");
    char in;
    scanf(" %c",&in);
    if (in != 'Y' && in != 'y') {
        printf("Cancelling installation...\n");
        return 0;
    }

    printf("Downloading Foglang2 from '%s' to '%s'...\n", FOGLANG_BIN_LINK, FOGLANG_INSTALL_PATH);

    if (http_download(FOGLANG_BIN_LINK, FOGLANG_INSTALL_PATH) == 0) {
        printf("    Download successful!\n");
        chmod(FOGLANG_INSTALL_PATH, 0755);
    } else {
        printf("    Download unsuccessful. Exiting install...\n");
        return -1;
    }
    printf("Downloading Bandvagn from '%s' to '%s'...\n", BANDVAGN_BIN_LINK, BANDVAGN_INSTALL_PATH);

    if (http_download(BANDVAGN_BIN_LINK, BANDVAGN_INSTALL_PATH) == 0) {
        printf("    Download successful!\n");
        chmod(BANDVAGN_INSTALL_PATH, 0755);
    } else {
        printf("    Download unsuccessful. Exiting install...\n");
        return -1;
    }

    printf("Creating Library & Bandvagn package directory...\n");
    if (create_dirs() == 0) {
        printf("Successfully created Library & Bandvagn package directory!\n");
    } else {
        printf("Could not create Library & Bandvagn package directory. Exiting install...\n");
    }
    
    download_github_folder(LIB_LINK, LIB_PATH, "root");
    
    printf("Successfully installed Foglang2!\n");

    return 0;
}