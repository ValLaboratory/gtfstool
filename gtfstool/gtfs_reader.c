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
#include <stdio.h>
#include "base/common.h"
#include "base/file.h"
#include "base/csvfile.h"
#include "gtfs_io.h"
#include "gtfs_var.h"

#define MINIZ_HEADER_FILE_ONLY
#include "miniz.c"

static void check_id_size(int kind, const char* id)
{
    if (strlen(id) > GTFS_ID_SIZE)
        err_write("%s: [%s] size over.\n", g_gtfs_filename[kind], id);
}

int find_label_index(char** label_list, const char* target_label)
{
    if (label_list) {
        int n;
        int i;
        
        n = list_count((const char**)label_list);
        for (i = 0; i < n; i++) {
            char* p = quote(trim(label_list[i]));
            if (stricmp(p, target_label) == 0)
                return i;
        }
    }
    return -1;
}

static void gtfs_agency_reader(const char* csvptr, size_t size, struct gtfs_t* gtfs)
{
    char* ptr;
    char* tp;
    char** label_list;
    int agency_id_index, agency_name_index, agency_url_index, agency_timezone_index;
    int agency_lang_index, agency_phone_index, agency_fare_url_index, agency_email_index;
    int lineno = 0;

    ptr = malloc(size+1);
    memcpy(ptr, csvptr, size);
    ptr[size] = '\0';
    
    tp = strtok(ptr, LF_STR);   // ラベル行
    lineno++;
    strncpy(g_gtfs_label.agency, trim(tp), sizeof(g_gtfs_label.agency));
    label_list = split(tp, COMMA_CHAR);

    agency_id_index = find_label_index(label_list, "agency_id");
    agency_name_index = find_label_index(label_list, "agency_name");
    agency_url_index = find_label_index(label_list, "agency_url");
    agency_timezone_index = find_label_index(label_list, "agency_timezone");
    agency_lang_index = find_label_index(label_list, "agency_lang");
    agency_phone_index = find_label_index(label_list, "agency_phone");
    agency_fare_url_index = find_label_index(label_list, "agency_fare_url");
    agency_email_index = find_label_index(label_list, "agency_email");

    while (tp) {
        tp = strtok(NULL, LF_STR);
        lineno++;
        if (tp && strlen(trim(tp)) > 0) {
            char** list = split(tp, COMMA_CHAR);
            if (list) {
                int n = list_count((const char**)list);
                if (n > 0) {
                    struct agency_t* agency = calloc(1, sizeof(struct agency_t));

                    if (agency_id_index >= 0 && agency_id_index < n) {
                        char* p = quote(trim(list[agency_id_index]));
                        check_id_size(AGENCY, p);
                        strncpy(agency->agency_id, p, sizeof(agency->agency_id));
                    }
                    if (agency_name_index >= 0 && agency_name_index < n) {
                        char* p = quote(trim(list[agency_name_index]));
                        strncpy(agency->agency_name, p, sizeof(agency->agency_name));
                    }
                    if (agency_url_index >= 0 && agency_url_index < n) {
                        char* p = quote(trim(list[agency_url_index]));
                        strncpy(agency->agency_url, p, sizeof(agency->agency_url));
                    }
                    if (agency_timezone_index >= 0 && agency_timezone_index < n) {
                        char* p = quote(trim(list[agency_timezone_index]));
                        strncpy(agency->agency_timezone, p, sizeof(agency->agency_timezone));
                    }
                    if (agency_lang_index >= 0 && agency_lang_index < n) {
                        char* p = quote(trim(list[agency_lang_index]));
                        strncpy(agency->agency_lang, p, sizeof(agency->agency_lang));
                    }
                    if (agency_phone_index >= 0 && agency_phone_index < n) {
                        char* p = quote(trim(list[agency_phone_index]));
                        strncpy(agency->agency_phone, p, sizeof(agency->agency_phone));
                    }
                    if (agency_fare_url_index >= 0 && agency_fare_url_index < n) {
                        char* p = quote(trim(list[agency_fare_url_index]));
                        strncpy(agency->agency_fare_url, p, sizeof(agency->agency_fare_url));
                    }
                    if (agency_email_index >= 0 && agency_email_index < n) {
                        char* p = quote(trim(list[agency_email_index]));
                        strncpy(agency->agency_email, p, sizeof(agency->agency_email));
                    }
                    agency->lineno = lineno;
                    vect_append(gtfs->agency_tbl, agency);
                }
                list_free(list);
            }
        }
    }
    list_free(label_list);
    free(ptr);
}

static void gtfs_agency_jp_reader(const char* csvptr, size_t size, struct gtfs_t* gtfs)
{
    char* ptr;
    char* tp;
    char** label_list;
    int agency_id_index, agency_official_name_index, agency_zip_number_index, agency_address_index;
    int agency_president_pos_index, agency_president_name_index;
    int lineno = 0;
    
    ptr = malloc(size+1);
    memcpy(ptr, csvptr, size);
    ptr[size] = '\0';
    
    tp = strtok(ptr, LF_STR);   // ラベル行
    lineno++;
    strncpy(g_gtfs_label.agency_jp, trim(tp), sizeof(g_gtfs_label.agency_jp));
    label_list = split(tp, COMMA_CHAR);
    
    agency_id_index = find_label_index(label_list, "agency_id");
    agency_official_name_index = find_label_index(label_list, "agency_official_name");
    agency_zip_number_index = find_label_index(label_list, "agency_zip_number");
    agency_address_index = find_label_index(label_list, "agency_address");
    agency_president_pos_index = find_label_index(label_list, "agency_president_pos");
    agency_president_name_index = find_label_index(label_list, "agency_president_name");
    
    while (tp) {
        tp = strtok(NULL, LF_STR);
        lineno++;
        if (tp && strlen(trim(tp)) > 0) {
            char** list = split(tp, COMMA_CHAR);
            if (list) {
                int n = list_count((const char**)list);
                if (n > 0) {
                    struct agency_jp_t* agency_jp = calloc(1, sizeof(struct agency_jp_t));
                    
                    if (agency_id_index >= 0 && agency_id_index < n) {
                        char* p = quote(trim(list[agency_id_index]));
                        check_id_size(AGENCY_JP, p);
                        strncpy(agency_jp->agency_id, p, sizeof(agency_jp->agency_id));
                    }
                    if (agency_official_name_index >= 0 && agency_official_name_index < n) {
                        char* p = quote(trim(list[agency_official_name_index]));
                        strncpy(agency_jp->agency_official_name, p, sizeof(agency_jp->agency_official_name));
                    }
                    if (agency_zip_number_index >= 0 && agency_zip_number_index < n) {
                        char* p = quote(trim(list[agency_zip_number_index]));
                        strncpy(agency_jp->agency_zip_number, p, sizeof(agency_jp->agency_zip_number));
                    }
                    if (agency_address_index >= 0 && agency_address_index < n) {
                        char* p = quote(trim(list[agency_address_index]));
                        strncpy(agency_jp->agency_address, p, sizeof(agency_jp->agency_address));
                    }
                    if (agency_president_pos_index >= 0 && agency_president_pos_index < n) {
                        char* p = quote(trim(list[agency_president_pos_index]));
                        strncpy(agency_jp->agency_president_pos, p, sizeof(agency_jp->agency_president_pos));
                    }
                    if (agency_president_name_index >= 0 && agency_president_name_index < n) {
                        char* p = quote(trim(list[agency_president_name_index]));
                        strncpy(agency_jp->agency_president_name, p, sizeof(agency_jp->agency_president_name));
                    }
                    agency_jp->lineno = lineno;
                    vect_append(gtfs->agency_jp_tbl, agency_jp);
                }
                list_free(list);
            }
        }
    }
    list_free(label_list);
    free(ptr);
}

static void gtfs_stops_reader(const char* csvptr, size_t size, struct gtfs_t* gtfs)
{
    char* ptr;
    char* tp;
    char** label_list;
    int stop_id_index, stop_code_index, stop_name_index, stop_desc_index;
    int stop_lat_index, stop_lon_index, zone_id_index;
    int stop_url_index, location_type_index, parent_station_index;
    int stop_timezone_index, wheelchair_boarding_index;
    int lineno = 0;

    ptr = malloc(size+1);
    memcpy(ptr, csvptr, size);
    ptr[size] = '\0';

    tp = strtok(ptr, LF_STR);   // ラベル行
    lineno++;
    strncpy(g_gtfs_label.stops, trim(tp), sizeof(g_gtfs_label.stops));
    label_list = split(tp, COMMA_CHAR);
    
    stop_id_index = find_label_index(label_list, "stop_id");
    stop_code_index = find_label_index(label_list, "stop_code");
    stop_name_index = find_label_index(label_list, "stop_name");
    stop_desc_index = find_label_index(label_list, "stop_desc");
    stop_lat_index = find_label_index(label_list, "stop_lat");
    stop_lon_index = find_label_index(label_list, "stop_lon");
    zone_id_index = find_label_index(label_list, "zone_id");
    stop_url_index = find_label_index(label_list, "stop_url");
    location_type_index = find_label_index(label_list, "location_type");
    parent_station_index = find_label_index(label_list, "parent_station");
    stop_timezone_index = find_label_index(label_list, "stop_timezone");
    wheelchair_boarding_index = find_label_index(label_list, "wheelchair_boarding");

    while (tp) {
        tp = strtok(NULL, LF_STR);
        lineno++;
        if (tp && strlen(trim(tp)) > 0) {
            char** list = split(tp, COMMA_CHAR);
            if (list) {
                int n = list_count((const char**)list);
                if (n > 0) {
                    struct stop_t* stop = calloc(1, sizeof(struct stop_t));

                    if (stop_id_index >= 0 && stop_id_index < n) {
                        char* p = quote(trim(list[stop_id_index]));
                        check_id_size(STOPS, p);
                        strncpy(stop->stop_id, p, sizeof(stop->stop_id));
                    }
                    if (stop_code_index >= 0 && stop_code_index < n) {
                        char* p = quote(trim(list[stop_code_index]));
                        strncpy(stop->stop_code, p, sizeof(stop->stop_code));
                    }
                    if (stop_name_index >= 0 && stop_name_index < n) {
                        char* p = quote(trim(list[stop_name_index]));
                        strncpy(stop->stop_name, p, sizeof(stop->stop_name));
                    }
                    if (stop_desc_index >= 0 && stop_desc_index < n) {
                        char* p = quote(trim(list[stop_desc_index]));
                        strncpy(stop->stop_desc, p, sizeof(stop->stop_desc));
                    }
                    if (stop_lat_index >= 0 && stop_lat_index < n) {
                        char* p = quote(trim(list[stop_lat_index]));
                        strncpy(stop->stop_lat, p, sizeof(stop->stop_lat));
                    }
                    if (stop_lon_index >= 0 && stop_lon_index < n) {
                        char* p = quote(trim(list[stop_lon_index]));
                        strncpy(stop->stop_lon, p, sizeof(stop->stop_lon));
                    }
                    if (zone_id_index >= 0 && zone_id_index < n) {
                        char* p = quote(trim(list[zone_id_index]));
                        strncpy(stop->zone_id, p, sizeof(stop->zone_id));
                    }
                    if (stop_url_index >= 0 && stop_url_index < n) {
                        char* p = quote(trim(list[stop_url_index]));
                        strncpy(stop->stop_url, p, sizeof(stop->stop_url));
                    }
                    if (location_type_index >= 0 && location_type_index < n) {
                        char* p = quote(trim(list[location_type_index]));
                        strncpy(stop->location_type, p, sizeof(stop->location_type));
                    }
                    if (parent_station_index >= 0 && parent_station_index < n) {
                        char* p = quote(trim(list[parent_station_index]));
                        strncpy(stop->parent_station, p, sizeof(stop->parent_station));
                    }
                    if (stop_timezone_index >= 0 && stop_timezone_index < n) {
                        char* p = quote(trim(list[stop_timezone_index]));
                        strncpy(stop->stop_timezone, p, sizeof(stop->stop_timezone));
                    }
                    if (wheelchair_boarding_index >= 0 && wheelchair_boarding_index < n) {
                        char* p = quote(trim(list[wheelchair_boarding_index]));
                        strncpy(stop->wheelchair_boarding, p, sizeof(stop->wheelchair_boarding));
                    }
                    stop->lineno = lineno;
                    vect_append(gtfs->stops_tbl, stop);
                }
                list_free(list);
            }
        }
    }
    list_free(label_list);
    free(ptr);
}

static void gtfs_routes_reader(const char* csvptr, size_t size, struct gtfs_t* gtfs)
{
    char* ptr;
    char* tp;
    char** label_list;
    int route_id_index, agency_id_index;
    int route_short_name_index, route_long_name_index, route_desc_index;
    int route_type_index, route_url_index, route_color_index, route_text_color_index;
    int jp_parent_route_id_index;
    int lineno = 0;

    ptr = malloc(size+1);
    memcpy(ptr, csvptr, size);
    ptr[size] = '\0';

    tp = strtok(ptr, LF_STR);   // ラベル行
    lineno++;
    strncpy(g_gtfs_label.routes, trim(tp), sizeof(g_gtfs_label.routes));
    label_list = split(tp, COMMA_CHAR);

    route_id_index = find_label_index(label_list, "route_id");
    agency_id_index = find_label_index(label_list, "agency_id");
    route_short_name_index = find_label_index(label_list, "route_short_name");
    route_long_name_index = find_label_index(label_list, "route_long_name");
    route_desc_index = find_label_index(label_list, "route_desc");
    route_type_index = find_label_index(label_list, "route_type");
    route_url_index = find_label_index(label_list, "route_url");
    route_color_index = find_label_index(label_list, "route_color");
    route_text_color_index = find_label_index(label_list, "route_text_color");
    jp_parent_route_id_index = find_label_index(label_list, "jp_parent_route_id");

    while (tp) {
        tp = strtok(NULL, LF_STR);
        lineno++;
        if (tp && strlen(trim(tp)) > 0) {
            char** list = split(tp, COMMA_CHAR);
            if (list) {
                int n = list_count((const char**)list);
                if (n > 0) {
                    struct route_t* route = calloc(1, sizeof(struct route_t));

                    if (route_id_index >= 0 && route_id_index < n) {
                        char* p = quote(trim(list[route_id_index]));
                        check_id_size(ROUTES, p);
                        strncpy(route->route_id, p, sizeof(route->route_id));
                    }
                    if (agency_id_index >= 0 && agency_id_index < n) {
                        char* p = quote(trim(list[agency_id_index]));
                        strncpy(route->agency_id, p, sizeof(route->agency_id));
                    }
                    if (route_short_name_index >= 0 && route_short_name_index < n) {
                        char* p = quote(trim(list[route_short_name_index]));
                        strncpy(route->route_short_name, p, sizeof(route->route_short_name));
                    }
                    if (route_long_name_index >= 0 && route_long_name_index < n) {
                        char* p = quote(trim(list[route_long_name_index]));
                        strncpy(route->route_long_name, p, sizeof(route->route_long_name));
                    }
                    if (route_desc_index >= 0 && route_desc_index < n) {
                        char* p = quote(trim(list[route_desc_index]));
                        strncpy(route->route_desc, p, sizeof(route->route_desc));
                    }
                    if (route_type_index >= 0 && route_type_index < n) {
                        char* p = quote(trim(list[route_type_index]));
                        strncpy(route->route_type, p, sizeof(route->route_type));
                    }
                    if (route_url_index >= 0 && route_url_index < n) {
                        char* p = quote(trim(list[route_url_index]));
                        strncpy(route->route_url, p, sizeof(route->route_url));
                    }
                    if (route_color_index >= 0 && route_color_index < n) {
                        char* p = quote(trim(list[route_color_index]));
                        toupperstr(p);
                        strncpy(route->route_color, p, sizeof(route->route_color));
                    }
                    if (route_text_color_index >= 0 && route_text_color_index < n) {
                        char* p = quote(trim(list[route_text_color_index]));
                        toupperstr(p);
                        strncpy(route->route_text_color, p, sizeof(route->route_text_color));
                    }
                    if (jp_parent_route_id_index >= 0 && jp_parent_route_id_index < n) {
                        char* p = quote(trim(list[jp_parent_route_id_index]));
                        strncpy(route->jp_parent_route_id, p, sizeof(route->jp_parent_route_id));
                    }
                    route->lineno = lineno;
                    vect_append(gtfs->routes_tbl, route);
                }
                list_free(list);
            }
        }
    }
    list_free(label_list);
    free(ptr);
}

static void gtfs_routes_jp_reader(const char* csvptr, size_t size, struct gtfs_t* gtfs)
{
    char* ptr;
    char* tp;
    char** label_list;
    int route_id_index, route_update_index;
    int origin_stop_index, via_stop_index, destination_stop_index;
    int lineno = 0;
    
    ptr = malloc(size+1);
    memcpy(ptr, csvptr, size);
    ptr[size] = '\0';
    
    tp = strtok(ptr, LF_STR);   // ラベル行
    lineno++;
    strncpy(g_gtfs_label.routes_jp, trim(tp), sizeof(g_gtfs_label.routes_jp));
    label_list = split(tp, COMMA_CHAR);
    
    route_id_index = find_label_index(label_list, "route_id");
    route_update_index = find_label_index(label_list, "route_update_date");
    origin_stop_index = find_label_index(label_list, "origin_stop");
    via_stop_index = find_label_index(label_list, "via_stop");
    destination_stop_index = find_label_index(label_list, "destination_stop");
    
    while (tp) {
        tp = strtok(NULL, LF_STR);
        lineno++;
        if (tp && strlen(trim(tp)) > 0) {
            char** list = split(tp, COMMA_CHAR);
            if (list) {
                int n = list_count((const char**)list);
                if (n > 0) {
                    struct route_jp_t* routejp = calloc(1, sizeof(struct route_jp_t));
                    
                    if (route_id_index >= 0 && route_id_index < n) {
                        char* p = quote(trim(list[route_id_index]));
                        check_id_size(ROUTES_JP, p);
                        strncpy(routejp->route_id, p, sizeof(routejp->route_id));
                    }
                    if (route_update_index >= 0 && route_update_index < n) {
                        char* p = quote(trim(list[route_update_index]));
                        strncpy(routejp->route_update_date, p, sizeof(routejp->route_update_date));
                    }
                    if (origin_stop_index >= 0 && origin_stop_index < n) {
                        char* p = quote(trim(list[origin_stop_index]));
                        strncpy(routejp->origin_stop, p, sizeof(routejp->origin_stop));
                    }
                    if (via_stop_index >= 0 && via_stop_index < n) {
                        char* p = quote(trim(list[via_stop_index]));
                        strncpy(routejp->via_stop, p, sizeof(routejp->via_stop));
                    }
                    if (destination_stop_index >= 0 && destination_stop_index < n) {
                        char* p = quote(trim(list[destination_stop_index]));
                        strncpy(routejp->destination_stop, p, sizeof(routejp->destination_stop));
                    }
                    routejp->lineno = lineno;
                    vect_append(gtfs->routes_jp_tbl, routejp);
                }
                list_free(list);
            }
        }
    }
    list_free(label_list);
    free(ptr);
}

static void gtfs_trips_reader(const char* csvptr, size_t size, struct gtfs_t* gtfs)
{
    char* ptr;
    char* tp;
    char** label_list;
    int route_id_index, service_id_index, trip_id_index;
    int trip_headsign_index, trip_short_name_index, direction_id_index;
    int block_id_index, shape_id_index, wheelchair_accessible_index, bikes_allowed_index;
    int jp_trip_desc_index, jp_trip_desc_symbol_index, jp_office_id_index;
    int lineno = 0;

    ptr = malloc(size+1);
    memcpy(ptr, csvptr, size);
    ptr[size] = '\0';

    tp = strtok(ptr, LF_STR);   // ラベル行
    lineno++;
    strncpy(g_gtfs_label.trips, trim(tp), sizeof(g_gtfs_label.trips));
    label_list = split(tp, COMMA_CHAR);

    route_id_index = find_label_index(label_list, "route_id");
    service_id_index = find_label_index(label_list, "service_id");
    trip_id_index = find_label_index(label_list, "trip_id");
    trip_headsign_index = find_label_index(label_list, "trip_headsign");
    trip_short_name_index = find_label_index(label_list, "trip_short_name");
    direction_id_index = find_label_index(label_list, "direction_id");
    block_id_index = find_label_index(label_list, "block_id");
    shape_id_index = find_label_index(label_list, "shape_id");
    wheelchair_accessible_index = find_label_index(label_list, "wheelchair_accessible");
    bikes_allowed_index = find_label_index(label_list, "bikes_allowed");
    jp_trip_desc_index = find_label_index(label_list, "jp_trip_desc");
    jp_trip_desc_symbol_index = find_label_index(label_list, "jp_trip_desc_symbol");
    jp_office_id_index = find_label_index(label_list, "jp_office_id");

    while (tp) {
        tp = strtok(NULL, LF_STR);
        lineno++;
        if (tp && strlen(trim(tp)) > 0) {
            char** list = split(tp, COMMA_CHAR);
            if (list) {
                int n = list_count((const char**)list);
                if (n > 0) {
                    struct trip_t* trip = calloc(1, sizeof(struct trip_t));

                    if (route_id_index >= 0 && route_id_index < n) {
                        char* p = quote(trim(list[route_id_index]));
                        check_id_size(TRIPS, p);
                        strncpy(trip->route_id, p, sizeof(trip->route_id));
                    }
                    if (service_id_index >= 0 && service_id_index < n) {
                        char* p = quote(trim(list[service_id_index]));
                        strncpy(trip->service_id, p, sizeof(trip->service_id));
                    }
                    if (trip_id_index >= 0 && trip_id_index < n) {
                        char* p = quote(trim(list[trip_id_index]));
                        strncpy(trip->trip_id, p, sizeof(trip->trip_id));
                    }
                    if (trip_headsign_index >= 0 && trip_headsign_index < n) {
                        char* p = quote(trim(list[trip_headsign_index]));
                        strncpy(trip->trip_headsign, p, sizeof(trip->trip_headsign));
                    }
                    if (trip_short_name_index >= 0 && trip_short_name_index < n) {
                        char* p = quote(trim(list[trip_short_name_index]));
                        strncpy(trip->trip_short_name, p, sizeof(trip->trip_short_name));
                    }
                    if (direction_id_index >= 0 && direction_id_index < n) {
                        char* p = quote(trim(list[direction_id_index]));
                        strncpy(trip->direction_id, p, sizeof(trip->direction_id));
                    }
                    if (block_id_index >= 0 && block_id_index < n) {
                        char* p = quote(trim(list[block_id_index]));
                        strncpy(trip->block_id, p, sizeof(trip->block_id));
                    }
                    if (shape_id_index >= 0 && shape_id_index < n) {
                        char* p = quote(trim(list[shape_id_index]));
                        strncpy(trip->shape_id, p, sizeof(trip->shape_id));
                    }
                    if (wheelchair_accessible_index >= 0 && wheelchair_accessible_index < n) {
                        char* p = quote(trim(list[wheelchair_accessible_index]));
                        strncpy(trip->wheelchair_accessible, p, sizeof(trip->wheelchair_accessible));
                    }
                    if (bikes_allowed_index >= 0 && bikes_allowed_index < n) {
                        char* p = quote(trim(list[bikes_allowed_index]));
                        strncpy(trip->bikes_allowed, p, sizeof(trip->bikes_allowed));
                    }
                    if (jp_trip_desc_index >= 0 && jp_trip_desc_index < n) {
                        char* p = quote(trim(list[jp_trip_desc_index]));
                        strncpy(trip->jp_trip_desc, p, sizeof(trip->jp_trip_desc));
                    }
                    if (jp_trip_desc_symbol_index >= 0 && jp_trip_desc_symbol_index < n) {
                        char* p = quote(trim(list[jp_trip_desc_symbol_index]));
                        strncpy(trip->jp_trip_desc_symbol, p, sizeof(trip->jp_trip_desc_symbol));
                    }
                    if (jp_office_id_index >= 0 && jp_office_id_index < n) {
                        char* p = quote(trim(list[jp_office_id_index]));
                        strncpy(trip->jp_office_id, p, sizeof(trip->jp_office_id));
                    }
                    trip->lineno = lineno;
                    vect_append(gtfs->trips_tbl, trip);
                }
                list_free(list);
            }
        }
    }
    list_free(label_list);
    free(ptr);
}

static void gtfs_office_jp_reader(const char* csvptr, size_t size, struct gtfs_t* gtfs)
{
    char* ptr;
    char* tp;
    char** label_list;
    int office_id_index, office_name_index, office_url_index, office_phone_index;
    int lineno = 0;
    
    ptr = malloc(size+1);
    memcpy(ptr, csvptr, size);
    ptr[size] = '\0';
    
    tp = strtok(ptr, LF_STR);   // ラベル行
    lineno++;
    strncpy(g_gtfs_label.office_jp, trim(tp), sizeof(g_gtfs_label.office_jp));
    label_list = split(tp, COMMA_CHAR);
    
    office_id_index = find_label_index(label_list, "office_id");
    office_name_index = find_label_index(label_list, "office_name");
    office_url_index = find_label_index(label_list, "office_url");
    office_phone_index = find_label_index(label_list, "office_phone");
    
    while (tp) {
        tp = strtok(NULL, LF_STR);
        lineno++;
        if (tp && strlen(trim(tp)) > 0) {
            char** list = split(tp, COMMA_CHAR);
            if (list) {
                int n = list_count((const char**)list);
                if (n > 0) {
                    struct office_jp_t* officejp = calloc(1, sizeof(struct office_jp_t));
                    
                    if (office_id_index >= 0 && office_id_index < n) {
                        char* p = quote(trim(list[office_id_index]));
                        check_id_size(OFFICE_JP, p);
                        strncpy(officejp->office_id, p, sizeof(officejp->office_id));
                    }
                    if (office_name_index >= 0 && office_name_index < n) {
                        char* p = quote(trim(list[office_name_index]));
                        strncpy(officejp->office_name, p, sizeof(officejp->office_name));
                    }
                    if (office_url_index >= 0 && office_url_index < n) {
                        char* p = quote(trim(list[office_url_index]));
                        strncpy(officejp->office_url, p, sizeof(officejp->office_url));
                    }
                    if (office_phone_index >= 0 && office_phone_index < n) {
                        char* p = quote(trim(list[office_phone_index]));
                        strncpy(officejp->office_phone, p, sizeof(officejp->office_phone));
                    }
                    officejp->lineno = lineno;
                    vect_append(gtfs->office_jp_tbl, officejp);
                }
                list_free(list);
            }
        }
    }
    list_free(label_list);
    free(ptr);
}

static void gtfs_stop_times_reader(const char* csvptr, size_t size, struct gtfs_t* gtfs)
{
    char* ptr;
    char* tp;
    char** label_list;
    int trip_id_index, arrival_time_index, departure_time_index;
    int stop_id_index, stop_sequence_index, stop_headsign_index;
    int pickup_type_index, drop_off_type_index;
    int shape_dist_traveled_index, timepoint_index;
    int lineno = 0;

    ptr = malloc(size+1);
    memcpy(ptr, csvptr, size);
    ptr[size] = '\0';
    
    tp = strtok(ptr, LF_STR);   // ラベル行
    lineno++;
    strncpy(g_gtfs_label.stop_times, trim(tp), sizeof(g_gtfs_label.stop_times));
    label_list = split(tp, COMMA_CHAR);
    
    trip_id_index = find_label_index(label_list, "trip_id");
    arrival_time_index = find_label_index(label_list, "arrival_time");
    departure_time_index = find_label_index(label_list, "departure_time");
    stop_id_index = find_label_index(label_list, "stop_id");
    stop_sequence_index = find_label_index(label_list, "stop_sequence");
    stop_headsign_index = find_label_index(label_list, "stop_headsign");
    pickup_type_index = find_label_index(label_list, "pickup_type");
    drop_off_type_index = find_label_index(label_list, "drop_off_type");
    shape_dist_traveled_index = find_label_index(label_list, "shape_dist_traveled");
    timepoint_index = find_label_index(label_list, "timepoint");

    while (tp) {
        tp = strtok(NULL, LF_STR);
        lineno++;
        if (tp && strlen(trim(tp)) > 0) {
            char** list = split(tp, COMMA_CHAR);
            if (list) {
                int n = list_count((const char**)list);
                if (n > 0) {
                    struct stop_time_t* st = calloc(1, sizeof(struct stop_time_t));
                    
                    if (trip_id_index >= 0 && trip_id_index < n) {
                        char* p = quote(trim(list[trip_id_index]));
                        check_id_size(STOP_TIMES, p);
                        strncpy(st->trip_id, p, sizeof(st->trip_id));
                    }
                    if (arrival_time_index >= 0 && arrival_time_index < n) {
                        char* p = quote(trim(list[arrival_time_index]));
                        strncpy(st->arrival_time, p, sizeof(st->arrival_time));
                    }
                    if (departure_time_index >= 0 && departure_time_index < n) {
                        char* p = quote(trim(list[departure_time_index]));
                        strncpy(st->departure_time, p, sizeof(st->departure_time));
                    }
                    if (stop_id_index >= 0 && stop_id_index < n) {
                        char* p = quote(trim(list[stop_id_index]));
                        strncpy(st->stop_id, p, sizeof(st->stop_id));
                    }
                    if (stop_sequence_index >= 0 && stop_sequence_index < n) {
                        char* p = quote(trim(list[stop_sequence_index]));
                        strncpy(st->stop_sequence, p, sizeof(st->stop_sequence));
                    }
                    if (stop_headsign_index >= 0 && stop_headsign_index < n) {
                        char* p = quote(trim(list[stop_headsign_index]));
                        strncpy(st->stop_headsign, p, sizeof(st->stop_headsign));
                    }
                    if (pickup_type_index >= 0 && pickup_type_index < n) {
                        char* p = quote(trim(list[pickup_type_index]));
                        strncpy(st->pickup_type, p, sizeof(st->pickup_type));
                    }
                    if (drop_off_type_index >= 0 && drop_off_type_index < n) {
                        char* p = quote(trim(list[drop_off_type_index]));
                        strncpy(st->drop_off_type, p, sizeof(st->drop_off_type));
                    }
                    if (shape_dist_traveled_index >= 0 && shape_dist_traveled_index < n) {
                        char* p = quote(trim(list[shape_dist_traveled_index]));
                        strncpy(st->shape_dist_traveled, p, sizeof(st->shape_dist_traveled));
                    }
                    if (timepoint_index >= 0 && timepoint_index < n) {
                        char* p = quote(trim(list[timepoint_index]));
                        strncpy(st->timepoint, p, sizeof(st->timepoint));
                    }
                    st->lineno = lineno;
                    vect_append(gtfs->stop_times_tbl, st);
                }
                list_free(list);
            }
        }
    }
    list_free(label_list);
    free(ptr);
}

static void gtfs_calendar_reader(const char* csvptr, size_t size, struct gtfs_t* gtfs)
{
    char* ptr;
    char* tp;
    char** label_list;
    int service_id_index, monday_index, tuesday_index, wednesday_index;
    int thursday_index, friday_index, saturday_index, sunday_index;
    int start_date_index, end_date_index;
    int lineno = 0;

    ptr = malloc(size+1);
    memcpy(ptr, csvptr, size);
    ptr[size] = '\0';
    
    tp = strtok(ptr, LF_STR);   // ラベル行
    lineno++;
    strncpy(g_gtfs_label.calendar, trim(tp), sizeof(g_gtfs_label.calendar));
    label_list = split(tp, COMMA_CHAR);

    service_id_index = find_label_index(label_list, "service_id");
    monday_index = find_label_index(label_list, "monday");
    tuesday_index = find_label_index(label_list, "tuesday");
    wednesday_index = find_label_index(label_list, "wednesday");
    thursday_index = find_label_index(label_list, "thursday");
    friday_index = find_label_index(label_list, "friday");
    saturday_index = find_label_index(label_list, "saturday");
    sunday_index = find_label_index(label_list, "sunday");
    start_date_index = find_label_index(label_list, "start_date");
    end_date_index = find_label_index(label_list, "end_date");
    
    while (tp) {
        tp = strtok(NULL, LF_STR);
        lineno++;
        if (tp && strlen(trim(tp)) > 0) {
            char** list = split(tp, COMMA_CHAR);
            if (list) {
                int n = list_count((const char**)list);
                if (n > 0) {
                    struct calendar_t* cal = calloc(1, sizeof(struct calendar_t));

                    if (service_id_index >= 0 && service_id_index < n) {
                        char* p = quote(trim(list[service_id_index]));
                        check_id_size(CALENDAR, p);
                        strncpy(cal->service_id, p, sizeof(cal->service_id));
                    }
                    if (monday_index >= 0 && monday_index < n) {
                        char* p = quote(trim(list[monday_index]));
                        strncpy(cal->monday, p, sizeof(cal->monday));
                    }
                    if (tuesday_index >= 0 && tuesday_index < n) {
                        char* p = quote(trim(list[tuesday_index]));
                        strncpy(cal->tuesday, p, sizeof(cal->tuesday));
                    }
                    if (wednesday_index >= 0 && wednesday_index < n) {
                        char* p = quote(trim(list[wednesday_index]));
                        strncpy(cal->wednesday, p, sizeof(cal->wednesday));
                    }
                    if (thursday_index >= 0 && thursday_index < n) {
                        char* p = quote(trim(list[thursday_index]));
                        strncpy(cal->thursday, p, sizeof(cal->thursday));
                    }
                    if (friday_index >= 0 && friday_index < n) {
                        char* p = quote(trim(list[friday_index]));
                        strncpy(cal->friday, p, sizeof(cal->friday));
                    }
                    if (saturday_index >= 0 && saturday_index < n) {
                        char* p = quote(trim(list[saturday_index]));
                        strncpy(cal->saturday, p, sizeof(cal->saturday));
                    }
                    if (sunday_index >= 0 && sunday_index < n) {
                        char* p = quote(trim(list[sunday_index]));
                        strncpy(cal->sunday, p, sizeof(cal->sunday));
                    }
                    if (start_date_index >= 0 && start_date_index < n) {
                        char* p = quote(trim(list[start_date_index]));
                        strncpy(cal->start_date, p, sizeof(cal->start_date));
                    }
                    if (end_date_index >= 0 && end_date_index < n) {
                        char* p = quote(trim(list[end_date_index]));
                        strncpy(cal->end_date, p, sizeof(cal->end_date));
                    }
                    cal->lineno = lineno;
                    vect_append(gtfs->calendar_tbl, cal);
                }
                list_free(list);
            }
        }
    }
    list_free(label_list);
    free(ptr);
}

static void gtfs_calendar_dates_reader(const char* csvptr, size_t size, struct gtfs_t* gtfs)
{
    char* ptr;
    char* tp;
    char** label_list;
    int service_id_index, date_index, exception_type_index;
    int lineno = 0;
    
    ptr = malloc(size+1);
    memcpy(ptr, csvptr, size);
    ptr[size] = '\0';
    
    tp = strtok(ptr, LF_STR);   // ラベル行
    lineno++;
    strncpy(g_gtfs_label.calendar_dates, trim(tp), sizeof(g_gtfs_label.calendar_dates));
    label_list = split(tp, COMMA_CHAR);
    
    service_id_index = find_label_index(label_list, "service_id");
    date_index = find_label_index(label_list, "date");
    exception_type_index = find_label_index(label_list, "exception_type");
    
    while (tp) {
        tp = strtok(NULL, LF_STR);
        lineno++;
        if (tp && strlen(trim(tp)) > 0) {
            char** list = split(tp, COMMA_CHAR);
            if (list) {
                int n = list_count((const char**)list);
                if (n > 0) {
                    struct calendar_date_t* cald = calloc(1, sizeof(struct calendar_date_t));
                    
                    if (service_id_index >= 0 && service_id_index < n) {
                        char* p = quote(trim(list[service_id_index]));
                        check_id_size(CALENDAR_DATES, p);
                        strncpy(cald->service_id, p, sizeof(cald->service_id));
                    }
                    if (date_index >= 0 && date_index < n) {
                        char* p = quote(trim(list[date_index]));
                        strncpy(cald->date, p, sizeof(cald->date));
                    }
                    if (exception_type_index >= 0 && exception_type_index < n) {
                        char* p = quote(trim(list[exception_type_index]));
                        strncpy(cald->exception_type, p, sizeof(cald->exception_type));
                    }
                    cald->lineno = lineno;
                    vect_append(gtfs->calendar_dates_tbl, cald);
                }
                list_free(list);
            }
        }
    }
    list_free(label_list);
    free(ptr);
}

static void gtfs_fare_attributes_reader(const char* csvptr, size_t size, struct gtfs_t* gtfs)
{
    char* ptr;
    char* tp;
    char** label_list;
    int fare_id_index, price_index, currency_type_index, payment_method_index, transfers_index;
    int agency_id_index, transfer_duration_index;
    int lineno = 0;

    ptr = malloc(size+1);
    memcpy(ptr, csvptr, size);
    ptr[size] = '\0';
    
    tp = strtok(ptr, LF_STR);   // ラベル行
    lineno++;
    strncpy(g_gtfs_label.fare_attributes, trim(tp), sizeof(g_gtfs_label.fare_attributes));
    label_list = split(tp, COMMA_CHAR);
    
    fare_id_index = find_label_index(label_list, "fare_id");
    price_index = find_label_index(label_list, "price");
    currency_type_index = find_label_index(label_list, "currency_type");
    payment_method_index = find_label_index(label_list, "payment_method");
    transfers_index = find_label_index(label_list, "transfers");
    agency_id_index = find_label_index(label_list, "agency_id");
    transfer_duration_index = find_label_index(label_list, "transfer_duration");

    while (tp) {
        tp = strtok(NULL, LF_STR);
        lineno++;
        if (tp && strlen(trim(tp)) > 0) {
            char** list = split(tp, COMMA_CHAR);
            if (list) {
                int n = list_count((const char**)list);
                if (n > 0) {
                    struct fare_attribute_t* fattr = calloc(1, sizeof(struct fare_attribute_t));
                    
                    if (fare_id_index >= 0 && fare_id_index < n) {
                        char* p = quote(trim(list[fare_id_index]));
                        check_id_size(FARE_ATTRIBUTES, p);
                        strncpy(fattr->fare_id, p, sizeof(fattr->fare_id));
                    }
                    if (price_index >= 0 && price_index < n) {
                        char* p = quote(trim(list[price_index]));
                        strncpy(fattr->price, p, sizeof(fattr->price));
                    }
                    if (currency_type_index >= 0 && currency_type_index < n) {
                        char* p = quote(trim(list[currency_type_index]));
                        strncpy(fattr->currency_type, p, sizeof(fattr->currency_type));
                    }
                    if (payment_method_index >= 0 && payment_method_index < n) {
                        char* p = quote(trim(list[payment_method_index]));
                        strncpy(fattr->payment_method, p, sizeof(fattr->payment_method));
                    }
                    if (transfers_index >= 0 && transfers_index < n) {
                        char* p = quote(trim(list[transfers_index]));
                        strncpy(fattr->transfers, p, sizeof(fattr->transfers));
                    }
                    if (agency_id_index >= 0 && agency_id_index < n) {
                        char* p = quote(trim(list[agency_id_index]));
                        strncpy(fattr->agency_id, p, sizeof(fattr->agency_id));
                    }
                    if (transfer_duration_index >= 0 && transfer_duration_index < n) {
                        char* p = quote(trim(list[transfer_duration_index]));
                        strncpy(fattr->transfer_duration, p, sizeof(fattr->transfer_duration));
                    }
                    fattr->lineno = lineno;
                    vect_append(gtfs->fare_attrs_tbl, fattr);
                }
                list_free(list);
            }
        }
    }
    list_free(label_list);
    free(ptr);
}

static void gtfs_fare_rules_reader(const char* csvptr, size_t size, struct gtfs_t* gtfs)
{
    char* ptr;
    char* tp;
    char** label_list;
    int fare_id_index, route_id_index, origin_id_index, destination_id_index, contains_id_index;
    int lineno = 0;
    
    ptr = malloc(size+1);
    memcpy(ptr, csvptr, size);
    ptr[size] = '\0';
    
    tp = strtok(ptr, LF_STR);   // ラベル行
    lineno++;
    strncpy(g_gtfs_label.fare_rules, trim(tp), sizeof(g_gtfs_label.fare_rules));
    label_list = split(tp, COMMA_CHAR);
    
    fare_id_index = find_label_index(label_list, "fare_id");
    route_id_index = find_label_index(label_list, "route_id");
    origin_id_index = find_label_index(label_list, "origin_id");
    destination_id_index = find_label_index(label_list, "destination_id");
    contains_id_index = find_label_index(label_list, "contains_id");

    while (tp) {
        tp = strtok(NULL, LF_STR);
        lineno++;
        if (tp && strlen(trim(tp)) > 0) {
            char** list = split(tp, COMMA_CHAR);
            if (list) {
                int n = list_count((const char**)list);
                if (n > 0) {
                    struct fare_rule_t* fare = calloc(1, sizeof(struct fare_rule_t));
                    
                    if (fare_id_index >= 0 && fare_id_index < n) {
                        char* p = quote(trim(list[fare_id_index]));
                        check_id_size(FARE_RULES, p);
                        strncpy(fare->fare_id, p, sizeof(fare->fare_id));
                    }
                    if (route_id_index >= 0 && route_id_index < n) {
                        char* p = quote(trim(list[route_id_index]));
                        strncpy(fare->route_id, p, sizeof(fare->route_id));
                    }
                    if (origin_id_index >= 0 && origin_id_index < n) {
                        char* p = quote(trim(list[origin_id_index]));
                        strncpy(fare->origin_id, p, sizeof(fare->origin_id));
                    }
                    if (destination_id_index >= 0 && destination_id_index < n) {
                        char* p = quote(trim(list[destination_id_index]));
                        strncpy(fare->destination_id, p, sizeof(fare->destination_id));
                    }
                    if (contains_id_index >= 0 && contains_id_index < n) {
                        char* p = quote(trim(list[contains_id_index]));
                        strncpy(fare->contains_id, p, sizeof(fare->contains_id));
                    }
                    fare->lineno = lineno;
                    vect_append(gtfs->fare_rules_tbl, fare);
                }
                list_free(list);
            }
        }
    }
    list_free(label_list);
    free(ptr);
}

static void gtfs_shapes_reader(const char* csvptr, size_t size, struct gtfs_t* gtfs)
{
    char* ptr;
    char* tp;
    char** label_list;
    int shape_id_index, shape_pt_lat_index, shape_pt_lon_index;
    int shape_pt_sequence_index, shape_dist_traveled_index;
    int lineno = 0;
    
    ptr = malloc(size+1);
    memcpy(ptr, csvptr, size);
    ptr[size] = '\0';
    
    tp = strtok(ptr, LF_STR);   // ラベル行
    lineno++;
    strncpy(g_gtfs_label.shapes, trim(tp), sizeof(g_gtfs_label.shapes));
    label_list = split(tp, COMMA_CHAR);
    
    shape_id_index = find_label_index(label_list, "shape_id");
    shape_pt_lat_index = find_label_index(label_list, "shape_pt_lat");
    shape_pt_lon_index = find_label_index(label_list, "shape_pt_lon");
    shape_pt_sequence_index = find_label_index(label_list, "shape_pt_sequence");
    shape_dist_traveled_index = find_label_index(label_list, "shape_dist_traveled");
    
    while (tp) {
        tp = strtok(NULL, LF_STR);
        lineno++;
        if (tp && strlen(trim(tp)) > 0) {
            char** list = split(tp, COMMA_CHAR);
            if (list) {
                int n = list_count((const char**)list);
                if (n > 0) {
                    struct shape_t* shape = calloc(1, sizeof(struct shape_t));
                    
                    if (shape_id_index >= 0 && shape_id_index < n) {
                        char* p = quote(trim(list[shape_id_index]));
                        check_id_size(SHAPES, p);
                        strncpy(shape->shape_id, p, sizeof(shape->shape_id));
                    }
                    if (shape_pt_lat_index >= 0 && shape_pt_lat_index < n) {
                        char* p = quote(trim(list[shape_pt_lat_index]));
                        strncpy(shape->shape_pt_lat, p, sizeof(shape->shape_pt_lat));
                    }
                    if (shape_pt_lon_index >= 0 && shape_pt_lon_index < n) {
                        char* p = quote(trim(list[shape_pt_lon_index]));
                        strncpy(shape->shape_pt_lon, p, sizeof(shape->shape_pt_lon));
                    }
                    if (shape_pt_sequence_index >= 0 && shape_pt_sequence_index < n) {
                        char* p = quote(trim(list[shape_pt_sequence_index]));
                        strncpy(shape->shape_pt_sequence, p, sizeof(shape->shape_pt_sequence));
                    }
                    if (shape_dist_traveled_index >= 0 && shape_dist_traveled_index < n) {
                        char* p = quote(trim(list[shape_dist_traveled_index]));
                        strncpy(shape->shape_dist_traveled, p, sizeof(shape->shape_dist_traveled));
                    }
                    shape->lineno = lineno;
                    vect_append(gtfs->shapes_tbl, shape);
                }
                list_free(list);
            }
        }
    }
    list_free(label_list);
    free(ptr);
}

static void gtfs_frequencies_reader(const char* csvptr, size_t size, struct gtfs_t* gtfs)
{
    char* ptr;
    char* tp;
    char** label_list;
    int trip_id_index, start_time_index, end_time_index, headway_secs_index, exact_times_index;
    int lineno = 0;
    
    ptr = malloc(size+1);
    memcpy(ptr, csvptr, size);
    ptr[size] = '\0';
    
    tp = strtok(ptr, LF_STR);   // ラベル行
    lineno++;
    strncpy(g_gtfs_label.frequencies, trim(tp), sizeof(g_gtfs_label.frequencies));
    label_list = split(tp, COMMA_CHAR);
    
    trip_id_index = find_label_index(label_list, "trip_id");
    start_time_index = find_label_index(label_list, "start_time");
    end_time_index = find_label_index(label_list, "end_time");
    headway_secs_index = find_label_index(label_list, "headway_secs");
    exact_times_index = find_label_index(label_list, "exact_times");
    
    while (tp) {
        tp = strtok(NULL, LF_STR);
        lineno++;
        if (tp && strlen(trim(tp)) > 0) {
            char** list = split(tp, COMMA_CHAR);
            if (list) {
                int n = list_count((const char**)list);
                if (n > 0) {
                    struct frequency_t* freq = calloc(1, sizeof(struct frequency_t));
                    
                    if (trip_id_index >= 0 && trip_id_index < n) {
                        char* p = quote(trim(list[trip_id_index]));
                        check_id_size(FREQUENCIES, p);
                        strncpy(freq->trip_id, p, sizeof(freq->trip_id));
                    }
                    if (start_time_index >= 0 && start_time_index < n) {
                        char* p = quote(trim(list[start_time_index]));
                        strncpy(freq->start_time, p, sizeof(freq->start_time));
                    }
                    if (end_time_index >= 0 && end_time_index < n) {
                        char* p = quote(trim(list[end_time_index]));
                        strncpy(freq->end_time, p, sizeof(freq->end_time));
                    }
                    if (headway_secs_index >= 0 && headway_secs_index < n) {
                        char* p = quote(trim(list[headway_secs_index]));
                        strncpy(freq->headway_secs, p, sizeof(freq->headway_secs));
                    }
                    if (exact_times_index >= 0 && exact_times_index < n) {
                        char* p = quote(trim(list[exact_times_index]));
                        strncpy(freq->exact_times, p, sizeof(freq->exact_times));
                    }
                    freq->lineno = lineno;
                    vect_append(gtfs->frequencies_tbl, freq);
                }
                list_free(list);
            }
        }
    }
    list_free(label_list);
    free(ptr);
}

static void gtfs_transfers_reader(const char* csvptr, size_t size, struct gtfs_t* gtfs)
{
    char* ptr;
    char* tp;
    char** label_list;
    int from_stop_id_index, to_stop_id_index, transfer_type_index, min_transfer_time_index;
    int lineno = 0;
    
    ptr = malloc(size+1);
    memcpy(ptr, csvptr, size);
    ptr[size] = '\0';
    
    tp = strtok(ptr, LF_STR);   // ラベル行
    lineno++;
    strncpy(g_gtfs_label.transfers, trim(tp), sizeof(g_gtfs_label.transfers));
    label_list = split(tp, COMMA_CHAR);
    
    from_stop_id_index = find_label_index(label_list, "from_stop_id");
    to_stop_id_index = find_label_index(label_list, "to_stop_id");
    transfer_type_index = find_label_index(label_list, "transfer_type");
    min_transfer_time_index = find_label_index(label_list, "min_transfer_time");
    
    while (tp) {
        tp = strtok(NULL, LF_STR);
        lineno++;
        if (tp && strlen(trim(tp)) > 0) {
            char** list = split(tp, COMMA_CHAR);
            if (list) {
                int n = list_count((const char**)list);
                if (n > 0) {
                    struct transfer_t* transfer = calloc(1, sizeof(struct transfer_t));
                    
                    if (from_stop_id_index >= 0 && from_stop_id_index < n) {
                        char* p = quote(trim(list[from_stop_id_index]));
                        check_id_size(TRANSFERS, p);
                        strncpy(transfer->from_stop_id, p, sizeof(transfer->from_stop_id));
                    }
                    if (to_stop_id_index >= 0 && to_stop_id_index < n) {
                        char* p = quote(trim(list[to_stop_id_index]));
                        check_id_size(TRANSFERS, p);
                        strncpy(transfer->to_stop_id, p, sizeof(transfer->to_stop_id));
                    }
                    if (transfer_type_index >= 0 && transfer_type_index < n) {
                        char* p = quote(trim(list[transfer_type_index]));
                        strncpy(transfer->transfer_type, p, sizeof(transfer->transfer_type));
                    }
                    if (min_transfer_time_index >= 0 && min_transfer_time_index < n) {
                        char* p = quote(trim(list[min_transfer_time_index]));
                        strncpy(transfer->min_transfer_time, p, sizeof(transfer->min_transfer_time));
                    }
                    transfer->lineno = lineno;
                    vect_append(gtfs->transfers_tbl, transfer);
                }
                list_free(list);
            }
        }
    }
    list_free(label_list);
    free(ptr);
}

static void gtfs_feed_info_reader(const char* csvptr, size_t size, struct gtfs_t* gtfs)
{
    char* ptr;
    char* tp;
    char** label_list;
    int feed_publisher_name_index, feed_publisher_url_index, feed_lang_index;
    int feed_start_date_index, feed_end_date_index, feed_version_index;
    
    ptr = malloc(size+1);
    memcpy(ptr, csvptr, size);
    ptr[size] = '\0';
    
    tp = strtok(ptr, LF_STR);   // ラベル行
    strncpy(g_gtfs_label.feed_info, trim(tp), sizeof(g_gtfs_label.feed_info));
    label_list = split(tp, COMMA_CHAR);
    
    feed_publisher_name_index = find_label_index(label_list, "feed_publisher_name");
    feed_publisher_url_index = find_label_index(label_list, "feed_publisher_url");
    feed_lang_index = find_label_index(label_list, "feed_lang");
    feed_start_date_index = find_label_index(label_list, "feed_start_date");
    feed_end_date_index = find_label_index(label_list, "feed_end_date");
    feed_version_index = find_label_index(label_list, "feed_version");

    while (tp) {
        tp = strtok(NULL, LF_STR);
        if (tp && strlen(trim(tp)) > 0) {
            char** list = split(tp, COMMA_CHAR);
            if (list) {
                int n = list_count((const char**)list);
                if (n > 0) {
                    if (feed_publisher_name_index >= 0 && feed_publisher_name_index < n) {
                        char* p = quote(trim(list[feed_publisher_name_index]));
                        strncpy(g_feed_info.feed_publisher_name, p, sizeof(g_feed_info.feed_publisher_name));
                    }
                    if (feed_publisher_url_index >= 0 && feed_publisher_url_index < n) {
                        char* p = quote(trim(list[feed_publisher_url_index]));
                        strncpy(g_feed_info.feed_publisher_url, p, sizeof(g_feed_info.feed_publisher_url));
                    }
                    if (feed_lang_index >= 0 && feed_lang_index < n) {
                        char* p = quote(trim(list[feed_lang_index]));
                        strncpy(g_feed_info.feed_lang, p, sizeof(g_feed_info.feed_lang));
                    }
                    if (feed_start_date_index >= 0 && feed_start_date_index < n) {
                        char* p = quote(trim(list[feed_start_date_index]));
                        strncpy(g_feed_info.feed_start_date, p, sizeof(g_feed_info.feed_start_date));
                    }
                    if (feed_end_date_index >= 0 && feed_end_date_index < n) {
                        char* p = quote(trim(list[feed_end_date_index]));
                        strncpy(g_feed_info.feed_end_date, p, sizeof(g_feed_info.feed_end_date));
                    }
                    if (feed_version_index >= 0 && feed_version_index < n) {
                        char* p = quote(trim(list[feed_version_index]));
                        strncpy(g_feed_info.feed_version, p, sizeof(g_feed_info.feed_version));
                    }
                }
                list_free(list);
            }
        }
    }
    list_free(label_list);
    free(ptr);
}

static int table_type(const char* table_name)
{
    if (strcmp(table_name, "stops") == 0)
        return STOPS;
    return -1;
}

static void gtfs_translations_reader(const char* csvptr, size_t size, struct gtfs_t* gtfs)
{
    char* ptr;
    char* tp;
    char** label_list;
    int trans_id_index, lang_index, translation_index;
    int table_name_index, field_name_index, record_id_index, record_sub_id_index, field_value_index;
    int lineno = 0;

    ptr = malloc(size+1);
    memcpy(ptr, csvptr, size);
    ptr[size] = '\0';
    
    tp = strtok(ptr, LF_STR);   // ラベル行
    lineno++;
    strncpy(g_gtfs_label.translations, trim(tp), sizeof(g_gtfs_label.translations));
    label_list = split(tp, COMMA_CHAR);
    
    trans_id_index = find_label_index(label_list, "trans_id");
    lang_index = find_label_index(label_list, "lang");
    if (lang_index < 0) {
        // gtfs new version(2020/08/19)
        lang_index = find_label_index(label_list, "language");
    }
    translation_index = find_label_index(label_list, "translation");

    // gtfs new version(2020/08/21)
    table_name_index = find_label_index(label_list, "table_name");
    field_name_index = find_label_index(label_list, "field_name");
    record_id_index = find_label_index(label_list, "record_id");
    record_sub_id_index = find_label_index(label_list, "record_sub_id");
    field_value_index = find_label_index(label_list, "field_value");

    while (tp) {
        tp = strtok(NULL, LF_STR);
        lineno++;
        if (tp && strlen(trim(tp)) > 0) {
            char** list = split(tp, COMMA_CHAR);
            if (list) {
                int n = list_count((const char**)list);
                if (n > 0) {
                    struct translation_t* trans = calloc(1, sizeof(struct translation_t));

                    if (trans_id_index >= 0 && trans_id_index < n) {
                        char* p = quote(trim(list[trans_id_index]));
                        strncpy(trans->trans_id, p, sizeof(trans->trans_id));
                    }
                    if (lang_index >= 0 && lang_index < n) {
                        char* p = quote(trim(list[lang_index]));
                        strncpy(trans->lang, p, sizeof(trans->lang));
                    }
                    if (translation_index >= 0 && translation_index < n) {
                        char* p = quote(trim(list[translation_index]));
                        strncpy(trans->translation, p, sizeof(trans->translation));
                    }
                    // gtfs new version(2020/08/21)
                    if (table_name_index >= 0 && table_name_index < n) {
                        char* p = quote(trim(list[table_name_index]));
                        // stops以外は無視します
                        trans->table_type = table_type(p);
                        strncpy(trans->table_name, p, sizeof(trans->table_name));
                    }
                    if (field_name_index >= 0 && field_name_index < n) {
                        char* p = quote(trim(list[field_name_index]));
                        strncpy(trans->field_name, p, sizeof(trans->field_name));
                    }
                    if (record_id_index >= 0 && record_id_index < n) {
                        char* p = quote(trim(list[record_id_index]));
                        strncpy(trans->record_id, p, sizeof(trans->record_id));
                    }
                    if (record_sub_id_index >= 0 && record_sub_id_index < n) {
                        char* p = quote(trim(list[record_sub_id_index]));
                        strncpy(trans->record_sub_id, p, sizeof(trans->record_sub_id));
                    }
                    if (field_value_index >= 0 && field_value_index < n) {
                        char* p = quote(trim(list[field_value_index]));
                        strncpy(trans->field_value, p, sizeof(trans->field_value));
                    }
                    trans->lineno = lineno;
                    vect_append(gtfs->translations_tbl, trans);
                }
                list_free(list);
            }
        }
    }
    list_free(label_list);
    free(ptr);
}

static int is_http_url(const char* zippath)
{
    return (strnicmp(zippath, "http", 4) == 0);
}

static char* url_request(const char* url, size_t* res_size)
{
    struct http_header_t* hdr;
    char* ptr;
    int size;

    hdr = alloc_http_header();
    init_http_header(hdr);

    ptr = url_post(url, hdr, NULL, g_proxy_server, g_proxy_port, &size);
    *res_size = size;

    free_http_header(hdr);
    return ptr;
}

static const char* body_contents(const char* res_ptr, size_t ressize, size_t* body_size)
{
    int body_index;
    const char* p;

    body_index = indexofstr(res_ptr, "\r\n\r\n");
    if (body_index < 0) {
        *body_size = strlen(res_ptr);
        return res_ptr;
    }
    body_index += sizeof("\r\n\r\n") - 1;
    p = &res_ptr[body_index];
    *body_size = ressize - body_index;
    return p;
}

static int utf8_bom(char* str)
{
    int c1, c2, c3;

    c1 = (unsigned char)*str;
    c2 = (unsigned char)*(str+1);
    c3 = (unsigned char)*(str+2);
    if (c1 == 0xEF && c2 == 0xBB && c3 == 0xBF)
        return 3;
    return 0;
}

int gtfs_zip_archive_reader(const char* zippath, struct gtfs_t* gtfs)
{
    static mz_zip_archive zip_archive;
    mz_bool done = 0;
    char* csvptr;
    size_t csvsize;
    char* resptr = NULL;

    memset(&zip_archive, '\0', sizeof(zip_archive));
    if (is_http_url(zippath)) {
        size_t ressize;

        resptr = url_request(zippath, &ressize);
        if (resptr) {
            int status;

            status = url_http_status(resptr);
            if (status == 200) {
                const char* zipptr;
                size_t zipsize;
                
                zipptr = body_contents((const char*)resptr, ressize, &zipsize);
                if (zipptr)
                    done = mz_zip_reader_init_mem(&zip_archive, zipptr, zipsize, 0);
            } else {
                err_write("%s unsupport status=%d\n", zippath, status);
            }
        }
    } else {
        done = mz_zip_reader_init_file(&zip_archive, zippath, 0);
    }
    if (! done)
        return -1;

    csvptr = mz_zip_reader_extract_file_to_heap(&zip_archive, g_gtfs_filename[AGENCY], &csvsize, 0);
    if (csvptr) {
        int bomsize = utf8_bom(csvptr);
        gtfs_agency_reader(csvptr+bomsize, csvsize-bomsize, gtfs);
        mz_free(csvptr);
        gtfs->file_exist_bits |= GTFS_FILE_AGENCY;
    }

    csvptr = mz_zip_reader_extract_file_to_heap(&zip_archive, g_gtfs_filename[AGENCY_JP], &csvsize, 0);
    if (csvptr) {
        int bomsize = utf8_bom(csvptr);
        gtfs_agency_jp_reader(csvptr+bomsize, csvsize-bomsize, gtfs);
        mz_free(csvptr);
        gtfs->file_exist_bits |= GTFS_FILE_AGENCY_JP;
    }

    csvptr = mz_zip_reader_extract_file_to_heap(&zip_archive, g_gtfs_filename[STOPS], &csvsize, 0);
    if (csvptr) {
        int bomsize = utf8_bom(csvptr);
        gtfs_stops_reader(csvptr+bomsize, csvsize-bomsize, gtfs);
        mz_free(csvptr);
        gtfs->file_exist_bits |= GTFS_FILE_STOPS;
    }

    csvptr = mz_zip_reader_extract_file_to_heap(&zip_archive, g_gtfs_filename[ROUTES], &csvsize, 0);
    if (csvptr) {
        int bomsize = utf8_bom(csvptr);
        gtfs_routes_reader(csvptr+bomsize, csvsize-bomsize, gtfs);
        mz_free(csvptr);
        gtfs->file_exist_bits |= GTFS_FILE_ROUTES;
    }

    csvptr = mz_zip_reader_extract_file_to_heap(&zip_archive, g_gtfs_filename[ROUTES_JP], &csvsize, 0);
    if (csvptr) {
        int bomsize = utf8_bom(csvptr);
        gtfs_routes_jp_reader(csvptr+bomsize, csvsize-bomsize, gtfs);
        mz_free(csvptr);
        gtfs->file_exist_bits |= GTFS_FILE_ROUTES_JP;
    }

    csvptr = mz_zip_reader_extract_file_to_heap(&zip_archive, g_gtfs_filename[TRIPS], &csvsize, 0);
    if (csvptr) {
        int bomsize = utf8_bom(csvptr);
        gtfs_trips_reader(csvptr+bomsize, csvsize-bomsize, gtfs);
        mz_free(csvptr);
        gtfs->file_exist_bits |= GTFS_FILE_TRIPS;
    }

    csvptr = mz_zip_reader_extract_file_to_heap(&zip_archive, g_gtfs_filename[OFFICE_JP], &csvsize, 0);
    if (csvptr) {
        int bomsize = utf8_bom(csvptr);
        gtfs_office_jp_reader(csvptr+bomsize, csvsize-bomsize, gtfs);
        mz_free(csvptr);
        gtfs->file_exist_bits |= GTFS_FILE_OFFICE_JP;
    }

    csvptr = mz_zip_reader_extract_file_to_heap(&zip_archive, g_gtfs_filename[STOP_TIMES], &csvsize, 0);
    if (csvptr) {
        int bomsize = utf8_bom(csvptr);
        gtfs_stop_times_reader(csvptr+bomsize, csvsize-bomsize, gtfs);
        mz_free(csvptr);
        gtfs->file_exist_bits |= GTFS_FILE_STOP_TIMES;
    }

    csvptr = mz_zip_reader_extract_file_to_heap(&zip_archive, g_gtfs_filename[CALENDAR], &csvsize, 0);
    if (csvptr) {
        int bomsize = utf8_bom(csvptr);
        gtfs_calendar_reader(csvptr+bomsize, csvsize-bomsize, gtfs);
        mz_free(csvptr);
        gtfs->file_exist_bits |= GTFS_FILE_CALENDAR;
    }

    csvptr = mz_zip_reader_extract_file_to_heap(&zip_archive, g_gtfs_filename[CALENDAR_DATES], &csvsize, 0);
    if (csvptr) {
        int bomsize = utf8_bom(csvptr);
        gtfs_calendar_dates_reader(csvptr+bomsize, csvsize-bomsize, gtfs);
        mz_free(csvptr);
        gtfs->file_exist_bits |= GTFS_FILE_CALENDAR_DATES;
    }

    csvptr = mz_zip_reader_extract_file_to_heap(&zip_archive, g_gtfs_filename[FARE_ATTRIBUTES], &csvsize, 0);
    if (csvptr) {
        int bomsize = utf8_bom(csvptr);
        gtfs_fare_attributes_reader(csvptr+bomsize, csvsize-bomsize, gtfs);
        mz_free(csvptr);
        gtfs->file_exist_bits |= GTFS_FILE_FARE_ATTRIBUTES;
    }

    csvptr = mz_zip_reader_extract_file_to_heap(&zip_archive, g_gtfs_filename[FARE_RULES], &csvsize, 0);
    if (csvptr) {
        int bomsize = utf8_bom(csvptr);
        gtfs_fare_rules_reader(csvptr+bomsize, csvsize-bomsize, gtfs);
        mz_free(csvptr);
        gtfs->file_exist_bits |= GTFS_FILE_FARE_RULES;
    }

    csvptr = mz_zip_reader_extract_file_to_heap(&zip_archive, g_gtfs_filename[SHAPES], &csvsize, 0);
    if (csvptr) {
        int bomsize = utf8_bom(csvptr);
        gtfs_shapes_reader(csvptr+bomsize, csvsize-bomsize, gtfs);
        mz_free(csvptr);
        gtfs->file_exist_bits |= GTFS_FILE_SHAPES;
    }

    csvptr = mz_zip_reader_extract_file_to_heap(&zip_archive, g_gtfs_filename[FREQUENCIES], &csvsize, 0);
    if (csvptr) {
        int bomsize = utf8_bom(csvptr);
        gtfs_frequencies_reader(csvptr+bomsize, csvsize-bomsize, gtfs);
        mz_free(csvptr);
        gtfs->file_exist_bits |= GTFS_FILE_FREQUENCIES;
    }

    csvptr = mz_zip_reader_extract_file_to_heap(&zip_archive, g_gtfs_filename[TRANSFERS], &csvsize, 0);
    if (csvptr) {
        int bomsize = utf8_bom(csvptr);
        gtfs_transfers_reader(csvptr+bomsize, csvsize-bomsize, gtfs);
        mz_free(csvptr);
        gtfs->file_exist_bits |= GTFS_FILE_TRANSFERS;
    }

    csvptr = mz_zip_reader_extract_file_to_heap(&zip_archive, g_gtfs_filename[TRANSLATIONS], &csvsize, 0);
    if (csvptr) {
        int bomsize = utf8_bom(csvptr);
        gtfs_translations_reader(csvptr+bomsize, csvsize-bomsize, gtfs);
        mz_free(csvptr);
        gtfs->file_exist_bits |= GTFS_FILE_TRANSLATIONS;
    }

    csvptr = mz_zip_reader_extract_file_to_heap(&zip_archive, g_gtfs_filename[FEED_INFO], &csvsize, 0);
    if (csvptr) {
        int bomsize = utf8_bom(csvptr);
        gtfs_feed_info_reader(csvptr+bomsize, csvsize-bomsize, gtfs);
        mz_free(csvptr);
        gtfs->file_exist_bits |= GTFS_FILE_FEED_INFO;
    }

    done = mz_zip_reader_end(&zip_archive);
    if (resptr)
        recv_free(resptr);
    return (done == MZ_TRUE)? 0 : -1;
}
