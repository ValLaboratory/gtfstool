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

int safe_write(int fd, const void* buf, int count)
{
    const char* p = buf;
    const char* const endp = p + count;

    while (p < endp) {
        int num_bytes = 0;

        SAFE_SYSCALL(num_bytes, (int)write(fd, p, endp - p));
        if (num_bytes < 0) {
            err_write("safe_write(): write failed, %s", strerror(errno));
            break;
        }
        p += num_bytes;
    }

    if (p != endp)
        return -1;
    return count;
}

int safe_send(int s, const void* msg, int len, unsigned int flags)
{
    char* p = (char*)msg;
    const char* const endp = p + len;

    while (p < endp) {
        int num_bytes = 0;

        SAFE_SYSCALL(num_bytes, (int)send(s, p, endp - p, flags));
        if (num_bytes < 0) {
            err_write("safe_send(): send failed, %s", strerror(errno));
            break;
        }
        p += num_bytes;
    }

    if (p != endp)
        return -1;
    return len;
}
