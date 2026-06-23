#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#include "utils.h"


size_t get_timelines(grid_t *grid, int row, int column)
{
    grid_tile_t *tile = grid_get(grid, row, column);
    while (tile->value != '^' && tile != NULL_TILE) {
        tile = grid_get(grid, ++row, column);
    }

    if (tile->metadata.size_value > 0) {
        return tile->metadata.size_value;
    }

    if (tile->value == '^') {
        tile->metadata.size_value = get_timelines(grid, row, column - 1) + get_timelines(grid, row, column + 1);
        return tile->metadata.size_value;
    }

    return 1;
}


int main(void)
{
    grid_t *grid = file_read_grid(STR("data/2025/day7.test"));

    int start_row;
    int start_column;

    for (size_t row = 0; row < grid->rows; row++) {
        for (size_t column = 0; column < grid->columns; column++) {
            grid_tile_t *tile = grid_get(grid, row, column);
            if (tile->value == 'S') {
                start_row = row;
                start_column = column;
            }
        }
    }

    size_t timelines = get_timelines(grid, start_row, start_column);
    printf("Timelines: %zu\n", timelines);

    grid_free(grid);
}
