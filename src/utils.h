#ifndef _AOC_UTILS_H
#define _AOC_UTILS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Function types
typedef bool (*compare_f)(void *, void *);


// Math Utils
typedef struct vec2_t {
    size_t x;
    size_t y;
} vec2_t;

typedef struct vec3_t {
    size_t x;
    size_t y;
    size_t z;
} vec3_t;

size_t vec2_squared_distance(vec2_t *a, vec2_t *b);
size_t vec3_squared_distance(vec3_t *a, vec3_t *b);
double vec2_distance(vec2_t *a, vec2_t *b);
double vec3_distance(vec3_t *a, vec3_t *b);


// String and String List Utils
typedef struct string_t {
    void *buffer;
    uint8_t *cursor;
    size_t length;
} string_t;

typedef struct string_list_t {
    string_t *strings;
    size_t length;
    size_t capacity;
} string_list_t;

#define STR(s) (&((string_t){ \
    .buffer = NULL, \
    .cursor = (uint8_t*)(s), \
    .length = sizeof(s) - 1, \
}))

#define STR_FROM_PTR(ptr, ptr_length) (&((string_t){ \
    .buffer = NULL, \
    .cursor = (uint8_t*)(ptr), \
    .length = (ptr_length), \
}))

string_t *str_new(const void *buffer, size_t buffer_length);
string_t *str_copy(string_t *string);
void str_left_strip(string_t *string, char c);
void str_right_strip(string_t *string, char c);
void str_strip(string_t *string, char c);
string_list_t *str_split(string_t *string, string_t *separator, bool keep_empty);
string_t *str_concat(string_t *a, string_t *b);
void str_ensure_ownership(string_t *string);
string_t *str_substr(string_t *string, size_t start, size_t end);
void str_free(string_t *string);

string_list_t *str_list_new(void);
void str_list_free(string_list_t *list);
void str_list_append(string_list_t *list, string_t *string);
string_t *str_list_join(string_list_t *list, string_t *delimiter);
void str_list_ensure_ownership(string_list_t *list);


// Grid Utils
typedef union metadata_u {
    int int_value;
    size_t size_value;
    uintptr_t uintptr_value;
    float float_value;
    double double_value;
    void *ptr;
} metadata_u;

typedef struct grid_tile_t {
    uint8_t initial_value;
    uint8_t value;
    metadata_u metadata;
} grid_tile_t;

typedef struct grid_t {
    grid_tile_t *tiles;
    size_t rows;
    size_t columns;
} grid_t;

extern grid_tile_t *NULL_TILE;

grid_t *grid_create(size_t rows, size_t columns);
grid_t *grid_from_lines(string_list_t *lines);
grid_tile_t *grid_get(grid_t *grid, size_t row, size_t column);
void grid_free(grid_t *grid);


// Map Utils
typedef struct map_entry_t {
    void *key;
    void *value;
} map_entry_t;

typedef struct map_t {
    compare_f key_compare;
    map_entry_t *entries;
    size_t entry_count;
    size_t capacity;
} map_t;

map_t *map_create(compare_f key_compare);
void *map_get(map_t *map, void *key);
void map_set(map_t *map, void *key, void *value);
void map_remove_null_values(map_t *map);
bool map_contains(map_t *map, void *key);
void map_free(map_t *map);


// File Utils
string_t *file_read_contents(string_t *path);
string_list_t *file_read_lines(string_t *path, bool keep_empty);
grid_t *file_read_grid(string_t *path);


// Comparators
bool compare_strings(string_t *a, string_t *b);
bool compare_vec2(vec2_t *a, vec2_t *b);
bool compare_vec3(vec3_t *a, vec3_t *b);

#endif
