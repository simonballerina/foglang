


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
                    printf("        Download successful!\n");
                } else {
                    printf("        Download unsuccessful. Exiting...\n");
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

                #if defined(__APPLE__) || defined(__linux__)

                mkdir(dir_path, 0755);
                if (owner != NULL) {
                    struct passwd *pw = getpwnam(owner);
                    chown(dir_path, pw->pw_uid, (gid_t)-1);
                }
                

                #elif defined(_WIN32)
                
                CreateDirectoryA(dir_path, NULL);
                
                #endif

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



int ftw_rm(const char *path, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {

    return remove(path);

}

