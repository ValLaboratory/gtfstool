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

#ifdef _WIN32
#define _USE_MATH_DEFINES
#endif
#include <math.h>

#include "geo.h"

typedef struct geo_dms_t {
    int degree;
    int minute;
    int second;
    int sec_dec4;
} GEODMS;

static GEODMS convertD2DMS(double degree)
{
    GEODMS dms;
    
    dms.degree = (int)degree;
    degree = (degree - dms.degree) * 60;
    dms.minute = (int)degree;
    degree = (degree - dms.minute) * 60;
    dms.second = (int)degree;
    degree = (degree - dms.second) * 10000;
    dms.sec_dec4 = (int)degree;
    return dms;
}

static long geoDMS_to_256(GEODMS dms)
{
    long deg, minu, sec, sec_dec;
    long value;
    
    deg  = (long)dms.degree * 3600L;
    minu = (long)dms.minute * 60L;
    sec  = (long)dms.second;
    sec_dec = (long)dms.sec_dec4;
    
    value = ((deg + minu + sec) << 8L) + ((sec_dec << 8L) / 10000L);
    return value;
}

// 世界測地系を日本測地系(1/256秒)に変換します。
GEOJ256 geo_to_J256(double w_lat, double w_lon)
{
    GEOJ256 jp;

    double j_latitude = w_lat + 0.00010696 * w_lat - 0.000017467 * w_lon - 0.0046020;
    double j_longitude = w_lon + 0.000046047 * w_lat + 0.000083049 * w_lon - 0.010041;
    GEODMS dms_latitude = convertD2DMS(j_latitude);
    GEODMS dms_longitude = convertD2DMS(j_longitude);
    jp.j256_latitude = geoDMS_to_256(dms_latitude);
    jp.j256_longitude = geoDMS_to_256(dms_longitude);
    return jp;
}

// 世界測地系の２点間の距離（メートル）を求めます。
double geo_distance(double cur_lat, double cur_lon, double tar_lat, double tar_lon)
{
    // ヒュベニの公式

    double currentLa = cur_lat * M_PI / 180.0;
    double currentLo = cur_lon * M_PI / 180.0;
    double targetLa  = tar_lat * M_PI / 180.0;
    double targetLo  = tar_lon * M_PI / 180.0;
    
    double radLatDiff = currentLa - targetLa;
    double radLonDiff = currentLo - targetLo;
    double radLatAve = (currentLa + targetLa) / 2.0;
    
    double a = 6378137.0;   // world
//    double a = 6377397.155; // japan
    
    double b = 6356752.314140356;   // world
//    double b = 6356078.963;         // japan
    
    double e2 = (a * a - b * b) / (a * a);
    
    double a1e2 = a * (1 - e2);
    
    double sinLat = sin(radLatAve);
    double w2 = 1.0 - e2 * (sinLat * sinLat);
    double m = a1e2 / (sqrt(w2) * w2);
    double n = a / sqrt(w2);
    
    double t1 = m * radLatDiff;
    double t2 = n * cos(radLatAve) * radLonDiff;
    double distance = sqrt((t1 * t1) + (t2 * t2));
    return distance;
}
