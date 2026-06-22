#include <fcntl.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


typedef struct string_list_t {
    char **strings;
    size_t capacity;
    size_t length;
} string_list_t;


char *read_file(const char *path);
string_list_t *string_list_new(void);
bool string_list_append_n(string_list_t *list, const char *string, size_t string_length);
bool string_list_append(string_list_t *list, const char *string);
void string_list_free(string_list_t *list);
string_list_t *string_split(const char *string, const char *separator, bool keep_empty);


typedef struct tile_t {
    char initial;
    bool laser;
    size_t timelines;
} tile_t;

typedef struct grid_t {
    tile_t *tiles;
    size_t rows;
    size_t columns;
} grid_t;


tile_t null_tile = {0};


tile_t *get_tile(grid_t *grid, int row, int column)
{
    if (row < 0 || column < 0 || row >= grid->rows || column >= grid->columns) {
        return &null_tile;
    }
    return &grid->tiles[row * grid->columns + column];
}

size_t get_timelines(grid_t *grid, int row, int column)
{
    int i = row;
    tile_t *tile = get_tile(grid, i, column);
    while (tile->initial != '^' && tile->initial != 0) {
        tile = get_tile(grid, ++i, column);
    }

    if (tile->timelines > 0) {
        return tile->timelines;
    }

    if (tile->initial == '^') {
        tile->timelines = get_timelines(grid, i, column - 1) + get_timelines(grid, i, column + 1);
        return tile->timelines;
    }

    return 1;
}

int main(void)
{
    char *contents = read_file("data/2025/day7.real");
    string_list_t *lines = string_split(contents, "\n", false);
    int start_row;
    int start_column;

    grid_t *grid = malloc(sizeof(grid_t));
    grid->rows = lines->length;
    grid->columns = strlen(lines->strings[0]);
    grid->tiles = malloc(sizeof(tile_t) * grid->rows * grid->columns);
    for (size_t i=0; i < grid->rows; i++) {
        char *line = lines->strings[i];
        for (size_t j=0; j < grid->columns; j++) {
            tile_t *tile = &grid->tiles[(i * grid->columns) + j];
            tile->initial = line[j];
            tile->timelines = 0;
            if (tile->initial == 'S') {
                tile->laser = true;
                start_row = (int)i;
                start_column = (int)j;
            }
        }
    }

    size_t timelines = get_timelines(grid, start_row, start_column);
    printf("Timelines: %zu\n", timelines);
}


char *read_file(const char *path)
{
    bool success = false;
    char *contents = NULL;
    size_t capacity = 4096;
    size_t string_length = 0;
    int fd = -1;

    fd = open(path, O_RDONLY);
    if (fd == -1) {
        goto CLEANUP;
    }

    contents = malloc(4096);
    if (contents == NULL) {
        fprintf(stderr, "error: out of memory while allocating string capacity\n");
        goto CLEANUP;
    }

    ssize_t bytes_read;
    do {
        if (capacity - string_length < 4096) {
            capacity *= 2;
            char *resized_buffer = realloc(contents, capacity);
            if (resized_buffer == NULL) {
                fprintf(stderr, "error: out of memory while growing string capacity\n");
                goto CLEANUP;
            }
            contents = resized_buffer;
        }
        bytes_read = read(fd, contents + string_length, capacity - (string_length + 1));
        if (bytes_read < 0) {
            goto CLEANUP;
        }
        string_length += (ssize_t)bytes_read;
    } while (bytes_read > 0);
    contents[string_length] = '\0';

    success = true;

  CLEANUP:
    if (fd != -1) {
        close(fd);
    }

    if (success) {
        return contents;
    } else {
        if (contents != NULL) {
            free(contents);
        }
        return NULL;
    }
}


string_list_t *string_list_new(void)
{
    string_list_t *list = malloc(sizeof(string_list_t));
    if (list == NULL) {
        fprintf(stderr, "error: out of memory while allocating list\n");
        return NULL;
    }
    list->capacity = 32;
    list->length = 0;
    list->strings = malloc(list->capacity * sizeof(char *));
    if (list->strings == NULL) {
        fprintf(stderr, "error: out of memory while allocating list capacity\n");
        free(list);
        return NULL;
    }
    return list;
}


bool string_list_append_n(string_list_t *list, const char *string, size_t string_length)
{
    if (list->length == list->capacity) {
        size_t new_capacity = list->capacity * 2;
        char **resized_strings = realloc(list->strings, new_capacity * sizeof(char *));
        if (resized_strings == NULL) {
            fprintf(stderr, "error: out of memory while growing list capacity\n");
            return false;
        }
        list->capacity = new_capacity;
        list->strings = resized_strings;
    }
    char *copied_string = malloc(string_length + 1);
    if (copied_string == NULL) {
        fprintf(stderr, "error: out of memory while allocating string capacity\n");
        return false;
    }
    memcpy(copied_string, string, string_length);
    copied_string[string_length] = '\0';
    list->strings[list->length++] = copied_string;
    return true;
}


bool string_list_append(string_list_t *list, const char *string)
{
    return string_list_append_n(list, string, strlen(string));
}


void string_list_pop(string_list_t *list)
{
    if (list->length == 0) {
        return;
    }
    free(list->strings[list->length - 1]);
    list->length--;
}


void string_list_free(string_list_t *list)
{
    for (size_t i=0; i < list->length; i++) {
        free(list->strings[i]);
    }
    free(list->strings);
    free(list);
}


string_list_t *string_split(const char *string, const char *separator, bool keep_empty)
{
    size_t string_length = strlen(string);
    size_t separator_length = strlen(separator);

    string_list_t *list = string_list_new();
    if (list == NULL) {
        return NULL;
    }
    size_t substring_start = 0;

    for (size_t i=0; i < string_length; i++) {
        if ((string_length - i) >= separator_length && memcmp(string + i, separator, separator_length) == 0) {
            if (i - substring_start > 0 || keep_empty) {
                if (!string_list_append_n(list, string + substring_start, i - substring_start)) {
                    string_list_free(list);
                    return NULL;
                }
            }
            i += separator_length;
            substring_start = i;
        }
    }

    if (string_length - substring_start > 0 || keep_empty) {
        if (!string_list_append_n(list, string + substring_start, string_length - substring_start)) {
            string_list_free(list);
            return NULL;
        }
    }

    return list;
}
