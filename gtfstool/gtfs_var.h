/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * The MIT License
 *
 * Copyright (c) 2018-2020 Val Laboratory Corporation.
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
#ifndef _GTFS_VAR_H
#define _GTFS_VAR_H

#include "gtfs_io.h"

// gtfs global variables

#ifndef _MAIN
extern
#endif
struct gtfs_t* g_gtfs;

#ifndef _MAIN
extern
#endif
struct gtfs_label_t g_gtfs_label;

#ifndef _MAIN
extern
#endif
struct feed_info_t g_feed_info;

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

#ifndef _MAIN
extern
#endif
char* g_proxy_server;

#ifndef _MAIN
extern
#endif
unsigned int g_proxy_port;

#endif /* _GTFS_VAR_H */
