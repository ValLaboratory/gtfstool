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
#ifndef _MTFUNC_H_
#define _MTFUNC_H_

#include <time.h>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <netdb.h>
#endif

#include "common.h"

/* prototypes */
#ifdef __cplusplus
extern "C" {
#endif

APIEXPORT void mt_initialize(void);
APIEXPORT void mt_finalize(void);
APIEXPORT void mt_localtime(const time_t* timebuf, struct tm* dest);
APIEXPORT void mt_gmtime(const time_t* timebuf, struct tm* dest);
APIEXPORT void mt_inet_ntoa(struct in_addr saddr, char* dest);
APIEXPORT void mt_inet_addr(unsigned long ip, char* dest);
APIEXPORT void mt_increment(long* counter);
APIEXPORT void mt_decrement(long* counter);
APIEXPORT void mt_increment64(int64* counter);
APIEXPORT void mt_decrement64(int64* counter);

#ifdef __cplusplus
}
#endif

#endif /* _MTFUC_H_ */
