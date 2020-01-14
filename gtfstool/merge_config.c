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

#define R_BUF_SIZE 1024
#define MAX_NAME_SIZE   256
#define MAX_VALUE_SIZE  256

#define CMD_INCLUDE "include"

/*
 * コンフィグファイルを読んでパラメータを設定します。
 * 既知のパラメータ以外はユーザーパラメータとして登録します。
 * パラメータの形式は "name=value"の形式とします。
 *
 * conf_fname: コンフィグファイル名
 *
 * 戻り値
 *  0: 成功
 * -1: 失敗
 *
 * (config parameters)
 * key = value
 * ...
 * include = FILE_NAME
 */
int merge_config(const char* conf_fname)
{
    FILE *fp;
    char fpath[MAX_PATH+1];
    char buf[R_BUF_SIZE];
    int err = 0;

    get_abspath(fpath, conf_fname, MAX_PATH);
    if ((fp = fopen(fpath, "r")) == NULL) {
        fprintf(stderr, "file open error: %s\n", conf_fname);
        return -1;
    }

    while (fgets(buf, sizeof(buf), fp) != NULL) {
        int index;
        char name[R_BUF_SIZE];
        char value[R_BUF_SIZE];

        /* コメントの排除 */
        index = indexof(buf, '#');
        if (index >= 0) {
            buf[index] = '\0';
            if (strlen(buf) == 0)
                continue;
        }
        /* 名前と値の分離 */
        index = indexof(buf, '=');
        if (index <= 0)
            continue;

        substr(name, buf, 0, index);
        substr(value, buf, index+1, -1);

        /* 両端のホワイトスペースと引用符を取り除きます。*/
        quote(trim(name));
        quote(trim(value));

        if (strlen(name) > MAX_NAME_SIZE-1) {
            fprintf(stderr, "parameter name too large: %s\n", buf);
            err = -1;
            break;
        }
        if (strlen(value) > MAX_VALUE_SIZE-1) {
            fprintf(stderr, "parameter value too large: %s\n", buf);
            err = -1;
            break;
        }

        if (stricmp(name, "output_dir") == 0) {
            strcpy(g_merged_output_dir, value);
        } else if (stricmp(name, "output_gtfs_name") == 0) {
            strcpy(g_merged_gtfs_name, value);
        // agency.txt
        } else if (stricmp(name, "agency_id") == 0) {
            strcpy(g_merged_agency.agency_id, value);
        } else if (stricmp(name, "agency_name") == 0) {
            strcpy(g_merged_agency.agency_name, value);
        } else if (stricmp(name, "agency_url") == 0) {
            strcpy(g_merged_agency.agency_url, value);
        } else if (stricmp(name, "agency_timezone") == 0) {
            strcpy(g_merged_agency.agency_timezone, value);
        } else if (stricmp(name, "agency_lang") == 0) {
            strcpy(g_merged_agency.agency_lang, value);
        } else if (stricmp(name, "agency_phone") == 0) {
            strcpy(g_merged_agency.agency_phone, value);
        } else if (stricmp(name, "agency_fare_url") == 0) {
            strcpy(g_merged_agency.agency_fare_url, value);
        } else if (stricmp(name, "agency_email") == 0) {
            strcpy(g_merged_agency.agency_email, value);
        // agency_jp.txt
        } else if (stricmp(name, "agency_official_name") == 0) {
            strcpy(g_merged_agency_jp.agency_official_name, value);
        } else if (stricmp(name, "agency_zip_number") == 0) {
            strcpy(g_merged_agency_jp.agency_zip_number, value);
        } else if (stricmp(name, "agency_address") == 0) {
            strcpy(g_merged_agency_jp.agency_address, value);
        } else if (stricmp(name, "agency_president_pos") == 0) {
            strcpy(g_merged_agency_jp.agency_president_pos, value);
        } else if (stricmp(name, "agency_president_name") == 0) {
            strcpy(g_merged_agency_jp.agency_president_name, value);
        // feed_info.txt
        } else if (stricmp(name, "feed_publisher_name") == 0) {
            strcpy(g_merged_feed_info.feed_publisher_name, value);
        } else if (stricmp(name, "feed_publisher_url") == 0) {
            strcpy(g_merged_feed_info.feed_publisher_url, value);
        } else if (stricmp(name, "feed_lang") == 0) {
            strcpy(g_merged_feed_info.feed_lang, value);
        } else if (stricmp(name, "feed_start_date") == 0) {
            strcpy(g_merged_feed_info.feed_start_date, value);
        } else if (stricmp(name, "feed_end_date") == 0) {
            strcpy(g_merged_feed_info.feed_end_date, value);
        } else if (stricmp(name, "feed_version") == 0) {
            strcpy(g_merged_feed_info.feed_version, value);
        } else if (stricmp(name, CMD_INCLUDE) == 0) {
            /* 他のconfigファイルを再帰処理で読み込みます。*/
            if (merge_config(value) < 0)
                break;
        } else {
            if (strlen(name) > 0 && strlen(value) > 0) {
                struct merge_gtfs_prefix_t* mp = calloc(1, sizeof(struct merge_gtfs_prefix_t));
                strncpy(mp->gtfs_file_name, name, sizeof(mp->gtfs_file_name));
                strncpy(mp->prefix, value, sizeof(mp->prefix));
                vect_append(g_merge_gtfs_tbl, mp);
            }
        }
    }

    fclose(fp);
    return err;
}
