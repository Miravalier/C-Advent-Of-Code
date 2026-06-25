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
    vec3_t *a;
    vec3_t *b;
} dist_record_t;


int compare_dist_record(dist_record_t *a, dist_record_t *b)
{
    if (fabs(a->distance - b->distance) < 0.00001) {
        return 0;
    } else if (a->distance < b->distance) {
        return -1;
    } else {
        return 1;
    }
}


int circuit_length_compare(vec3_list_t *a, vec3_list_t *b)
{
    if (a->item_count < b->item_count) {
        return -1;
    } else if (a->item_count > b->item_count) {
        return 1;
    } else {
        return 0;
    }
}


int main(void)
{
    map_t *neighborhoods = map_create(vec3_equals);
    map_t *circuit_map = map_create(vec3_equals);
    string_list_t *lines = file_read_lines(STR("data/2025/day8.real"), false);
    vec3_t *points = malloc(sizeof(vec3_t) * lines->length);
    size_t point_count = lines->length;
    vec3_list_t **circuits = malloc(sizeof(vec3_list_t*) * point_count);

    for (size_t i=0; i < lines->length; i++) {
        string_list_t *line = str_split(&lines->strings[i], STR(","), false);
        vec3_t *point = &points[i];
        point->x = str_to_size_t(&line->strings[0]);
        point->y = str_to_size_t(&line->strings[1]);
        point->z = str_to_size_t(&line->strings[2]);
        str_list_free(line);
    }
    str_list_free(lines);

    printf("Point Coint: %zu\n", point_count);

    // Create a one-node circuit for every point
    for (size_t i=0; i < point_count; i++) {
        vec3_t *point = &points[i];
        vec3_list_t *circuit = vec3_list_new();
        circuits[i] = circuit;
        vec3_list_append(circuit, point);
        map_set(circuit_map, point, circuit);
    }

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
    size_t grid_distance = ((size_t)grid_distance_double);
    printf("Grid Distance: %zu\n", grid_distance);

    // Add every point to its nearest neighborhood
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

    // Debug Print Neighborhoods
    printf("Neighborhood Count: %zu\n", neighborhoods->entry_count);
    for (size_t i=0; i < neighborhoods->entry_count; i++) {
        map_entry_t *entry = &neighborhoods->entries[i];
        vec3_list_t *list = entry->value;
        vec3_t *key = entry->key;
        // printf("Neighborhood (%zu,%zu,%zu): %zu\n", key->x, key->y, key->z, list->item_count);
    }

    // Create sorted list of distance records
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
                        dist_record_t *record = malloc(sizeof(dist_record_t));
                        record->distance = vec3_distance(point, nearby_point);
                        if (vec3_compare(point, nearby_point) < 0) {
                            record->a = point;
                            record->b = nearby_point;
                        } else {
                            record->a = nearby_point;
                            record->b = point;
                        }
                        if (!linked_list_insert_sorted_unique(distance_records, record, (compare_f)compare_dist_record)) {
                            free(record);
                        }
                    }
                }
            }
        }
    }

    printf("Distance Records: %zu\n", distance_records->count);

    // Create connections and consolidate circuits
    size_t i=0;
    size_t connection_count = 1000;
    for (linked_list_node_t *node = distance_records->head; node && i < connection_count; node = node->next, i++) {
        dist_record_t *record = node->value;
        if (i < 10) {
        printf("%0.2f (%zu,%zu,%zu)---(%zu,%zu,%zu)\n",
            record->distance,
            record->a->x, record->a->y, record->a->z,
            record->b->x, record->b->y, record->b->z);
        }
        // Combine the two circuits
        vec3_list_t *circuit_a = map_get(circuit_map, record->a);
        vec3_list_t *circuit_b = map_get(circuit_map, record->b);
        if (circuit_a == circuit_b) {
            continue;
        }
        for (size_t j=0; j < circuit_b->item_count; j++) {
            vec3_t *vec = &circuit_b->items[j];
            vec3_list_append(circuit_a, vec);
            map_set(circuit_map, vec, circuit_a);
        }
    }

    // De-dup circuits
    linked_list_t *unique_circuits = linked_list_new();
    for (size_t i=0; i < point_count; i++) {
        vec3_list_t *circuit = map_get(circuit_map, &points[i]);
        linked_list_insert_sorted_unique(unique_circuits, circuit, pointer_compare);
    }
    printf("Unique Circuits: %zu\n", unique_circuits->count);

    // Sort circuits
    linked_list_t *sorted_circuits = linked_list_new();
    for (linked_list_node_t *node = unique_circuits->head; node; node = node->next) {
        vec3_list_t *circuit = node->value;
        linked_list_insert_sorted(sorted_circuits, circuit, (compare_f)circuit_length_compare);
    }
    printf("Largest Circuit: %zu\n", ((vec3_list_t*)sorted_circuits->tail->value)->item_count);

    size_t result = 1;
    size_t results_calculated = 0;
    size_t result_count = 3;
    for (linked_list_node_t *node = sorted_circuits->tail; node && results_calculated < result_count; node = node->prev, results_calculated++) {
        vec3_list_t *circuit = node->value;
        result *= circuit->item_count;
    }
    printf("Result: %zu\n", result);

    // Cleanup
    free(points);
    linked_list_free(unique_circuits);
    linked_list_free(sorted_circuits);

    for (size_t i=0; i < neighborhoods->entry_count; i++) {
        map_entry_t *entry = &neighborhoods->entries[i];
        free(entry->key);
        vec3_list_free(entry->value);
    }
    map_free(neighborhoods);

    for (size_t i=0; i < point_count; i++) {
        vec3_list_free(circuits[i]);
    }
    free(circuits);
    map_free(circuit_map);

    for (linked_list_node_t *node = distance_records->head; node; node = node->next) {
        free(node->value);
    }
    linked_list_free(distance_records);
}
