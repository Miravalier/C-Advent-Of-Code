#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/param.h>

#include "utils.h"
#include "pcg_basic.h"


typedef struct dist_record_t {
    double distance;
} dist_record_t;


int compare_dist_record(dist_record_t *a, dist_record_t *b) {
    return (int)round(a->distance - b->distance);
}


int main(void)
{
    map_t *neighborhoods = map_create(vec3_equals);
    string_list_t *lines = file_read_lines(STR("data/2025/day8.test"), false);
    vec3_t *points = malloc(sizeof(vec3_t) * lines->length);
    size_t point_count = lines->length;

    for (size_t i=0; i < lines->length; i++) {
        string_list_t *line = str_split(&lines->strings[i], STR(","), false);
        vec3_t *point = &points[i];
        point->x = str_to_size_t(&line->strings[0]);
        point->y = str_to_size_t(&line->strings[1]);
        point->z = str_to_size_t(&line->strings[2]);
        str_list_free(line);
    }
    str_list_free(lines);

    double grid_distance_double = 0.0;
    for (size_t i=0; i < point_count; i++) {
        // Get two random points (that aren't the same point)
        size_t a_index = (size_t)pcg32_boundedrand((uint32_t)point_count);
        size_t b_index = (size_t)pcg32_boundedrand((uint32_t)point_count);
        while (a_index == b_index) {
            b_index = (size_t)pcg32_boundedrand((uint32_t)point_count);
        }
        vec3_t *a = &points[a_index];
        vec3_t *b = &points[b_index];

        // See if this is our new lowest distance
        double distance = vec3_distance(a, b);
        if (i == 0) {
            grid_distance_double = distance;
        } else {
            grid_distance_double = MIN(grid_distance_double, distance);
        }
    }
    size_t grid_distance = ((size_t)grid_distance_double) * 2;
    printf("Grid Distance: %zu\n", grid_distance);

    for (size_t i=0; i < point_count; i++) {
        vec3_t *point = &points[i];
        vec3_t neighborhood = {
            .x = ((point->x + (grid_distance / 2)) / grid_distance) * grid_distance,
            .y = ((point->y + (grid_distance / 2)) / grid_distance) * grid_distance,
            .z = ((point->z + (grid_distance / 2)) / grid_distance) * grid_distance,
        };
        vec3_list_t *neighborhood_points = map_get(neighborhoods, &neighborhood);
        if (neighborhood_points == NULL) {
            vec3_t *neighborhood_key = vec3_copy(&neighborhood);
            neighborhood_points = vec3_list_new();
            map_set(neighborhoods, neighborhood_key, neighborhood_points);
        }
        vec3_list_append(neighborhood_points, point);
    }

    map_t *best_distances = map_create((equal_f)compare_dist_record);
    linked_list_t *distance_records = linked_list_new();
    for (size_t i=0; i < point_count; i++) {
        vec3_t *point = &points[i];
        vec3_t initial_neighborhood = {
            .x = ((point->x + (grid_distance / 2)) / grid_distance) * grid_distance,
            .y = ((point->y + (grid_distance / 2)) / grid_distance) * grid_distance,
            .z = ((point->z + (grid_distance / 2)) / grid_distance) * grid_distance,
        };
        for (ssize_t x = -1; x <= 1; x++) {
            for (ssize_t y = -1; y <= 1; y++) {
                for (ssize_t z = -1; z <= 1; z++) {
                    vec3_t neighborhood = {
                        .x = (size_t)((ssize_t)initial_neighborhood.x + x * (ssize_t)grid_distance),
                        .y = (size_t)((ssize_t)initial_neighborhood.y + y * (ssize_t)grid_distance),
                        .z = (size_t)((ssize_t)initial_neighborhood.z + z * (ssize_t)grid_distance),
                    };
                    vec3_list_t *neighborhood_points = map_get(neighborhoods, &neighborhood);
                    if (neighborhood_points == NULL) {
                        continue;
                    }
                    for (size_t j=0; j < neighborhood_points->item_count; j++) {
                        vec3_t *nearby_point = &neighborhood_points->items[j];
                        if (vec3_equals(point, nearby_point)) {
                            continue;
                        }
                        double distance = vec3_distance(point, nearby_point);
                        // store_distance(best_distances, distance, point, nearby_point);
                    }
                }
            }
        }
    }

    free(points);
    for (size_t i=0; i < neighborhoods->entry_count; i++) {
        map_entry_t *entry = &neighborhoods->entries[i];
        free(entry->key);
        vec3_list_free(entry->value);
    }
    map_free(neighborhoods);
    map_free(best_distances);
}
