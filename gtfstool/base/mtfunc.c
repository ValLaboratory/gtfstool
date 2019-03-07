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

#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
#include <WinSock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#define API_INTERNAL
#include "common.h"

static int init_count = 0;
static CS_DEF(localtime_critical_section);
static CS_DEF(gmtime_critical_section);
static CS_DEF(inet_ntoa_critical_section);
static CS_DEF(counter_critical_section);

APIEXPORT void mt_initialize()
{
    if (init_count == 0) {
        CS_INIT(&localtime_critical_section);
        CS_INIT(&gmtime_critical_section);
        CS_INIT(&inet_ntoa_critical_section);
        CS_INIT(&counter_critical_section);
    }
    init_count++;
}

APIEXPORT void mt_finalize()
{
    init_count--;
    if (init_count == 0) {
        CS_DELETE(&localtime_critical_section);
        CS_DELETE(&gmtime_critical_section);
        CS_DELETE(&inet_ntoa_critical_section);
        CS_DELETE(&counter_critical_section);
    }
}

APIEXPORT void mt_localtime(const time_t* timebuf, struct tm* dest)
{
    struct tm* now; 

    CS_START(&localtime_critical_section);
    now = localtime(timebuf);
    memcpy(dest, now, sizeof(struct tm));
    CS_END(&localtime_critical_section);
}

APIEXPORT void mt_gmtime(const time_t* timebuf, struct tm* dest)
{
    struct tm* now; 

    CS_START(&gmtime_critical_section);
    now = gmtime(timebuf);
    memcpy(dest, now, sizeof(struct tm));
    CS_END(&gmtime_critical_section);
}

APIEXPORT void mt_inet_ntoa(struct in_addr saddr, char* dest)
{
    char* ip;
    
    CS_START(&inet_ntoa_critical_section);
    ip = (char*)inet_ntoa(saddr);
    if (ip == NULL)
        *dest = '\0';
    else
        strcpy(dest, ip);
    CS_END(&inet_ntoa_critical_section);
}

APIEXPORT void mt_inet_addr(unsigned long ip, char* dest)
{
    struct in_addr addr;
    
    addr.s_addr = (uint)ip;
    mt_inet_ntoa(addr, dest);
}

APIEXPORT void mt_increment(long* counter)
{
    CS_START(&counter_critical_section);
    (*counter)++;
    CS_END(&counter_critical_section);
}

APIEXPORT void mt_decrement(long* counter)
{
    CS_START(&counter_critical_section);
    (*counter)--;
    CS_END(&counter_critical_section);
}

APIEXPORT void mt_increment64(int64* counter)
{
    CS_START(&counter_critical_section);
    (*counter)++;
    CS_END(&counter_critical_section);
}

APIEXPORT void mt_decrement64(int64* counter)
{
    CS_START(&counter_critical_section);
    (*counter)--;
    CS_END(&counter_critical_section);
}
