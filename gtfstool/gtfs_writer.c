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
#include "gtfstool.h"

#define MINIZ_HEADER_FILE_ONLY
#include "miniz.c"

#define CRLF    "\r\n"

static int is_necessary_quote(const char* str)
{
    int index;

    index = indexof(str, ',');
    if (index < 0)
        index = indexof(str, '\"');
    if (index < 0)
        index = indexof(str, ' ');

    return (index < 0)? 0 : 1;
}

static char* add_quote(char* qstr, const char* str)
{
    *qstr = '\0';
    if (is_necessary_quote(str)) {
        if (strlen(str) > 0) {
            strcpy(qstr, "\"");
            strcat(qstr, str);
            strcat(qstr, "\"");
        }
    } else {
        strcpy(qstr, str);
    }
    return qstr;
}

static void gtfs_agency_writer(const char* dir, struct vector_t* tbl)
{
    char csvpath[MAX_PATH];
    int count, i;

    char agency_name[256];

    strcpy(csvpath, dir);
    catpath(csvpath, g_gtfs_filename[AGENCY]);
    TRACE("%s ... ", csvpath);

    csv_initialize(csvpath);
    csv_write("%s%s",
              "agency_id,agency_name,agency_url,agency_timezone,agency_lang,"
              "agency_phone,agency_fare_url,agency_email",
              CRLF);

    count = vect_count(tbl);
    for (i = 0; i < count; i++) {
        struct agency_t* a;

        a = (struct agency_t*)vect_get(tbl, i);

        add_quote(agency_name, a->agency_name);

        csv_write("%s,%s,%s,%s,%s,%s,%s,%s%s",
                  a->agency_id, agency_name, a->agency_url, a->agency_timezone,
                  a->agency_lang, a->agency_phone, a->agency_fare_url, a->agency_email,
                  CRLF);
    }

    csv_finalize();
    TRACE("%s\n", "done");
}

static void gtfs_agency_jp_writer(const char* dir, struct vector_t* tbl)
{
    char csvpath[MAX_PATH];
    int count, i;

    char agency_id[64];
    char agency_official_name[256];
    char agency_zip_number[16];
    char agency_address[256];
    char agency_president_pos[128];
    char agency_president_name[128];
    
    strcpy(csvpath, dir);
    catpath(csvpath, g_gtfs_filename[AGENCY_JP]);
    TRACE("%s ... ", csvpath);

    csv_initialize(csvpath);
    csv_write("%s%s",
              "agency_id,agency_official_name,agency_zip_number,agency_address,"
              "agency_president_pos,agency_president_name",
              CRLF);

    count = vect_count(tbl);
    for (i = 0; i < count; i++) {
        struct agency_jp_t* ajp;
        
        ajp = (struct agency_jp_t*)vect_get(tbl, i);

        add_quote(agency_id, ajp->agency_id);
        add_quote(agency_official_name, ajp->agency_official_name);
        add_quote(agency_zip_number, ajp->agency_zip_number);
        add_quote(agency_address, ajp->agency_address);
        add_quote(agency_president_pos, ajp->agency_president_pos);
        add_quote(agency_president_name, ajp->agency_president_name);
        
        csv_write("%s,%s,%s,%s,%s,%s%s",
                  agency_id, agency_official_name, agency_zip_number, agency_address,
                  agency_president_pos, agency_president_name,
                  CRLF);
    }
    
    csv_finalize();
    TRACE("%s\n", "done");
}

static void gtfs_stops_writer(const char* dir, struct vector_t* tbl)
{
    char csvpath[MAX_PATH];
    int count, i;

    strcpy(csvpath, dir);
    catpath(csvpath, g_gtfs_filename[STOPS]);
    TRACE("%s ... ", csvpath);

    csv_initialize(csvpath);
    csv_write("%s%s",
              "stop_id,stop_code,stop_name,"
              "stop_desc,stop_lat,stop_lon,"
              "zone_id,stop_url,location_type,"
              "parent_station,stop_timezone,wheelchair_boarding",
              CRLF);

    count = vect_count(tbl);
    for (i = 0; i < count; i++) {
        struct stop_t* s;
        char stop_name[MAX_STATION_NAME*2];

        s = (struct stop_t*)vect_get(tbl, i);

        add_quote(stop_name, s->stop_name);

        csv_write("%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s%s",
                  s->stop_id, s->stop_code, stop_name, s->stop_desc,
                  s->stop_lat, s->stop_lon, s->zone_id, s->stop_url,
                  s->location_type, s->parent_station, s->stop_timezone,
                  s->wheelchair_boarding,
                  CRLF);
    }

    csv_finalize();
    TRACE("%s[%d]\n", "done", count);
}

static void gtfs_routes_writer(const char* dir, struct vector_t* tbl)
{
    char csvpath[MAX_PATH];
    int count, i;
    
    strcpy(csvpath, dir);
    catpath(csvpath, g_gtfs_filename[ROUTES]);
    TRACE("%s ... ", csvpath);

    csv_initialize(csvpath);
    csv_write("%s%s",
              "route_id,agency_id,route_short_name,"
              "route_long_name,route_desc,route_type,"
              "route_url,route_color,route_text_color,jp_parent_route_id",
              CRLF);

    count = vect_count(tbl);
    for (i = 0; i < count; i++) {
        struct route_t* r;
        char route_short_name[MAX_RAIL_NAME];
        char route_long_name[MAX_RAIL_NAME];
        char route_desc[256];

        r = (struct route_t*)vect_get(tbl, i);

        add_quote(route_short_name, r->route_short_name);
        add_quote(route_long_name, r->route_long_name);
        add_quote(route_desc, r->route_desc);

        csv_write("%s,%s,%s,%s,%s,%s,%s,%s,%s,%s%s",
                  r->route_id, r->agency_id, route_short_name, route_long_name,
                  route_desc, r->route_type, r->route_url, r->route_color,
                  r->route_text_color, r->jp_parent_route_id,
                  CRLF);
    }

    csv_finalize();
    TRACE("%s[%d]\n", "done", count);
}

static void gtfs_trips_writer(const char* dir, struct vector_t* tbl)
{
    char csvpath[MAX_PATH];
    int count;
    int i;
    
    strcpy(csvpath, dir);
    catpath(csvpath, g_gtfs_filename[TRIPS]);
    TRACE("%s ... ", csvpath);
    
    csv_initialize(csvpath);
    csv_write("%s%s",
              "route_id,service_id,trip_id,"
              "trip_headsign,trip_short_name,direction_id,"
              "block_id,shape_id,wheelchair_accessible,bikes_allowed,"
              "jp_trip_desc,jp_trip_desc_symbol,jp_office_id",
              CRLF);
    
    count = vect_count(tbl);
    for (i = 0; i < count; i++) {
        struct trip_t* t;
        char trip_headsign[256];
        char trip_short_name[256];
        char jp_trip_desc[512];
        
        t = (struct trip_t*)vect_get(tbl, i);
        
        add_quote(trip_headsign, t->trip_headsign);
        add_quote(trip_short_name, t->trip_short_name);
        add_quote(jp_trip_desc, t->jp_trip_desc);
        
        csv_write("%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s%s",
                  t->route_id, t->service_id, t->trip_id, trip_headsign,
                  trip_short_name, t->direction_id, t->block_id, t->shape_id,
                  t->wheelchair_accessible, t->bikes_allowed,
                  jp_trip_desc, t->jp_trip_desc_symbol, t->jp_office_id,
                  CRLF);
    }
    
    csv_finalize();
    TRACE("%s[%d]\n", "done", count);
}

static void gtfs_stop_times_writer(const char* dir, struct vector_t* tbl)
{
    char csvpath[MAX_PATH];
    int count, i;
    
    strcpy(csvpath, dir);
    catpath(csvpath, g_gtfs_filename[STOP_TIMES]);
    TRACE("%s ... ", csvpath);
    
    csv_initialize(csvpath);
    csv_write("%s%s",
              "trip_id,arrival_time,departure_time,stop_id,"
              "stop_sequence,stop_headsign,pickup_type,drop_off_type,"
              "shape_dist_traveled,timepoint",
              CRLF);
    
    count = vect_count(tbl);
    for (i = 0; i < count; i++) {
        struct stop_time_t* s;
        char headsign[256];
        
        s = (struct stop_time_t*)vect_get(tbl, i);
        
        add_quote(headsign, s->stop_headsign);
        
        csv_write("%s,%s,%s,%s,%s,%s,%s,%s,%s,%s%s",
                  s->trip_id, s->arrival_time, s->departure_time, s->stop_id,
                  s->stop_sequence, headsign, s->pickup_type, s->drop_off_type,
                  s->shape_dist_traveled, s->timepoint,
                  CRLF);
    }
    
    csv_finalize();
    TRACE("%s[%d]\n", "done", count);
}

static void gtfs_calendar_writer(const char* dir, struct vector_t* tbl)
{
    char csvpath[MAX_PATH];
    int count, i;
    
    strcpy(csvpath, dir);
    catpath(csvpath, g_gtfs_filename[CALENDAR]);
    TRACE("%s ... ", csvpath);
    
    csv_initialize(csvpath);
    csv_write("%s%s",
              "service_id,"
              "monday,tuesday,wednesday,thursday,friday,saturday,sunday,"
              "start_date,end_date",
              CRLF);
    
    count = vect_count(tbl);
    for (i = 0; i < count; i++) {
        struct calendar_t* cal;
        
        cal = (struct calendar_t*)vect_get(tbl, i);
        
        csv_write("%s,%s,%s,%s,%s,%s,%s,%s,%s,%s%s",
                  cal->service_id,
                  cal->monday, cal->tuesday, cal->wednesday, cal->thursday,
                  cal->friday, cal->saturday, cal->sunday,
                  cal->start_date, cal->end_date,
                  CRLF);
    }
    
    csv_finalize();
    TRACE("%s\n", "done");
}

static void gtfs_calendar_dates_writer(const char* dir, struct vector_t* tbl)
{
    char csvpath[MAX_PATH];
    int count, i;
    
    strcpy(csvpath, dir);
    catpath(csvpath, g_gtfs_filename[CALENDAR_DATES]);
    TRACE("%s ... ", csvpath);
    
    csv_initialize(csvpath);
    csv_write("%s%s",
              "service_id,date,exception_type",
              CRLF);
    
    count = vect_count(tbl);
    for (i = 0; i < count; i++) {
        struct calendar_date_t* cd;
        
        cd = (struct calendar_date_t*)vect_get(tbl, i);
        
        csv_write("%s,%s,%s%s",
                  cd->service_id, cd->date, cd->exception_type,
                  CRLF);
    }
    
    csv_finalize();
    TRACE("%s[%d]\n", "done", count);
}

static void gtfs_fare_attributes_writer(const char* dir, struct vector_t* tbl)
{
    char csvpath[MAX_PATH];
    int count, i;

    strcpy(csvpath, dir);
    catpath(csvpath, g_gtfs_filename[FARE_ATTRIBUTES]);
    TRACE("%s ... ", csvpath);

    csv_initialize(csvpath);
    csv_write("%s%s",
              "fare_id,price,currency_type,payment_method,"
              "transfers,agency_id,transfer_duration",
              CRLF);

    count = vect_count(tbl);
    for (i = 0; i < count; i++) {
        struct fare_attribute_t * fattr;

        fattr = (struct fare_attribute_t*)vect_get(tbl, i);
        
        csv_write("%s,%s,%s,%s,%s,%s,%s%s",
                  fattr->fare_id, fattr->price, fattr->currency_type, fattr->payment_method,
                  fattr->transfers, fattr->agency_id, fattr->transfer_duration,
                  CRLF);
    }

    csv_finalize();
    TRACE("%s[%d]\n", "done", count);
}

static void gtfs_fare_rules_writer(const char* dir, struct vector_t* tbl)
{
    char csvpath[MAX_PATH];
    int count, i;

    strcpy(csvpath, dir);
    catpath(csvpath, g_gtfs_filename[FARE_RULES]);
    TRACE("%s ... ", csvpath);

    csv_initialize(csvpath);
    csv_write("%s%s",
              "fare_id,route_id,origin_id,destination_id,contains_id",
              CRLF);

    count = vect_count(tbl);
    for (i = 0; i < count; i++) {
        struct fare_rule_t* f;
        
        f = (struct fare_rule_t*)vect_get(tbl, i);
        
        csv_write("%s,%s,%s,%s,%s%s",
                  f->fare_id, f->route_id, f->origin_id, f->destination_id, f->contains_id,
                  CRLF);
    }

    csv_finalize();
    TRACE("%s[%d]\n", "done", count);
}

static void gtfs_shapes_writer(const char* dir, struct vector_t* tbl)
{
    char csvpath[MAX_PATH];
    int count, i;
    
    strcpy(csvpath, dir);
    catpath(csvpath, g_gtfs_filename[SHAPES]);
    TRACE("%s ... ", csvpath);
    
    csv_initialize(csvpath);
    csv_write("%s%s",
              "shape_id,shape_pt_lat,shape_pt_lon,shape_pt_sequence,shape_dist_traveled",
              CRLF);
    
    count = vect_count(tbl);
    for (i = 0; i < count; i++) {
        struct shape_t* s;
        
        s = (struct shape_t*)vect_get(tbl, i);
        
        csv_write("%s,%s,%s,%s,%s%s",
                  s->shape_id, s->shape_pt_lat, s->shape_pt_lon,
                  s->shape_pt_sequence, s->shape_dist_traveled,
                  CRLF);
    }
    
    csv_finalize();
    TRACE("%s[%d]\n", "done", count);
}

static void gtfs_frequencies_writer(const char* dir, struct vector_t* tbl)
{
    char csvpath[MAX_PATH];
    int count, i;
    
    strcpy(csvpath, dir);
    catpath(csvpath, g_gtfs_filename[FREQUENCIES]);
    TRACE("%s ... ", csvpath);
    
    csv_initialize(csvpath);
    csv_write("%s%s",
              "trip_id,start_time,end_time,headway_secs,exact_times",
              CRLF);
    
    count = vect_count(tbl);
    for (i = 0; i < count; i++) {
        struct frequency_t* f;
        
        f = (struct frequency_t*)vect_get(tbl, i);
        
        csv_write("%s,%s,%s,%s,%s%s",
                  f->trip_id, f->start_time, f->end_time,
                  f->headway_secs, f->exact_times,
                  CRLF);
    }
    
    csv_finalize();
    TRACE("%s[%d]\n", "done", count);
}

static void gtfs_transfers_writer(const char* dir, struct vector_t* tbl)
{
    char csvpath[MAX_PATH];
    int count, i;
    
    strcpy(csvpath, dir);
    catpath(csvpath, g_gtfs_filename[TRANSFERS]);
    TRACE("%s ... ", csvpath);
    
    csv_initialize(csvpath);
    csv_write("%s%s",
              "from_stop_id,to_stop_id,transfer_type,min_transfer_time",
              CRLF);
    
    count = vect_count(tbl);
    for (i = 0; i < count; i++) {
        struct transfer_t* t;
        
        t = (struct transfer_t*)vect_get(tbl, i);
        
        csv_write("%s,%s,%s,%s%s",
                  t->from_stop_id, t->to_stop_id, t->transfer_type, t->min_transfer_time,
                  CRLF);
    }
    
    csv_finalize();
    TRACE("%s[%d]\n", "done", count);
}

static void gtfs_feed_info_writer(const char* dir, struct feed_info_t* feed_info)
{
    char csvpath[MAX_PATH];
    
    char feed_publisher_name[256];
    char feed_publisher_url[256];
    char feed_lang[4];
    char feed_start_date[16];
    char feed_end_date[16];
    char feed_version[256];
    
    strcpy(csvpath, dir);
    catpath(csvpath, g_gtfs_filename[FEED_INFO]);
    TRACE("%s ... ", csvpath);
    
    csv_initialize(csvpath);
    csv_write("%s%s",
              "feed_publisher_name,feed_publisher_url,feed_lang,"
              "feed_start_date,feed_end_date,feed_version",
              CRLF);
    
    add_quote(feed_publisher_name, feed_info->feed_publisher_name);
    add_quote(feed_publisher_url, feed_info->feed_publisher_url);
    add_quote(feed_lang, feed_info->feed_lang);
    add_quote(feed_start_date, feed_info->feed_start_date);
    add_quote(feed_end_date, feed_info->feed_end_date);
    add_quote(feed_version, feed_info->feed_version);
    
    csv_write("%s,%s,%s,%s,%s,%s%s",
              feed_publisher_name, feed_publisher_url, feed_lang,
              feed_start_date, feed_end_date, feed_version,
              CRLF);
    
    csv_finalize();
    TRACE("%s\n", "done");
}

static void gtfs_translations_writer(const char* dir, struct vector_t* tbl)
{
    char csvpath[MAX_PATH];
    int count, i;

    strcpy(csvpath, dir);
    catpath(csvpath, g_gtfs_filename[TRANSLATIONS]);
    TRACE("%s ... ", csvpath);

    csv_initialize(csvpath);
    csv_write("%s%s",
              "trans_id,lang,translation",
              CRLF);

    count = vect_count(tbl);
    for (i = 0; i < count; i++) {
        struct translation_t* t;
        char trans_id[256];
        char translation[512];

        t = (struct translation_t*)vect_get(tbl, i);

        add_quote(trans_id, t->trans_id);
        add_quote(translation, t->translation);

        csv_write("%s,%s,%s%s",
                  trans_id, t->lang, translation,
                  CRLF);
    }

    csv_finalize();
    TRACE("%s[%d]\n", "done", count);
}

static void gtfs_routes_jp_writer(const char* dir, struct vector_t* tbl)
{
    char csvpath[MAX_PATH];
    int count, i;
    
    strcpy(csvpath, dir);
    catpath(csvpath, g_gtfs_filename[ROUTES_JP]);
    TRACE("%s ... ", csvpath);
    
    csv_initialize(csvpath);
    csv_write("%s%s",
              "route_id,route_update_date,origin_stop,via_stop,destination_stop",
              CRLF);
    
    count = vect_count(tbl);
    for (i = 0; i < count; i++) {
        struct route_jp_t* r;
        
        r = (struct route_jp_t*)vect_get(tbl, i);

        csv_write("%s,%s,%s,%s,%s%s",
                  r->route_id, r->route_update_date, r->origin_stop, r->via_stop, r->destination_stop,
                  CRLF);
    }
    
    csv_finalize();
    TRACE("%s[%d]\n", "done", count);
}

static void gtfs_office_jp_writer(const char* dir, struct vector_t* tbl)
{
    char csvpath[MAX_PATH];
    int count, i;
    
    strcpy(csvpath, dir);
    catpath(csvpath, g_gtfs_filename[OFFICE_JP]);
    TRACE("%s ... ", csvpath);
    
    csv_initialize(csvpath);
    csv_write("%s%s",
              "office_id,office_name,office_url,office_phone",
              CRLF);
    
    count = vect_count(tbl);
    for (i = 0; i < count; i++) {
        struct office_jp_t* o;
        char office_name[256];

        o = (struct office_jp_t*)vect_get(tbl, i);

        add_quote(office_name, o->office_name);

        csv_write("%s,%s,%s,%s%s",
                  o->office_id, office_name, o->office_url, o->office_phone,
                  CRLF);
    }
    
    csv_finalize();
    TRACE("%s[%d]\n", "done", count);
}

void gtfs_feed_writer(const char* dir, struct gtfs_t* gtfs)
{
    if (gtfs->file_exist_bits & GTFS_FILE_AGENCY)
        gtfs_agency_writer(dir, gtfs->agency_tbl);

    if (gtfs->file_exist_bits & GTFS_FILE_AGENCY_JP)
        gtfs_agency_jp_writer(dir, gtfs->agency_jp_tbl);

    if (gtfs->file_exist_bits & GTFS_FILE_STOPS)
        gtfs_stops_writer(dir, gtfs->stops_tbl);

    if (gtfs->file_exist_bits & GTFS_FILE_ROUTES)
        gtfs_routes_writer(dir, gtfs->routes_tbl);
    
    if (gtfs->file_exist_bits & GTFS_FILE_TRIPS)
        gtfs_trips_writer(dir, gtfs->trips_tbl);
    
    if (gtfs->file_exist_bits & GTFS_FILE_STOP_TIMES)
        gtfs_stop_times_writer(dir, gtfs->stop_times_tbl);

    if (gtfs->file_exist_bits & GTFS_FILE_CALENDAR)
        gtfs_calendar_writer(dir, gtfs->calendar_tbl);

    if (gtfs->file_exist_bits & GTFS_FILE_CALENDAR_DATES)
        gtfs_calendar_dates_writer(dir, gtfs->calendar_dates_tbl);

    if (gtfs->file_exist_bits & GTFS_FILE_FARE_ATTRIBUTES)
        gtfs_fare_attributes_writer(dir, gtfs->fare_attrs_tbl);

    if (gtfs->file_exist_bits & GTFS_FILE_FARE_RULES)
        gtfs_fare_rules_writer(dir, gtfs->fare_rules_tbl);

    if (gtfs->file_exist_bits & GTFS_FILE_SHAPES)
        gtfs_shapes_writer(dir, gtfs->shapes_tbl);

    if (gtfs->file_exist_bits & GTFS_FILE_FREQUENCIES)
        gtfs_frequencies_writer(dir, gtfs->frequencies_tbl);

    if (gtfs->file_exist_bits & GTFS_FILE_TRANSFERS)
        gtfs_transfers_writer(dir, gtfs->transfers_tbl);

    if (gtfs->file_exist_bits & GTFS_FILE_FEED_INFO)
        gtfs_feed_info_writer(dir, &g_feed_info);

    if (gtfs->file_exist_bits & GTFS_FILE_TRANSLATIONS)
        gtfs_translations_writer(dir, gtfs->translations_tbl);

    if (gtfs->file_exist_bits & GTFS_FILE_ROUTES_JP)
        gtfs_routes_jp_writer(dir, gtfs->routes_jp_tbl);

    if (gtfs->file_exist_bits & GTFS_FILE_OFFICE_JP)
        gtfs_office_jp_writer(dir, gtfs->office_jp_tbl);
}

static void remove_csvfile(const char* dir, const char* fname)
{
    char csvpath[MAX_PATH];

    strcpy(csvpath, dir);
    catpath(csvpath, fname);
    rmfile(csvpath);
}

void gtfs_feed_delete(const char* dir, struct gtfs_t* gtfs)
{
    int i;

    for (i = 0; g_gtfs_filename[i]; i++) {
        if (gtfs->file_exist_bits & g_gtfs_filemap[i])
            remove_csvfile(dir, g_gtfs_filename[i]);
    }
}

int gtfs_zip_archive_writer(const char* dir, const char* zipname, struct gtfs_t* gtfs)
{
    char zippath[MAX_PATH];
    mz_zip_archive zip_archive;
    mz_bool done;
    int i;
    
    strcpy(zippath, dir);
    catpath(zippath, zipname);

    memset(&zip_archive, '\0', sizeof(zip_archive));
    done = mz_zip_writer_init_file(&zip_archive, zippath, 0);
    if (done != MZ_TRUE)
        return -1;

    for (i = 0; g_gtfs_filename[i]; i++) {
        if (gtfs->file_exist_bits & g_gtfs_filemap[i]) {
            char csvpath[MAX_PATH];

            strcpy(csvpath, dir);
            catpath(csvpath, g_gtfs_filename[i]);
            mz_zip_writer_add_file(&zip_archive, g_gtfs_filename[i], csvpath, NULL, 0, MZ_DEFAULT_LEVEL);
        }
    }

    done = mz_zip_writer_finalize_archive(&zip_archive);
    done = mz_zip_writer_end(&zip_archive);
    return (done == MZ_TRUE)? 0 : -1;
}
