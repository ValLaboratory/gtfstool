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

static void gtfs_diff_agency(const char* dir, struct vector_t* tbl, struct vector_t* diff_tbl)
{
    char csvpath[MAX_PATH];
    int count, i, diff_count;

    char agency_name[256];

    strcpy(csvpath, dir);
    catpath(csvpath, g_gtfs_filename[AGENCY]);

    csv_initialize(csvpath);
    gtfs_agency_label_writer();

    count = vect_count(tbl);
    diff_count = vect_count(diff_tbl);

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
}

int gtfs_diff(const char* diff_zip)
{
    int err = 0;
    struct gtfs_t* diff_gtfs = NULL;
    struct gtfs_hash_t* diff_gtfs_hash = NULL;

    TRACE("%s\n", "*GTFS(zip)の読み込み*");
    if (gtfs_zip_archive_reader(g_gtfs_zip, g_gtfs) < 0) {
        err_write("gtfs_check: zip_archive_reader error (%s).\n",
                  utf8_conv(g_gtfs_zip, (char*)alloca(256), 256));
        return -1;
    }
    
    diff_gtfs = gtfs_alloc();
    diff_gtfs_hash = gtfs_hash_alloc();

    TRACE("%s\n", "*GTFS(diff zip)の読み込み*");
    if (gtfs_zip_archive_reader(diff_zip, diff_gtfs) < 0) {
        err_write("gtfs_check: zip_archive_reader error (%s).\n",
                  utf8_conv(diff_zip, (char*)alloca(256), 256));
        err = -1;
        goto final;
    }

    if (is_gtfs_file_exist(g_gtfs, GTFS_FILE_AGENCY)) {
        if (is_gtfs_file_exist(diff_gtfs, GTFS_FILE_AGENCY))
            gtfs_diff_agency(g_output_dir, g_gtfs->agency_tbl, diff_gtfs->agency_tbl);
    }

final:
    if (diff_gtfs_hash)
        gtfs_hash_free(diff_gtfs_hash);
    if (diff_gtfs)
        gtfs_free(diff_gtfs, 1);

    return err;
}
