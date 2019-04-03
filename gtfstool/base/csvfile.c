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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "common.h"

static int _csv_fd = -1;

void csv_initialize(const char* file_name)
{
    if (file_name != NULL && *file_name != '\0') {
        _csv_fd = FILE_OPEN(file_name, O_WRONLY|O_CREAT|O_BINARY, CREATE_MODE);
        if (_csv_fd < 0) {
            fprintf(stderr, "csv file can't open [%d]: ", errno);
            perror("");
        }
        FILE_TRUNCATE(_csv_fd, 0LL);
    }
}

void csv_finalize()
{
    if (_csv_fd >= 0) {
        FILE_CLOSE(_csv_fd);
        _csv_fd = -1;
    }
}

static void output(const char* buf)
{
    if (_csv_fd < 0)
        fprintf(stdout, "%s", buf);
    else
        FILE_WRITE(_csv_fd, buf, (int)strlen(buf));
}

void csv_write(const char* fmt, ...)
{
    char outbuf[2048];
    va_list argptr;
    
    va_start(argptr, fmt);
    vsnprintf(outbuf, sizeof(outbuf), fmt, argptr);
    output(outbuf);
}

static long csv_filesize(int fd)
{
    struct stat statbuf;
    
    if (fstat(fd, &statbuf) == 0)
        return statbuf.st_size;
    return -1L;
}

char* csv_alloc(const char* file_name)
{
    struct stat st;
    size_t filesize;
    char* buf;

    if (stat(file_name, &st) != 0)
        return NULL;

    if (file_name != NULL && *file_name != '\0') {
        _csv_fd = FILE_OPEN(file_name, O_RDONLY);
        if (_csv_fd < 0) {
            fprintf(stderr, "csv file can't open [%d]: ", errno);
            perror("");
        }
    }
    filesize = csv_filesize(_csv_fd);
    buf = malloc(filesize+1);
    FILE_READ(_csv_fd, buf, filesize);
    buf[filesize] = '\0';
    FILE_CLOSE(_csv_fd);
    _csv_fd = -1;
    return buf;
}

void csv_free(const char* buf)
{
    free((void*)buf);
}
