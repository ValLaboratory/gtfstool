/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * The MIT License
 *
 * Copyright (c) 2021 Val Laboratory Corporation.
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

static void fare_list_line(struct route_t* route, struct vector_t* stop_times_tbl, int index)
{
    int n, i;
    struct stop_time_t* st;
    struct stop_t* stop;

    n = vect_count(stop_times_tbl);

    // route_id column
    printf(",");

    for (i = 0; i < index; i++) {
        printf(",");
    }

    st = (struct stop_time_t*)vect_get(stop_times_tbl, index);
    stop = (struct stop_t*)hash_get(g_gtfs_hash->stops_htbl, st->stop_id);
    printf("%s",
           utf8_conv(stop->stop_name, (char*)alloca(256), 256));
//           utf8_conv(stop->zone_id, (char*)alloca(256), 256));

    // fare
    for (i = index+1; i < n; i++) {
        struct stop_time_t* dest_st;
        struct stop_t* dest_stop;
        char hkey[128];
        struct fare_rule_t* frule;
        struct fare_attribute_t* fattr;

        printf(",");
        dest_st = (struct stop_time_t*)vect_get(stop_times_tbl, i);
        dest_stop = (struct stop_t*)hash_get(g_gtfs_hash->stops_htbl, dest_st->stop_id);
        fare_rule_key(route->route_id, stop->zone_id, dest_stop->zone_id, hkey);
        frule = hash_get(g_gtfs_hash->fare_rules_htbl, hkey);
        if (! frule) {
            // odを逆にして検索
            fare_rule_key(route->route_id, dest_stop->zone_id, stop->zone_id, hkey);
            frule = hash_get(g_gtfs_hash->fare_rules_htbl, hkey);
        }
        if (frule) {
            fattr = hash_get(g_gtfs_hash->fare_attrs_htbl, frule->fare_id);
            printf("%s", fattr->price);
        }
    }
    printf("\n");
}

static void fare_route_trips(struct route_t* route, struct vector_t* trips_tbl)
{
    int count, i;
    struct trip_t* trip;
    struct vector_t* stop_times_tbl;
    int stop_times_count;

    count = vect_count(trips_tbl);
    if (count == 0)
        return;

    // stopnames
    trip = (struct trip_t*)vect_get(trips_tbl, 0);
    stop_times_tbl = (struct vector_t*)hash_get(g_vehicle_timetable, trip->trip_id);
    stop_times_count = vect_count(stop_times_tbl);

    for (i = stop_times_count-1; i >= 0; i--) {
        fare_list_line(route, stop_times_tbl, i);
    }
}

int gtfs_fare()
{
    int count, i;
    
    TRACE("%s\n", "*GTFS(zip)の読み込み*");
    if (gtfs_zip_archive_reader(g_gtfs_zip, g_gtfs) < 0) {
        err_write("gtfs_check: zip_archive_reader error (%s).\n",
                  utf8_conv(g_gtfs_zip, (char*)alloca(256), 256));
        return -1;
    }

    TRACE("%s\n", "*キーの重複チェック（ハッシュ化）*");
    if (gtfs_hash_table_key_check() == GTFS_FATAL_ERROR)
        return -1;

    TRACE("%s\n", "*通過時刻表の作成*");
    gtfs_vehicle_timetable();

    TRACE("%s\n", "*経路の停車パターンを作成*");
    gtfs_route_trips();

    count = vect_count(g_gtfs->routes_tbl);
    for (i = 0; i < count; i++) {
        struct route_t* route;
        struct vector_t* trips_tbl;
        
        route = (struct route_t*)vect_get(g_gtfs->routes_tbl, i);
        if (i > 0)
            printf("\n");
        printf("route_id:%s\n[%s]\n[%s]\n",
               utf8_conv(route->route_id, (char*)alloca(256), 256),
               utf8_conv(route->route_short_name, (char*)alloca(256), 256),
               utf8_conv(route->route_long_name, (char*)alloca(256), 256));
        trips_tbl = (struct vector_t*)hash_get(g_route_trips_htbl, route->route_id);
        fare_route_trips(route, trips_tbl);
    }
    return 0;
}
