#pragma once

struct url_data {
    size_t size;
    char* data;
};

size_t write_data(void *ptr, size_t size, size_t nmemb, void *userdata);

int http_get(const char *url, char **out);