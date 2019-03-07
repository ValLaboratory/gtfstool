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

int gtfs_fatal_error(const char* fmt, ...)
{
    char outbuf[1024];
    va_list argptr;
    
    va_start(argptr, fmt);
    vsnprintf(outbuf, sizeof(outbuf), fmt, argptr);
    printf("[致命的エラー] %s\n", outbuf);
    return GTFS_FATAL_ERROR;
}

int gtfs_error(const char* fmt, ...)
{
    char outbuf[1024];
    va_list argptr;
    
    va_start(argptr, fmt);
    vsnprintf(outbuf, sizeof(outbuf), fmt, argptr);
    printf("[エラー] %s\n", outbuf);
    mt_increment(&g_error_count);
    return GTFS_ERROR;
}

int gtfs_warning(const char* fmt, ...)
{
    char outbuf[1024];
    va_list argptr;

    if (g_ignore_warning)
        return GTFS_SUCCESS;

    va_start(argptr, fmt);
    vsnprintf(outbuf, sizeof(outbuf), fmt, argptr);
    printf("[警告] %s\n", outbuf);
    mt_increment(&g_warning_count);
    return GTFS_WARNING;
}

int is_gtfs_file_exist(unsigned int file_kind)
{
    return (g_gtfs->file_exist_bits & file_kind);
}

char* utf8_conv(const char* str, char* enc_buf, int enc_bufsize)
{
#ifdef _WIN32
    if (convert("UTF-8", str, strlen(str), "CP932", enc_buf, enc_bufsize) < 0)
        return "???";
    return enc_buf;
#else
    strncpy(enc_buf, str, enc_bufsize);
#endif
    return enc_buf;
}
