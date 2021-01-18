/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * The MIT License
 *
 * Copyright (c) 2018-2021 Val Laboratory Corporation.
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
#define _MAIN
#include "gtfstool.h"
#include "gtfs_var.h"

static void version()
{
    fprintf(stdout, "%s version %s\n", PROGRAM_NAME, PROGRAM_VERSION);
    fprintf(stdout, "Copyright (c) 2018-2021 Val Laboratory Corporation.\n");
}

static void usage()
{
    version();
    fprintf(stdout, "\n使い方: %s [action] [options]  gtfs.zip ...\n", PROGRAM_NAME);
    fprintf(stdout, "action:  [-c] GTFS-JPの整合性チェックを行います(default)\n");
    fprintf(stdout, "         [-d] GTFS-JPのルート別にバス時刻表を表示します\n");
    fprintf(stdout, "         [-u] GTFS-JPのルート別の運賃三角表を表示します\n");
    fprintf(stdout, "         [-s output_dir] 複数のagencyを分割します\n");
    fprintf(stdout, "         [-m merge.conf] 複数のGTFS-JPを一つにマージします\n");
    fprintf(stdout, "         [-b output_dir] 停車パターンが違うroute_idを複数に分割します\n");
    fprintf(stdout, "         [-f output_dir] 2つのGTFS-JPの内容を比較して差分を抽出します\n");
    fprintf(stdout, "         [-v] プログラムバージョンを表示します\n");
    fprintf(stdout, "options: [-w] 整合性チェック時の警告を無視します\n");
    fprintf(stdout, "         [-i] チェック時にcalendar_dates.txtのservice_idがcalender.txtに\n"
                    "              存在するかチェックします\n");
    fprintf(stdout, "         [-r] 同一経路で停車パターンが違う場合にエラーとしません(Ver.2仕様)\n");
    fprintf(stdout, "         [-a] 発着が同じバス停名でも運賃区間が登録されているかチェックします\n");
    fprintf(stdout, "         [-p proxy_server:port] プロキシサーバとポート番号を指定します\n");
    fprintf(stdout, "         [-e error_file] システムエラーを出力するファイルを指定します\n");
    fprintf(stdout, "         [-t] トレースモードをオンにして実行します\n");
}

static int startup()
{
    /* マルチスレッド対応関数の初期化 */
    mt_initialize();
    /* ソケット関数の初期化 */
    sock_initialize();

    /* エラーファイルの初期化 */
    err_initialize(g_error_file);
    return 0;
}

static void cleanup()
{
    err_finalize();
    sock_finalize();
    mt_finalize();
}

struct gtfs_t* gtfs_alloc()
{
    struct gtfs_t* gtfs;

    gtfs = calloc(1, sizeof(struct gtfs_t));
    gtfs->agency_tbl = vect_initialize(5);
    gtfs->agency_jp_tbl = vect_initialize(5);
    gtfs->routes_tbl = vect_initialize(300);
    gtfs->stops_tbl = vect_initialize(1000);
    gtfs->fare_rules_tbl = vect_initialize(10000);
    gtfs->fare_attrs_tbl = vect_initialize(1000);
    gtfs->trips_tbl = vect_initialize(1000);
    gtfs->stop_times_tbl = vect_initialize(10000);
    gtfs->calendar_tbl = vect_initialize(20);
    gtfs->calendar_dates_tbl = vect_initialize(100);
    gtfs->translations_tbl = vect_initialize(1000);
    gtfs->shapes_tbl = vect_initialize(10000);
    gtfs->frequencies_tbl = vect_initialize(100);
    gtfs->transfers_tbl = vect_initialize(100);
    gtfs->routes_jp_tbl = vect_initialize(300);
    gtfs->office_jp_tbl = vect_initialize(20);
    return gtfs;
}

struct gtfs_hash_t* gtfs_hash_alloc()
{
    struct gtfs_hash_t* gtfs_hash;

    gtfs_hash = calloc(1, sizeof(struct gtfs_hash_t));
    gtfs_hash->agency_htbl = hash_initialize(17);
    gtfs_hash->routes_htbl = hash_initialize(307);
    gtfs_hash->stops_htbl = hash_initialize(1009);
    gtfs_hash->trips_htbl = hash_initialize(1009);
    gtfs_hash->calendar_htbl = hash_initialize(41);
    gtfs_hash->calendar_dates_htbl = hash_initialize(211);
    gtfs_hash->fare_attrs_htbl = hash_initialize(1009);
    gtfs_hash->fare_rules_htbl = hash_initialize(999983);
    gtfs_hash->translations_htbl = hash_initialize(1009);
    gtfs_hash->routes_jp_htbl = hash_initialize(307);
    return gtfs_hash;
}

static void init_gtfs()
{
    g_gtfs = gtfs_alloc();
    g_gtfs_hash = gtfs_hash_alloc();

    g_vehicle_timetable = hash_initialize(1009);
    g_route_trips_htbl = hash_initialize(1009);

    g_error_count = 0;
    g_warning_count = 0;
}

static void vector_elements_free(struct vector_t* vect)
{
    int i, rc;
    
    rc = vect_count(vect);
    for (i = 0; i < rc; i++) {
        void* ptr;
        
        ptr = vect_get(vect, i);
        if (ptr)
            free(ptr);
    }
}

static void hash_elements_free(struct hash_t* hash)
{
    void** ptbl = hash_list(hash);
    if (ptbl) {
        int i = 0;
        
        while (ptbl[i]) {
            vect_finalize(ptbl[i++]);
        }
        hash_list_free(ptbl);
    }
}

void gtfs_free(struct gtfs_t* gtfs, int is_element_free)
{
    if (gtfs->agency_tbl) {
        if (is_element_free)
            vector_elements_free(gtfs->agency_tbl);
        vect_finalize(gtfs->agency_tbl);
    }
    if (gtfs->agency_jp_tbl) {
        if (is_element_free)
            vector_elements_free(gtfs->agency_jp_tbl);
        vect_finalize(gtfs->agency_jp_tbl);
    }
    if (gtfs->routes_tbl) {
        if (is_element_free)
            vector_elements_free(gtfs->routes_tbl);
        vect_finalize(gtfs->routes_tbl);
    }
    if (gtfs->stops_tbl) {
        if (is_element_free)
            vector_elements_free(gtfs->stops_tbl);
        vect_finalize(gtfs->stops_tbl);
    }
    if (gtfs->fare_rules_tbl) {
        if (is_element_free)
            vector_elements_free(gtfs->fare_rules_tbl);
        vect_finalize(gtfs->fare_rules_tbl);
    }
    if (gtfs->fare_attrs_tbl) {
        if (is_element_free)
            vector_elements_free(gtfs->fare_attrs_tbl);
        vect_finalize(gtfs->fare_attrs_tbl);
    }
    if (gtfs->trips_tbl) {
        if (is_element_free)
            vector_elements_free(gtfs->trips_tbl);
        vect_finalize(gtfs->trips_tbl);
    }
    if (gtfs->stop_times_tbl) {
        if (is_element_free)
            vector_elements_free(gtfs->stop_times_tbl);
        vect_finalize(gtfs->stop_times_tbl);
    }
    if (gtfs->calendar_tbl) {
        if (is_element_free)
            vector_elements_free(gtfs->calendar_tbl);
        vect_finalize(gtfs->calendar_tbl);
    }
    if (gtfs->calendar_dates_tbl) {
        if (is_element_free)
            vector_elements_free(gtfs->calendar_dates_tbl);
        vect_finalize(gtfs->calendar_dates_tbl);
    }
    if (gtfs->translations_tbl) {
        if (is_element_free)
            vector_elements_free(gtfs->translations_tbl);
        vect_finalize(gtfs->translations_tbl);
    }
    if (gtfs->shapes_tbl) {
        if (is_element_free)
            vector_elements_free(gtfs->shapes_tbl);
        vect_finalize(gtfs->shapes_tbl);
    }
    if (gtfs->frequencies_tbl) {
        if (is_element_free)
            vector_elements_free(gtfs->frequencies_tbl);
        vect_finalize(gtfs->frequencies_tbl);
    }
    if (gtfs->transfers_tbl) {
        if (is_element_free)
            vector_elements_free(gtfs->transfers_tbl);
        vect_finalize(gtfs->transfers_tbl);
    }
    if (gtfs->routes_jp_tbl) {
        if (is_element_free)
            vector_elements_free(gtfs->routes_jp_tbl);
        vect_finalize(gtfs->routes_jp_tbl);
    }
    if (gtfs->office_jp_tbl) {
        if (is_element_free)
            vector_elements_free(gtfs->office_jp_tbl);
        vect_finalize(gtfs->office_jp_tbl);
    }
    free(gtfs);
}

void gtfs_hash_free(struct gtfs_hash_t* gtfs_hash)
{
    if (gtfs_hash->agency_htbl)
        hash_finalize(gtfs_hash->agency_htbl);

    if (gtfs_hash->routes_htbl)
        hash_finalize(gtfs_hash->routes_htbl);

    if (gtfs_hash->stops_htbl)
        hash_finalize(gtfs_hash->stops_htbl);

    if (gtfs_hash->fare_rules_htbl)
        hash_finalize(gtfs_hash->fare_rules_htbl);

    if (gtfs_hash->fare_attrs_htbl)
        hash_finalize(gtfs_hash->fare_attrs_htbl);

    if (gtfs_hash->trips_htbl)
        hash_finalize(gtfs_hash->trips_htbl);

    if (gtfs_hash->calendar_htbl)
        hash_finalize(gtfs_hash->calendar_htbl);

    if (gtfs_hash->calendar_dates_htbl)
        hash_finalize(gtfs_hash->calendar_dates_htbl);

    if (gtfs_hash->translations_htbl)
        hash_finalize(gtfs_hash->translations_htbl);

    if (gtfs_hash->routes_jp_htbl)
        hash_finalize(gtfs_hash->routes_jp_htbl);

    free(gtfs_hash);
}

static void final_gtfs()
{
    if (g_vehicle_timetable) {
        hash_elements_free(g_vehicle_timetable);
        hash_finalize(g_vehicle_timetable);
    }
    if (g_route_trips_htbl) {
        hash_elements_free(g_route_trips_htbl);
        hash_finalize(g_route_trips_htbl);
    }

    if (g_gtfs_hash)
        gtfs_hash_free(g_gtfs_hash);

    if (g_gtfs)
        gtfs_free(g_gtfs, 1);
}

static int args(int argc, const char * argv[])
{
    int i;

    if (argc < 2) {
        usage();
        return 1;
    }

    g_exec_mode = GTFS_CHECK_MODE;
    g_ignore_warning = 0;
    g_same_stops_fare_rule_check = 0;

    for (i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (strcmp(argv[i], "-c") == 0) {
                g_exec_mode = GTFS_CHECK_MODE;
            } else if (strcmp(argv[i], "-t") == 0) {
                g_trace_mode = 1;
            } else if (strcmp(argv[i], "-e") == 0) {
                if (i < argc-1)
                    g_error_file = argv[++i];
            } else if (strcmp(argv[i], "-p") == 0) {
                if (i < argc-1) {
                    int index;

                    g_proxy_server = (char*)argv[++i];
                    index = indexof(g_proxy_server, ':');
                    if (index >= 0) {
                        g_proxy_port = atoi(&g_proxy_server[index+1]);
                        g_proxy_server[index] = '\0';
                    }
                }
            } else if (strcmp(argv[i], "-w") == 0) {
                g_ignore_warning = 1;
            } else if (strcmp(argv[i], "-i") == 0) {
                g_calendar_dates_service_id_check = 1;
            } else if (strcmp(argv[i], "-r") == 0) {
                g_route_stop_pattern_valid = 1;
            } else if (strcmp(argv[i], "-a") == 0) {
                g_same_stops_fare_rule_check = 1;
            } else if (strcmp(argv[i], "-s") == 0) {
                if (i < argc-1) {
                    g_output_dir = argv[++i];
                    g_exec_mode = GTFS_SPLIT_MODE;
                } else {
                    usage();
                    return 1;
                }
            } else if (strcmp(argv[i], "-m") == 0) {
                if (i < argc-1) {
                    g_config_file = argv[++i];
                    g_exec_mode = GTFS_MERGE_MODE;
                } else {
                    usage();
                    return 1;
                }
            } else if (strcmp(argv[i], "-d") == 0) {
                g_exec_mode = GTFS_DUMP_MODE;
            } else if (strcmp(argv[i], "-u") == 0) {
                g_exec_mode = GTFS_FARE_MODE;
            } else if (strcmp(argv[i], "-b") == 0) {
                if (i < argc-1) {
                    g_output_dir = argv[++i];
                    g_exec_mode = GTFS_ROUTE_BRANCH_MODE;
                } else {
                    usage();
                    return 1;
                }
            } else if (strcmp(argv[i], "-f") == 0) {
                    if (i < argc-1) {
                        g_output_dir = argv[++i];
                        g_exec_mode = GTFS_DIFF_MODE;
                    } else {
                        usage();
                        return 1;
                    }
            } else if (strcmp(argv[i], "-v") == 0) {
                g_exec_mode = GTFS_VERSION_MODE;
            } else {
                usage();
                return 1;
            }
        }
    }
    return 0;
}

static void start_print()
{
    char datebuf[256];
    
    TRACE("%s", "-------------------- START --------------------\n");
    TRACE("%s\n", now_jststr(datebuf, sizeof(datebuf)));
    if (strlen(g_gtfs_zip) > 0)
        printf("GTFS(JP): %s\n\n", g_gtfs_zip);
}

static void process_time_print()
{
    int64 lap_time_usec;
    long lap_time_ss;

    lap_time_usec = system_time() - g_start_time;
    lap_time_ss = (long)(lap_time_usec / 1000000L);
    if (lap_time_ss > 0) {
        int time_hh, time_mm, time_ss;

        time_hh = (int)(lap_time_ss / 3600);
        time_mm = (int)((lap_time_ss - (time_hh * 3600)) / 60);
        time_ss = (int)((lap_time_ss - (time_hh * 3600 + time_mm * 60)) % 60);
        printf("Processing time: %d:%d:%d\n", time_hh, time_mm, time_ss);
    } else {
        printf("Processing time: %ld(msec)\n", (long)(lap_time_usec/1000L));
    }
}

static void check_statistics_print()
{
    int count, i;

    printf("\n");
    printf("Agency: ");
    count = vect_count(g_gtfs->agency_tbl);
    for (i = 0; i < count; i++) {
        struct agency_t* agency;
    
        agency = vect_get(g_gtfs->agency_tbl, i);
        if (i > 0)
            printf(", ");
        printf("%s", utf8_conv(agency->agency_name, (char*)alloca(256), 256));
    }
    printf("\n");
    printf("Feed-Publisher: %s\n", utf8_conv(g_feed_info.feed_publisher_name, (char*)alloca(256), 256));

    printf("agency.txt: %d\n", vect_count(g_gtfs->agency_tbl));
    printf("agency_jp.txt: %d\n", vect_count(g_gtfs->agency_jp_tbl));
    printf("stops.txt: %d\n", vect_count(g_gtfs->stops_tbl));
    printf("routes.txt: %d\n", vect_count(g_gtfs->routes_tbl));
    printf("routes_jp.txt: %d\n", vect_count(g_gtfs->routes_jp_tbl));
    printf("trips.txt: %d\n", vect_count(g_gtfs->trips_tbl));
    printf("office_jp.txt: %d\n", vect_count(g_gtfs->office_jp_tbl));
    printf("stop_times.txt: %d\n", vect_count(g_gtfs->stop_times_tbl));
    printf("calendar.txt: %d\n", vect_count(g_gtfs->calendar_tbl));
    printf("calendar_dates.txt: %d\n", vect_count(g_gtfs->calendar_dates_tbl));
    printf("fare_attributes.txt: %d\n", vect_count(g_gtfs->fare_attrs_tbl));
    printf("fare_rules.txt: %d\n", vect_count(g_gtfs->fare_rules_tbl));
    printf("shapes.txt: %d\n", vect_count(g_gtfs->shapes_tbl));
    printf("frequencies.txt: %d\n", vect_count(g_gtfs->frequencies_tbl));
    printf("transfers.txt: %d\n", vect_count(g_gtfs->transfers_tbl));
    printf("translations.txt: %d\n", vect_count(g_gtfs->translations_tbl));
    printf("\n");

    if (g_ignore_warning)
        printf("Error: %ld\n", g_error_count);
    else
        printf("Error: %ld, Warning: %ld\n", g_error_count, g_warning_count);

    process_time_print();
    TRACE("%s", "-------------------- END --------------------\n\n");
}

static void statistics_print()
{
    process_time_print();
    TRACE("%s", "-------------------- END --------------------\n\n");
}

static void merge_statistics_print()
{
    int count, i;

    printf("merged gtfs(jp): \n");
    count = vect_count(g_merge_gtfs_tbl);
    for (i = 0; i < count; i++) {
        struct merge_gtfs_prefix_t* m;
            
        m = (struct merge_gtfs_prefix_t*)vect_get(g_merge_gtfs_tbl, i);
        printf("  (%d) %s\n", i+1, m->gtfs_file_name);
    }
    printf("total %d gtfs(jp).\n", count);
    printf("output: %s\n", g_merged_gtfs_name);
    process_time_print();
    TRACE("%s", "-------------------- END --------------------\n\n");
}

static void branch_routes_statistics_print()
{
    printf("gtfs(jp): %s\n", g_gtfs_zip);
    printf("branch routes count: %d\n", g_branch_routes_count);
    process_time_print();
    TRACE("%s", "-------------------- END --------------------\n\n");
}

static int merge_mode()
{
    TRACE("%s\n", "*GTFS MERGE START*");
    g_merge_gtfs_tbl = vect_initialize(20);
    if (merge_config(g_config_file) < 0)
        return 1;
    g_start_time = system_time();
    start_print();
    if (gtfs_merge() == 0)
        merge_statistics_print();
    vector_elements_free(g_merge_gtfs_tbl);
    vect_finalize(g_merge_gtfs_tbl);
    return 0;
}

static int is_skip_argument(const char* argv)
{
    if (strcmp(argv, "-s") == 0 || strcmp(argv, "-m") == 0 ||
        strcmp(argv, "-e") == 0 || strcmp(argv, "-p") == 0 ||
        strcmp(argv, "-b") == 0 || strcmp(argv, "-f") == 0)
        return 1;
    return 0;
}

static void dump_mode(int argc, const char* argv[])
{
    int i;
    
    TRACE("%s\n", "*GTFS DUMP START*");
    g_start_time = system_time();

    for (i = 1; i < argc; i++) {
        if (*argv[i] == '-') {
            if (is_skip_argument(argv[i]))
                i++;    // skip argument
            continue;
        }
        
        init_gtfs();
        strcpy(g_gtfs_zip, argv[i]);
        gtfs_dump();
        final_gtfs();
    }
}

static void fare_mode(int argc, const char* argv[])
{
    int i;
    
    TRACE("%s\n", "*GTFS FARE START*");
    g_start_time = system_time();

    for (i = 1; i < argc; i++) {
        if (*argv[i] == '-') {
            if (is_skip_argument(argv[i]))
                i++;    // skip argument
            continue;
        }
        
        init_gtfs();
        strcpy(g_gtfs_zip, argv[i]);
        gtfs_fare();
        final_gtfs();
    }
}

static void route_branch_mode(int argc, const char* argv[])
{
    int i;
    
    TRACE("%s\n", "*GTFS ROUTE BRANCH MODE START*");
    g_start_time = system_time();
    
    for (i = 1; i < argc; i++) {
        if (*argv[i] == '-') {
            if (is_skip_argument(argv[i]))
                i++;    // skip argument
            continue;
        }
        
        init_gtfs();
        strcpy(g_gtfs_zip, argv[i]);
        g_ignore_warning = 1;
        if (gtfs_route_branch() == 0)
            branch_routes_statistics_print();
        final_gtfs();
    }
}

static void check_split_mode(int argc, const char* argv[])
{
    int i;
    
    for (i = 1; i < argc; i++) {
        if (*argv[i] == '-') {
            if (is_skip_argument(argv[i]))
                i++;    // skip argument
            continue;
        }
        
        init_gtfs();
        strcpy(g_gtfs_zip, argv[i]);
        g_start_time = system_time();
        start_print();
        
        if (g_exec_mode == GTFS_CHECK_MODE) {
            TRACE("%s\n", "*GTFS CHECK START*");
            if (gtfs_check() == 0)
                check_statistics_print();
        } else if (g_exec_mode == GTFS_SPLIT_MODE) {
            TRACE("%s\n", "*GTFS SPLIT START*");
            if (gtfs_split() == 0)
                statistics_print();
        }
        final_gtfs();
    }
}

static void diff_mode(int argc, const char* argv[])
{
    int i;
    const char* diff_zip;
    
    TRACE("%s\n", "*GTFS DIFF START*");
    g_start_time = system_time();

    for (i = 1; i < argc; i++) {
        if (*argv[i] == '-') {
            if (is_skip_argument(argv[i]))
                i++;    // skip argument
            continue;
        }
        
        init_gtfs();
        strcpy(g_gtfs_zip, argv[i]);
        i++;
        if (i < argc)
            diff_zip = argv[i];

        if (gtfs_diff(diff_zip) == 0)
            ;
        final_gtfs();
    }
}

int main(int argc, const char * argv[])
{
    args(argc, argv);
    if (startup() < 0)
        return 1;

    if (g_exec_mode == GTFS_MERGE_MODE) {
        if (merge_mode() != 0)
            return 1;
    } else if (g_exec_mode == GTFS_DUMP_MODE) {
        dump_mode(argc, argv);
    } else if (g_exec_mode == GTFS_FARE_MODE) {
        fare_mode(argc, argv);
    } else if (g_exec_mode == GTFS_ROUTE_BRANCH_MODE) {
        route_branch_mode(argc, argv);
    } else if (g_exec_mode == GTFS_VERSION_MODE) {
        version();
    } else if (g_exec_mode == GTFS_CHECK_MODE || g_exec_mode == GTFS_SPLIT_MODE) {
        check_split_mode(argc, argv);
    } else if (g_exec_mode == GTFS_DIFF_MODE) {
        diff_mode(argc, argv);
    }
    cleanup();

#ifdef _WIN32
    _CrtDumpMemoryLeaks();
#endif
    return 0;
}
