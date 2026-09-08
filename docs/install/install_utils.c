// Custom Foglang implementation of the libc function getline() due to windows incompatibility.
void fg_getline(char **line){
    char c = 0;

    int i = 0;
    int size = 16;

    *line = malloc(size);
    if (!(*line)) goto malloc_error;

    while (c != EOF && c != '\n') {
        c = getc(stdin);

        if (i >= size) {
            *line = realloc(*line, size+16);
            size += 16;
            if (!(*line)) goto malloc_error;
        }
        (*line)[i++] = c;
    }
    (*line)[i-1] = '\0'; // i-1 removes '\n' character

    return;

    malloc_error:
        printf("Memory allocation failed\n");
        exit(1);
}