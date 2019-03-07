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

#define API_INTERNAL
#include "common.h"

#define AUTO_EXTEND_SIZE  1024

static int extend_buffer(struct membuf_t* mb, int ext_size)
{
    int nsize;
    char* tp;

    nsize = mb->alloc_size + ext_size;
    tp = (char*)realloc(mb->buf, nsize);
    if (tp == NULL)
        return -1;
    mb->buf = tp;
    mb->alloc_size = nsize;
    return 0;
}

/*
 * 可変長のメモリバッファを確保します。
 *
 * init_size: 初期に確保するバイト数
 *
 * 戻り値
 *  struct membuf_t のポインタを返します。
 *  メモリが確保できない場合は NULL を返します。
 */
APIEXPORT struct membuf_t* mb_alloc(int init_size)
{
    struct membuf_t* mb;

    mb = (struct membuf_t*)malloc(sizeof(struct membuf_t));
    if (mb == NULL)
        return NULL;

    mb->buf = (char*)malloc(init_size);
    if (mb->buf == NULL) {
        free(mb);
        return NULL;
    }
    mb->alloc_size = init_size;
    mb->size = 0;
    return mb;
}

/*
 * 可変長のメモリバッファを解放します。
 *
 * mb: struct membuf_t のポインタ
 *
 * 戻り値
 *  なし
 */
APIEXPORT void mb_free(struct membuf_t* mb)
{
    if (mb) {
        if (mb->buf)
            free(mb->buf);
        free(mb);
    }
}

/*
 * 可変長のメモリバッファの最後にデータを追加します。
 *
 * mb: struct membuf_t のポインタ
 * buf: 追加するデータポインタ
 * size: データサイズ
 *
 * 戻り値
 *  正常に処理された場合は現在のバイト数を返します。
 *  エラーの場合は -1 を返します。
 */
APIEXPORT int mb_append(struct membuf_t* mb, const char* buf, int size)
{
    if (mb->buf == NULL)
        return -1;
    if (size < 1)
        return mb->size;

    if (mb->size + size > mb->alloc_size) {
        if (extend_buffer(mb, size + AUTO_EXTEND_SIZE) < 0)
            return -1;
    }
    memcpy(&mb->buf[mb->size], buf, size);
    mb->size += size;
    return mb->size;
}


/*
 * 可変長のメモリバッファのサイズをゼロにします。
 * 確保された領域は解放されません。
 *
 * mb: struct membuf_t のポインタ
 *
 * 戻り値
 *  なし
 */
APIEXPORT void mb_reset(struct membuf_t* mb)
{
    mb->size = 0;
}


/*
 * 可変長のメモリバッファを文字列として返します。
 *
 * mb: struct membuf_t のポインタ
 *
 * 戻り値
 *  終端が'\0'の文字列のポインタを返します。
 *  領域が不足した場合は NULL を返します。
 */
APIEXPORT char* mb_string(struct membuf_t* mb)
{
    if (mb->size + 1 > mb->alloc_size) {
        /* 終端を追加する領域が不足している。*/
        if (extend_buffer(mb, AUTO_EXTEND_SIZE) < 0)
            return NULL;
    }
    mb->buf[mb->size] = '\0';
    return mb->buf;
}
