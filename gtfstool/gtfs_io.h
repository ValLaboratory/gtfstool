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
#ifndef _GTFS_IO_H
#define _GTFS_IO_H

#define AGENCY              0
#define STOPS               1
#define ROUTES              2
#define TRIPS               3
#define STOP_TIMES          4
#define CALENDAR            5
#define CALENDAR_DATES      6
#define FARE_ATTRIBUTES     7
#define FARE_RULES          8
#define SHAPES              9
#define FREQUENCIES         10
#define TRANSFERS           11
#define FEED_INFO           12
#define TRANSLATIONS        13
#define AGENCY_JP           14
#define ROUTES_JP           15
#define OFFICE_JP           16

// GTFS files
#define GTFS_FILE_AGENCY            0x00000001
#define GTFS_FILE_STOPS             0x00000002
#define GTFS_FILE_ROUTES            0x00000004
#define GTFS_FILE_TRIPS             0x00000008
#define GTFS_FILE_STOP_TIMES        0x00000010
#define GTFS_FILE_CALENDAR          0x00000020
#define GTFS_FILE_CALENDAR_DATES    0x00000040
#define GTFS_FILE_FARE_ATTRIBUTES   0x00000080
#define GTFS_FILE_FARE_RULES        0x00000100
#define GTFS_FILE_SHAPES            0x00000200
#define GTFS_FILE_FREQUENCIES       0x00000400
#define GTFS_FILE_TRANSFERS         0x00000800
#define GTFS_FILE_FEED_INFO         0x00001000
#define GTFS_FILE_TRANSLATIONS      0x00002000
#define GTFS_FILE_AGENCY_JP         0x00004000
#define GTFS_FILE_ROUTES_JP         0x00008000
#define GTFS_FILE_OFFICE_JP         0x00010000

#define MAX_CORP_NAME               256
#define MAX_RAIL_NAME               256
#define MAX_STATION_NAME            256

#define CHOICE_ROUTE_NAME           1
#define CHOICE_ROUTE_LONG_NAME      2
#define CHOICE_TRIP_HEADSIGN        3
#define CHOICE_TRIP_SHORT_NAME      4

#define GTFS_ID_SIZE                128

// agency.txt
struct agency_t {
    char agency_id[GTFS_ID_SIZE];
    char agency_name[256];
    char agency_url[256];
    char agency_timezone[64];
    char agency_lang[64];
    char agency_phone[64];
    char agency_fare_url[256];
    char agency_email[256];
    int lineno;
};

// agency_jp.txt
struct agency_jp_t {
    char agency_id[GTFS_ID_SIZE];
    char agency_official_name[256];
    char agency_zip_number[64];
    char agency_address[256];
    char agency_president_pos[256];
    char agency_president_name[256];
    int lineno;
};

// routes.txt
struct route_t {
    char route_id[GTFS_ID_SIZE];        // 経路ID
    char agency_id[GTFS_ID_SIZE];       // 事業者ID
    char route_short_name[256];         // 経路略称
    char route_long_name[512];          // 経路名
    char route_desc[256];               // 経路情報
    char route_type[8];                 // 経路タイプ（バス事業者は3固定）
    char route_url[256];                // 経路URL
    char route_color[64];               // 経路色 ex.(00FFFF)
    char route_text_color[64];          // 経路文字色 ex.(0000FF)
    char jp_parent_route_id[256];       // 路線ID
    int lineno;                         // 行番号
};

// stops.txt
struct stop_t {
    char stop_id[GTFS_ID_SIZE];         // 停留所・標柱ID
    char stop_code[256];                // 停留所・標柱番号
    char stop_name[256];                // 停留所・標柱名称
    char stop_desc[256];                // 停留所・標柱付加情報
    char stop_lat[64];                  // 緯度
    char stop_lon[64];                  // 経度
    char zone_id[GTFS_ID_SIZE];         // 運賃用ゾーンID
    char stop_url[256];                 // 停留所・標柱URL
    char location_type[8];              // 停留所・標柱区分(0:標柱 1:停留所)
    char parent_station[GTFS_ID_SIZE];  // 標柱の場合の停留所ID
    char stop_timezone[64];             // タイムゾーン
    char wheelchair_boarding[8];        // 車椅子情報
    int lineno;                         // 行番号
};

// trips.txt
struct trip_t {
    char route_id[GTFS_ID_SIZE];        // 経路ID
    char service_id[GTFS_ID_SIZE];      // 運行日ID
    char trip_id[GTFS_ID_SIZE];         // 便ID
    char trip_headsign[256];            // 行き先名
    char trip_short_name[256];          // 便名称
    char direction_id[8];               // 0:復路（上り） 1:往路（下り）
    char block_id[GTFS_ID_SIZE];        // 便結合区分
    char shape_id[GTFS_ID_SIZE];        // 描画ID
    char wheelchair_accessible[8];      // 車椅子利用区分
    char bikes_allowed[8];              // 自転車持込区分
    char jp_trip_desc[256];             // （追加JP）便情報
    char jp_trip_desc_symbol[64];       // （追加JP）便記号
    char jp_office_id[GTFS_ID_SIZE];    // （追加JP）営業所ID
    int lineno;                         // 行番号
};

// stop_times.txt
struct stop_time_t {
    char trip_id[GTFS_ID_SIZE];     // 便ID
    char arrival_time[16];          // 到着時刻（HH:MM:SS）
    char departure_time[16];        // 出発時刻（HH:MM:SS）
    char stop_id[GTFS_ID_SIZE];     // 駅コード
    char stop_sequence[16];         // 連番
    char stop_headsign[256];        // 停留所行き先
    char pickup_type[8];            // 乗車区分（0:通常の乗車地）
    char drop_off_type[8];          // 降車区分（0:通常の降車地）
    char shape_dist_traveled[16];   // 通算距離（メートル）
    char timepoint[64];             // 発着時間精度
    int lineno;                     // 行番号
};

// calendar.txt
struct calendar_t {
    char service_id[GTFS_ID_SIZE];  // サービスID
    char monday[8];                 // 月曜日(1 or 0)
    char tuesday[8];                // 火曜日(1 or 0)
    char wednesday[8];              // 水曜日(1 or 0)
    char thursday[8];               // 木曜日(1 or 0)
    char friday[8];                 // 金曜日(1 or 0)
    char saturday[8];               // 土曜日(1 or 0)
    char sunday[8];                 // 日曜日(1 or 0)
    char start_date[16];            // 開始日(YYYYMMDD)
    char end_date[16];              // 終了日(YYYYMMDD)
    int lineno;                     // 行番号
};

// calendar_dates.txt
struct calendar_date_t {
    char service_id[GTFS_ID_SIZE];  // サービスID
    char date[16];                  // 日付
    char exception_type[8];         // 利用タイプ(1 or 2)
    int lineno;                     // 行番号
};

// fare_attributes.txt
struct fare_attribute_t {
    char fare_id[GTFS_ID_SIZE];     // 運賃ID
    char price[64];                 // 運賃
    char currency_type[8];          // 通貨(JPY)
    char payment_method[8];         // 支払いタイミング
    char transfers[8];              // 乗換
    char agency_id[GTFS_ID_SIZE];   // 事業者ID（agency.txtで複数の事業者が定義された場合は必要）
    char transfer_duration[16];     // 乗換有効期限
    int lineno;                     // 行番号
};

// fare_rules.txt
struct fare_rule_t {
    char fare_id[GTFS_ID_SIZE];         // 運賃ID
    char route_id[GTFS_ID_SIZE];        // 経路ID
    char origin_id[GTFS_ID_SIZE];       // 乗車駅ゾーン
    char destination_id[GTFS_ID_SIZE];  // 降車駅ゾーン
    char contains_id[GTFS_ID_SIZE];     // 通過ゾーン
    int lineno;                         // 行番号
};

// shapes.txt
struct shape_t {
    char shape_id[GTFS_ID_SIZE];        // 描画ID
    char shape_pt_lat[64];              // 描画緯度
    char shape_pt_lon[64];              // 描画経度
    char shape_pt_sequence[16];         // 描画順序
    char shape_dist_traveled[16];       // 描画距離
    int lineno;                         // 行番号
};

// frequencies.txt
struct frequency_t {
    char trip_id[GTFS_ID_SIZE];         // 便ID
    char start_time[16];                // 開始時刻
    char end_time[16];                  // 終了時刻
    char headway_secs[16];              // 運行間隔
    char exact_times[8];                // 案内精度
    int lineno;                         // 行番号
};

// transfers.txt
struct transfer_t {
    char from_stop_id[GTFS_ID_SIZE];    // 乗継元標柱ID
    char to_stop_id[GTFS_ID_SIZE];      // 乗継先標柱ID
    char transfer_type[8];              // 乗換タイプ
    char min_transfer_time[16];         // 乗継時間（秒）
    int lineno;                         // 行番号
};

// feed_info.txt
struct feed_info_t {
    char feed_publisher_name[256];
    char feed_publisher_url[256];
    char feed_lang[64];
    char feed_start_date[64];
    char feed_end_date[64];
    char feed_version[256];
    int lineno;
};

// translations.txt (langが"ja-Hrkt"のみ)
struct translation_t {
    char trans_id[GTFS_ID_SIZE];    // 原語（キー）
    char lang[16];                  // 言語
    char translation[512];          // 翻訳語
    int lineno;                     // 行番号
};

// routes_jp.txt
struct route_jp_t {
    char route_id[GTFS_ID_SIZE];    // 経路ID
    char route_update_date[64];     // ダイヤ改正日
    char origin_stop[256];          // 起点
    char via_stop[256];             // 経由地
    char destination_stop[256];     // 終点
    int lineno;                     // 行番号
};

// office_jp.txt
struct office_jp_t {
    char office_id[GTFS_ID_SIZE];   // 営業所ID
    char office_name[256];          // 営業所名
    char office_url[256];           // 営業所URL
    char office_phone[64];          // 営業所電話番号
    int lineno;                     // 行番号
};

struct gtfs_t {
    unsigned int file_exist_bits;           // ファイル存在情報（GTFS filesのビットがON）
    struct vector_t* agency_tbl;            // 事業者情報テーブル
    struct vector_t* agency_jp_tbl;         // 事業者追加情報テーブル
    struct vector_t* routes_tbl;            // 経路情報テーブル
    struct vector_t* stops_tbl;             // 停留所・標柱情報テーブル
    struct vector_t* trips_tbl;             // 便情報テーブル
    struct vector_t* stop_times_tbl;        // 通過時刻情報テーブル
    struct vector_t* calendar_tbl;          // カレンダーテーブル
    struct vector_t* calendar_dates_tbl;    // 運行日情報テーブル
    struct vector_t* fare_attrs_tbl;        // 運賃テーブル
    struct vector_t* fare_rules_tbl;        // 区間運賃テーブル
    struct vector_t* shapes_tbl;            // 描画情報テーブル
    struct vector_t* frequencies_tbl;       // 運行間隔情報テーブル
    struct vector_t* transfers_tbl;         // 乗換情報テーブル
    struct vector_t* translations_tbl;      // 翻訳情報テーブル
    struct vector_t* routes_jp_tbl;         // 経路追加情報テーブル
    struct vector_t* office_jp_tbl;         // 営業所情報テーブル
};

struct gtfs_hash_t {
    struct hash_t* agency_htbl;             // 事業者情報テーブル
    struct hash_t* routes_htbl;             // 経路情報テーブル
    struct hash_t* stops_htbl;              // 停留所・標柱情報テーブル
    struct hash_t* trips_htbl;              // 便情報テーブル
    struct hash_t* calendar_htbl;           // カレンダーテーブル
    struct hash_t* calendar_dates_htbl;     // 運行日情報テーブル（利用タイプが"1"の「運行区分適用」のみ登録される）
    struct hash_t* fare_attrs_htbl;         // 運賃テーブル
    struct hash_t* fare_rules_htbl;         // 区間運賃テーブル
    struct hash_t* translations_htbl;       // 翻訳情報テーブル
    struct hash_t* routes_jp_htbl;          // 経路追加情報テーブル
};

struct gtfs_label_t {
    char agency[256];
    char agency_jp[256];
    char stops[256];
    char routes[256];
    char trips[256];
    char stop_times[256];
    char calendar[256];
    char calendar_dates[256];
    char fare_attributes[256];
    char fare_rules[256];
    char shapes[256];
    char frequencies[256];
    char transfers[256];
    char translations[256];
    char feed_info[256];
    char routes_jp[256];
    char office_jp[256];
};

/* prototypes */
#ifdef __cplusplus
extern "C" {
#endif

// gtfs_reader.c
int gtfs_zip_archive_reader(const char* zippath);

// gtfs_writer.c
void gtfs_feed_writer(const char* dir, struct gtfs_t* gtfs);
void gtfs_feed_delete(const char* dir, struct gtfs_t* gtfs);
int gtfs_zip_archive_writer(const char* dir, const char* zipname, struct gtfs_t* gtfs);

#ifdef __cplusplus
}
#endif

#endif /* _GTFS_IO_H */
