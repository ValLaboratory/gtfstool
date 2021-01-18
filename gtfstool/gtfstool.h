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
#ifndef gtfstool_h
#define gtfstool_h

#include <stdio.h>
#include "base/common.h"
#include "base/file.h"
#include "base/csvfile.h"
#include "gtfs_io.h"
#include "gtfs_var.h"

#ifndef _WIN32
#define MAX_PATH  PATH_MAX
#endif

#ifdef _MAIN
#define PROGRAM_NAME        "gtfstool"
#define PROGRAM_VERSION     "1.1"
#endif

#define GTFS_SUCCESS        0
#define GTFS_FATAL_ERROR    (-2)
#define GTFS_ERROR          (-1)
#define GTFS_WARNING        1

#define GTFS_CHECK_MODE         1
#define GTFS_SPLIT_MODE         2
#define GTFS_MERGE_MODE         3
#define GTFS_DUMP_MODE          4
#define GTFS_ROUTE_BRANCH_MODE  5
#define GTFS_DIFF_MODE          6
#define GTFS_FARE_MODE          7
#define GTFS_VERSION_MODE       9

struct merge_gtfs_prefix_t {
    char gtfs_file_name[MAX_PATH];
    char prefix[256];
};

// macros
#define TRACE(fmt, ...) \
if (g_trace_mode) { \
fprintf(stdout, fmt, __VA_ARGS__); \
}

#ifdef _WIN32
#define get_abspath(abs_path, path, maxlen) \
_fullpath(abs_path, path, maxlen)
#else
#define get_abspath(abs_path, path, maxlen) \
realpath(path, abs_path)
#endif

#ifndef _WIN32
#define stricmp strcasecmp
#define strnicmp strncasecmp
#endif

// global variables
#ifndef _MAIN
extern
#endif
const char* g_error_file;

#ifndef _MAIN
extern
#endif
int64 g_start_time;

#ifndef _MAIN
extern
#endif
struct agency_t g_agency;

#ifndef _MAIN
extern
#endif
struct agency_jp_t g_agency_jp;

/*
#ifndef _MAIN
extern
#endif
struct gtfs_t* g_gtfs;
*/

#ifndef _MAIN
extern
#endif
struct gtfs_hash_t* g_gtfs_hash;

/*
#ifndef _MAIN
extern
#endif
struct gtfs_label_t g_gtfs_label;

#ifndef _MAIN
extern
#endif
struct feed_info_t g_feed_info;
*/

#ifndef _MAIN
extern
#endif
int g_payment_method;

#ifndef _MAIN
extern
#endif
int g_exec_mode;

#ifndef _MAIN
extern
#endif
int g_trace_mode;

#ifndef _MAIN
extern
#endif
char g_gtfs_zip[256];

#ifndef _MAIN
extern
#endif
struct hash_t* g_vehicle_timetable;     // key:trip_id value:pointer of vector(struct stop_time_t)

#ifndef _MAIN
extern
#endif
struct hash_t* g_route_trips_htbl;     // ket:route_id value:pointer of vector(struct trip_t)

#ifndef _MAIN
extern
#endif
long g_error_count;

#ifndef _MAIN
extern
#endif
long g_warning_count;

#ifndef _MAIN
extern
#endif
int g_ignore_warning;

#ifndef _MAIN
extern
#endif
int g_calendar_dates_service_id_check;

#ifndef _MAIN
extern
#endif
int g_route_stop_pattern_valid;

#ifndef _MAIN
extern
#endif
const char* g_output_dir;

/*
#ifdef _MAIN
const char* g_gtfs_filename[] = {
    "agency.txt",
    "stops.txt",
    "routes.txt",
    "trips.txt",
    "stop_times.txt",
    "calendar.txt",
    "calendar_dates.txt",
    "fare_attributes.txt",
    "fare_rules.txt",
    "shapes.txt",
    "frequencies.txt",
    "transfers.txt",
    "feed_info.txt",
    "translations.txt",
    "agency_jp.txt",
    "routes_jp.txt",
    "office_jp.txt",
    NULL
};
#else
extern const char* g_gtfs_filename[];
#endif

#ifdef _MAIN
const unsigned int g_gtfs_filemap[] = {
    GTFS_FILE_AGENCY,
    GTFS_FILE_STOPS,
    GTFS_FILE_ROUTES,
    GTFS_FILE_TRIPS,
    GTFS_FILE_STOP_TIMES,
    GTFS_FILE_CALENDAR,
    GTFS_FILE_CALENDAR_DATES,
    GTFS_FILE_FARE_ATTRIBUTES,
    GTFS_FILE_FARE_RULES,
    GTFS_FILE_SHAPES,
    GTFS_FILE_FREQUENCIES,
    GTFS_FILE_TRANSFERS,
    GTFS_FILE_FEED_INFO,
    GTFS_FILE_TRANSLATIONS,
    GTFS_FILE_AGENCY_JP,
    GTFS_FILE_ROUTES_JP,
    GTFS_FILE_OFFICE_JP,
};
#else
extern const unsigned int g_gtfs_filemap[];
#endif
*/

#ifndef _MAIN
extern
#endif
const char* g_config_file;

#ifndef _MAIN
extern
#endif
struct agency_t g_merged_agency;

#ifndef _MAIN
extern
#endif
struct agency_jp_t g_merged_agency_jp;

#ifndef _MAIN
extern
#endif
struct feed_info_t g_merged_feed_info;

#ifndef _MAIN
extern
#endif
char g_merged_output_dir[MAX_PATH];

#ifndef _MAIN
extern
#endif
char g_merged_gtfs_name[MAX_PATH];

#ifndef _MAIN
extern
#endif
struct vector_t* g_merge_gtfs_tbl;

#ifndef _MAIN
extern
#endif
int g_is_free_bus;

/*
#ifndef _MAIN
extern
#endif
char* g_proxy_server;

#ifndef _MAIN
extern
#endif
unsigned int g_proxy_port;
*/

#ifndef _MAIN
extern
#endif
int g_branch_routes_count;

#ifndef _MAIN
extern
#endif
int g_same_stops_fare_rule_check;

// prototypes
#ifdef __cplusplus
extern "C" {
#endif

// main.c
struct gtfs_t* gtfs_alloc(void);
void gtfs_free(struct gtfs_t* gtfs, int is_element_free);
struct gtfs_hash_t* gtfs_hash_alloc(void);
void gtfs_hash_free(struct gtfs_hash_t* gtfs_hash);

// merge_config.c
int merge_config(const char* conf_fname);

// gtfstool.c
int gtfs_fatal_error(const char* fmt, ...);
int gtfs_error(const char* fmt, ...);
int gtfs_warning(const char* fmt, ...);
int is_gtfs_file_exist(struct gtfs_t* gtfs, unsigned int file_kind);
char* utf8_conv(const char* str, char* enc_buf, int enc_bufsize);

// gtfs_check.c
char* fare_rule_key(const char* route_id, const char* origin_id, const char* dest_id, char* key);
int gtfs_hash_table_key_check(void);
int gtfs_vehicle_timetable(void);
int gtfs_route_trips(void);
int gtfs_trips_base_index(struct vector_t* trips_tbl);
int equals_stop_times_stop_id(struct stop_time_t* bst, struct stop_time_t* st);
int gtfs_check(void);

// gtfs_split.c
int gtfs_split(void);

// gtfs_merge.c
int gtfs_merge(void);

// gtfs_dump.c
int gtfs_dump(void);

// gtfs_fare.c
int gtfs_fare(void);

// gtfs_route_branch.c
int gtfs_route_branch(void);

// gtfs_diff.c
int gtfs_diff(const char* diff_zip);

#ifdef __cplusplus
}
#endif

#endif /* gtfstool_h */
