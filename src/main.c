#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#include "utils.h"


// Task Specific
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
    string_t *contents = file_read_contents(STR("data/2025/day7.real"));
    string_list_t *lines = str_split(contents, STR("\n"), false);
    int start_row;
    int start_column;

    grid_t *grid = malloc(sizeof(grid_t));
    grid->rows = lines->length;
    grid->columns = lines->strings[0].length;
    grid->tiles = malloc(sizeof(tile_t) * grid->rows * grid->columns);
    for (size_t i=0; i < grid->rows; i++) {
        char *line = (char *)lines->strings[0].cursor;
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

    str_free(contents);
    str_list_free(lines);
}
