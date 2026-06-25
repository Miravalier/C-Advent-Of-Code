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
    if (a->distance == b->distance || fabs(a->distance - b->distance) < 0.000000001) {
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
    string_list_t *lines = file_read_lines(STR("data/2025/day8.real"), false);
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
    printf("Point Coint: %zu\n", point_count);

    map_t *circuit_map = map_create(vec3_equals);
    vec3_list_t **circuits = malloc(sizeof(vec3_list_t*) * point_count);
    size_t distance_record_capacity = 4096;
    size_t distance_record_count = 0;
    dist_record_t *distance_records = malloc(sizeof(dist_record_t) * distance_record_capacity);

    // Create a one-node circuit for every point
    for (size_t i=0; i < point_count; i++) {
        vec3_t *point = &points[i];
        vec3_list_t *circuit = vec3_list_new();
        circuits[i] = circuit;
        vec3_list_append(circuit, point);
        map_set(circuit_map, point, circuit);
    }

    // Create list of distance records
    for (size_t i=0; i < point_count; i++) {
        vec3_t *point = &points[i];
        for (size_t j=0; j < point_count; j++) {
            vec3_t *other_point = &points[j];
            if (vec3_equals(point, other_point)) {
                continue;
            }
            if (distance_record_count == distance_record_capacity) {
                distance_record_capacity *= 2;
                distance_records = realloc(distance_records, sizeof(dist_record_t) * distance_record_capacity);
            }
            dist_record_t *record = &distance_records[distance_record_count++];
            record->distance = vec3_distance(point, other_point);
            if (vec3_compare(point, other_point) < 0) {
                record->a = point;
                record->b = other_point;
            } else {
                record->a = other_point;
                record->b = point;
            }
        }
    }

    printf("Distance Records: %zu\n", distance_record_count);

    // Sort the distance records
    qsort(distance_records, distance_record_count, sizeof(dist_record_t), (compare_f)compare_dist_record);

    // Create connections and consolidate circuits
    size_t circuit_count = point_count;
    size_t result = 0;
    for (size_t i=0; i < distance_record_count; i++) {
        dist_record_t *record = &distance_records[i];
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
        result = record->a->x * record->b->x;
        circuit_count--;
        if (circuit_count == 1) {
            break;
        }
    }
    printf("Circuit Count: %zu\n", circuit_count);
    printf("Result: %zu\n", result);

    // Cleanup
    free(points);
    for (size_t i=0; i < point_count; i++) {
        vec3_list_free(circuits[i]);
    }
    free(circuits);
    map_free(circuit_map);
    free(distance_records);
}
