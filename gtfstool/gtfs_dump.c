/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * The MIT License
 *
 * Copyright (c) 2018-2019 Val Laboratory Corporation.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include "gtfstool.h"

static void dump_stop_times(int cols, int rows, struct vector_t** v_tbl)
{
    int x, y;

    for (y = 0; y < rows; y++) {
        for (x = 0; x < cols; x++) {
            struct vector_t* vt;

            vt = v_tbl[x];
            if (y == 0) {
                if (x == 0) {
                    char* p;

                    p = (char*)vect_get(vt, y);
                    printf("%s", utf8_conv(p, (char*)alloca(256), 256));
                } else {
                    struct trip_t* trip;

                    trip = (struct trip_t*)vect_get(vt, y);
                    printf("%s", utf8_conv(trip->trip_id, (char*)alloca(256), 256));
                }
            } else {
                if (x == 0) {
                    struct stop_t* stop;
                    
                    stop = (struct stop_t*)vect_get(vt, y);
                    printf("%s", utf8_conv(stop->stop_name, (char*)alloca(256), 256));
                } else {
                    struct stop_time_t* st;
                    
                    st = (struct stop_time_t*)vect_get(vt, y);
                    printf("%s(%s)", st->departure_time, st->arrival_time);
                }
            }
            if (x < cols-1)
                printf(",");
            else
                printf("\n");
        }
    }
}

static void dump_route_trips(struct route_t* route, struct vector_t* trips_tbl)
{
    int count, i;
    struct vector_t** v_tbl;
    int rows = 0;

    count = vect_count(trips_tbl);
    v_tbl = (struct vector_t**)calloc(count+1, sizeof(struct vector_t*));

    for (i = 0; i < count; i++) {
        struct trip_t* trip;
        struct vector_t* stop_times_tbl;
        int stop_times_count, j;

        trip = (struct trip_t*)vect_get(trips_tbl, i);
        stop_times_tbl = (struct vector_t*)hash_get(g_vehicle_timetable, trip->trip_id);
        stop_times_count = vect_count(stop_times_tbl);
        v_tbl[i] = vect_initialize(stop_times_count);
        if (i == 0) {
            rows = stop_times_count + 1;
            vect_append(v_tbl[i], "");
            for (j = 0; j < stop_times_count; j++) {
                struct stop_time_t* st;
                struct stop_t* stop;
                    
                st = (struct stop_time_t*)vect_get(stop_times_tbl, j);
                stop = (struct stop_t*)hash_get(g_gtfs_hash->stops_htbl, st->stop_id);
                vect_append(v_tbl[i], stop);
            }
        } else {
            vect_append(v_tbl[i], trip);
            for (j = 0; j < stop_times_count; j++) {
                struct stop_time_t* st;

                st = (struct stop_time_t*)vect_get(stop_times_tbl, j);
                vect_append(v_tbl[i], st);
            }
        }
    }
    dump_stop_times(count, rows, v_tbl);

    for (i = 0; i < count; i++) {
        vect_finalize(v_tbl[i]);
    }
    free(v_tbl);
}

int gtfs_dump()
{
    int count, i;
    
    count = vect_count(g_gtfs->routes_tbl);
    for (i = 0; i < count; i++) {
        struct route_t* route;
        struct vector_t* trips_tbl;
        
        route = (struct route_t*)vect_get(g_gtfs->routes_tbl, i);
        printf("Route_id:%s,route_short_name[%s],route_long_name[%s]\n",
               utf8_conv(route->route_id, (char*)alloca(256), 256),
               utf8_conv(route->route_short_name, (char*)alloca(256), 256),
               utf8_conv(route->route_long_name, (char*)alloca(256), 256));
        trips_tbl = (struct vector_t*)hash_get(g_route_trips_htbl, route->route_id);
        dump_route_trips(route, trips_tbl);
    }
    return 0;
}
