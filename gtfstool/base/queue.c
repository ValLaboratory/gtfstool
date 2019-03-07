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

/*
 * キューの初期処理を行ないます。
 * データを管理するためのメモリを確保します。
 *
 * 戻り値
 *  キュー構造体のポインタ
 */
APIEXPORT struct queue_t* que_initialize()
{
    struct queue_t *que;

    que = (struct queue_t*)malloc(sizeof(struct queue_t));
    if (que == NULL) {
        err_write("queue: no memory.");
        return NULL;
    }

    que->top = NULL;
    que->last = NULL;

    /* クリティカルセクションの初期化 */
    CS_INIT(&que->critical_section);

    return que;
}

/*
 * キューの使用を終了します。
 * 確保された領域は解放されます。
 *
 * que: キュー構造体のポインタ
 *
 * 戻り値
 *  なし
 */
APIEXPORT void que_finalize(struct queue_t* que)
{
    /* クリティカルセクションの削除 */
    CS_DELETE(&que->critical_section);
    free(que);
}

/*
 * キューが空か調べます。
 *
 * que: キュー構造体のポインタ
 *
 * 戻り値
 *  空の場合はゼロ以外の値を返します。
 *  キューにデータがある場合はゼロを返します。
 */
APIEXPORT int que_empty(struct queue_t* que)
{
    int empty;

    CS_START(&que->critical_section);
    empty = (que->top == NULL);
    CS_END(&que->critical_section);
    return empty;
}

/*
 * キューの最後にデータを追加します。
 *
 * que: キュー構造体のポインタ
 *
 * 戻り値
 *  正常に追加された場合はゼロを返します。
 *  エラーの場合は-1を返します。
 */
APIEXPORT int que_push(struct queue_t* que, void* data)
{
    struct queue_data_t* q;

    q = (struct queue_data_t*)malloc(sizeof(struct queue_data_t));
    if (q == NULL) {
        err_write("que_push: no memory.");
        return -1;
    }
    q->data = data;
    q->next = NULL;

    CS_START(&que->critical_section);
    if (que->top == NULL) {
        que->top = q;
        que->last = q;
    } else {
        que->last->next = q;
        que->last = q;
    }
    CS_END(&que->critical_section);
    return 0;
}

/*
 * キューから先頭データを取り出します。
 *
 * que: キュー構造体のポインタ
 *
 * 戻り値
 *  データのポインタを返します。
 *  キューが空の場合はNULLを返します。
 */
APIEXPORT void* que_pop(struct queue_t* que)
{
    void* data = NULL;

    CS_START(&que->critical_section);
    if (que->top != NULL) {
        struct queue_data_t* q;

        q = que->top;
        data = q->data;

        que->top = q->next;
        if (que->top == NULL)
            que->last = NULL;
        free(q);
    }
    CS_END(&que->critical_section);
    return data;
}

/*
 * キューにあるデータの個数を取得します。
 *
 * que: キュー構造体のポインタ
 *
 * 戻り値
 *  個数を返します。
 *  キューが空の場合はゼロを返します。
 */
APIEXPORT int que_count(struct queue_t* que)
{
    int n = 0;
    struct queue_data_t* q;

    CS_START(&que->critical_section);
    q = que->top;
    while (q != NULL) {
        n++;
        q = q->next;
    }
    CS_END(&que->critical_section);
    return n;
}
