



size_t write_data(void *ptr, size_t size, size_t nmemb, void *userdata) {
    struct url_data *data = userdata;
    size_t n = size * nmemb;

    char *tmp = realloc(data->data, data->size + n + 1);
    if (!tmp) {
        return 0; // abort safely
    }

    data->data = tmp;
    memcpy(data->data + data->size, ptr, n);

    data->size += n;
    data->data[data->size] = '\0';

    return n;
}

int http_get(const char *url, char **out) {
    CURL *curl = curl_easy_init();
    if (!curl) return -1;

    struct url_data data = {0};

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        curl_easy_cleanup(curl);
        free(data.data);
        return -1;
    }

    curl_easy_cleanup(curl);
    *out = data.data;
    return 0;
}

size_t write_file(void *ptr, size_t size, size_t nmemb, void *stream) {
    FILE *file = (FILE *)stream;
    return fwrite(ptr, size, nmemb, file);
}

int http_download(const char *url, const char *filename) {
    CURL *curl = curl_easy_init();
    if (!curl) return -1;

    FILE *file = fopen(filename, "wb");
    if (!file) {
        curl_easy_cleanup(curl);
        return -1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_file);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

    CURLcode res = curl_easy_perform(curl);

    fclose(file);
    curl_easy_cleanup(curl);

    return (res == CURLE_OK) ? 0 : -1;
}