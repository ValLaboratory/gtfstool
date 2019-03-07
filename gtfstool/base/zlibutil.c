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

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#ifdef HAVE_ZLIB
#include <zlib.h>
#endif

#define API_INTERNAL
#include "common.h"

/*
 * zlib の利用については三重大学の奥村晴彦教授の「zlib 入門」
 * http://oku.edu.mie-u.ac.jp/~okumura/compression/zlib.html を
 * 参考にさせていただきました。
 *
 */
#define OUTBUFSIZE  1024
#define INBUFSIZE   1024

/*
 * データをgzip形式で圧縮します。
 *
 * 圧縮は zlib(http://zlib.net/) を使用して行ないます。
 *
 * src_buf: 圧縮するバッファ
 * src_size: 圧縮するバッファのサイズ
 * comp_size: 圧縮されたサイズが設定されます。
 *
 * 戻り値
 *   圧縮された領域のポインタを返します。
 *   このポインタ領域は関数内で動的に確保された領域なので使用後はgz_free()関数で解放する必要があります。
 */
char* gz_comp(const char* src_buf, int src_size, int* comp_size)
{
#ifdef HAVE_ZLIB
    z_stream z;  /* zlibとのやり取りの構造体 */
    int status;
    int flush;
    char outbuf[OUTBUFSIZE];
    char* comp_buf = NULL;
    int rest_count;
    int cur_index = 0;

    *comp_size = 0;

    /* すべてのメモリ管理をライブラリに任せる */
    z.zalloc = Z_NULL;
    z.zfree = Z_NULL;
    z.opaque = Z_NULL;

    /* 初期化 */
    /* 第2引数は圧縮の度合。0～9 の範囲の整数で，0 は無圧縮 */
    /* Z_DEFAULT_COMPRESSION (= 6) が標準 */
    if (deflateInit(&z, Z_DEFAULT_COMPRESSION) != Z_OK) {
        err_write("zlib: deflateInit error: %s", z.msg);
        return NULL;
    }

    z.avail_in = 0;             /* 入力バッファ中のデータのバイト数 */
    z.next_out = (unsigned char*)outbuf;        /* 出力ポインタ */
    z.avail_out = OUTBUFSIZE;   /* 出力バッファのサイズ */

    /* 通常は deflate() の第2引数は Z_NO_FLUSH にして呼び出す */
    flush = Z_NO_FLUSH;

    while (1) {
        if (z.avail_in == 0) {
            int in_rest_size;

            in_rest_size = src_size - cur_index;
            z.next_in = (unsigned char*)&src_buf[cur_index];
            z.avail_in = (in_rest_size > INBUFSIZE)? INBUFSIZE : in_rest_size;
            /* 入力が最後になったら deflate() の第2引数は Z_FINISH にする */
            if (z.avail_in < INBUFSIZE)
                flush = Z_FINISH;
            cur_index += z.avail_in;
        }

         /* 圧縮する */
        status = deflate(&z, flush);

        if (status == Z_STREAM_END)
            break; /* 完了 */
        if (status != Z_OK) {
            /* エラー */
            err_write("zlib: deflate error: %s", z.msg);
            if (comp_buf != NULL)
                free(comp_buf);
            return NULL;
        }
        if (z.avail_out == 0) {
            /* 出力バッファが尽きれば comp_buf にコピーする */
            if (comp_buf == NULL) {
                comp_buf = (char*)malloc(OUTBUFSIZE);
            } else {
                char* tp;
                tp = (char*)realloc(comp_buf, *comp_size + OUTBUFSIZE);
                if (tp == NULL)
                    free(comp_buf);
                comp_buf = tp;
            }
            if (comp_buf == NULL) {
                err_write("zlib: no memory.");
                return NULL;
            }
            memcpy(&comp_buf[*comp_size], outbuf, OUTBUFSIZE);
            *comp_size += OUTBUFSIZE;
            z.next_out = (unsigned char*)outbuf; /* 出力ポインタを元に戻す */
            z.avail_out = OUTBUFSIZE; /* 出力バッファ残量を元に戻す */
        }
    }

    /* 残りを吐き出す */
    rest_count = OUTBUFSIZE - z.avail_out;
    if (rest_count > 0) {
        if (comp_buf == NULL) {
            comp_buf = (char*)malloc(rest_count);
        } else {
            char* tp;
            tp = (char*)realloc(comp_buf, *comp_size + rest_count);
            if (tp == NULL)
                free(comp_buf);
            comp_buf = tp;
        }
        if (comp_buf == NULL) {
            err_write("zlib: no memory.");
            return NULL;
        }
        memcpy(&comp_buf[*comp_size], outbuf, rest_count);
        *comp_size += rest_count;
    }

    /* 後始末 */
    if (deflateEnd(&z) != Z_OK) {
        err_write("zlib: deflateEnd error: %s", z.msg);
    }
    return comp_buf;
#else
    return NULL;
#endif  // HAVE_ZLIB
}

/*
 * gzip形式で圧縮されたバッファを展開します。
 *
 * 展開は zlib(http://zlib.net/) を使用して行ないます。
 *
 * comp_buf: 圧縮されたバッファ
 * comp_size: 圧縮されたバッファのサイズ
 * size: 展開されたサイズが設定されます。
 *
 * 戻り値
 *   展開された領域のポインタを返します。
 *   領域はsize+1が確保されているので文字列の場合は終端'\0'を設定することが可能です。
 *   このポインタ領域は関数内で動的に確保された領域なので使用後はgz_free()関数で解放する必要があります。
 */
char* gz_decomp(const char* comp_buf, int comp_size, int* size)
{
#ifdef HAVE_ZLIB
    z_stream z;  /* zlibとのやり取りの構造体 */
    int status;
    char outbuf[OUTBUFSIZE];
    char* decomp_buf = NULL;
    int rest_count;

    *size = 0;

    /* すべてのメモリ管理をライブラリに任せる */
    z.zalloc = Z_NULL;
    z.zfree = Z_NULL;
    z.opaque = Z_NULL;

    /* 初期化 */
    z.next_in = Z_NULL;
    z.avail_in = 0;
    if (inflateInit(&z) != Z_OK) {
        err_write("zlib: inflateInit error: %s", z.msg);
        return NULL;
    }

    z.next_out = (unsigned char*)outbuf;        /* 出力ポインタ */
    z.avail_out = OUTBUFSIZE;   /* 出力バッファ残量 */
    status = Z_OK;

    while (status != Z_STREAM_END) {
        if (z.avail_in == 0) {
            /* 初回のみ実行される */
            z.next_in = (unsigned char*)comp_buf;
            z.avail_in = comp_size;
       }

        /* 展開 */
        status = inflate(&z, Z_NO_FLUSH);

        if (status == Z_STREAM_END)
            break; /* 完了 */
        if (status != Z_OK) {
            /* エラー */
            err_write("zlib: inflate error: %s", z.msg);
            if (decomp_buf != NULL)
                free(decomp_buf);
            return NULL;
        }
        if (z.avail_out == 0) {
            /* 出力バッファが尽きれば decomp_buf にコピーする */
            if (decomp_buf == NULL) {
                decomp_buf = (char*)malloc(OUTBUFSIZE + 1);
            } else {
                char* tp;
                tp = (char*)realloc(decomp_buf, *size + OUTBUFSIZE + 1);
                if (tp == NULL)
                    free(decomp_buf);
                decomp_buf = tp;
            }
            if (decomp_buf == NULL) {
                err_write("zlib: no memory.");
                return NULL;
            }
            memcpy(&decomp_buf[*size], outbuf, OUTBUFSIZE);
            *size += OUTBUFSIZE;
            z.next_out = (unsigned char*)outbuf; /* 出力ポインタを元に戻す */
            z.avail_out = OUTBUFSIZE; /* 出力バッファ残量を元に戻す */
        }
    }

    /* 残りを吐き出す */
    rest_count = OUTBUFSIZE - z.avail_out;
    if (rest_count > 0) {
        if (decomp_buf == NULL) {
            decomp_buf = malloc(rest_count + 1);
        } else {
            char* tp;
            tp = realloc(decomp_buf, *size + rest_count + 1);
            if (tp == NULL)
                free(decomp_buf);
            decomp_buf = tp;
        }
        if (decomp_buf == NULL) {
            err_write("zlib: no memory.");
            return NULL;
        }
        memcpy(&decomp_buf[*size], outbuf, rest_count);
        *size += rest_count;
    }

    /* 後始末 */
    if (inflateEnd(&z) != Z_OK) {
        err_write("zlib: inflateEnd error: %s", z.msg);
    }
    return decomp_buf;
#else
    return NULL;
#endif  // HAVE_ZLIB
}

void gz_free(const char* ptr)
{
    if (ptr != NULL)
        free((void*)ptr);
}
