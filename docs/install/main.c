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

    #define FOGLANG_BIN_LINK "https://github.com/simonballerina/foglang-test/raw/refs/heads/main/foglang2-linux-x86_64"
    #define BANDVAGN_BIN_LINK "https://github.com/simonballerina/foglang-test/raw/refs/heads/main/vagn-linux-x86_64"

    #define FOGLANG_INSTALL_PATH "/usr/local/bin/foglang2"
    #define BANDVAGN_INSTALL_PATH "/usr/local/bin/vagn"

    #define PACK_PATH_SUFFIX "/.local/share/foglang4/packages/"
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

    struct passwd *pw = getpwnam(new_owner);

    char origin_dir[PATH_MAX];
    #if defined(__linux__) || defined(__APPLE__)
        getcwd(origin_dir, sizeof(origin_dir));
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

    mkdir_p_chown(pack_path, 0770, sudo_user);
    free(pack_path);

    char* lib_path = strdup(LIB_PATH);
    mkdir_p_chown(lib_path, 0770, "root");
    free(lib_path);


    return 0;
    malloc_error:
        printf("Memory allocation failed\n");
        return 1;
    
    #endif

}

char* get_json_item(char* json, char* key) {

    int json_len = strlen(json);
    int key_len = strlen(key);

    for (int j = 0; j < json_len; j++) {
        if (j + key_len <= json_len && !strncmp(key, json + j, key_len)) {
            char* value = json + j + key_len;

            while (*value == ' ' || *value == '\t' || *value == '\n' || *value == '\r' || *value == ':') {
                value++;
            }

            if (*value != '\"') {
                return NULL;
            }

            value++;
            char* end = value;
            while (*end != '\0') {
                if (*end == '\\' && end[1] != '\0') {
                    end += 2;
                    continue;
                }
                if (*end == '\"') {
                    break;
                }
                end++;
            }

            int item_len = (int)(end - value);
            char* item = malloc(item_len + 1);
            if (!item) {
                printf("Memory allocation failed\n");
                return NULL;
            }

            memcpy(item, value, item_len);
            item[item_len] = '\0';
            return item;
        }
    }

    return NULL;

}

int download_github_folder(const char* link, const char* path, const char* owner) {
    
    char* list;
    
    if (http_get(link, &list) != 0){
        printf("Could not resolve list of files. Aborting...\n");
        return -1;
    }
    int len = strlen(list);

    for (int i = 0; i < len; i++){
        if (i + 7 < len && !strncmp("\"name\":", list + i, 7)) {
            char* name = get_json_item(list + i, "\"name\"");
            char* type = get_json_item(list + i, "\"type\"");
            char* download_link = get_json_item(list + i, "\"download_url\"");


            if (download_link && !strcmp(type, "file")) {
                printf("    Downloading file '%s'...", name);
                char* file_path;
                if (path) {
                    int path_len = strlen(path);
                    int name_len = strlen(name);
                    file_path = malloc(path_len + name_len + 2);
                    if (!file_path) goto malloc_error;

                    memcpy(file_path, path, path_len);
                    file_path[path_len] = '/';
                    memcpy(file_path + path_len + 1, name, name_len);
                    file_path[path_len + name_len + 1] = '\0';
                } else {
                    file_path = strdup(name);
                }

                if (http_download(download_link, file_path) == 0) {
                    printf("        Download successful! (%s)\n", file_path);
                } else {
                    printf("        Download unsuccessful. Exiting install...\n");
                    free(file_path);
                    return -1;
                }
                free(file_path);

            } else if (!strcmp(type, "dir")) {
                char* dir_path;
                if (path) {
                    int path_len = strlen(path);
                    int name_len = strlen(name);
                    dir_path = malloc(path_len + name_len + 2);
                    if (!dir_path) goto malloc_error;

                    memcpy(dir_path, path, path_len);
                    dir_path[path_len] = '/';
                    memcpy(dir_path + path_len + 1, name, name_len);
                    dir_path[path_len + name_len + 1] = '\0';
                } else {
                    dir_path = strdup(name);
                }

                mkdir(dir_path, 0770);
                struct passwd *pw = getpwnam(owner);
                chown(dir_path, pw->pw_uid, (gid_t)-1);

                char* new_link = get_json_item(list + i, "\"self\"");

                download_github_folder(new_link, dir_path, owner);

                free(new_link);
                free(dir_path);

            }



            free(name);
            free(type);
            free(download_link);
        }
    }
    free(list);

    return 0;

    malloc_error:
        printf("Memory allocation failed\n");
        return 1;

}


int main() {
    if (!is_admin()) {
        printf("You need to run the installation as root/administrator to install Foglang!\n");
        return -1;
    }

    printf("Downloading Foglang2 from '%s' to '%s'...\n", FOGLANG_BIN_LINK, FOGLANG_INSTALL_PATH);

    if (http_download(FOGLANG_BIN_LINK, FOGLANG_INSTALL_PATH) == 0) {
        printf("    Download successful!\n");
        chmod(FOGLANG_INSTALL_PATH, 0777);
    } else {
        printf("    Download unsuccessful. Exiting install...\n");
        return -1;
    }
    printf("Downloading Bandvagn from '%s' to '%s'...\n", BANDVAGN_BIN_LINK, BANDVAGN_INSTALL_PATH);

    if (http_download(BANDVAGN_BIN_LINK, BANDVAGN_INSTALL_PATH) == 0) {
        printf("    Download successful!\n");
        chmod(BANDVAGN_INSTALL_PATH, 0777);
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