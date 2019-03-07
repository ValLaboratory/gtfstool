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

static struct gtfs_t* _ext_gtfs;    // splitで抽出されたGTFS

static void extract_fare_rules(struct hash_t* fare_id_htbl)
{
    int count, i;
    
    count = vect_count(g_gtfs->fare_rules_tbl);
    for (i = 0; i < count; i++) {
        struct fare_rule_t* frule;
        
        frule = (struct fare_rule_t*)vect_get(g_gtfs->fare_rules_tbl, i);
        if (hash_get(fare_id_htbl, frule->fare_id))
            vect_append(_ext_gtfs->fare_rules_tbl, frule);
    }

    if (vect_count(_ext_gtfs->fare_rules_tbl) > 0)
        _ext_gtfs->file_exist_bits |= GTFS_FILE_FARE_RULES;
}

static void extract_fare_attributes(const char* agency_id)
{
    int count, i;
    struct hash_t* fare_id_htbl;

    count = vect_count(g_gtfs->fare_attrs_tbl);
    fare_id_htbl = hash_initialize(count*2+1);

    for (i = 0; i < count; i++) {
        struct fare_attribute_t* fattr;
        
        fattr = (struct fare_attribute_t*)vect_get(g_gtfs->fare_attrs_tbl, i);
        if (strlen(fattr->agency_id) < 1) {
            err_write("複数のagency(事業者)の場合はfare_attributes.txtのagency_idは必須になります。\n", g_gtfs_zip);
            break;
        }
        if (strcmp(fattr->agency_id, agency_id) == 0) {
            vect_append(_ext_gtfs->fare_attrs_tbl, fattr);
            if (! hash_get(fare_id_htbl, fattr->fare_id))
                hash_put(fare_id_htbl, fattr->fare_id, fattr);
        }
    }

    extract_fare_rules(fare_id_htbl);
    hash_finalize(fare_id_htbl);

    if (vect_count(_ext_gtfs->fare_attrs_tbl) > 0)
        _ext_gtfs->file_exist_bits |= GTFS_FILE_FARE_ATTRIBUTES;
}

static void extract_parent_stops(struct hash_t* parent_id_htbl)
{
    int count, i;
    
    count = vect_count(g_gtfs->stops_tbl);
    
    for (i = 0; i < count; i++) {
        struct stop_t* stop;
        
        stop = (struct stop_t*)vect_get(g_gtfs->stops_tbl, i);
        if (hash_get(parent_id_htbl, stop->stop_id))
            vect_append(_ext_gtfs->stops_tbl, stop);
    }

    if (vect_count(_ext_gtfs->stops_tbl) > 0)
        _ext_gtfs->file_exist_bits |= GTFS_FILE_STOPS;
}

static void extract_stops(struct hash_t* stop_id_htbl)
{
    int count, i;
    struct hash_t* parent_htbl;
    
    count = vect_count(g_gtfs->stops_tbl);
    parent_htbl = hash_initialize(count+1);

    for (i = 0; i < count; i++) {
        struct stop_t* stop;
        
        stop = (struct stop_t*)vect_get(g_gtfs->stops_tbl, i);
        if (hash_get(stop_id_htbl, stop->stop_id)) {
            vect_append(_ext_gtfs->stops_tbl, stop);
            if (atoi(stop->location_type) == 0 && strlen(stop->parent_station) > 0) {
                // 標柱で親停留所が設定されている場合
                if (! hash_get(parent_htbl, stop->parent_station))
                    hash_put(parent_htbl, stop->parent_station, stop);
            }
        }
    }

    extract_parent_stops(parent_htbl);
    hash_finalize(parent_htbl);

    if (vect_count(_ext_gtfs->stops_tbl) > 0)
        _ext_gtfs->file_exist_bits |= GTFS_FILE_STOPS;
}

static void extract_transfers(struct hash_t* stop_id_htbl)
{
    int count, i;
    
    count = vect_count(g_gtfs->transfers_tbl);
    
    for (i = 0; i < count; i++) {
        struct transfer_t* tr;
        
        tr = (struct transfer_t*)vect_get(g_gtfs->transfers_tbl, i);
        if (hash_get(stop_id_htbl, tr->from_stop_id) && hash_get(stop_id_htbl, tr->to_stop_id))
            vect_append(_ext_gtfs->transfers_tbl, tr);
    }
    
    if (vect_count(_ext_gtfs->stops_tbl) > 0)
        _ext_gtfs->file_exist_bits |= GTFS_FILE_TRANSFERS;
}

static void extract_stop_times(struct hash_t* trip_id_htbl)
{
    int count, i;
    struct hash_t* stop_id_htbl;
    
    count = vect_count(g_gtfs->stop_times_tbl);
    stop_id_htbl = hash_initialize(count+1);
    
    for (i = 0; i < count; i++) {
        struct stop_time_t* st;
        
        st = (struct stop_time_t*)vect_get(g_gtfs->stop_times_tbl, i);
        if (hash_get(trip_id_htbl, st->trip_id)) {
            vect_append(_ext_gtfs->stop_times_tbl, st);
            if (! hash_get(stop_id_htbl, st->stop_id))
                hash_put(stop_id_htbl, st->stop_id, st);
        }
    }
    
    extract_stops(stop_id_htbl);
    extract_transfers(stop_id_htbl);
    hash_finalize(stop_id_htbl);
    
    if (vect_count(_ext_gtfs->stop_times_tbl) > 0)
        _ext_gtfs->file_exist_bits |= GTFS_FILE_STOP_TIMES;
}

static void extract_calendar(struct hash_t* service_id_htbl)
{
    int count, i;
    
    count = vect_count(g_gtfs->calendar_tbl);
    for (i = 0; i < count; i++) {
        struct calendar_t* cal;
        
        cal = (struct calendar_t*)vect_get(g_gtfs->calendar_tbl, i);
        if (hash_get(service_id_htbl, cal->service_id))
            vect_append(_ext_gtfs->calendar_tbl, cal);
    }
    
    if (vect_count(_ext_gtfs->calendar_tbl) > 0)
        _ext_gtfs->file_exist_bits |= GTFS_FILE_CALENDAR;
}

static void extract_calendar_dates(struct hash_t* service_id_htbl)
{
    int count, i;
    
    count = vect_count(g_gtfs->calendar_dates_tbl);
    for (i = 0; i < count; i++) {
        struct calendar_date_t* cdate;
        
        cdate = (struct calendar_date_t*)vect_get(g_gtfs->calendar_dates_tbl, i);
        if (hash_get(service_id_htbl, cdate->service_id))
            vect_append(_ext_gtfs->calendar_dates_tbl, cdate);
    }
    
    if (vect_count(_ext_gtfs->calendar_dates_tbl) > 0)
        _ext_gtfs->file_exist_bits |= GTFS_FILE_CALENDAR_DATES;
}

static void extract_shapes(struct hash_t* shape_id_htbl)
{
    int count, i;
    
    count = vect_count(g_gtfs->shapes_tbl);
    for (i = 0; i < count; i++) {
        struct shape_t* shape;
        
        shape = (struct shape_t*)vect_get(g_gtfs->shapes_tbl, i);
        if (hash_get(shape_id_htbl, shape->shape_id))
            vect_append(_ext_gtfs->shapes_tbl, shape);
    }
    
    if (vect_count(_ext_gtfs->shapes_tbl) > 0)
        _ext_gtfs->file_exist_bits |= GTFS_FILE_SHAPES;
}

static void extract_office_jp(struct hash_t* office_id_htbl)
{
    int count, i;
    
    count = vect_count(g_gtfs->office_jp_tbl);
    for (i = 0; i < count; i++) {
        struct office_jp_t* ojp;
        
        ojp = (struct office_jp_t*)vect_get(g_gtfs->office_jp_tbl, i);
        if (hash_get(office_id_htbl, ojp->office_id))
            vect_append(_ext_gtfs->office_jp_tbl, ojp);
    }

    if (vect_count(_ext_gtfs->office_jp_tbl) > 0)
        _ext_gtfs->file_exist_bits |= GTFS_FILE_OFFICE_JP;
}

static void extract_frequencies(struct hash_t* trip_id_htbl)
{
    int count, i;
    
    count = vect_count(g_gtfs->frequencies_tbl);
    for (i = 0; i < count; i++) {
        struct frequency_t* freq;
        
        freq = (struct frequency_t*)vect_get(g_gtfs->frequencies_tbl, i);
        if (hash_get(trip_id_htbl, freq->trip_id))
            vect_append(_ext_gtfs->frequencies_tbl, freq);
    }
    
    if (vect_count(_ext_gtfs->frequencies_tbl) > 0)
        _ext_gtfs->file_exist_bits |= GTFS_FILE_FREQUENCIES;
}

static void extract_trips(struct hash_t* route_id_htbl)
{
    int count, i;
    struct hash_t* trip_id_htbl;
    struct hash_t* service_id_htbl;
    struct hash_t* shape_id_htbl;
    struct hash_t* office_id_htbl;

    count = vect_count(g_gtfs->trips_tbl);
    trip_id_htbl = hash_initialize(count*2+1);
    service_id_htbl = hash_initialize(count*2+1);
    shape_id_htbl = hash_initialize(count*2+1);
    office_id_htbl = hash_initialize(31);

    for (i = 0; i < count; i++) {
        struct trip_t* trip;
        
        trip = (struct trip_t*)vect_get(g_gtfs->trips_tbl, i);
        if (hash_get(route_id_htbl, trip->route_id)) {
            vect_append(_ext_gtfs->trips_tbl, trip);

            if (! hash_get(trip_id_htbl, trip->trip_id))
                hash_put(trip_id_htbl, trip->trip_id, trip);
            if (! hash_get(service_id_htbl, trip->service_id))
                hash_put(service_id_htbl, trip->service_id, trip);
            if (strlen(trip->shape_id) > 0) {
                if (! hash_get(shape_id_htbl, trip->shape_id))
                    hash_put(shape_id_htbl, trip->shape_id, trip);
            }
            if (strlen(trip->jp_office_id) > 0) {
                if (! hash_get(office_id_htbl, trip->jp_office_id))
                    hash_put(office_id_htbl, trip->jp_office_id, trip);
            }
        }
    }

    extract_stop_times(trip_id_htbl);
    extract_calendar(service_id_htbl);
    extract_calendar_dates(service_id_htbl);
    extract_shapes(shape_id_htbl);
    extract_office_jp(office_id_htbl);
    extract_frequencies(trip_id_htbl);

    hash_finalize(trip_id_htbl);
    hash_finalize(service_id_htbl);
    hash_finalize(shape_id_htbl);
    hash_finalize(office_id_htbl);

    if (vect_count(_ext_gtfs->trips_tbl) > 0)
        _ext_gtfs->file_exist_bits |= GTFS_FILE_TRIPS;
}

static void extract_routes_jp(struct hash_t* route_id_htbl)
{
    int count, i;
    
    count = vect_count(g_gtfs->routes_jp_tbl);
    for (i = 0; i < count; i++) {
        struct route_jp_t* rjp;
        
        rjp = (struct route_jp_t*)vect_get(g_gtfs->routes_jp_tbl, i);
        if (hash_get(route_id_htbl, rjp->route_id))
            vect_append(_ext_gtfs->routes_jp_tbl, rjp);
    }

    if (vect_count(_ext_gtfs->routes_jp_tbl) > 0)
        _ext_gtfs->file_exist_bits |= GTFS_FILE_ROUTES_JP;
}

static void extract_routes(const char* agency_id)
{
    int count, i;
    struct hash_t* route_id_htbl;

    count = vect_count(g_gtfs->routes_tbl);
    route_id_htbl = hash_initialize(count*2+1);

    for (i = 0; i < count; i++) {
        struct route_t* route;

        route = (struct route_t*)vect_get(g_gtfs->routes_tbl, i);
        if (strcmp(agency_id, route->agency_id) == 0) {
            vect_append(_ext_gtfs->routes_tbl, route);
            if (! hash_get(route_id_htbl, route->route_id))
                hash_put(route_id_htbl, route->route_id, route);
        }
    }

    extract_routes_jp(route_id_htbl);
    extract_trips(route_id_htbl);
    extract_fare_rules(route_id_htbl);
    hash_finalize(route_id_htbl);

    if (vect_count(_ext_gtfs->routes_tbl) > 0)
        _ext_gtfs->file_exist_bits |= GTFS_FILE_ROUTES;
}

static void extract_agency_jp(const char* agency_id)
{
    int count, i;
    
    count = vect_count(g_gtfs->agency_jp_tbl);
    for (i = 0; i < count; i++) {
        struct agency_jp_t* jp;
        
        jp = (struct agency_jp_t*)vect_get(g_gtfs->agency_jp_tbl, i);
        if (strcmp(agency_id, jp->agency_id) == 0)
            vect_append(_ext_gtfs->agency_jp_tbl, jp);
    }

    if (vect_count(_ext_gtfs->agency_jp_tbl) > 0)
        _ext_gtfs->file_exist_bits |= GTFS_FILE_AGENCY_JP;
}

static void extract_agency(const struct agency_t* agency)
{
    vect_append(_ext_gtfs->agency_tbl, agency);

    extract_agency_jp(agency->agency_id);
    extract_routes(agency->agency_id);
    extract_fare_attributes(agency->agency_id);

    _ext_gtfs->file_exist_bits |= GTFS_FILE_AGENCY;
}

static void extract_translations()
{
    int count, i;
    
    count = vect_count(g_gtfs->translations_tbl);
    for (i = 0; i < count; i++) {
        struct translation_t* trans;
        
        trans = (struct translation_t*)vect_get(g_gtfs->translations_tbl, i);
        vect_append(_ext_gtfs->translations_tbl, trans);
    }
    
    if (vect_count(_ext_gtfs->translations_tbl) > 0)
        _ext_gtfs->file_exist_bits |= GTFS_FILE_TRANSLATIONS;
}

static void extract_feed_info()
{
    // feed_info.txt is g_feed_info
    _ext_gtfs->file_exist_bits |= GTFS_FILE_FEED_INFO;
}

int gtfs_split()
{
    int count, i;

    TRACE("%s\n", "*GTFS(zip)の読み込み*");
    if (gtfs_zip_archive_reader(g_gtfs_zip) < 0) {
        err_write("gtfs_check: zip_archive_reader error (%s).\n", g_gtfs_zip);
        return -1;
    }

    count = vect_count(g_gtfs->agency_tbl);
    if (count < 2) {
        err_write("指定されたGTFS(%s)には複数のagency(事業者)が含まれていません。\n", g_gtfs_zip);
        return -1;
    }

    makedir(g_output_dir);

    for (i = 0; i < count; i++) {
        struct agency_t* agency;
        char zipname[MAX_PATH], datebuf[16];

        _ext_gtfs = gtfs_alloc();

        agency = (struct agency_t*)vect_get(g_gtfs->agency_tbl, i);
        if (agency)
            extract_agency(agency);

        extract_translations();
        extract_feed_info();

        gtfs_feed_writer(g_output_dir, _ext_gtfs);

        snprintf(zipname, sizeof(zipname), "gtfs_%s_%s.zip",
                 agency->agency_name, todays_date(datebuf, sizeof(datebuf), ""));
        gtfs_zip_archive_writer(g_output_dir, zipname, _ext_gtfs);

        gtfs_feed_delete(g_output_dir, _ext_gtfs);
        gtfs_free(_ext_gtfs, 0);
    }
    return 0;
}
