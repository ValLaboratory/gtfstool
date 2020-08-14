/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * The MIT License
 *
 * Copyright (c) 2020 Val Laboratory Corporation.
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

static struct hash_t* _branch_routes_htbl = NULL;   // key:route_id value:(void*)枝番件数

static void copy_fare_rules_route(const char* route_id, const char* new_route_id)
{
    int count, i;
    
    count = vect_count(g_gtfs->fare_rules_tbl);
    for (i = 0; i < count; i++) {
        struct fare_rule_t* frule;
        
        frule = vect_get(g_gtfs->fare_rules_tbl, i);
        if (strcmp(frule->route_id, route_id) == 0) {
            struct fare_rule_t* bfrule;
            char hkey[128];

            bfrule = malloc(sizeof(struct fare_rule_t));
            memcpy(bfrule, frule, sizeof(struct fare_rule_t));
            // 新しいroute_idでfare_ruleを追加
            strncpy(bfrule->route_id, new_route_id, sizeof(bfrule->route_id));

            fare_rule_key(bfrule->route_id, bfrule->origin_id, bfrule->destination_id, hkey);
            hash_put(g_gtfs_hash->fare_rules_htbl, hkey, bfrule);

            vect_append(g_gtfs->fare_rules_tbl, bfrule);
        }
    }
}

static void update_trips_route(const char* route_id, const char* new_route_id, const char* trip_id)
{
    int count, i;

    count = vect_count(g_gtfs->trips_tbl);
    for (i = 0; i < count; i++) {
        struct trip_t* trip;

        trip = vect_get(g_gtfs->trips_tbl, i);
        if (strcmp(trip->route_id, route_id) == 0 && strcmp(trip->trip_id, trip_id) == 0)
            strncpy(trip->route_id, new_route_id, sizeof(trip->route_id));
    }
}

static void branch_route_id(const char* route_id, const char* trip_id)
{
    void* p;
    int64 cur_route_count = 0;
    char branch_route_id[256];
    struct route_t* route;

    p = hash_get(_branch_routes_htbl, route_id);
    if (p)
        cur_route_count = (int)p;
    cur_route_count++;
    snprintf(branch_route_id, sizeof(branch_route_id), "%s_%lld", route_id, cur_route_count);

    // routes.txtの更新
    route = hash_get(g_gtfs_hash->routes_htbl, route_id);
    if (route) {
        struct route_t* broute;
        
        broute = malloc(sizeof(struct route_t));
        memcpy(broute, route, sizeof(struct route_t));
        // 新しいroute_idでrouteを追加
        strncpy(broute->route_id, branch_route_id, sizeof(broute->route_id));
        hash_put(g_gtfs_hash->routes_htbl, branch_route_id, broute);
        vect_append(g_gtfs->routes_tbl, broute);
    }
    // trips.txtのroute_idの更新
    update_trips_route(route_id, branch_route_id, trip_id);

    // fare_rules.txtのroute_idをbranch_route_idで複写
    copy_fare_rules_route(route_id, branch_route_id);

    // 枝番件数を更新
    hash_put(_branch_routes_htbl, route_id, (void*)cur_route_count);
    g_branch_routes_count++;
}

static int check_route_stop_pattern()
{
    int result = 0;
    char** keylist;
    char** keys;        // route_id
    
    keylist = keys = hash_keylist(g_route_trips_htbl);
    while (*keys) {
        char* route_id;
        struct vector_t* trips_tbl;
        int count, i;
        int base_index = -1;
        struct vector_t* base_stop_time_tbl = NULL;
        int base_stops_count = 0;
        
        route_id = *keys;
        trips_tbl = (struct vector_t*)hash_get(g_route_trips_htbl, route_id);

        base_index = gtfs_trips_base_index(trips_tbl);
        if (base_index >= 0) {
            struct trip_t* trip;

            trip = (struct trip_t*)vect_get(trips_tbl, base_index);
            base_stop_time_tbl = (struct vector_t*)hash_get(g_vehicle_timetable, trip->trip_id);
            base_stops_count = vect_count(base_stop_time_tbl);
        }

        count = vect_count(trips_tbl);
        for (i = 0; i < count; i++) {
            struct trip_t* trip;
            struct vector_t* stop_time_tbl;
            int stop_count, j;
            
            trip = (struct trip_t*)vect_get(trips_tbl, i);
            stop_time_tbl = (struct vector_t*)hash_get(g_vehicle_timetable, trip->trip_id);
            stop_count = vect_count(stop_time_tbl);
            
            if (base_stops_count != stop_count) {
                // 新たなroute_idとして分割します。
                branch_route_id(route_id, trip->trip_id);
                continue;
            }

            // 停車パターンチェック
            for (j = 0; j < stop_count; j++) {
                struct stop_time_t* bst = NULL;
                struct stop_time_t* st = NULL;

                if (j < base_stops_count)
                    bst = (struct stop_time_t*)vect_get(base_stop_time_tbl, j);
                st = (struct stop_time_t*)vect_get(stop_time_tbl, j);
                if (equals_stop_times_stop_id(bst, st) == 0) {
                    // 新たなroute_idとして分割します。
                    branch_route_id(route_id, st->trip_id);
                    break;
                }
            }
        }
        keys++;
    }
    hash_list_free((void**)keylist);
    return result;
}

static char* path_filename(char* fname, const char* path)
{
    char buf[256];
    char **list;
    int n;
    
    *fname = '\0';
    strcpy(buf, path);
    list = split(buf, '/');
    n = list_count((const char**)list);
    if (n >= 0)
        strcpy(fname, list[n-1]);
    list_free(list);
    return fname;
}

static void output_gtfs()
{
    char zipname[MAX_PATH];
    
    gtfs_feed_writer(g_output_dir, g_gtfs);
    
    // GTFSファイルをzip形式でアーカイブ
    path_filename(zipname, g_gtfs_zip);
    gtfs_zip_archive_writer(g_output_dir, zipname, g_gtfs);
    gtfs_feed_delete(g_output_dir, g_gtfs);
}

int gtfs_route_branch()
{
    int count;

    g_branch_routes_count = 0;

    TRACE("%s\n", "*GTFS(zip)の読み込み*");
    if (gtfs_zip_archive_reader(g_gtfs_zip, g_gtfs) < 0) {
        err_write("gtfs_check: zip_archive_reader error (%s).\n", g_gtfs_zip);
        return -1;
    }

    TRACE("%s\n", "*キーの重複チェック（ハッシュ化）*");
    if (gtfs_hash_table_key_check() == GTFS_FATAL_ERROR)
        return -1;

    TRACE("%s\n", "*通過時刻表の作成*");
    gtfs_vehicle_timetable();

    TRACE("%s\n", "*経路の停車パターンを作成*");
    gtfs_route_trips();

    TRACE("%s\n", "*経路の停車パターンをチェック*");
    count = vect_count(g_gtfs->routes_tbl);
    _branch_routes_htbl = hash_initialize(count * 11);
    check_route_stop_pattern();
    hash_finalize(_branch_routes_htbl);

    TRACE("%s\n", "*GTFSの出力*");
    makedir(g_output_dir);
    output_gtfs();

    return 0;
}
