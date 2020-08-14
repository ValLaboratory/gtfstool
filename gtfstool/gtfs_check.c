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

static const char* get_stop_name(const char* stop_id)
{
    struct stop_t* stop;

    stop = (struct stop_t*)hash_get(g_gtfs_hash->stops_htbl, stop_id);
    if (stop && strlen(stop->parent_station) > 0)
        stop = (struct stop_t*)hash_get(g_gtfs_hash->stops_htbl, stop->parent_station);
    return (stop)? stop->stop_name : stop_id;
}

// stops.txtのzone_idが定義されているか調べます。
// 定義されていない場合は無料と見なし、fare_attributes.txtとfare_rule.txtが無くてもOKとします。
static int gtfs_is_free_bus()
{
    int is_free = 1;
    int count, i;

    count = vect_count(g_gtfs->stops_tbl);
    for (i = 0; i < count; i++) {
        struct stop_t* stop;

        stop = (struct stop_t*)vect_get(g_gtfs->stops_tbl, i);
        if (atoi(stop->location_type) == 0) {
            if (strlen(stop->zone_id) != 0) {
                is_free = 0;
                break;
            }
        }
    }
    return is_free;
}

static int gtfs_file_exist_check(struct gtfs_t* gtfs)
{
    int result = 0;

    if (! (is_gtfs_file_exist(gtfs, GTFS_FILE_AGENCY))) {
        int ret = gtfs_error("agency.txtが存在していません。agency.txtは必須ファイルです。");
        if (ret < result)
            result = ret;
    }
    if (! (is_gtfs_file_exist(gtfs, GTFS_FILE_STOPS))) {
        int ret = gtfs_error("stops.txtが存在していません。stops.txtは必須ファイルです。");
        if (ret < result)
            result = ret;
    }
    if (! (is_gtfs_file_exist(gtfs, GTFS_FILE_ROUTES))) {
        int ret = gtfs_error("routes.txtが存在していません。routes.txtは必須ファイルです。");
        if (ret < result)
            result = ret;
    }
    if (! (is_gtfs_file_exist(gtfs, GTFS_FILE_TRIPS))) {
        int ret = gtfs_error("trips.txtが存在していません。trips.txtは必須ファイルです。");
        if (ret < result)
            result = ret;
    }
    if (! (is_gtfs_file_exist(gtfs, GTFS_FILE_STOP_TIMES))) {
        int ret = gtfs_error("stop_times.txtが存在していません。stop_times.txtは必須ファイルです。");
        if (ret < result)
            result = ret;
    }
    if (! (is_gtfs_file_exist(gtfs, GTFS_FILE_CALENDAR))) {
        int ret = gtfs_error("calendar.txtが存在していません。calendar.txtは必須ファイルです。");
        if (ret < result)
            result = ret;
    }
    if (! (is_gtfs_file_exist(gtfs, GTFS_FILE_FEED_INFO))) {
        int ret = gtfs_error("feed_info.txtが存在していません。feed_info.txtは必須ファイルです。");
        if (ret < result)
            result = ret;
    }
    if (! (is_gtfs_file_exist(gtfs, GTFS_FILE_FARE_ATTRIBUTES))) {
        if (! g_is_free_bus) {
            int ret = gtfs_error("fare_attributes.txtが存在していません。有料の場合は必須となります。");
            if (ret < result)
                result = ret;
        }
    }
    if (! (is_gtfs_file_exist(gtfs, GTFS_FILE_FARE_RULES))) {
        if (! g_is_free_bus) {
            // fare_attributes.txtが1行の場合は均一運賃としてfare_rules.txtは省略可能とします。
            if (vect_count(g_gtfs->fare_attrs_tbl) != 1) {
                int ret = gtfs_error("fare_rules.txtが存在していません。有料の場合は必須となります。");
                if (ret < result)
                    result = ret;
            }
        }
    }
    if (! (is_gtfs_file_exist(gtfs, GTFS_FILE_TRANSLATIONS))) {
        int ret = gtfs_error("translations.txtが存在していません。「駅すぱあと」では停留所・標柱名称の「よみがな」が必要となります。");
        if (ret < result)
            result = ret;
    }
    return result;
}

static int is_gtfs_file_label(const char* label_line, const char* first_label)
{
    char label[256];
    char* ptr;

    strncpy(label, label_line, sizeof(label));
    ptr = quote((trim(label)));
    return (memcmp(ptr, first_label, strlen(first_label)) == 0)? 1 : 0;
}

static int gtfs_file_label_check()
{
    int result = 0;

    if (is_gtfs_file_exist(g_gtfs, GTFS_FILE_AGENCY)) {
        if (! is_gtfs_file_label(g_gtfs_label.agency, "agency_id")) {
            int ret = gtfs_error("agency.txtの先頭行にラベル行が存在していません。");
            if (ret < result)
                result = ret;
        }
    }
    if (is_gtfs_file_exist(g_gtfs, GTFS_FILE_STOPS)) {
        if (! is_gtfs_file_label(g_gtfs_label.stops, "stop_id")) {
            int ret = gtfs_error("stops.txtの先頭行にラベル行が存在していません。");
            if (ret < result)
                result = ret;
        }
    }
    if (is_gtfs_file_exist(g_gtfs, GTFS_FILE_ROUTES)) {
        if (! is_gtfs_file_label(g_gtfs_label.routes, "route_id")) {
            int ret = gtfs_error("routes.txtの先頭行にラベル行が存在していません。");
            if (ret < result)
                result = ret;
        }
    }
    if (is_gtfs_file_exist(g_gtfs, GTFS_FILE_TRIPS)) {
        if (! is_gtfs_file_label(g_gtfs_label.routes, "route_id")) {
            int ret = gtfs_error("trips.txtの先頭行にラベル行が存在していません。");
            if (ret < result)
                result = ret;
        }
    }
    if (is_gtfs_file_exist(g_gtfs, GTFS_FILE_STOP_TIMES)) {
        if (! is_gtfs_file_label(g_gtfs_label.stop_times, "trip_id")) {
            int ret = gtfs_error("stop_times.txtの先頭行にラベル行が存在していません。");
            if (ret < result)
                result = ret;
        }
    }
    if (is_gtfs_file_exist(g_gtfs, GTFS_FILE_CALENDAR)) {
        if (! is_gtfs_file_label(g_gtfs_label.calendar, "service_id")) {
            int ret = gtfs_error("calendar.txtの先頭行にラベル行が存在していません。");
            if (ret < result)
                result = ret;
        }
    }
    if (is_gtfs_file_exist(g_gtfs, GTFS_FILE_CALENDAR_DATES)) {
        if (! is_gtfs_file_label(g_gtfs_label.calendar_dates, "service_id")) {
            int ret = gtfs_error("calendar_dates.txtの先頭行にラベル行が存在していません。");
            if (ret < result)
                result = ret;
        }
    }
    if (is_gtfs_file_exist(g_gtfs, GTFS_FILE_FEED_INFO)) {
        if (! is_gtfs_file_label(g_gtfs_label.feed_info, "feed_publisher_name")) {
            int ret = gtfs_error("feed_info.txtの先頭行にラベル行が存在していません。");
            if (ret < result)
                result = ret;
        }
    }
    if (is_gtfs_file_exist(g_gtfs, GTFS_FILE_FARE_ATTRIBUTES)) {
        if (! is_gtfs_file_label(g_gtfs_label.fare_attributes, "fare_id")) {
            int ret = gtfs_error("fare_attributes.txtの先頭行にラベル行が存在していません。");
            if (ret < result)
                result = ret;
        }
    }
    if (is_gtfs_file_exist(g_gtfs, GTFS_FILE_FARE_RULES)) {
        if (! is_gtfs_file_label(g_gtfs_label.fare_rules, "fare_id")) {
            int ret = gtfs_error("fare_rules.txtの先頭行にラベル行が存在していません。");
            if (ret < result)
                result = ret;
        }
    }
    if (is_gtfs_file_exist(g_gtfs, GTFS_FILE_TRANSLATIONS)) {
        if (! is_gtfs_file_label(g_gtfs_label.translations, "trans_id")) {
            int ret = gtfs_error("translations.txtの先頭行にラベル行が存在していません。");
            if (ret < result)
                result = ret;
        }
    }
    if (is_gtfs_file_exist(g_gtfs, GTFS_FILE_ROUTES_JP)) {
        if (! is_gtfs_file_label(g_gtfs_label.routes_jp, "route_id")) {
            int ret = gtfs_error("routes_jp.txtの先頭行にラベル行が存在していません。");
            if (ret < result)
                result = ret;
        }
    }
    return result;

}

// fare_rule_t のキーを作成
char* fare_rule_key(const char* route_id, const char* origin_id, const char* dest_id, char* key)
{
    strcpy(key, route_id);
    if (origin_id && dest_id) {
        if (*origin_id && *dest_id) {
            strcat(key, "/");
            strcat(key, origin_id);
            strcat(key, "/");
            strcat(key, dest_id);
        }
    }
    return key;
}

// キーの重複チェック
static int gtfs_hash_agency_table()
{
    int result = 0;
    int count, i;
    
    count = vect_count(g_gtfs->agency_tbl);
    for (i = 0; i < count; i++) {
        struct agency_t* agency;
        
        agency = (struct agency_t*)vect_get(g_gtfs->agency_tbl, i);
        if (strlen(agency->agency_id) > 0) {
            struct agency_t* agency2;
            
            agency2 = hash_get(g_gtfs_hash->agency_htbl, agency->agency_id);
            if (agency2) {
                int ret = gtfs_warning("agency.txtの%d行目のagency_id[%s]は%d行目にすでに登録済みです。",
                                       agency->lineno,
                                       utf8_conv(agency->agency_id, (char*)alloca(256), 256),
                                       agency2->lineno);
                if (ret < result)
                    result = ret;
            } else {
                hash_put(g_gtfs_hash->agency_htbl, agency->agency_id, agency);
            }
        }
    }
    return result;
}

static int gtfs_hash_routes_table()
{
    int result = 0;
    int count, i;
    
    count = vect_count(g_gtfs->routes_tbl);
    for (i = 0; i < count; i++) {
        struct route_t* route;
        
        route = (struct route_t*)vect_get(g_gtfs->routes_tbl, i);
        if (strlen(route->route_id) > 0) {
            struct route_t* route2;

            route2 = hash_get(g_gtfs_hash->routes_htbl, route->route_id);
            if (route2) {
                int ret = gtfs_warning("routes.txtの%d行目のroute_id[%s]は%d行目にすでに登録済みです。",
                                       route->lineno,
                                       utf8_conv(route->route_id, (char*)alloca(256), 256),
                                       route2->lineno);
                if (ret < result)
                    result = ret;
            } else {
                hash_put(g_gtfs_hash->routes_htbl, route->route_id, route);
            }
        }
    }
    return result;
}

static int gtfs_hash_stops_table()
{
    int result = 0;
    int count, i;
    
    count = vect_count(g_gtfs->stops_tbl);
    for (i = 0; i < count; i++) {
        struct stop_t* stop;
        
        stop = (struct stop_t*)vect_get(g_gtfs->stops_tbl, i);
        if (strlen(stop->stop_id) > 0) {
            struct stop_t* stop2;

            stop2 = hash_get(g_gtfs_hash->stops_htbl, stop->stop_id);
            if (stop2) {
                int ret = gtfs_warning("stops.txtの%d行目のstop_id[%s]は%d行目にすでに登録済みです。",
                                       stop->lineno,
                                       utf8_conv(stop->stop_id, (char*)alloca(256), 256),
                                       stop2->lineno);
                if (ret < result)
                    result = ret;
            } else {
                hash_put(g_gtfs_hash->stops_htbl, stop->stop_id, stop);
            }
        }
    }
    return result;
}

static int gtfs_hash_trips_table()
{
    int result = 0;
    int count, i;
    
    count = vect_count(g_gtfs->trips_tbl);
    for (i = 0; i < count; i++) {
        struct trip_t* trip;
        
        trip = (struct trip_t*)vect_get(g_gtfs->trips_tbl, i);
        if (strlen(trip->trip_id) > 1) {
            struct trip_t* trip2;

            trip2 = hash_get(g_gtfs_hash->trips_htbl, trip->trip_id);
            if (trip2) {
                int ret = gtfs_warning("trips.txtの%d行目のtrip_id[%s]は%d行目にすでに登録済みです。",
                                       trip->lineno,
                                       utf8_conv(trip->trip_id, (char*)alloca(256), 256),
                                       trip2->lineno);
                if (ret < result)
                    result = ret;
            } else {
                hash_put(g_gtfs_hash->trips_htbl, trip->trip_id, trip);
            }
        }
    }
    return result;
}

static int gtfs_hash_calendar_table()
{
    int result = 0;
    int count, i;
    
    count = vect_count(g_gtfs->calendar_tbl);
    for (i = 0; i < count; i++) {
        struct calendar_t* cal;
        
        cal = (struct calendar_t*)vect_get(g_gtfs->calendar_tbl, i);
        if (strlen(cal->service_id) > 0) {
            struct calendar_t* cal2;

            cal2 = hash_get(g_gtfs_hash->calendar_htbl, cal->service_id);
            if (cal2) {
                int ret = gtfs_warning("calendar.txtの%d行目のservice_id[%s]は%d行目にすでに登録済みです。",
                                       cal->lineno,
                                       utf8_conv(cal->service_id, (char*)alloca(256), 256),
                                       cal2->lineno);
                if (ret < result)
                    result = ret;
            } else {
                hash_put(g_gtfs_hash->calendar_htbl, cal->service_id, cal);
            }
        }
    }
    return result;
}

static int gtfs_hash_calendar_dates_table()
{
    int result = 0;
    int count, i;
    
    count = vect_count(g_gtfs->calendar_dates_tbl);
    for (i = 0; i < count; i++) {
        struct calendar_date_t* cdate;
        
        cdate = (struct calendar_date_t*)vect_get(g_gtfs->calendar_dates_tbl, i);
        // 利用タイプが"1"(運行区分適用)の場合で運行日に関係なく登録
        if (strlen(cdate->service_id) > 0 && strcmp(cdate->exception_type, "1") == 0) {
            if (! hash_get(g_gtfs_hash->calendar_dates_htbl, cdate->service_id)) {
                hash_put(g_gtfs_hash->calendar_dates_htbl, cdate->service_id, cdate);
            }
        }
    }
    return result;
}

static int gtfs_hash_fare_attributes_table()
{
    int result = 0;
    int count, i;
    
    count = vect_count(g_gtfs->fare_attrs_tbl);
    for (i = 0; i < count; i++) {
        struct fare_attribute_t* fattr;
        
        fattr = (struct fare_attribute_t*)vect_get(g_gtfs->fare_attrs_tbl, i);
        if (strlen(fattr->fare_id) > 0) {
            struct fare_attribute_t* fattr2;

            fattr2 = hash_get(g_gtfs_hash->fare_attrs_htbl, fattr->fare_id);
            if (fattr2) {
                int ret = gtfs_warning("fare_attributes.txtの%d行目のfare_id[%s]は%d行目にすでに登録済みです。",
                                       fattr->lineno,
                                       utf8_conv(fattr->fare_id, (char*)alloca(256), 256),
                                       fattr2->lineno);
                if (ret < result)
                    result = ret;
            } else {
                hash_put(g_gtfs_hash->fare_attrs_htbl, fattr->fare_id, fattr);
            }
        }
    }
    return result;
}

static int gtfs_hash_fare_rules_table()
{
    int result = 0;
    int count, i;
    
    count = vect_count(g_gtfs->fare_rules_tbl);
    for (i = 0; i < count; i++) {
        struct fare_rule_t* frule;
        
        frule = (struct fare_rule_t*)vect_get(g_gtfs->fare_rules_tbl, i);
        if (strlen(frule->fare_id) > 0) {
            char hkey[128];
            struct fare_rule_t* frule2;

            fare_rule_key(frule->route_id, frule->origin_id, frule->destination_id, hkey);
            frule2 = hash_get(g_gtfs_hash->fare_rules_htbl, hkey);
            if (frule2) {
                if (strcmp(frule->fare_id, frule2->fare_id) == 0) {
                    int ret;
                    char rid[256], oid[256], did[256];
                    
                    ret = gtfs_warning("fare_rules.txtの%d行目のroute_id[%s],origin_id[%s],destination_id[%s]はすでに%d行目に登録済みです。",
                                       frule->lineno,
                                       utf8_conv(frule->route_id, rid, sizeof(rid)),
                                       utf8_conv(frule->origin_id, oid, sizeof(oid)),
                                       utf8_conv(frule->destination_id, did, sizeof(did)),
                                       frule2->lineno);
                    if (ret < result)
                        result = ret;
                }
            } else {
                hash_put(g_gtfs_hash->fare_rules_htbl, hkey, frule);
            }
        }
    }
    return result;
}

static int gtfs_hash_translations_table()
{
    int result = 0;
    int count, i;
    
    count = vect_count(g_gtfs->translations_tbl);
    for (i = 0; i < count; i++) {
        struct translation_t* trans;
        
        trans = (struct translation_t*)vect_get(g_gtfs->translations_tbl, i);
        if (strlen(trans->trans_id) > 0 && stricmp(trans->lang, "ja-Hrkt") == 0) {
            struct translation_t* trans2;

            trans2 = hash_get(g_gtfs_hash->translations_htbl, trans->trans_id);
            if (trans2) {
                int ret;
                char tid[256];

                ret = gtfs_warning("translations.txtの%d行目のtrans_id[%s]は%d行目にすでに登録済みです。",
                                   trans->lineno,
                                   utf8_conv(trans->trans_id, tid, sizeof(tid)),
                                   trans2->lineno);
                if (ret < result)
                    result = ret;
            } else {
                hash_put(g_gtfs_hash->translations_htbl, trans->trans_id, trans);
            }
        }
    }
    return result;
}

static int gtfs_hash_routes_jp_table()
{
    int result = 0;
    int count, i;
    
    count = vect_count(g_gtfs->routes_jp_tbl);
    for (i = 0; i < count; i++) {
        struct route_jp_t* routejp;
        
        routejp = (struct route_jp_t*)vect_get(g_gtfs->routes_jp_tbl, i);
        if (strlen(routejp->route_id) > 1) {
            struct route_jp_t* rjp2;
            
            rjp2 = hash_get(g_gtfs_hash->routes_jp_htbl, routejp->route_id);
            if (rjp2) {
                int ret;
                char rid[256];

                ret = gtfs_warning("routes_jp.txtの%d行目のroute_id[%s]は%d行目にすでに登録済みです。",
                                   routejp->lineno,
                                   utf8_conv(routejp->route_id, rid, sizeof(rid)),
                                   rjp2->lineno);
                if (ret < result)
                    result = ret;
            } else {
                hash_put(g_gtfs_hash->routes_jp_htbl, routejp->route_id, routejp);
            }
        }
    }
    return result;
}

int gtfs_hash_table_key_check()
{
    int result = 0;
    int ret;

    ret = gtfs_hash_agency_table();
    if (ret < result)
        result = ret;

    ret = gtfs_hash_routes_table();
    if (ret < result)
        result = ret;

    ret = gtfs_hash_stops_table();
    if (ret < result)
        result = ret;

    ret = gtfs_hash_trips_table();
    if (ret < result)
        result = ret;

    ret = gtfs_hash_calendar_table();
    if (ret < result)
        result = ret;

    ret = gtfs_hash_calendar_dates_table();
    if (ret < result)
        result = ret;

    ret = gtfs_hash_fare_attributes_table();
    if (ret < result)
        result = ret;

    ret = gtfs_hash_fare_rules_table();
    if (ret < result)
        result = ret;

    ret = gtfs_hash_translations_table();
    if (ret < result)
        result = ret;

    if (is_gtfs_file_exist(g_gtfs, GTFS_FILE_ROUTES_JP)) {
        ret = gtfs_hash_routes_jp_table();
        if (ret < result)
            result = ret;
    }
    return result;
}

// foreign key check
static struct agency_t* agency_id_check(const char* agency_id)
{
    return (struct agency_t*)hash_get(g_gtfs_hash->agency_htbl, agency_id);
}

static struct stop_t* stop_id_check(const char* stop_id)
{
    return (struct stop_t*)hash_get(g_gtfs_hash->stops_htbl, stop_id);
}

static struct route_t* route_id_check(const char* route_id)
{
    return (struct route_t*)hash_get(g_gtfs_hash->routes_htbl, route_id);
}

static void* service_id_check(const char* service_id)
{
    struct calendar_t* cal;

    cal = (struct calendar_t*)hash_get(g_gtfs_hash->calendar_htbl, service_id);
    if (cal) {
        return cal;
    } else {
        struct calendar_date_t* cdate;

        cdate = (struct calendar_date_t*)hash_get(g_gtfs_hash->calendar_dates_htbl, service_id);
        if (cdate)
            return cdate;
    }
    return NULL;
}

static struct trip_t* trip_id_check(const char* trip_id)
{
    return (struct trip_t*)hash_get(g_gtfs_hash->trips_htbl, trip_id);
}

static struct fare_attribute_t* fare_id_check(const char* fare_id)
{
    return (struct fare_attribute_t*)hash_get(g_gtfs_hash->fare_attrs_htbl, fare_id);
}

static int agency_column_check()
{
    int result = 0;
    int count, i;
    
    count = vect_count(g_gtfs->agency_tbl);
    for (i = 0; i < count; i++) {
        struct agency_t* agency;

        agency = (struct agency_t*)vect_get(g_gtfs->agency_tbl, i);
        if (strlen(agency->agency_id) < 1) {
            int ret = gtfs_error("agency.txtのagency_idは必須項目です。");
            if (ret < result)
                result = ret;
        }
        if (strlen(agency->agency_name) < 1) {
            int ret = gtfs_error("agency.txtのagency_nameは必須項目です。");
            if (ret < result)
                result = ret;
        }
        if (strlen(agency->agency_url) < 1) {
            int ret = gtfs_error("agency.txtのagency_urlは必須項目です。");
            if (ret < result)
                result = ret;
        }
        if (strcmp(agency->agency_timezone, "Asia/Tokyo") != 0) {
            int ret = gtfs_error("agency.txtのagency_timezoneが Asia/Tokyo ではありません。");
            if (ret < result)
                result = ret;
        }
    }
    return result;
}

static int stops_column_check()
{
    int result = 0;
    int count, i;

    count = vect_count(g_gtfs->stops_tbl);
    for (i = 0; i < count; i++) {
        struct stop_t* stop;

        stop = (struct stop_t*)vect_get(g_gtfs->stops_tbl, i);
        if (strlen(stop->stop_id) < 1) {
            int ret = gtfs_error("stops.txtの%d行目のstop_idは必須項目です。", stop->lineno);
            if (ret < result)
                result = ret;
        }
        if (strlen(stop->stop_name) < 1) {
            int ret = gtfs_error("stops.txtの%d行目のstop_nameは必須項目です。", stop->lineno);
            if (ret < result)
                result = ret;
        }
        if (strlen(stop->stop_lat) < 1) {
            int ret = gtfs_error("stops.txtの%d行目のstop_latは必須項目です。", stop->lineno);
            if (ret < result)
                result = ret;
        }
        if (strlen(stop->stop_lon) < 1) {
            int ret = gtfs_error("stops.txtの%d行目のstop_lonは必須項目です。", stop->lineno);
            if (ret < result)
                result = ret;
        }
    }
    return result;
}

static int routes_column_check()
{
    int result = 0;
    int count, i;
    
    count = vect_count(g_gtfs->routes_tbl);
    for (i = 0; i < count; i++) {
        struct route_t* route;
        
        route = (struct route_t*)vect_get(g_gtfs->routes_tbl, i);
        if (strlen(route->route_id) < 1) {
            int ret = gtfs_error("routes.txtの%d行目のroute_idは必須項目です。", route->lineno);
            if (ret < result)
                result = ret;
        }
        if (strlen(route->agency_id) < 1) {
            int ret = gtfs_error("routes.txtの%d行目のagency_idは必須項目です。", route->lineno);
            if (ret < result)
                result = ret;
        }
        if ((strlen(route->route_short_name) < 1) && (strlen(route->route_long_name) < 1)) {
            int ret = gtfs_error("routes.txtの%d行目のroute_short_nameかroute_long_nameのどちらかに設定してください。", route->lineno);
            if (ret < result)
                result = ret;
        }
        if (strlen(route->route_type) < 1) {
            int ret = gtfs_error("routes.txtの%d行目のroute_typeは必須項目です。", route->lineno);
            if (ret < result)
                result = ret;
        }
        // agency_idがagency.txtに登録されているかチェック
        if (! agency_id_check(route->agency_id)) {
            int ret = gtfs_error("route.txtの%d行目のagency_id[%s]がagency.txtに存在していません。",
                                 route->lineno,
                                 utf8_conv(route->agency_id, (char*)alloca(256), 256));
            if (ret < result)
                result = ret;
        }
    }
    return result;
}

static int trips_column_check()
{
    int result = 0;
    int count, i;
    
    count = vect_count(g_gtfs->trips_tbl);
    for (i = 0; i < count; i++) {
        struct trip_t* trip;
        
        trip = (struct trip_t*)vect_get(g_gtfs->trips_tbl, i);
        if (strlen(trip->route_id) < 1) {
            int ret = gtfs_error("trips.txtの%d行目のroute_idは必須項目です。", trip->lineno);
            if (ret < result)
                result = ret;
        }
        if (strlen(trip->service_id) < 1) {
            int ret = gtfs_error("trips.txtの%d行目のservice_idは必須項目です。", trip->lineno);
            if (ret < result)
                result = ret;
        }
        if (strlen(trip->trip_id) < 1) {
            int ret = gtfs_error("trips.txtの%d行目のtrip_idは必須項目です。", trip->lineno);
            if (ret < result)
                result = ret;
        }
        // route_idがroutes.txtに登録されているかチェック
        if (! route_id_check(trip->route_id)) {
            int ret = gtfs_error("trips.txtの%d行目のroute_id[%s]がroutes.txtに存在していません。",
                                 trip->lineno,
                                 utf8_conv(trip->route_id, (char*)alloca(256), 256));
            if (ret < result)
                result = ret;
        }
        // service_idがcalendr.txtかcalendar_dates.txtに登録されているかチェック
        if (! service_id_check(trip->service_id)) {
            int ret = gtfs_error("trips.txtの%d行目のservice_id[%s]がcalendar.txtまたはcalendar_dates.txtに存在していません。",
                                 trip->lineno,
                                 utf8_conv(trip->service_id, (char*)alloca(256), 256));
            if (ret < result)
                result = ret;
        }
    }
    return result;
}

static int stop_times_column_check()
{
    int result = 0;
    int count, i;
    
    count = vect_count(g_gtfs->stop_times_tbl);
    for (i = 0; i < count; i++) {
        struct stop_time_t* st;
        
        st = (struct stop_time_t*)vect_get(g_gtfs->stop_times_tbl, i);
        if (strlen(st->trip_id) < 1) {
            int ret = gtfs_error("stop_times.txtの%d行目のtrip_idは必須項目です。", st->lineno);
            if (ret < result)
                result = ret;
        }
        if (strlen(st->arrival_time) < 1) {
            int ret = gtfs_error("stop_times.txtの%d行目のarrival_timeは必須項目です。", st->lineno);
            if (ret < result)
                result = ret;
        }
        if (strlen(st->departure_time) < 1) {
            int ret = gtfs_error("stop_times.txtの%d行目のdeparture_timeは必須項目です。", st->lineno);
            if (ret < result)
                result = ret;
        }
        if (strlen(st->stop_id) < 1) {
            int ret = gtfs_error("stop_times.txtの%d行目のstop_idは必須項目です。", st->lineno);
            if (ret < result)
                result = ret;
        }
        if (strlen(st->stop_sequence) < 1) {
            int ret = gtfs_error("stop_times.txtの%d行目のstop_sequenceは必須項目です。", st->lineno);
            if (ret < result)
                result = ret;
        }
        // trip_idがtrips.txtに登録されているかチェック
        if (! trip_id_check(st->trip_id)) {
            int ret = gtfs_error("stop_times.txtの%d行目のtrip_id[%s]がtrips.txtに存在していません。",
                                 st->lineno,
                                 utf8_conv(st->trip_id, (char*)alloca(256), 256));
            if (ret < result)
                result = ret;
        }
        // stop_idがstops.txtに登録されているかチェック
        if (! stop_id_check(st->stop_id)) {
            int ret = gtfs_error("stop_times.txtの%d行目のstop_id[%s]がstops.txtに存在していません。",
                                 st->lineno,
                                 utf8_conv(st->stop_id, (char*)alloca(256), 256));
            if (ret < result)
                result = ret;
        }
    }
    return result;
}

static int split_yyyymmdd(const char* yyyymmdd, int* y, int* m, int* d)
{
    char tbuf[16];

    if (strlen(yyyymmdd) != 8)
        return -1;

    memcpy(tbuf, yyyymmdd, 4);
    tbuf[4] = '\0';
    *y = atoi(tbuf);

    memcpy(tbuf, yyyymmdd+4, 2);
    tbuf[2] = '\0';
    *m = atoi(tbuf);

    memcpy(tbuf, yyyymmdd+6, 2);
    tbuf[2] = '\0';
    *d = atoi(tbuf);
    return 0;
}

static int calendar_column_check()
{
    int result = 0;
    int count, i;
    
    count = vect_count(g_gtfs->calendar_tbl);
    for (i = 0; i < count; i++) {
        struct calendar_t* cal;
        int y, m, d;
        
        cal = (struct calendar_t*)vect_get(g_gtfs->calendar_tbl, i);
        if (strlen(cal->service_id) < 1) {
            int ret = gtfs_error("calendar.txtの%d行目のservice_idは必須項目です。", cal->lineno);
            if (ret < result)
                result = ret;
        }
        if (! (strcmp(cal->monday, "0") == 0 || strcmp(cal->monday, "1") == 0)) {
            int ret = gtfs_error("calendar.txtの%d行目のmondayは1か0を指定してください。", cal->lineno);
            if (ret < result)
                result = ret;
        }
        if (! (strcmp(cal->tuesday, "0") == 0 || strcmp(cal->tuesday, "1") == 0)) {
            int ret = gtfs_error("calendar.txtの%d行目のtuesdayは1か0を指定してください。", cal->lineno);
            if (ret < result)
                result = ret;
        }
        if (! (strcmp(cal->wednesday, "0") == 0 || strcmp(cal->wednesday, "1") == 0)) {
            int ret = gtfs_error("calendar.txtの%d行目のwednesdayは1か0を指定してください。", cal->lineno);
            if (ret < result)
                result = ret;
        }
        if (! (strcmp(cal->thursday, "0") == 0 || strcmp(cal->thursday, "1") == 0)) {
            int ret = gtfs_error("calendar.txtの%d行目のthursdayは1か0を指定してください。", cal->lineno);
            if (ret < result)
                result = ret;
        }
        if (! (strcmp(cal->friday, "0") == 0 || strcmp(cal->friday, "1") == 0)) {
            int ret = gtfs_error("calendar.txtの%d行目のfridayは1か0を指定してください。", cal->lineno);
            if (ret < result)
                result = ret;
        }
        if (! (strcmp(cal->saturday, "0") == 0 || strcmp(cal->saturday, "1") == 0)) {
            int ret = gtfs_error("calendar.txtの%d行目のsaturdayは1か0を指定してください。", cal->lineno);
            if (ret < result)
                result = ret;
        }
        if (! (strcmp(cal->sunday, "0") == 0 || strcmp(cal->sunday, "1") == 0)) {
            int ret = gtfs_error("calendar.txtの%d行目のsundayは1か0を指定してください。", cal->lineno);
            if (ret < result)
                result = ret;
        }

        if (split_yyyymmdd(cal->start_date, &y, &m, &d) == 0) {
            if (! is_valid_dates(y, m, d)) {
                int ret = gtfs_error("calendar.txtの%d行目のstart_dateの日付が不正(%s)です。",
                                     cal->lineno,
                                     utf8_conv(cal->start_date, (char*)alloca(256), 256));
                if (ret < result)
                    result = ret;
            }
        } else {
            int ret = gtfs_error("calendar.txtの%d行目のstart_dateの日付(%s)が正しくありません。",
                                 cal->lineno,
                                 utf8_conv(cal->start_date, (char*)alloca(256), 256));
            if (ret < result)
                result = ret;
        }

        if (split_yyyymmdd(cal->end_date, &y, &m, &d) == 0) {
            if (! is_valid_dates(y, m, d)) {
                int ret = gtfs_error("calendar.txtの%d行目のend_dateの日付が不正(%s)です。",
                                     cal->lineno,
                                     utf8_conv(cal->end_date, (char*)alloca(256), 256));
                if (ret < result)
                    result = ret;
            }
        } else {
            int ret = gtfs_error("calendar.txtの%d行目のend_dateの日付(%s)が正しくありません。",
                                 cal->lineno,
                                 utf8_conv(cal->end_date, (char*)alloca(256), 256));
            if (ret < result)
                result = ret;
        }
    }
    return result;
}

static int calendar_dates_column_check()
{
    int result = 0;
    int count, i;
    
    count = vect_count(g_gtfs->calendar_dates_tbl);
    for (i = 0; i < count; i++) {
        struct calendar_date_t* cdate;
        int y, m, d;
        
        cdate = (struct calendar_date_t*)vect_get(g_gtfs->calendar_dates_tbl, i);
        if (strlen(cdate->service_id) < 1) {
            int ret = gtfs_error("calendar_dates.txtの%d行目のservice_idは必須項目です。", cdate->lineno);
            if (ret < result)
                result = ret;
        }
        if (strlen(cdate->date) < 1) {
            int ret = gtfs_error("calendar_dates.txtの%d行目のdateは必須項目です。", cdate->lineno);
            if (ret < result)
                result = ret;
        }
        if (split_yyyymmdd(cdate->date, &y, &m, &d) == 0) {
            if (! is_valid_dates(y, m, d)) {
                int ret = gtfs_error("calendar_dates.txtの%d行目のdateの日付が不正(%s)です。",
                                     cdate->lineno,
                                     utf8_conv(cdate->date, (char*)alloca(256), 256));
                if (ret < result)
                    result = ret;
            }
        } else {
            int ret = gtfs_error("calendar_dates.txtの%d行目のdateの日付(%s)が正しくありません。",
                                 cdate->lineno,
                                 utf8_conv(cdate->date, (char*)alloca(256), 256));
            if (ret < result)
                result = ret;
        }
        if (! (strcmp(cdate->exception_type, "1") == 0 || strcmp(cdate->exception_type, "2") == 0)) {
            int ret = gtfs_error("calendar_dates.txtの%d行目のexception_typeは1か2を指定してください。", cdate->lineno);
            if (ret < result)
                result = ret;
        }

        if (g_calendar_dates_service_id_check) {
            // service_idがcalendr.txtに登録されているかチェック
            if (! hash_get(g_gtfs_hash->calendar_htbl, cdate->service_id)) {
                int ret = gtfs_error("calendar_dates.txtの%d行目のservice_id[%s]がcalendar.txtに存在していません。",
                                     cdate->lineno,
                                     utf8_conv(cdate->service_id, (char*)alloca(256), 256));
                if (ret < result)
                    result = ret;
            }
        }
    }
    return result;
}

static int fare_attributes_column_check()
{
    int result = 0;
    int agency_count;
    int count, i;

    agency_count = vect_count(g_gtfs->agency_tbl);

    count = vect_count(g_gtfs->fare_attrs_tbl);
    for (i = 0; i < count; i++) {
        struct fare_attribute_t* fattr;
        
        fattr = (struct fare_attribute_t*)vect_get(g_gtfs->fare_attrs_tbl, i);
        if (strlen(fattr->fare_id) < 1) {
            int ret = gtfs_error("fare_attributes.txtの%d行目のfare_idは必須項目です。", fattr->lineno);
            if (ret < result)
                result = ret;
        }
        if (strlen(fattr->price) < 1) {
            int ret = gtfs_error("fare_attributes.txtの%d行目のpriceは必須項目です。", fattr->lineno);
            if (ret < result)
                result = ret;
        }
        if (strlen(fattr->currency_type) < 1) {
            int ret = gtfs_error("fare_attributes.txtの%d行目のcurrency_typeにはJPYを設定してください。", fattr->lineno);
            if (ret < result)
                result = ret;
        }
        if (! (strcmp(fattr->payment_method, "0") == 0 || strcmp(fattr->payment_method, "1") == 0)) {
            int ret = gtfs_error("fare_attributes.txtの%d行目のpayment_methodは0か1を指定してください。", fattr->lineno);
            if (ret < result)
                result = ret;
        }
        if (! (strlen(fattr->transfers) == 0 ||
               strcmp(fattr->transfers, "0") == 0 ||
               strcmp(fattr->transfers, "1") == 0 ||
               strcmp(fattr->transfers, "2") == 0)) {
            int ret = gtfs_error("fare_attributes.txtの%d行目のpayment_methodは0か1を指定してください。", fattr->lineno);
            if (ret < result)
                result = ret;
        }
        if (agency_count > 1) {
            if (strlen(fattr->agency_id) < 1) {
                int ret = gtfs_error("fare_attributes.txtの%d行目のagency_idが設定されていません。複数の事業者を設定する場合は必須です。", fattr->lineno);
                if (ret < result)
                    result = ret;
            }
        }
    }
    return result;
}

static int fare_rules_column_check()
{
    int result = 0;
    int count, i;
    
    count = vect_count(g_gtfs->fare_rules_tbl);
    for (i = 0; i < count; i++) {
        struct fare_rule_t* frule;
        
        frule = (struct fare_rule_t*)vect_get(g_gtfs->fare_rules_tbl, i);
        if (strlen(frule->fare_id) < 1) {
            int ret = gtfs_error("fare_rules.txtの%d行目のfare_idは必須項目です。", frule->lineno);
            if (ret < result)
                result = ret;
        }
        // fare_idがfare_attributes.txtに登録されているかチェック
        if (! fare_id_check(frule->fare_id)) {
            int ret = gtfs_error("fare_rules.txtの%d行目のfare_id[%s]がfare_attributes.txtに存在していません。",
                                 frule->lineno,
                                 utf8_conv(frule->fare_id, (char*)alloca(256), 256));
            if (ret < result)
                result = ret;
        }
    }
    return result;
}

static int feed_info_column_check()
{
    int result = 0;
    
    if (strlen(g_feed_info.feed_publisher_name) < 1) {
        int ret = gtfs_error("feed_info.txtのfeed_publisher_nameは必須項目です。");
        if (ret < result)
            result = ret;
    }
    if (strlen(g_feed_info.feed_publisher_url) < 1) {
        int ret = gtfs_error("feed_info.txtのfeed_publisher_urlは必須項目です。");
        if (ret < result)
            result = ret;
    }
    if (strcmp(g_feed_info.feed_lang, "ja") != 0) {
        int ret = gtfs_error("feed_info.txtのfeed_langにはjaを設定してください。");
        if (ret < result)
            result = ret;
    }
    return result;
}

static int translations_column_check()
{
    int result = 0;
    int count, i;
    
    count = vect_count(g_gtfs->translations_tbl);
    for (i = 0; i < count; i++) {
        struct translation_t* tr;
        
        tr = (struct translation_t*)vect_get(g_gtfs->translations_tbl, i);
        if (strlen(tr->trans_id) < 1) {
            int ret = gtfs_error("translations.txtの%d行目のtrans_idは必須項目です。", tr->lineno);
            if (ret < result)
                result = ret;
        }
        if (strlen(tr->translation) < 1) {
            int ret = gtfs_error("translations.txtの%d行目のtranslationは必須項目です。", tr->lineno);
            if (ret < result)
                result = ret;
        }
    }
    return result;
}

static int gtfs_column_exist_check()
{
    int result = 0;

    if (is_gtfs_file_exist(g_gtfs, GTFS_FILE_AGENCY)) {
        int ret = agency_column_check();
        if (ret < result)
            result = ret;
    }
    if (is_gtfs_file_exist(g_gtfs, GTFS_FILE_STOPS)) {
        int ret = stops_column_check();
        if (ret < result)
            result = ret;
    }
    if (is_gtfs_file_exist(g_gtfs, GTFS_FILE_ROUTES)) {
        int ret = routes_column_check();
        if (ret < result)
            result = ret;
    }
    if (is_gtfs_file_exist(g_gtfs, GTFS_FILE_TRIPS)) {
        int ret = trips_column_check();
        if (ret < result)
            result = ret;
    }
    if (is_gtfs_file_exist(g_gtfs, GTFS_FILE_STOP_TIMES)) {
        int ret = stop_times_column_check();
        if (ret < result)
            result = ret;
    }
    if (is_gtfs_file_exist(g_gtfs, GTFS_FILE_CALENDAR)) {
        int ret = calendar_column_check();
        if (ret < result)
            result = ret;
    }
    if (is_gtfs_file_exist(g_gtfs, GTFS_FILE_CALENDAR_DATES)) {
        int ret = calendar_dates_column_check();
        if (ret < result)
            result = ret;
    }
    if (is_gtfs_file_exist(g_gtfs, GTFS_FILE_FARE_ATTRIBUTES)) {
        int ret = fare_attributes_column_check();
        if (ret < result)
            result = ret;
    }
    if (is_gtfs_file_exist(g_gtfs, GTFS_FILE_FARE_RULES)) {
        int ret = fare_rules_column_check();
        if (ret < result)
            result = ret;
    }
    if (is_gtfs_file_exist(g_gtfs, GTFS_FILE_FEED_INFO)) {
        int ret = feed_info_column_check();
        if (ret < result)
            result = ret;
    }
    if (is_gtfs_file_exist(g_gtfs, GTFS_FILE_TRANSLATIONS)) {
        int ret = translations_column_check();
        if (ret < result)
            result = ret;
    }
    return result;
}

static int trip_timetable_put_index(struct vector_t* times, struct stop_time_t* stoptime)
{
    int count, i;
    struct stop_time_t* st;

    count = vect_count(times);
    if (count == 0)
        return -1;

    // 最後のエントリよりも大きければ追加
    st = vect_get(times, count-1);
    if (st) {
        if (atoi(stoptime->stop_sequence) > atoi(st->stop_sequence))
            return -1;
    }

    // 挿入位置を検索
    for (i = 0; i < count; i++) {
        st = vect_get(times, i);
        if (st) {
            if (atoi(stoptime->stop_sequence) < atoi(st->stop_sequence))
                return i;
        }
    }
    return -1;
}

// 通過時刻表を作成
int gtfs_vehicle_timetable()
{
    int result = 0;
    int count, i;
    
    // tripごとに時刻表を作成
    count = vect_count(g_gtfs->trips_tbl);
    for (i = 0; i < count; i++) {
        struct trip_t* trip;
        struct vector_t* trip_timetable;

        trip = (struct trip_t*)vect_get(g_gtfs->trips_tbl, i);
        trip_timetable = vect_initialize(100);
        hash_put(g_vehicle_timetable, trip->trip_id, trip_timetable);
    }

    // stop_times
    count = vect_count(g_gtfs->stop_times_tbl);
    for (i = 0; i < count; i++) {
        struct stop_time_t* st;
        struct vector_t* trip_timetable;
        int index;

        st = (struct stop_time_t*)vect_get(g_gtfs->stop_times_tbl, i);
        trip_timetable = (struct vector_t*)hash_get(g_vehicle_timetable, st->trip_id);
        if (! trip_timetable) {
            err_write("gtfs_vehicle_timetable(): trip_id not found[%s]\n",
                      utf8_conv(st->trip_id, (char*)alloca(256), 256));
            continue;
        }
        index = trip_timetable_put_index(trip_timetable, st);
        if (index < 0)
            vect_append(trip_timetable, st);
        else
            vect_insert(trip_timetable, index, st);
    }
    return result;
}

// 経路ごとの停車パターン(trip)を作成
int gtfs_route_trips()
{
    int result = 0;
    int count, i;

    count = vect_count(g_gtfs->routes_tbl);
    for (i = 0; i < count; i++) {
        struct route_t* route;
        struct vector_t* trips_tbl;
        
        route = (struct route_t*)vect_get(g_gtfs->routes_tbl, i);
        trips_tbl = vect_initialize(100);
        hash_put(g_route_trips_htbl, route->route_id, trips_tbl);
    }

    // trips
    count = vect_count(g_gtfs->trips_tbl);
    for (i = 0; i < count; i++) {
        struct trip_t* trip;
        struct vector_t* trips_tbl;
        
        trip = (struct trip_t*)vect_get(g_gtfs->trips_tbl, i);
        trips_tbl = (struct vector_t*)hash_get(g_route_trips_htbl, trip->route_id);
        if (! trips_tbl) {
            err_write("gtfs_route_trips(): route_id not found[%s]\n",
                      utf8_conv(trip->route_id, (char*)alloca(256), 256));
            continue;
        }
        vect_append(trips_tbl, trip);
    }
    return result;
}

static int gtfs_time_to_seconds(const char* time)
{
    int seconds = 0;
    char strbuf[64];
    const char* p;

    strncpy(strbuf, time, sizeof(strbuf));
    p = strtok(strbuf, ":");
    if (p)
        seconds += atoi(p) * 3600;
    p = strtok(NULL, ":");
    if (p)
        seconds += atoi(p) * 60;
    p = strtok(NULL, ":");
    if (p)
        seconds += atoi(p);
    return seconds;
}

static int gtfs_trips_time_check()
{
    int result = 0;
    char** keylist;
    char** keys;        // trip_id
    
    keylist = keys = hash_keylist(g_vehicle_timetable);
    while (*keys) {
        char* trip_id;
        struct vector_t* trip_timetable;
        int count, i;
        struct stop_time_t* last_st = NULL;
        int last_dept_seconds = 0;
        
        trip_id = *keys;
        trip_timetable = (struct vector_t*)hash_get(g_vehicle_timetable, trip_id);
        
        count = vect_count(trip_timetable);
        for (i = 0; i < count-1; i++) {
            struct stop_time_t* st;
            int asec, dsec;
            
            st = vect_get(trip_timetable, i);
            asec = gtfs_time_to_seconds(st->arrival_time);
            dsec = gtfs_time_to_seconds(st->departure_time);
            if (asec > dsec) {
                int ret = gtfs_error("stop_times.txtの%d行目の出発時刻が到着時刻よりも前になっています。",
                                     st->lineno);
                if (ret < result)
                    result = ret;
            }
            if (last_dept_seconds > dsec) {
                int ret = gtfs_error("stop_times.txtの%d行目の出発時刻が%d行目の出発時刻よりも過去の時刻になっています。",
                                     st->lineno, last_st->lineno);
                if (ret < result)
                    result = ret;
            }
            last_st = st;
            last_dept_seconds = dsec;
        }
        keys++;
    }
    hash_list_free((void**)keylist);
    return result;
}

// tripsの中で停車数が同じである基準となるtripのインデックスを求めます。
int gtfs_trips_base_index(struct vector_t* trips_tbl)
{
    int count, i;
    int* stops_count_array = NULL;
    int* same_stops_array = NULL;
    int base_stops_count = 0;
    int base_index = -1;

    count = vect_count(trips_tbl);
    if (count == 0)
        return -1;

    // 停車数の配列
    stops_count_array = calloc(count, sizeof(int));
    same_stops_array = calloc(count, sizeof(int));

    // 停車数のチェック
    for (i = 0; i < count; i++) {
        struct trip_t* trip;
        struct vector_t* stop_time_tbl;
        int stop_count;

        trip = (struct trip_t*)vect_get(trips_tbl, i);
        stop_time_tbl = (struct vector_t*)hash_get(g_vehicle_timetable, trip->trip_id);
        stop_count = vect_count(stop_time_tbl);
        stops_count_array[i] = stop_count;
    }
    for (i = 0; i < count; i++) {
        int j;

        for (j = 0; j < count; j++) {
            if (stops_count_array[i] == stops_count_array[j])
                same_stops_array[i] = same_stops_array[i] + 1;
        }
    }
    for (i = 0; i < count; i++) {
        if (same_stops_array[i] > base_stops_count) {
            base_stops_count = same_stops_array[i];
            base_index = i;
        }
    }
    free(stops_count_array);
    free(same_stops_array);
    return base_index;
}

int equals_stop_times_stop_id(struct stop_time_t* bst, struct stop_time_t* st)
{
    if (bst && st) {
        return (strcmp(bst->stop_id, st->stop_id) == 0);
    }
    return 0;
}

static int gtfs_route_stop_pattern_check()
{
    int result = 0;
    char** keylist;
    char** keys;        // route_id
    
    keylist = keys = hash_keylist(g_route_trips_htbl);
    while (*keys) {
        char* route_id;
        struct vector_t* trips_tbl;
        int count, i, j;
        struct vector_t* base_stop_time_tbl = NULL;
        int base_stops_count = 0;
        int base_index = -1;

        route_id = *keys;
        trips_tbl = (struct vector_t*)hash_get(g_route_trips_htbl, route_id);

        base_index = gtfs_trips_base_index(trips_tbl);

        count = vect_count(trips_tbl);
        if (count == 0) {
            struct route_t* route = (struct route_t*)hash_get(g_gtfs_hash->routes_htbl, route_id);
            int ret = gtfs_warning("routes.txtの%d行目のroute_id(%s)は使用されていません。",
                                   route->lineno,
                                   utf8_conv(route_id, (char*)alloca(256), 256));
            if (ret < result)
                result = ret;
        }
        if (base_index >= 0) {
            struct trip_t* trip;

            trip = (struct trip_t*)vect_get(trips_tbl, base_index);
            base_stop_time_tbl = (struct vector_t*)hash_get(g_vehicle_timetable, trip->trip_id);
            base_stops_count = vect_count(base_stop_time_tbl);
        }
        for (i = 0; i < count; i++) {
            struct trip_t* trip;
            struct vector_t* stop_time_tbl = NULL;
            int stops_count = 0;

            trip = (struct trip_t*)vect_get(trips_tbl, i);
            stop_time_tbl = (struct vector_t*)hash_get(g_vehicle_timetable, trip->trip_id);
            stops_count = vect_count(stop_time_tbl);
            if (base_stops_count != stops_count) {
                if (! g_route_stop_pattern_valid) {
                    int ret = gtfs_error("route_id(%s):trip(%s)の停車数が違います。GTFS-JPの場合はroute_idを分けて経路情報を作成してください。",
                                         utf8_conv(route_id, (char*)alloca(256), 256),
                                         utf8_conv(trip->trip_id, (char*)alloca(256), 256));
                    if (ret < result)
                        result = ret;
                }
            }
        }
        if (result == GTFS_FATAL_ERROR)
            break;

        for (i = 0; i < count; i++) {
            struct trip_t* trip;
            struct vector_t* stop_time_tbl;
            int stop_count;

            trip = (struct trip_t*)vect_get(trips_tbl, i);
            stop_time_tbl = (struct vector_t*)hash_get(g_vehicle_timetable, trip->trip_id);
            stop_count = vect_count(stop_time_tbl);
            for (j = 0; j < stop_count; j++) {
                struct stop_time_t* bst = NULL;
                struct stop_time_t* st = NULL;

                if (j < base_stops_count)
                    bst = (struct stop_time_t*)vect_get(base_stop_time_tbl, j);
                st = (struct stop_time_t*)vect_get(stop_time_tbl, j);
                if (equals_stop_times_stop_id(bst, st) == 0) {
                    if (! g_route_stop_pattern_valid) {
                        int ret = gtfs_error("route_id(%s):(trip(%s)とtrip(%s))の停車パターンが違います。GTFS-JPの場合はroute_idを分けて経路情報を作成してください。",
                                             utf8_conv(route_id, (char*)alloca(256), 256),
                                             utf8_conv(bst->trip_id, (char*)alloca(256), 256),
                                             utf8_conv(st->trip_id, (char*)alloca(256), 256));
                        if (ret < result)
                            result = ret;
                        break;
                    }
                }
            }
        }
        if (result == GTFS_FATAL_ERROR)
            break;
        keys++;
    }
    hash_list_free((void**)keylist);
    return result;
}

static char* get_zone_id(const char* stop_id)
{
    struct stop_t* stop;
    
    stop = hash_get(g_gtfs_hash->stops_htbl, stop_id);
    if (! stop)
        return NULL;
    return stop->zone_id;
}

static int is_same_stop(const char* origin_stop_id, const char* dest_stop_id)
{
    struct stop_t* origin_stop;
    struct stop_t* dest_stop;

    if (strcmp(origin_stop_id, dest_stop_id) == 0)
        return 1;

    origin_stop = hash_get(g_gtfs_hash->stops_htbl, origin_stop_id);
    dest_stop = hash_get(g_gtfs_hash->stops_htbl, dest_stop_id);
    if (origin_stop == NULL || dest_stop == NULL)
        return 0;

    if (strcmp(origin_stop->stop_name, dest_stop->stop_name) == 0)
        return 1;
    return 0;
}

static int is_pickup_stop(struct stop_time_t* st)
{
    if (strlen(st->pickup_type) > 0) {
        if (atoi(st->pickup_type) != 0)
            return 0;
    }
    return 1;   // 乗車可能
}

static int is_dropoff_stop(struct stop_time_t* st)
{
    if (strlen(st->drop_off_type) > 0) {
        if (atoi(st->drop_off_type) != 0)
            return 0;
    }
    return 1;   // 降車可能
}

static struct fare_rule_t* get_another_fare_rule(struct trip_t* trip,
                                                 const char* origin_stop_id,
                                                 const char* dest_stop_id)
{
    struct stop_t* origin_stop;
    struct stop_t* dest_stop;
    struct stop_t** stop_list;
    struct fare_rule_t* fare_rule = NULL;
    int i, j;

#if 0
    if (strcmp(origin_stop_id, "1001_01") == 0 && strcmp(dest_stop_id, "1006_02") == 0)
        printf("target\n");
#endif

    origin_stop = hash_get(g_gtfs_hash->stops_htbl, origin_stop_id);
    dest_stop = hash_get(g_gtfs_hash->stops_htbl, dest_stop_id);
    if (origin_stop == NULL || dest_stop == NULL)
        return NULL;
    
    stop_list = (struct stop_t**)hash_list(g_gtfs_hash->stops_htbl);

    for (i = 0; stop_list[i]; i++) {
        if (strcmp(origin_stop->stop_name, stop_list[i]->stop_name) == 0) {
            for (j = 0; stop_list[j]; j++) {
                if (strcmp(dest_stop->stop_name, stop_list[j]->stop_name) == 0) {
                    char hkey[128];
                
                    fare_rule_key(trip->route_id, stop_list[i]->zone_id, stop_list[j]->zone_id, hkey);
                    fare_rule = hash_get(g_gtfs_hash->fare_rules_htbl, hkey);
                    if (fare_rule)
                        goto final;
                    // 発着の逆も探す
                    fare_rule_key(trip->route_id, stop_list[j]->zone_id, stop_list[i]->zone_id, hkey);
                    fare_rule = hash_get(g_gtfs_hash->fare_rules_htbl, hkey);
                    if (fare_rule)
                        goto final;
                }
            }
        }
    }

final:
    hash_list_free((void**)stop_list);
    return fare_rule;
}

static char* get_dist_traveled(struct stop_time_t* st)
{
    if (strlen(st->shape_dist_traveled) == 0)
        return "--";
    return st->shape_dist_traveled;
}

// 区間運賃が登録されているかチェック
static int gtfs_od_fare_check()
{
    int result = 0;
    struct hash_t* checked_route_htbl;
    char** keylist;
    char** keys;        // trip_id

    checked_route_htbl = hash_initialize(1009);

    keylist = keys = hash_keylist(g_vehicle_timetable);
    while (*keys) {
        char* trip_id;
        struct trip_t* trip;
        struct vector_t* trip_timetable;
        int count, i;
        struct hash_t* fare_error_htbl;

        trip_id = *keys;
        trip = (struct trip_t*)hash_get(g_gtfs_hash->trips_htbl, trip_id);

        // 便の経路がチェック済みか調べる
        if (hash_get(checked_route_htbl, trip->route_id) != NULL) {
            keys++;
            continue;
        }
        hash_put(checked_route_htbl, trip->route_id, trip);

        fare_error_htbl = hash_initialize(1009);
        trip_timetable = (struct vector_t*)hash_get(g_vehicle_timetable, trip_id);

        count = vect_count(trip_timetable);
        for (i = 0; i < count-1; i++) {
            struct stop_time_t* origin_st;
            char* origin_zone;
            int j;
            int prev_price = 0;
            struct stop_time_t* prev_dest_st = NULL;
            struct hash_t* fare_checked_htbl;

            origin_st = vect_get(trip_timetable, i);
            origin_zone = get_zone_id(origin_st->stop_id);
            fare_checked_htbl = hash_initialize(101);

            for (j = i+1; j < count; j++) {
                struct stop_time_t* dest_st;
                char* dest_zone;
                char hkey[128];
                struct fare_rule_t* fare_rule;
                struct fare_attribute_t* fare_attr;

                dest_st = vect_get(trip_timetable, j);
                dest_zone = get_zone_id(dest_st->stop_id);

                // 乗車/降車が可能か調べる
                if (is_pickup_stop(origin_st) == 0 || is_dropoff_stop(dest_st) == 0)
                    continue;
                // 発着が同じ駅は運賃が登録されていないので無視する（「の」の字路線）
                if (is_same_stop(origin_st->stop_id, dest_st->stop_id))
                    continue;
                // 路線+区間で運賃を検索
                fare_rule_key(trip->route_id, origin_zone, dest_zone, hkey);
                fare_rule = hash_get(g_gtfs_hash->fare_rules_htbl, hkey);
                if (! fare_rule) {
                    // 路線を省略して区間のみで検索
                    fare_rule_key("", origin_zone, dest_zone, hkey);
                    fare_rule = hash_get(g_gtfs_hash->fare_rules_htbl, hkey);
                    if (! fare_rule) {
                        // 均一料金の可能性があるので路線のみで検索
                        fare_rule_key(trip->route_id, "", "", hkey);
                        fare_rule = hash_get(g_gtfs_hash->fare_rules_htbl, hkey);
                    }
                    if (! fare_rule) {
                        // 往路と復路の標柱で別のzone_idが採番されている可能性があるためstop_nameが同じ別のzone_idで検索してみる
                        // また往路と復路のどちらかしか登録されていない場合もあるので発着を入れ替えて検索する
                        fare_rule = get_another_fare_rule(trip, origin_st->stop_id, dest_st->stop_id);
                    }
                }
                if (! fare_rule) {
                    int ret;
                    char rid[256], sname[256], sid[256], oz[256], dsname[256], dsid[256], dz[256];

                    // 未登録エラーが検出済みか調べる（巡回経路の場合に複数回検索されるため）
                    fare_rule_key(trip->route_id, origin_zone, dest_zone, hkey);
                    if (hash_get(fare_error_htbl, hkey))
                        continue;
                    ret = gtfs_error("route_id[%s]の[%s(%s(%s))]-[%s(%s(%s))]区間の運賃がfare_rules.txtに登録されていません。",
                                     utf8_conv(trip->route_id, rid, sizeof(rid)),
                                     utf8_conv(get_stop_name(origin_st->stop_id), sname, sizeof(sname)),
                                     utf8_conv(origin_st->stop_id, sid, sizeof(sid)),
                                     utf8_conv(origin_zone, oz, sizeof(oz)),
                                     utf8_conv(get_stop_name(dest_st->stop_id), dsname, sizeof(dsname)),
                                     utf8_conv(dest_st->stop_id, dsid, sizeof(dsid)),
                                     utf8_conv(dest_zone, dz, sizeof(dz)));
                    if (ret < result)
                        result = ret;
                    hash_put(fare_error_htbl, hkey, "");
                    continue;
                }

                // 一度チェックした区間は無視する（巡回路線のように途中から出発地へ戻ってくる場合の回避）
                if (hash_get(fare_checked_htbl, hkey)) {
                    prev_price = 0;
                    prev_dest_st = NULL;
                    continue;
                }

                fare_attr = hash_get(g_gtfs_hash->fare_attrs_htbl, fare_rule->fare_id);
                if (fare_attr) {
                    int price = atoi(fare_attr->price);
                    if (price < prev_price) {
                        // 運賃が下がっている
                        int ret;
                        char cb1[256], cb2[256], cb3[256], cb4[256], cb5[256], cb6[256];
                        char cb7[256], cb8[256], cb9[256], cb10[256], cb11[256], cb12[256], cb13[256];

                        ret = gtfs_warning("route_id[%s]の[%s(%s(%s))]-[%s(%s(%s))]の運賃(%d)距離(%s)が前区間[%s(%s(%s))]-[%s(%s(%s))]の運賃(%d)距離(%s)より安く設定されています。",
                                           utf8_conv(trip->route_id, cb1, sizeof(cb1)),
                                           utf8_conv(get_stop_name(origin_st->stop_id), cb2, sizeof(cb2)),
                                           utf8_conv(origin_st->stop_id, cb3, sizeof(cb3)),
                                           utf8_conv(get_zone_id(origin_st->stop_id), cb4, sizeof(cb4)),
                                           utf8_conv(get_stop_name(dest_st->stop_id), cb5, sizeof(cb5)),
                                           utf8_conv(dest_st->stop_id, cb6, sizeof(cb6)),
                                           utf8_conv(get_zone_id(dest_st->stop_id), cb7, sizeof(cb7)),
                                           price,
                                           get_dist_traveled(dest_st),
                                           utf8_conv(get_stop_name(origin_st->stop_id), cb8, sizeof(cb8)),
                                           utf8_conv(origin_st->stop_id, cb9, sizeof(cb9)),
                                           utf8_conv(get_zone_id(origin_st->stop_id), cb10, sizeof(cb10)),
                                           utf8_conv(get_stop_name(prev_dest_st->stop_id), cb11, sizeof(cb11)),
                                           utf8_conv(prev_dest_st->stop_id, cb12, sizeof(cb12)),
                                           utf8_conv(get_zone_id(prev_dest_st->stop_id), cb13, sizeof(cb13)),
                                           prev_price,
                                           get_dist_traveled(prev_dest_st));
                        if (ret < result)
                            result = ret;
                    }
                    prev_price = price;
                    prev_dest_st = dest_st;
                }
                hash_put(fare_checked_htbl, hkey, "");
            }
            hash_finalize(fare_checked_htbl);
        }
        hash_finalize(fare_error_htbl);
        keys++;
    }
    hash_list_free((void**)keylist);
    hash_finalize(checked_route_htbl);
    return result;
}

// stop_id が未使用かチェックする
static int is_stopid_unused(const char* stop_id)
{
    int count, i;
    
    count = vect_count(g_gtfs->stop_times_tbl);
    for (i = 0; i < count; i++) {
        struct stop_time_t* st;
        
        st = (struct stop_time_t*)vect_get(g_gtfs->stop_times_tbl, i);
        if (strcmp(stop_id, st->stop_id) == 0)
            return 0;   // used
    }
    return 1;   // unuse
}

static int gtfs_stop_name_yomi_check()
{
    int result = 0;
    int count, i;
    
    count = vect_count(g_gtfs->stops_tbl);
    for (i = 0; i < count; i++) {
        int ret = 0;
        struct stop_t* stop;
        
        stop = (struct stop_t*)vect_get(g_gtfs->stops_tbl, i);
        // 親子関係の場合は子のstop_nameはチェックしない。
        if (strlen(stop->stop_name) > 0 && strlen(stop->parent_station) == 0) {
            int notfound_flag = 0;
            struct translation_t* trans;

            trans = hash_get(g_gtfs_hash->translations_htbl, stop->stop_name);
            if (trans) {
                if (strlen(trans->translation) < 1) {
                    if (! is_stopid_unused(stop->stop_id))
                        notfound_flag = 1;
                }
            } else {
                if (! is_stopid_unused(stop->stop_id))
                    notfound_flag = 1;
            }
            if (notfound_flag) {
                ret = gtfs_error("stops.txtの%d行目の[%s]の読み(ja-Hrkt)がtranslations.txtに存在していません。",
                                 stop->lineno,
                                 utf8_conv(stop->stop_name, (char*)alloca(256), 256));
            }
        }
        if (ret < result)
            result = ret;
    }
    return result;
}

static int gtfs_stop_name_duplicate_check()
{
    int result = 0;
    int count, i;
    struct hash_t* stop_name_htbl;

    count = vect_count(g_gtfs->stops_tbl);
    stop_name_htbl = hash_initialize(count*2);

    for (i = 0; i < count; i++) {
        int ret = 0;
        struct stop_t* stop;
        
        stop = (struct stop_t*)vect_get(g_gtfs->stops_tbl, i);
        if (strlen(stop->stop_name) > 0) {
            char* stop_id;

            stop_id = hash_get(stop_name_htbl, stop->stop_name);
            if (stop_id) {
                if (strcmp(stop->stop_id, stop_id) == 0) {
                    ret = gtfs_error("stops.txtの%d行目のstop_name[%s]が重複しています。",
                                     stop->lineno,
                                     utf8_conv(stop->stop_name, (char*)alloca(256), 256));
                }
/*
                } else {
                    if (atoi(stop->location_type) == 0) {
                        ret = gtfs_warning("stops.txtの%d行目のstop_name[%s]が重複していますが、stop_idが違う標柱のため受け入れ可能です。",
                                        stop->lineno, stop->stop_name);
                    } else {
                        ret = gtfs_warning("stops.txtの%d行目のstop_name[%s]が重複していますが、stop_idが違うため受け入れ可能です。",
                                           stop->lineno, stop->stop_name);
                    }
                }
 */
            } else {
                hash_put(stop_name_htbl, stop->stop_name, stop->stop_id);
            }
        }
        if (ret < result)
            result = ret;
    }
    hash_finalize(stop_name_htbl);
    return result;
}

static int gtfs_route_destination_check(const char* r1_id, const char* r2_id)
{
    if (is_gtfs_file_exist(g_gtfs, GTFS_FILE_ROUTES_JP)) {
        struct route_jp_t* rjp1;
        struct route_jp_t* rjp2;

        rjp1 = hash_get(g_gtfs_hash->routes_jp_htbl, r1_id);
        rjp2 = hash_get(g_gtfs_hash->routes_jp_htbl, r2_id);
        if (strcmp(rjp1->destination_stop, rjp2->destination_stop) == 0)
            return 0;   // 行き先が同じ
        return 1;   // 行き先が違う
    }
    return -1;  // 不明
}

static int gtfs_route_name_duplicate_check()
{
    int result = 0;
    int count, i;
    struct hash_t* route_name_htbl;
    
    count = vect_count(g_gtfs->routes_tbl);
    route_name_htbl = hash_initialize(count*2);

    for (i = 0; i < count; i++) {
        int ret = 0;
        struct route_t* route;
        const char* name = NULL;

        route = (struct route_t*)vect_get(g_gtfs->routes_tbl, i);
        if (strlen(route->route_long_name) > 0)
            name = route->route_long_name;
        else if (strlen(route->route_short_name) > 0)
            name = route->route_short_name;

        if (name == NULL) {
            ret = gtfs_error("routes.txtの%d行目のroute_short_nameとroute_long_nameが空白です。経路名は必ず指定してください。",
                             route->lineno);
        } else {
            struct route_t* hroute;

            hroute = hash_get(route_name_htbl, name);
            if (hroute) {
                if (is_gtfs_file_exist(g_gtfs, GTFS_FILE_ROUTES_JP)) {
                    struct route_jp_t* rjp1;
                    struct route_jp_t* rjp2;

                    rjp1 = hash_get(g_gtfs_hash->routes_jp_htbl, route->route_id);
                    rjp2 = hash_get(g_gtfs_hash->routes_jp_htbl, hroute->route_id);
                    if (rjp1 && rjp2) {
                        int dest_flag;
                        
                        dest_flag = gtfs_route_destination_check(rjp1->route_id, rjp2->route_id);
                        if (dest_flag <= 0) {
                            if (strcmp(route->route_id, hroute->route_id) == 0) {
                                ret = gtfs_error("routes.txtの%d行目の経路名[%s]が重複しています。",
                                                 route->lineno,
                                                 utf8_conv(name, (char*)alloca(256), 256));
                            } else {
                                ret = gtfs_warning("routes.txtの%d行目の経路名[%s]が重複していますが、route_idが違うため別経路とします。",
                                                   route->lineno,
                                                   utf8_conv(name, (char*)alloca(256), 256));
                            }
                        } else {
                            ret = gtfs_warning("routes.txtの%d行目の経路名[%s]が重複していますが、行き先が違うため別経路とします。",
                                               route->lineno,
                                               utf8_conv(name, (char*)alloca(256), 256));
                        }
                    }
                } else {
                    if (strcmp(route->route_id, hroute->route_id) == 0) {
                        ret = gtfs_error("routes.txtの%d行目の経路名[%s]が重複しています。",
                                         route->lineno,
                                         utf8_conv(name, (char*)alloca(256), 256));
                    } else {
                        ret = gtfs_warning("routes.txtの%d行目の経路名[%s]が重複していますが、route_idが違うため別経路とします。",
                                           route->lineno,
                                           utf8_conv(name, (char*)alloca(256), 256));
                    }
                }
            } else {
                hash_put(route_name_htbl, name, route);
            }
        }
        if (ret < result)
            result = ret;
    }
    hash_finalize(route_name_htbl);
    return result;
}

// 均一運賃かどうかを調べる
static int is_flat_rate(const char* route_id, const char* origin_zone, const char* dest_zone)
{
    char hkey[128];
    struct fare_rule_t* fare_rule;

    // 運賃が１件しか登録されていなければ均一運賃とする
    if (vect_count(g_gtfs->fare_attrs_tbl) == 1)
        return 1;

    // 路線+区間で運賃を検索
    fare_rule_key(route_id, origin_zone, dest_zone, hkey);
    fare_rule = hash_get(g_gtfs_hash->fare_rules_htbl, hkey);
    if (fare_rule)
        return 0;   // 距離別運賃

    // 均一料金の可能性があるので路線のみで検索
    fare_rule_key(route_id, "", "", hkey);
    fare_rule = hash_get(g_gtfs_hash->fare_rules_htbl, hkey);
    if (fare_rule)
        return 1;   // 均一運賃
    return -1;  // 不明
}

static int gtfs_trips_stop_id_duplicate_check()
{
    int result = 0;
    struct hash_t* checked_route_htbl;
    char** keylist;
    char** keys;        // trip_id

    checked_route_htbl = hash_initialize(1009);
    keylist = keys = hash_keylist(g_vehicle_timetable);
    while (*keys) {
        char* trip_id;
        struct trip_t* trip;
        struct vector_t* trip_timetable;
        int count, i;
        
        trip_id = *keys;
        trip = (struct trip_t*)hash_get(g_gtfs_hash->trips_htbl, trip_id);
        if (trip == NULL) {
            keys++;
            continue;
        }

        // 便の経路がチェック済みか調べる
        if (hash_get(checked_route_htbl, trip->route_id) != NULL) {
            keys++;
            continue;
        }
        hash_put(checked_route_htbl, trip->route_id, trip);

        trip_timetable = (struct vector_t*)hash_get(g_vehicle_timetable, trip_id);
        
        count = vect_count(trip_timetable);
        for (i = 0; i < count-1; i++) {
            struct stop_time_t* dst;
            const char* origin_zone;
            int ret = 0;
            int j;

            dst = vect_get(trip_timetable, i);
            origin_zone = get_zone_id(dst->stop_id);

            for (j = i+1; j < count; j++) {
                struct stop_time_t* ast;
                const char* dest_zone;

                ast = vect_get(trip_timetable, j);
                if (strcmp(dst->stop_id, ast->stop_id) != 0)
                    continue;
                if (! is_dropoff_stop(ast))
                    continue;   // 降車不可なので無視
                if (i == 0 && j == count-1)
                    continue;   // 始点と終点が同じ場合は巡回経路とみなすので無視

                dest_zone = get_zone_id(ast->stop_id);
                // 無料バスか均一運賃の場合は無視
                if (g_is_free_bus != 1 && is_flat_rate(trip->route_id, origin_zone, dest_zone) != 1) {
                    const char* stop_name;

                    stop_name = get_stop_name(ast->stop_id);
                    ret = gtfs_warning("route_id[%s]の経路で[%s(%s)]に複数回停車します。距離別運賃の場合は最初の[%s]までの運賃が適用されます。",
                                       utf8_conv(trip->route_id, (char*)alloca(256), 256),
                                       utf8_conv(stop_name, (char*)alloca(256), 256),
                                       utf8_conv(ast->stop_id, (char*)alloca(256), 256),
                                       utf8_conv(stop_name, (char*)alloca(256), 256));
                    if (ret < result)
                        result = ret;
                }
            }
        }
        keys++;
    }
    hash_list_free((void**)keylist);
    hash_finalize(checked_route_htbl);
    return result;
}

int gtfs_check()
{
    TRACE("%s\n", "*GTFS(zip)の読み込み*");
    if (gtfs_zip_archive_reader(g_gtfs_zip, g_gtfs) < 0) {
        err_write("gtfs_check: zip_archive_reader error (%s).\n",
                  utf8_conv(g_gtfs_zip, (char*)alloca(256), 256));
        return -1;
    }

    // 無料バスか判定します。
    g_is_free_bus = gtfs_is_free_bus();

    TRACE("%s\n", "*必須ファイルの存在チェック*");
    if (gtfs_file_exist_check(g_gtfs) == GTFS_FATAL_ERROR)
        return -1;

    TRACE("%s\n", "*ファイルの先頭行のラベルをチェック*");
    if (gtfs_file_label_check() == GTFS_FATAL_ERROR)
        return -1;

    TRACE("%s\n", "*キーの重複チェック（ハッシュ化）*");
    if (gtfs_hash_table_key_check() == GTFS_FATAL_ERROR)
        return -1;

    TRACE("%s\n", "*必須項目のチェック*");
    if (gtfs_column_exist_check() == GTFS_FATAL_ERROR)
        return -1;

    TRACE("%s\n", "*通過時刻表の作成*");
    gtfs_vehicle_timetable();

    TRACE("%s\n", "*経路の停車パターンを作成*");
    gtfs_route_trips();

    TRACE("%s\n", "*trips.txtの発着時刻が昇順に並んでいるかチェック*");
    if (gtfs_trips_time_check() == GTFS_FATAL_ERROR)
        return -1;

    TRACE("%s\n", "*経路(route_id)の停車パターンが同じかチェック*");
    if (gtfs_route_stop_pattern_check() == GTFS_FATAL_ERROR)
        return -1;

    if (is_gtfs_file_exist(g_gtfs, GTFS_FILE_FARE_RULES)) {
        // 運賃関係のチェック
        TRACE("%s\n", "*通過時刻表の区間運賃がfare_rules.txtに登録されているかチェック*");
        if (gtfs_od_fare_check() == GTFS_FATAL_ERROR)
            return -1;
    }

    if (is_gtfs_file_exist(g_gtfs, GTFS_FILE_TRANSLATIONS)) {
        TRACE("%s\n", "*stops.txtの読みがtranslations.txtに存在するかチェック*");
        if (gtfs_stop_name_yomi_check() == GTFS_FATAL_ERROR)
            return -1;
    }

    TRACE("%s\n", "*stops.txtのstop_nameに重複がないかチェック*");
    if (gtfs_stop_name_duplicate_check() == GTFS_FATAL_ERROR)
        return -1;

    TRACE("%s\n", "*routes.txtの経路名に重複がないかチェック*");
    if (gtfs_route_name_duplicate_check() == GTFS_FATAL_ERROR)
        return -1;

    TRACE("%s\n", "*stop_times.txtのtrip経路にstop_idの重複がないかチェック*");
    if (gtfs_trips_stop_id_duplicate_check() == GTFS_FATAL_ERROR)
        return -1;

    return 0;
}
