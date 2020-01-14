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

static struct gtfs_t* _mrg_gtfs;                // マージされたGTFS
static struct hash_t* _mrg_translations_htbl;   // マージされた翻訳情報のハッシュテーブル（キーは"trans_id/lang"）

static void gtfs_merge_stops(struct vector_t* src_tbl, struct vector_t* merge_tbl, const char* prefix)
{
    int count, i;

    count = vect_count(src_tbl);
    for (i = 0; i < count; i++) {
        struct stop_t* s;
        struct stop_t* t;

        s = (struct stop_t*)vect_get(src_tbl, i);
        t = (struct stop_t*)malloc(sizeof(struct stop_t));
        memcpy(t, s, sizeof(struct stop_t));
        if (strlen(s->stop_id) > 0)
            snprintf(t->stop_id, sizeof(t->stop_id), "%s%s", prefix, s->stop_id);
        if (strlen(s->zone_id) > 0)
            snprintf(t->zone_id, sizeof(t->zone_id), "%s%s", prefix, s->zone_id);
        vect_append(merge_tbl, t);
    }
}

static void gtfs_merge_routes(struct vector_t* src_tbl, struct vector_t* merge_tbl, const char* prefix)
{
    int count, i;
    
    count = vect_count(src_tbl);
    for (i = 0; i < count; i++) {
        struct route_t* s;
        struct route_t* t;
        
        s = (struct route_t*)vect_get(src_tbl, i);
        t = (struct route_t*)malloc(sizeof(struct route_t));
        memcpy(t, s, sizeof(struct route_t));
        if (strlen(s->route_id) > 0)
            snprintf(t->route_id, sizeof(t->route_id), "%s%s", prefix, s->route_id);
        if (strlen(s->jp_parent_route_id) > 0)
            snprintf(t->jp_parent_route_id, sizeof(t->jp_parent_route_id), "%s%s", prefix, s->jp_parent_route_id);
        // agency_idを更新
        strcpy(t->agency_id, g_merged_agency.agency_id);
        vect_append(merge_tbl, t);
    }
}

static void gtfs_merge_routes_jp(struct vector_t* src_tbl, struct vector_t* merge_tbl, const char* prefix)
{
    int count, i;
    
    count = vect_count(src_tbl);
    for (i = 0; i < count; i++) {
        struct route_jp_t* s;
        struct route_jp_t* t;
        
        s = (struct route_jp_t*)vect_get(src_tbl, i);
        t = (struct route_jp_t*)malloc(sizeof(struct route_jp_t));
        memcpy(t, s, sizeof(struct route_jp_t));
        if (strlen(s->route_id) > 0)
            snprintf(t->route_id, sizeof(t->route_id), "%s%s", prefix, s->route_id);
        vect_append(merge_tbl, t);
    }
}

static void gtfs_merge_trips(struct vector_t* src_tbl, struct vector_t* merge_tbl, const char* prefix)
{
    int count, i;
    
    count = vect_count(src_tbl);
    for (i = 0; i < count; i++) {
        struct trip_t* s;
        struct trip_t* t;
        
        s = (struct trip_t*)vect_get(src_tbl, i);
        t = (struct trip_t*)malloc(sizeof(struct trip_t));
        memcpy(t, s, sizeof(struct trip_t));
        if (strlen(s->trip_id) > 0)
            snprintf(t->trip_id, sizeof(t->trip_id), "%s%s", prefix, s->trip_id);
        if (strlen(s->service_id) > 0)
            snprintf(t->service_id, sizeof(t->service_id), "%s%s", prefix, s->service_id);
        if (strlen(s->route_id) > 0)
            snprintf(t->route_id, sizeof(t->route_id), "%s%s", prefix, s->route_id);
        if (strlen(s->block_id) > 0)
            snprintf(t->block_id, sizeof(t->block_id), "%s%s", prefix, s->block_id);
        if (strlen(s->shape_id) > 0)
            snprintf(t->shape_id, sizeof(t->shape_id), "%s%s", prefix, s->shape_id);
        if (strlen(s->jp_office_id) > 0)
            snprintf(t->jp_office_id, sizeof(t->jp_office_id), "%s%s", prefix, s->jp_office_id);
        vect_append(merge_tbl, t);
    }
}

static void gtfs_merge_office_jp(struct vector_t* src_tbl, struct vector_t* merge_tbl, const char* prefix)
{
    int count, i;
    
    count = vect_count(src_tbl);
    for (i = 0; i < count; i++) {
        struct office_jp_t* s;
        struct office_jp_t* t;
        
        s = (struct office_jp_t*)vect_get(src_tbl, i);
        t = (struct office_jp_t*)malloc(sizeof(struct office_jp_t));
        memcpy(t, s, sizeof(struct office_jp_t));
        if (strlen(s->office_id) > 0)
            snprintf(t->office_id, sizeof(t->office_id), "%s%s", prefix, s->office_id);
        vect_append(merge_tbl, t);
    }
}

static void gtfs_merge_stop_times(struct vector_t* src_tbl, struct vector_t* merge_tbl, const char* prefix)
{
    int count, i;
    
    count = vect_count(src_tbl);
    for (i = 0; i < count; i++) {
        struct stop_time_t* s;
        struct stop_time_t* t;
        
        s = (struct stop_time_t*)vect_get(src_tbl, i);
        t = (struct stop_time_t*)malloc(sizeof(struct stop_time_t));
        memcpy(t, s, sizeof(struct stop_time_t));
        if (strlen(s->trip_id) > 0)
            snprintf(t->trip_id, sizeof(t->trip_id), "%s%s", prefix, s->trip_id);
        if (strlen(s->stop_id) > 0)
            snprintf(t->stop_id, sizeof(t->stop_id), "%s%s", prefix, s->stop_id);
        vect_append(merge_tbl, t);
    }
}

static void gtfs_merge_calendar(struct vector_t* src_tbl, struct vector_t* merge_tbl, const char* prefix)
{
    int count, i;
    
    count = vect_count(src_tbl);
    for (i = 0; i < count; i++) {
        struct calendar_t* s;
        struct calendar_t* t;
        
        s = (struct calendar_t*)vect_get(src_tbl, i);
        t = (struct calendar_t*)malloc(sizeof(struct calendar_t));
        memcpy(t, s, sizeof(struct calendar_t));
        if (strlen(s->service_id) > 0)
            snprintf(t->service_id, sizeof(t->service_id), "%s%s", prefix, s->service_id);
        vect_append(merge_tbl, t);
    }
}

static void gtfs_merge_calendar_dates(struct vector_t* src_tbl, struct vector_t* merge_tbl, const char* prefix)
{
    int count, i;
    
    count = vect_count(src_tbl);
    for (i = 0; i < count; i++) {
        struct calendar_date_t* s;
        struct calendar_date_t* t;
        
        s = (struct calendar_date_t*)vect_get(src_tbl, i);
        t = (struct calendar_date_t*)malloc(sizeof(struct calendar_date_t));
        memcpy(t, s, sizeof(struct calendar_date_t));
        if (strlen(s->service_id) > 0)
            snprintf(t->service_id, sizeof(t->service_id), "%s%s", prefix, s->service_id);
        vect_append(merge_tbl, t);
    }
}

static void gtfs_merge_fare_attributes(struct vector_t* src_tbl, struct vector_t* merge_tbl, const char* prefix)
{
    int count, i;
    
    count = vect_count(src_tbl);
    for (i = 0; i < count; i++) {
        struct fare_attribute_t* s;
        struct fare_attribute_t* t;
        
        s = (struct fare_attribute_t*)vect_get(src_tbl, i);
        t = (struct fare_attribute_t*)malloc(sizeof(struct fare_attribute_t));
        memcpy(t, s, sizeof(struct fare_attribute_t));
        if (strlen(s->fare_id) > 0)
            snprintf(t->fare_id, sizeof(t->fare_id), "%s%s", prefix, s->fare_id);
        vect_append(merge_tbl, t);
    }
}

static void gtfs_merge_fare_rules(struct vector_t* src_tbl, struct vector_t* merge_tbl, const char* prefix)
{
    int count, i;
    
    count = vect_count(src_tbl);
    for (i = 0; i < count; i++) {
        struct fare_rule_t* s;
        struct fare_rule_t* t;
        
        s = (struct fare_rule_t*)vect_get(src_tbl, i);
        t = (struct fare_rule_t*)malloc(sizeof(struct fare_rule_t));
        memcpy(t, s, sizeof(struct fare_rule_t));
        if (strlen(s->fare_id) > 0)
            snprintf(t->fare_id, sizeof(t->fare_id), "%s%s", prefix, s->fare_id);
        if (strlen(s->route_id) > 0)
            snprintf(t->route_id, sizeof(t->route_id), "%s%s", prefix, s->route_id);
        if (strlen(s->origin_id) > 0)
            snprintf(t->origin_id, sizeof(t->origin_id), "%s%s", prefix, s->origin_id);
        if (strlen(s->destination_id) > 0)
            snprintf(t->destination_id, sizeof(t->destination_id), "%s%s", prefix, s->destination_id);
        if (strlen(s->contains_id) > 0)
            snprintf(t->contains_id, sizeof(t->contains_id), "%s%s", prefix, s->contains_id);
        vect_append(merge_tbl, t);
    }
}

static void gtfs_merge_shapes(struct vector_t* src_tbl, struct vector_t* merge_tbl, const char* prefix)
{
    int count, i;
    
    count = vect_count(src_tbl);
    for (i = 0; i < count; i++) {
        struct shape_t* s;
        struct shape_t* t;
        
        s = (struct shape_t*)vect_get(src_tbl, i);
        t = (struct shape_t*)malloc(sizeof(struct shape_t));
        memcpy(t, s, sizeof(struct shape_t));
        if (strlen(s->shape_id) > 0)
            snprintf(t->shape_id, sizeof(t->shape_id), "%s%s", prefix, s->shape_id);
        vect_append(merge_tbl, t);
    }
}

static void gtfs_merge_frequencies(struct vector_t* src_tbl, struct vector_t* merge_tbl, const char* prefix)
{
    int count, i;
    
    count = vect_count(src_tbl);
    for (i = 0; i < count; i++) {
        struct frequency_t* s;
        struct frequency_t* t;
        
        s = (struct frequency_t*)vect_get(src_tbl, i);
        t = (struct frequency_t*)malloc(sizeof(struct frequency_t));
        memcpy(t, s, sizeof(struct frequency_t));
        if (strlen(s->trip_id) > 0)
            snprintf(t->trip_id, sizeof(t->trip_id), "%s%s", prefix, s->trip_id);
        vect_append(merge_tbl, t);
    }
}

static void gtfs_merge_transfers(struct vector_t* src_tbl, struct vector_t* merge_tbl, const char* prefix)
{
    int count, i;
    
    count = vect_count(src_tbl);
    for (i = 0; i < count; i++) {
        struct transfer_t* s;
        struct transfer_t* t;
        
        s = (struct transfer_t*)vect_get(src_tbl, i);
        t = (struct transfer_t*)malloc(sizeof(struct transfer_t));
        memcpy(t, s, sizeof(struct transfer_t));
        if (strlen(s->from_stop_id) > 0)
            snprintf(t->from_stop_id, sizeof(t->from_stop_id), "%s%s", prefix, s->from_stop_id);
        if (strlen(s->to_stop_id) > 0)
            snprintf(t->to_stop_id, sizeof(t->to_stop_id), "%s%s", prefix, s->to_stop_id);
        vect_append(merge_tbl, t);
    }
}

static void gtfs_merge_translations(struct vector_t* src_tbl, struct vector_t* merge_tbl, const char* prefix)
{
    int count, i;
    
    count = vect_count(src_tbl);
    for (i = 0; i < count; i++) {
        struct translation_t* s;
        char hkey[256];

        s = (struct translation_t*)vect_get(src_tbl, i);
        snprintf(hkey, sizeof(hkey), "%s/%s", s->trans_id, s->lang);
        if (! hash_get(_mrg_translations_htbl, hkey)) {
            struct translation_t* t;

            hash_put(_mrg_translations_htbl, hkey, s);

            t = (struct translation_t*)malloc(sizeof(struct translation_t));
            memcpy(t, s, sizeof(struct translation_t));
            vect_append(merge_tbl, t);
        }
    }
}

static void set_default_agency()
{
    
    if (is_gtfs_file_exist(GTFS_FILE_AGENCY)) {
        struct agency_t* agency;
        
        agency = (struct agency_t*)vect_get(g_gtfs->agency_tbl, 0);
        if (! agency)
            return;
        
        if (strlen(g_merged_agency.agency_id) == 0)
            strcpy(g_merged_agency.agency_id, agency->agency_id);
        
        if (strlen(g_merged_agency.agency_name) == 0)
            strcpy(g_merged_agency.agency_name, agency->agency_name);
        
        if (strlen(g_merged_agency.agency_url) == 0)
            strcpy(g_merged_agency.agency_url, agency->agency_url);
        
        if (strlen(g_merged_agency.agency_timezone) == 0)
            strcpy(g_merged_agency.agency_timezone, agency->agency_timezone);
        
        if (strlen(g_merged_agency.agency_lang) == 0)
            strcpy(g_merged_agency.agency_lang, agency->agency_lang);
        
        if (strlen(g_merged_agency.agency_phone) == 0)
            strcpy(g_merged_agency.agency_phone, agency->agency_phone);
        
        if (strlen(g_merged_agency.agency_fare_url) == 0)
            strcpy(g_merged_agency.agency_fare_url, agency->agency_fare_url);
        
        if (strlen(g_merged_agency.agency_email) == 0)
            strcpy(g_merged_agency.agency_email, agency->agency_email);
    }
}

static void set_default_agency_jp()
{
    
    if (is_gtfs_file_exist(GTFS_FILE_AGENCY_JP)) {
        struct agency_jp_t* ajp;
        
        ajp = (struct agency_jp_t*)vect_get(g_gtfs->agency_jp_tbl, 0);
        if (! ajp)
            return;
        
        if (strlen(g_merged_agency_jp.agency_official_name) == 0)
            strcpy(g_merged_agency_jp.agency_official_name, ajp->agency_official_name);
        
        if (strlen(g_merged_agency_jp.agency_zip_number) == 0)
            strcpy(g_merged_agency_jp.agency_zip_number, ajp->agency_zip_number);
        
        if (strlen(g_merged_agency_jp.agency_address) == 0)
            strcpy(g_merged_agency_jp.agency_address, ajp->agency_address);
        
        if (strlen(g_merged_agency_jp.agency_president_pos) == 0)
            strcpy(g_merged_agency_jp.agency_president_pos, ajp->agency_president_pos);
        
        if (strlen(g_merged_agency_jp.agency_president_name) == 0)
            strcpy(g_merged_agency_jp.agency_president_name, ajp->agency_president_name);
    }
}

static int gtfs_merge_one(struct merge_gtfs_prefix_t* m, int index)
{
    g_gtfs = gtfs_alloc();
    
    if (gtfs_zip_archive_reader(m->gtfs_file_name) < 0) {
        err_write("GTFSファイルを読み込めませんでした(%s)。\n", m->gtfs_file_name);
        gtfs_free(g_gtfs, 1);
        return -1;
    }

    if (is_gtfs_file_exist(GTFS_FILE_STOPS)) {
        gtfs_merge_stops(g_gtfs->stops_tbl, _mrg_gtfs->stops_tbl, m->prefix);
        _mrg_gtfs->file_exist_bits |= GTFS_FILE_STOPS;
    }
    if (is_gtfs_file_exist(GTFS_FILE_ROUTES)) {
        gtfs_merge_routes(g_gtfs->routes_tbl, _mrg_gtfs->routes_tbl, m->prefix);
        _mrg_gtfs->file_exist_bits |= GTFS_FILE_ROUTES;
    }
    if (is_gtfs_file_exist(GTFS_FILE_ROUTES_JP)) {
        gtfs_merge_routes_jp(g_gtfs->routes_jp_tbl, _mrg_gtfs->routes_jp_tbl, m->prefix);
        _mrg_gtfs->file_exist_bits |= GTFS_FILE_ROUTES_JP;
    }
    if (is_gtfs_file_exist(GTFS_FILE_TRIPS)) {
        gtfs_merge_trips(g_gtfs->trips_tbl, _mrg_gtfs->trips_tbl, m->prefix);
        _mrg_gtfs->file_exist_bits |= GTFS_FILE_TRIPS;
    }
    if (is_gtfs_file_exist(GTFS_FILE_OFFICE_JP)) {
        gtfs_merge_office_jp(g_gtfs->office_jp_tbl, _mrg_gtfs->office_jp_tbl, m->prefix);
        _mrg_gtfs->file_exist_bits |= GTFS_FILE_OFFICE_JP;
    }
    if (is_gtfs_file_exist(GTFS_FILE_STOP_TIMES)) {
        gtfs_merge_stop_times(g_gtfs->stop_times_tbl, _mrg_gtfs->stop_times_tbl, m->prefix);
        _mrg_gtfs->file_exist_bits |= GTFS_FILE_STOP_TIMES;
    }
    if (is_gtfs_file_exist(GTFS_FILE_CALENDAR)) {
        gtfs_merge_calendar(g_gtfs->calendar_tbl, _mrg_gtfs->calendar_tbl, m->prefix);
        _mrg_gtfs->file_exist_bits |= GTFS_FILE_CALENDAR;
    }
    if (is_gtfs_file_exist(GTFS_FILE_CALENDAR_DATES)) {
        gtfs_merge_calendar_dates(g_gtfs->calendar_dates_tbl, _mrg_gtfs->calendar_dates_tbl, m->prefix);
        _mrg_gtfs->file_exist_bits |= GTFS_FILE_CALENDAR_DATES;
    }
    if (is_gtfs_file_exist(GTFS_FILE_FARE_ATTRIBUTES)) {
        gtfs_merge_fare_attributes(g_gtfs->fare_attrs_tbl, _mrg_gtfs->fare_attrs_tbl, m->prefix);
        _mrg_gtfs->file_exist_bits |= GTFS_FILE_FARE_ATTRIBUTES;
    }
    if (is_gtfs_file_exist(GTFS_FILE_FARE_RULES)) {
        gtfs_merge_fare_rules(g_gtfs->fare_rules_tbl, _mrg_gtfs->fare_rules_tbl, m->prefix);
        _mrg_gtfs->file_exist_bits |= GTFS_FILE_FARE_RULES;
    }
    if (is_gtfs_file_exist(GTFS_FILE_SHAPES)) {
        gtfs_merge_shapes(g_gtfs->shapes_tbl, _mrg_gtfs->shapes_tbl, m->prefix);
        _mrg_gtfs->file_exist_bits |= GTFS_FILE_SHAPES;
    }
    if (is_gtfs_file_exist(GTFS_FILE_FREQUENCIES)) {
        gtfs_merge_frequencies(g_gtfs->frequencies_tbl, _mrg_gtfs->frequencies_tbl, m->prefix);
        _mrg_gtfs->file_exist_bits |= GTFS_FILE_FREQUENCIES;
    }
    if (is_gtfs_file_exist(GTFS_FILE_TRANSFERS)) {
        gtfs_merge_transfers(g_gtfs->transfers_tbl, _mrg_gtfs->transfers_tbl, m->prefix);
        _mrg_gtfs->file_exist_bits |= GTFS_FILE_TRANSFERS;
    }
    if (is_gtfs_file_exist(GTFS_FILE_TRANSLATIONS)) {
        gtfs_merge_translations(g_gtfs->translations_tbl, _mrg_gtfs->translations_tbl, m->prefix);
        _mrg_gtfs->file_exist_bits |= GTFS_FILE_TRANSLATIONS;
    }

    if (index == 0) {
        // 初回のみ設定を試みる（configで設定されていれば無視される）
        set_default_agency();
        set_default_agency_jp();
    }

    gtfs_free(g_gtfs, 1);
    return 0;
}

static void set_default_feed_info()
{
    if (is_gtfs_file_exist(GTFS_FILE_FEED_INFO)) {
        if (strlen(g_merged_feed_info.feed_publisher_name) > 0)
            strcpy(g_feed_info.feed_publisher_name, g_merged_feed_info.feed_publisher_name);

        if (strlen(g_merged_feed_info.feed_publisher_url) > 0)
            strcpy(g_feed_info.feed_publisher_url, g_merged_feed_info.feed_publisher_url);

        if (strlen(g_merged_feed_info.feed_lang) > 0)
            strcpy(g_feed_info.feed_lang, g_merged_feed_info.feed_lang);

        if (strlen(g_merged_feed_info.feed_start_date) > 0)
            strcpy(g_feed_info.feed_start_date, g_merged_feed_info.feed_start_date);

        if (strlen(g_merged_feed_info.feed_end_date) > 0)
            strcpy(g_feed_info.feed_end_date, g_merged_feed_info.feed_end_date);

        if (strlen(g_merged_feed_info.feed_version) > 0)
            strcpy(g_feed_info.feed_version, g_merged_feed_info.feed_version);
    }
}

int gtfs_merge()
{
    int count, i;
    struct agency_t* agency;
    struct agency_jp_t* agency_jp;

    count = vect_count(g_merge_gtfs_tbl);
    if (count < 2) {
        err_write("マージする場合は複数のGTFS(JP)をconfigファイルで設定してください。\n");
        return -1;
    }
    _mrg_gtfs = gtfs_alloc();
    _mrg_translations_htbl = hash_initialize(1009);

    for (i = 0; i < count; i++) {
        struct merge_gtfs_prefix_t* m;

        m = (struct merge_gtfs_prefix_t*)vect_get(g_merge_gtfs_tbl, i);
        TRACE("start:(%s)%s\n", m->prefix, m->gtfs_file_name);
        gtfs_merge_one(m, i);
        TRACE("%s\n", "ended.");
    }

    // agency.txt, agency_jp.txt
    if (strlen(g_merged_agency.agency_id) == 0) {
        err_write("agency_idが設定されていません。configファイルで設定してください。\n");
        return -1;
    }
    agency = malloc(sizeof(struct agency_t));
    agency_jp = malloc(sizeof(struct agency_jp_t));
    memcpy(agency, &g_merged_agency, sizeof(struct agency_t));
    memcpy(agency_jp, &g_merged_agency_jp, sizeof(struct agency_jp_t));
    strcpy(agency_jp->agency_id, agency->agency_id);
    vect_append(_mrg_gtfs->agency_tbl, agency);
    vect_append(_mrg_gtfs->agency_jp_tbl, agency_jp);
    _mrg_gtfs->file_exist_bits |= (GTFS_FILE_AGENCY | GTFS_FILE_AGENCY_JP);

    // feed_info.txt (g_feed_info)
    set_default_feed_info();
    _mrg_gtfs->file_exist_bits |= GTFS_FILE_FEED_INFO;

    makedir(g_merged_output_dir);

    gtfs_feed_writer(g_merged_output_dir, _mrg_gtfs);

    if (strlen(g_merged_gtfs_name) < 1) {
        char datebuf[16];

        snprintf(g_merged_gtfs_name, sizeof(g_merged_gtfs_name), "merged_gtfs_%s.zip",
                 todays_date(datebuf, sizeof(datebuf), ""));
    }
    gtfs_zip_archive_writer(g_merged_output_dir, g_merged_gtfs_name, _mrg_gtfs);
    gtfs_feed_delete(g_merged_output_dir, _mrg_gtfs);

    hash_finalize(_mrg_translations_htbl);
    gtfs_free(_mrg_gtfs, 1);
    return 0;
}
