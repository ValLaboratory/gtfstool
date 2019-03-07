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
 * ポインタを管理するベクターテーブルの関数群です。
 * スレッドセーフで動作します。
 *
 * 管理できるポインタ数に制限はありません。
 * init_capacity(要素数)が足りなくなった場合は自動的に増加します。
 */

#define INC_SIZE 100

static int increase_vector(struct vector_t* vt)
{
    void** tp;
    
    /* 管理領域の増分 */
    tp = (void**)realloc(vt->ptr, (vt->capacity+INC_SIZE) * sizeof(void*));
    if (tp == NULL) {
        err_write("vector: no memory.");
        return -1;
    }
    vt->ptr = tp;
    vt->capacity += INC_SIZE;
    return 0;
}

/*
 * ベクタテーブルの初期処理を行ないます。
 * データを管理するためのメモリを確保します。
 *
 * init_capacity: ベクタ数の初期値
 *
 * 戻り値
 *  ベクタ構造体のポインタ
 */
APIEXPORT struct vector_t* vect_initialize(int init_capacity)
{
    struct vector_t *vt;

    vt = (struct vector_t*)malloc(sizeof(struct vector_t));
    if (vt == NULL) {
        err_write("vector: no memory.");
        return NULL;
    }

    vt->ptr = (void**)calloc(init_capacity, sizeof(void*));
    if (vt->ptr == NULL) {
        free(vt);
        err_write("vector: no memory.");
        return NULL;
    }
    vt->capacity = init_capacity;
    vt->count = 0;

    /* クリティカルセクションの初期化 */
    CS_INIT(&vt->critical_section);

    return vt;
}

/*
 * ベクタテーブルにポインタを追加します。
 *
 * vt: ベクタテーブル構造体のポインタ
 * ptr: 追加するポインタ値
 *
 * 戻り値
 *  正常に追加された場合はゼロを返します。
 *  エラーの場合は -1 を返します。
 */
APIEXPORT int vect_append(struct vector_t* vt, const void* ptr)
{
    int result = 0;

    CS_START(&vt->critical_section);
    if (vt->count >= vt->capacity)
        result = increase_vector(vt);

    if (result == 0) {
        vt->ptr[vt->count] = (void*)ptr;
        vt->count++;
    }
    CS_END(&vt->critical_section);
    return result;
}

/*
 * ベクタテーブルからポインタを削除します。
 *
 * vt: ベクタテーブル構造体のポインタ
 * ptr: 削除するポインタ値
 *
 * 戻り値
 *  ベクタテーブルのポインタの数を返します。
 */
APIEXPORT int vect_delete(struct vector_t* vt, const void* ptr)
{
    int i;

    CS_START(&vt->critical_section);
    for (i = 0; i < vt->count; i++) {
        if (vt->ptr[i] == ptr) {
            /* 数をマイナスします。*/
            vt->count--;
            if (i < vt->count) {
                int size;

                /* 領域をシフトします。*/
                size = (vt->count - i) * sizeof(void*);
                memmove(&vt->ptr[i], &vt->ptr[i+1], size);
            }
            break;
        }
    }
    CS_END(&vt->critical_section);
    return vt->count;
}

/*
 * ベクタテーブルにポインタを挿入します。
 *
 * vt: ベクタテーブル構造体のポインタ
 * index: 挿入する位置のゼロからのインデックス
 * ptr: 挿入するポインタ値
 *
 * 戻り値
 *  正常に追加された場合はゼロを返します。
 *  エラーの場合は -1 を返します。
 */
APIEXPORT int vect_insert(struct vector_t* vt, int index, const void* ptr)
{
    int result = 0;

    CS_START(&vt->critical_section);
    if (index < 0 || index >= vt->count) {
        err_write("vector: index is out of bounds[%d], count=%d.", index, vt->count);
        result = -1;
    } else {
        if (vt->count >= vt->capacity)
            result = increase_vector(vt);

        if (result == 0) {
            if (index < vt->count) {
                int size;
                
                /* 領域をシフトします。*/
                size = (vt->count - index) * sizeof(void*);
                memmove(&vt->ptr[index+1], &vt->ptr[index], size);
            }
            vt->ptr[index] = (void*)ptr;
            vt->count++;
        }
    }
    CS_END(&vt->critical_section);
    return result;
}

/*
 * ベクタテーブルのポインタを更新します。
 *
 * vt: ベクタテーブル構造体のポインタ
 * ptr: 元のポインタ
 * new_ptr: 更新するポインタ
 *
 * 戻り値
 *  正常に追加された場合はゼロを返します。
 *  エラーの場合は -1 を返します。
 */
APIEXPORT int vect_update(struct vector_t* vt, const void* ptr, const void* new_ptr)
{
    int i;
    int result = -1;

    CS_START(&vt->critical_section);
    for (i = 0; i < vt->count; i++) {
        if (vt->ptr[i] == ptr) {
            vt->ptr[i] = (void*)new_ptr;
            result = 0;
            break;
        }
    }
    CS_END(&vt->critical_section);
    return result;
}

/*
 * ベクタテーブルからポインタ値を取得します。
 *
 * vt: ベクタテーブル構造体のポインタ
 * index: ゼロからのインデックス
 *
 * 戻り値
 *  ベクタテーブルのポインタを返します。
 *  エラーの場合は NULL を返します。
 */
APIEXPORT void* vect_get(struct vector_t* vt, int index)
{
    void* p = NULL;

    CS_START(&vt->critical_section);
    if (index < 0 || index >= vt->count)
        err_write("vector: index is out of bounds[%d], count=%d.", index, vt->count);
    else
        p = vt->ptr[index];
    CS_END(&vt->critical_section);
    return p;
}

/*
 * ベクタテーブルの個数を返します。
 *
 * 個数は関数を呼び出した時点のものになります。
 * 関数の呼び出し後、ベクタへの追加や削除が行われた場合は内容を保証できません。
 *
 * vt: ベクタテーブル構造体のポインタ
 *
 * 戻り値
 *  ベクタテーブルのポインタの数を返します。
 */
APIEXPORT int vect_count(struct vector_t* vt)
{
    return vt->count;
}

/*
 * ベクタテーブルのポインタ配列を取得します。
 *
 * 取得したポインタ配列の内容は関数を呼び出した時点のものになります。
 * 関数の呼び出し後、ベクタへの追加や削除が行われた場合は内容を保証できません。
 *
 * vt: ベクタテーブル構造体のポインタ
 * list: ポインタ配列が設定される領域のポインタ
 * count: ポインタ配列の個数
 *
 * 戻り値
 *  ポインタ配列の個数を返します。
 */
APIEXPORT int vect_list(struct vector_t* vt, void** list, int count)
{
    int n;
    int i;

    CS_START(&vt->critical_section);
    n = (count < vt->count)? count : vt->count;
    for (i = 0; i < n; i++)
        list[i] = vt->ptr[i];
    CS_END(&vt->critical_section);
    return n;
}

/*
 * ベクタテーブルの使用を終了します。
 *
 * vt: ハッシュテーブル構造体のポインタ
 *
 * 戻り値
 *  なし
 */
APIEXPORT void vect_finalize(struct vector_t* vt)
{
    /* クリティカルセクションの削除 */
    CS_DELETE(&vt->critical_section);
    /* ベクタ構造体の解放 */
    free(vt->ptr);
    free(vt);
}
