#include <ctype.h>
#include <fcntl.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/param.h>

#include "utils.h"

// Math Utils
size_t vec2_squared_distance(vec2_t *a, vec2_t *b)
{
    ssize_t delta_x = ((ssize_t)a->x - (ssize_t)b->x);
    ssize_t delta_y = ((ssize_t)a->y - (ssize_t)b->y);
    return (size_t)(delta_x * delta_x + delta_y * delta_y);
}

double vec2_distance(vec2_t *a, vec2_t *b)
{
    ssize_t delta_x = ((ssize_t)a->x - (ssize_t)b->x);
    ssize_t delta_y = ((ssize_t)a->y - (ssize_t)b->y);
    return sqrt((double)(delta_x * delta_x + delta_y * delta_y));
}

vec2_t *vec2_copy(vec2_t *vec)
{
    vec2_t *new_vec = malloc(sizeof(vec2_t));
    new_vec->x = vec->x;
    new_vec->y = vec->y;
    return new_vec;
}

size_t vec3_squared_distance(vec3_t *a, vec3_t *b)
{
    ssize_t delta_x = ((ssize_t)a->x - (ssize_t)b->x);
    ssize_t delta_y = ((ssize_t)a->y - (ssize_t)b->y);
    ssize_t delta_z = ((ssize_t)a->z - (ssize_t)b->z);
    return (size_t)(delta_x * delta_x + delta_y * delta_y + delta_z * delta_z);
}

double vec3_distance(vec3_t *a, vec3_t *b)
{
    ssize_t delta_x = ((ssize_t)a->x - (ssize_t)b->x);
    ssize_t delta_y = ((ssize_t)a->y - (ssize_t)b->y);
    ssize_t delta_z = ((ssize_t)a->z - (ssize_t)b->z);
    return sqrt((double)(delta_x * delta_x + delta_y * delta_y + delta_z * delta_z));
}

vec3_t *vec3_copy(vec3_t *vec)
{
    vec3_t *new_vec = malloc(sizeof(vec3_t));
    new_vec->x = vec->x;
    new_vec->y = vec->y;
    new_vec->z = vec->z;
    return new_vec;
}

vec2_list_t *vec2_list_new()
{
    vec2_list_t *list = malloc(sizeof(vec2_list_t));
    list->capacity = 16;
    list->item_count = 0;
    list->items = malloc(sizeof(vec2_t) * list->capacity);
    return list;
}

void vec2_list_append(vec2_list_t *list, vec2_t *vec)
{
    if (list->item_count == list->capacity) {
        list->capacity *= 2;
        list->items = realloc(list->items, sizeof(vec2_t) * list->capacity);
    }
    vec2_t *stored_vec = &list->items[list->item_count++];
    memcpy(stored_vec, vec, sizeof(vec2_t));
}

void vec2_list_free(vec2_list_t *list)
{
    free(list->items);
    free(list);
}

vec3_list_t *vec3_list_new()
{
    vec3_list_t *list = malloc(sizeof(vec3_list_t));
    list->capacity = 16;
    list->item_count = 0;
    list->items = malloc(sizeof(vec3_t) * list->capacity);
    return list;
}

void vec3_list_append(vec3_list_t *list, vec3_t *vec)
{
    if (list->item_count == list->capacity) {
        list->capacity *= 2;
        list->items = realloc(list->items, sizeof(vec3_t) * list->capacity);
    }
    vec3_t *stored_vec = &list->items[list->item_count++];
    memcpy(stored_vec, vec, sizeof(vec3_t));
}

void vec3_list_free(vec3_list_t *list)
{
    free(list->items);
    free(list);
}

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

string_t *str_substr(string_t *string, size_t start, size_t end)
{
    string_t *result = malloc(sizeof(string_t));
    result->cursor = string->cursor + start;
    result->length = end - start;
    return result;
}

ssize_t str_to_ssize_t(string_t *string)
{
    if (string->length == 0) {
        return 0;
    }

    if (string->cursor[0] == '-') {
        return -(ssize_t)str_to_size_t(STR_FROM_PTR(string->cursor + 1, string->length - 1));
    } else {
        return (ssize_t)str_to_size_t(string);
    }
}

size_t str_to_size_t(string_t *string)
{
    size_t result = 0;

    for (size_t i=0; i < string->length; i++) {
        if (!isdigit(string->cursor[i])) {
            return 0;
        }
        result *= 10;
        result += string->cursor[i] - '0';
    }

    return result;
}

// String List Utils
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
    dst->buffer = NULL;
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

// Linked List Utils
linked_list_t *linked_list_new(void)
{
    return calloc(1, sizeof(linked_list_t));
}

void linked_list_append(linked_list_t *list, void *value)
{
    linked_list_node_t *node = calloc(1, sizeof(linked_list_node_t));
    node->value = value;
    if (list->head == NULL) {
        list->head = node;
        list->tail = node;
    } else {
        list->tail->next = node;
    }
    list->tail = node;
}

void linked_list_insert_sorted(linked_list_t *list, void *value, compare_f compare_values)
{
    linked_list_node_t *node = calloc(1, sizeof(linked_list_node_t));
    node->value = value;

    if (list->head == NULL) {
        list->head = node;
        list->tail = node;
        return;
    }

    if (compare_values(value, list->head->value) <= 0) {
        node->next = list->head;
        list->head = node;
        return;
    }

    linked_list_node_t *previous = list->head;
    linked_list_node_t *cursor = list->head->next;
    while (cursor) {
        if (compare_values(value, cursor->value) <= 0) {
            node->next = cursor;
            previous->next = node;
            return;
        }
        previous = cursor;
        cursor = cursor->next;
    }

    list->tail->next = node;
    list->tail = node;
}

// Map Utils
map_t *map_create(equal_f key_equals)
{
    map_t *map = malloc(sizeof(map_t));
    map->key_equals = key_equals;
    map->capacity = 32;
    map->entry_count = 0;
    map->entries = calloc(map->capacity, sizeof(map_entry_t));
    return map;
}

void *map_get(map_t *map, void *key)
{
    for (size_t i=0; i < map->entry_count; i++) {
        map_entry_t *entry = &map->entries[i];
        if (map->key_equals(entry->key, key) == 0) {
            return entry->value;
        }
    }

    return NULL;
}

void map_set(map_t *map, void *key, void *value)
{
    for (size_t i=0; i < map->entry_count; i++) {
        map_entry_t *entry = &map->entries[i];
        if (map->key_equals(entry->key, key) == 0) {
            entry->value = value;
            return;
        }
    }

    if (value == NULL) {
        return;
    }

    if (map->entry_count == map->capacity) {
        map->capacity *= 2;
        map->entries = realloc(map->entries, sizeof(map_entry_t) * map->capacity);
    }

    map_entry_t *entry = &map->entries[map->entry_count++];
    entry->key = key;
    entry->value = value;
}

void map_remove_null_values(map_t *map)
{
    size_t new_entry_count = 0;
    map_entry_t *new_entries = malloc(sizeof(map_entry_t) * map->capacity);
    for (size_t i=0; i < map->entry_count; i++) {
        map_entry_t *entry = &map->entries[i];
        if (entry->value != NULL) {
            map_entry_t *new_entry = &new_entries[new_entry_count++];
            new_entry->key = entry->key;
            new_entry->value = entry->value;
        }
    }
    free(map->entries);
    map->entries = new_entries;
    map->entry_count = new_entry_count;
}

bool map_contains(map_t *map, void *key)
{
    return map_get(map, key) != NULL;
}

void map_free(map_t *map)
{
    free(map->entries);
    free(map);
}

// Grid Utils
grid_tile_t NULL_TILE_STORAGE = {0};
grid_tile_t *NULL_TILE = &NULL_TILE_STORAGE;

grid_t *grid_create(size_t rows, size_t columns)
{
    grid_t *grid = malloc(sizeof(grid_t));
    grid->rows = rows;
    grid->columns = columns;
    grid->tiles = calloc(rows * columns, sizeof(grid_tile_t));
    return grid;
}

grid_t *grid_from_lines(string_list_t *lines)
{
    size_t rows = lines->length;
    size_t columns = 0;
    for (size_t i=0; i < lines->length; i++) {
        columns = MAX(columns, lines->strings[i].length);
    }
    grid_t *grid = grid_create(rows, columns);
    for (size_t row=0; row < lines->length; row++) {
        string_t *line = &lines->strings[row];
        for (size_t column=0; column < line->length; column++) {
            grid_tile_t *tile = grid_get(grid, row, column);
            tile->initial_value = line->cursor[column];
            tile->value = line->cursor[column];
        }
    }
    return grid;
}

grid_tile_t *grid_get(grid_t *grid, size_t row, size_t column)
{
    if (row < grid->rows && column < grid->columns) {
        return grid->tiles + (row * grid->columns + column);
    } else {
        return NULL_TILE;
    }
}

void grid_free(grid_t *grid)
{
    free(grid->tiles);
    free(grid);
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

string_list_t *file_read_lines(string_t *path, bool keep_empty)
{
    string_t *contents = file_read_contents(path);
    str_strip(contents, '\n');

    string_list_t *lines = str_split(contents, STR("\n"), keep_empty);
    str_list_ensure_ownership(lines);
    str_free(contents);

    return lines;
}

grid_t *file_read_grid(string_t *path)
{
    string_list_t *lines = file_read_lines(path, false);
    grid_t *grid = grid_from_lines(lines);
    str_list_free(lines);
    return grid;
}

// Ready to go comparators
bool string_equals(const void *a, const void *b)
{
    const string_t *a_string = a;
    const string_t *b_string = b;
    if (a_string->length != b_string->length) {
        return false;
    }
    return memcmp(a_string->cursor, b_string->cursor, a_string->length) == 0;
}

bool vec2_equals(const void *a, const void *b)
{
    const vec2_t *a_vec = a;
    const vec2_t *b_vec = b;
    return (a_vec->x == b_vec->x) && (a_vec->y == b_vec->y);
}

bool vec3_equals(const void *a, const void *b)
{
    const vec3_t *a_vec = a;
    const vec3_t *b_vec = b;
    return (a_vec->x == b_vec->x) && (a_vec->y == b_vec->y) && (a_vec->z == b_vec->z);
}
