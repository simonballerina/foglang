char *read_file(const char *filename)
{
    FILE *f = fopen(filename, "rb");
    if (!f)
        return NULL;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    char *buffer = malloc(size + 1);
    if (!buffer)
        return NULL;

    fread(buffer, 1, size, f);
    buffer[size] = '\0'; // null-terminate

    fclose(f);
    return buffer;
}