/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * The MIT License
 *
 * Copyright (c) 2008-2019 YAMAMOTO Naoki
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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define API_INTERNAL
#include "common.h"
#include <time.h>

#ifndef _WIN32
#include <sys/time.h>
#endif

/*
 * 現在日付を取得します。
 * フォーマットは YYYY/MM/DD になります。
 * 区切り文字は sep で指定します。
 *
 * buf: 編集されるバッファ
 * buf_size: バッファサイズ
 * sep: 年月日の区切り文字
 *
 * 戻り値
 *  bufのポインタを返します。
 */
APIEXPORT char* todays_date(char* buf, int buf_size, char* sep)
{
    time_t timebuf;
    struct tm now; 

    /* 現在時刻の取得 */
    time(&timebuf);
    mt_localtime(&timebuf, &now);
    snprintf(buf, buf_size, "%d%s%02d%s%02d",
         now.tm_year+1900, sep, now.tm_mon+1, sep, now.tm_mday);
    return buf;
}

/*
 * 現在時刻を取得します。
 * フォーマットは HH:MM:SS になります。
 * 区切り文字は sep で指定します。
 *
 * buf: 編集されるバッファ
 * buf_size: バッファサイズ
 * sep: 時分秒の区切り文字
 *
 * 戻り値
 *  bufのポインタを返します。
 */
APIEXPORT char* todays_time(char* buf, int buf_size, char* sep)
{
    time_t timebuf;
    struct tm now;
    
    /* 現在時刻の取得 */
    time(&timebuf);
    mt_localtime(&timebuf, &now);
    snprintf(buf, buf_size, "%d%s%02d%s%02d",
             now.tm_hour, sep, now.tm_min, sep, now.tm_sec);
    return buf;
}

/*
 * 指定されたGMT日時をフォーマットします。
 * フォーマットは"Fri, 28 Nov 2008 14:28:01 GMT"になります。
 *
 * buf: 編集されるバッファ
 * buf_size: バッファサイズ
 * gmtime: GMTの日時
 *
 * 戻り値
 *  bufのポインタを返します。
 */
APIEXPORT char* gmtstr(char* buf, int buf_size, struct tm* gmtime)
{
    strftime(buf, buf_size, "%a, %d %b %Y %H:%M:%S GMT", gmtime);
    return buf;
}

/*
 * 現在日時をGMTで取得してフォーマットします。
 * フォーマットは"Fri, 28 Nov 2008 14:28:01 GMT"になります。
 *
 * buf: 編集されるバッファ
 * buf_size: バッファサイズ
 *
 * 戻り値
 *  bufのポインタを返します。
 */
APIEXPORT char* now_gmtstr(char* buf, int buf_size)
{
    struct tm now; 
    time_t timebuf;

    /* 現在時刻の取得 */
    time(&timebuf);
    /* GMT変換 */
    mt_gmtime(&timebuf, &now);
    return gmtstr(buf, buf_size, &now);
}
/*
 * 指定された日時をフォーマットします。
 * フォーマットは"Fri, 28 Nov 2008 14:28:01 +0900"になります。
 *
 * buf: 編集されるバッファ
 * buf_size: バッファサイズ
 * jstime: 日本標準時間の日時
 *
 * 戻り値
 *  bufのポインタを返します。
 */
APIEXPORT char* jststr(char* buf, int buf_size, struct tm* jstime)
{
    strftime(buf, buf_size, "%a, %d %b %Y %H:%M:%S +0900", jstime);
    return buf;
}

/*
 * 現在日時を日本標準時間で取得してフォーマットします。
 * フォーマットは"Fri, 28 Nov 2008 14:28:01 +0900"になります。
 *
 * buf: 編集されるバッファ
 * buf_size: バッファサイズ
 *
 * 戻り値
 *  bufのポインタを返します。
 */
APIEXPORT char* now_jststr(char* buf, int buf_size)
{
    struct tm now; 
    time_t timebuf;

    /* 現在時刻の取得 */
    time(&timebuf);
    /* JST変換 */
    mt_localtime(&timebuf, &now);
    return jststr(buf, buf_size, &now);
}

/*
 * システム時間をマイクロ秒で返します。
 * 1970年1月1日00:00:00(GMT)からの経過時間をマイクロ秒で返します。
 *
 * 1秒 = 1,000,000マイクロ秒
 *
 * 戻り値
 *  システム時間を返します。
 */
APIEXPORT int64 system_time()
{
#ifdef _WIN32
#define EPOCHFILETIME (116444736000000000LL) /* 100ns intervals between 1601-01-01 */
                                             /* (1970-1601)*365.24119241192411924119241192412 * 24 * 60 * 60 * 1000000 */
    FILETIME ft;
    LARGE_INTEGER largeint;
    int64 t;

    GetSystemTimeAsFileTime(&ft);
    largeint.LowPart  = ft.dwLowDateTime;
    largeint.HighPart = ft.dwHighDateTime;
    t = largeint.QuadPart - EPOCHFILETIME;
    t /= 10;  /* t is micro seconds */
    return t;
#elif defined(MAC_OSX)
    struct timeval t;

    gettimeofday(&t, NULL);
    return (int64)t.tv_sec * 1000000LL + (int64)t.tv_usec;
#else
    /* Linux */
    struct timespec ts;

    clock_gettime(CLOCK_REALTIME, &ts);
    return (int64)ts.tv_sec * 1000000LL + (int64)(ts.tv_nsec / 1000);
#endif
}

/*
 * システム時間を秒で返します。
 * 1970年1月1日00:00:00(GMT)からの経過時間を秒で返します。
 *
 * 戻り値
 *  システム時間秒を返します。
 */
APIEXPORT uint system_seconds()
{
#ifdef _WIN32
    time_t t;
    time(&t);
    return (uint)t;
#elif defined(MAC_OSX)
    struct timeval t;
    gettimeofday(&t, NULL);
    return (uint)t.tv_sec;
#else
    /* Linux */
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec;
#endif
}

#define IS_LEAP_YEAR(y) ((0 == ((y) % 400)) || ((0 != ((y) % 100)) && (0 == ((y) % 4))))

static int is_days_in_month(int year, int month)
{
	const int days_in_month[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	if ((month < 1) || (12 < month))
		return 0;
	if ((2 == month) && IS_LEAP_YEAR(year))
		return 29;

	return days_in_month[month-1];
}

/*
 * 妥当な日付かチェックします。
 *
 * 戻り値
 *  妥当な場合は真の値を返します。カレンダーに存在日付の場合は偽の値を返します。
 */
APIEXPORT int is_valid_dates(int year, int month, int day)
{
	return ((month >= 1) && (month <= 12) && (day >= 1) && (day <= is_days_in_month(year, month)));
}
