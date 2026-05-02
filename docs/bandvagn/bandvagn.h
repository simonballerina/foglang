

struct url_data {
    size_t size;
    char* data;
};

typedef struct {
    char* name;
    char* version;
    char* url;
} Token;

typedef struct {
    Token* tokens;
    size_t size;
} Token_List;

size_t write_data(void *ptr, size_t size, size_t nmemb, void *userdata);

int http_get(const char *url, char **out);

Token_List parse_packages(char* data);
char* get_lib_path_unix(char* base, char* name);
int check_and_create_dir(char* path);

int install_package(char* package_name);
int remove_package(char* package_name);
