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

#include <time.h>
#define API_INTERNAL
#include "common.h"

/*
 * ERROR FILE FORMAT:
 *   err_write(): [DATE TIME] MESSAGE<LF>
 */

static int err_fd = -1;

APIEXPORT void err_initialize(const char* error_file)
{
    /* ファイルオープン */
    if (error_file != NULL && *error_file != '\0') {
        err_fd = FILE_OPEN(error_file, O_WRONLY|O_APPEND|O_CREAT, CREATE_MODE);
        if (err_fd < 0) {
            fprintf(stderr, "error file can't open [%d]: ", errno);
            perror("");
        }
    }
}

APIEXPORT void err_finalize()
{
    /* ファイルクローズ */
    if (err_fd >= 0) {
        FILE_CLOSE(err_fd);
        err_fd = -1;
    }
}

static void output(const char* buf)
{
    char outbuf[2048];
    time_t timebuf;
    struct tm now;

    /* 現在時刻の取得 */
    time(&timebuf);
    mt_localtime(&timebuf, &now);

    /* エラー内容の編集 */
    snprintf(outbuf, sizeof(outbuf), "[%d/%02d/%02d %02d:%02d:%02d] %s\n",
             now.tm_year+1900, now.tm_mon+1, now.tm_mday,
             now.tm_hour, now.tm_min, now.tm_sec,
             buf);

    if (err_fd < 0)
        fprintf(stderr, "%s", outbuf);
    else
        FILE_WRITE(err_fd, outbuf, (int)strlen(outbuf));
}

APIEXPORT void err_write(const char* fmt, ...)
{
    char outbuf[1024];
    va_list argptr;

    va_start(argptr, fmt);
    vsnprintf(outbuf, sizeof(outbuf), fmt, argptr);
    output(outbuf);
}
