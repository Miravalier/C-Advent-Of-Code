#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/param.h>

#include "utils.h"

// String Utils
string_t *str_new(const void *buffer, size_t buffer_length)
{
    string_t *string = malloc(sizeof(string_t));
    string->buffer = malloc(buffer_length);
    string->cursor = string->buffer;
    string->length = buffer_length;
    if (buffer) {
        memcpy(string->cursor, buffer, buffer_length);
    } else {
        memset(string->cursor, 0, buffer_length);
    }
    return string;
}

string_t *str_copy(string_t *string)
{
    string_t *result = malloc(sizeof(string_t));
    result->buffer = malloc(result->length);
    result->cursor = result->buffer;
    result->length = string->length;
    memcpy(result->cursor, string->cursor, string->length);
    return result;
}

void str_left_strip(string_t *string, char c)
{
    while (string->cursor[0] == (uint8_t)c) {
        string->cursor++;
        string->length--;
    }
}

void str_right_strip(string_t *string, char c)
{
    while (string->cursor[string->length - 1] == (uint8_t)c) {
        string->length--;
    }
}

void str_strip(string_t *string, char c)
{
    str_left_strip(string, c);
    str_right_strip(string, c);
}

string_list_t *str_split(string_t *string, string_t *separator, bool keep_empty)
{
    string_list_t *list = str_list_new();
    size_t substring_start = 0;

    for (size_t i=0; i < string->length; i++) {
        if ((string->length - i) >= separator->length && memcmp(string->cursor + i, separator->cursor, separator->length) == 0) {
            if (i - substring_start > 0 || keep_empty) {
                str_list_append(list, STR_FROM_PTR(string->cursor + substring_start, i - substring_start));
            }
            i += separator->length;
            substring_start = i;
        }
    }

    if (string->length - substring_start > 0 || keep_empty) {
        str_list_append(list, STR_FROM_PTR(string->cursor + substring_start, string->length - substring_start));
    }

    return list;
}

string_t *str_concat(string_t *a, string_t *b)
{
    string_t *result = str_new(NULL, a->length + b->length);
    memcpy(result->cursor, a->cursor, a->length);
    memcpy(result->cursor + a->length, b->cursor, b->length);
    return result;
}

void str_ensure_ownership(string_t *string)
{
    if (!string->buffer) {
        string->buffer = malloc(string->length);
        memcpy(string->buffer, string->cursor, string->length);
        string->cursor = string->buffer;
    }
}

void str_free(string_t *string)
{
    if (string->buffer) {
        free(string->buffer);
    }
    free(string);
}

string_list_t *str_list_new(void)
{
    string_list_t *list = malloc(sizeof(string_list_t));
    list->capacity = 32;
    list->length = 0;
    list->strings = malloc(list->capacity * sizeof(string_t));
    return list;
}

void str_list_free(string_list_t *list)
{
    for (size_t i=0; i < list->length; i++) {
        string_t *string = &list->strings[i];
        if (string->buffer) {
            free(string->buffer);
        }
    }
    free(list->strings);
    free(list);
}

void str_list_append(string_list_t *list, string_t *string)
{
    if (list->length == list->capacity) {
        list->capacity *= 2;
        list->strings = realloc(list->strings, list->capacity * sizeof(string_t));
    }
    string_t *dst = &list->strings[list->length++];
    dst->cursor = string->cursor;
    dst->length = string->length;
}

string_t *str_list_join(string_list_t *list, string_t *delimiter)
{
    size_t result_length = 0;
    for (size_t i=0; i < list->length; i++) {
        if (i != 0) {
            result_length += delimiter->length;
        }
        result_length += list->strings[i].length;
    }

    string_t *result = str_new(NULL, result_length);
    uint8_t *cursor = result->cursor;
    for (size_t i=0; i < list->length; i++) {
        string_t *string = &list->strings[i];
        if (i != 0) {
            memcpy(cursor, delimiter->cursor, delimiter->length);
            cursor += delimiter->length;
        }
        memcpy(cursor, string->cursor, string->length);
        cursor += string->length;
    }

    return result;
}

void str_list_ensure_ownership(string_list_t *list)
{
    for (size_t i=0; i < list->length; i++) {
        str_ensure_ownership(&list->strings[i]);
    }
}

// File Utils
string_t *file_read_contents(string_t *path)
{
    bool success = false;
    char *contents = NULL;
    size_t capacity = 4096;
    size_t string_length = 0;
    int fd = -1;
    char c_path[256] = {0};
    memcpy(c_path, path->cursor, MIN(255, path->length));

    fd = open(c_path, O_RDONLY);
    if (fd == -1) {
        goto CLEANUP;
    }

    contents = malloc(4096);

    ssize_t bytes_read;
    do {
        if (capacity - string_length < 4096) {
            capacity *= 2;
            contents = realloc(contents, capacity);
        }
        bytes_read = read(fd, contents + string_length, capacity - string_length);
        if (bytes_read < 0) {
            goto CLEANUP;
        }
        string_length += (ssize_t)bytes_read;
    } while (bytes_read > 0);

    success = true;

  CLEANUP:
    if (fd != -1) {
        close(fd);
    }

    if (success) {
        string_t *result = malloc(sizeof(string_t));
        result->buffer = contents;
        result->cursor = (uint8_t *)contents;
        result->length = string_length;
        return result;
    } else {
        if (contents != NULL) {
            free(contents);
        }
        return NULL;
    }
}
