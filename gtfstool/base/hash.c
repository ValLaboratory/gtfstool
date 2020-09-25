/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * The MIT License
 *
 * Copyright (c) 2008-2020 YAMAMOTO Naoki
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
 * キーと値をペアで管理するハッシュテーブルの関数群です。
 * スレッドセーフで動作します。
 *
 * キーの最大サイズの制限はありません（2020/09/25以前は255バイトでした）。
 * キーの衝突が起こった場合はリンクリストで管理されます。
 *
 * ハッシュとして追加できる要素数に制限はありませんが、
 * capacity(ハッシュ要素数)が小さい場合はキー値からハッシュ値の変換で
 * 衝突が起きてしまい、同じハッシュ値にリンクリストでつながれるので
 * 検索コストが大きくなります。
 *
 * ハッシュ関数には MurmurHash2A, by Austin Appleby を使用しています。
 */

/*
 * ハッシュ要素のメモリを解放します。
 *
 * e: ハッシュ要素構造体のポインタ
 *
 * 戻り値
 *  なし
 */
static void free_element(struct hash_element_t* e)
{
    if (e == NULL)
        return;

    if (e->next != NULL) {
        while (e != NULL) {
            struct hash_element_t* en;

            en = e->next;
            free(e->key);
            free(e);
            e = en;
        }
    } else {
        free(e->key);
        free(e);
    }
}

/*
 * ハッシュ値を算出します。
 *
 * ht: ハッシュテーブル構造体のポインタ
 * key: キー値
 *
 * 戻り値
 *  ハッシュテーブルのインデックスを返します。
 */
#define HASH_SEED 1487
static int hash_calc(struct hash_t* ht, const char* key)
{
    unsigned int hash_val;

    hash_val = MurmurHash2A(key, (int)strlen(key), HASH_SEED);
    return (hash_val % ht->capacity);
}

/*
 * ハッシュテーブルからキーの要素を取得します。
 *
 * ht: ハッシュテーブル構造体のポインタ
 * key: キー値
 *
 * 戻り値
 *  ハッシュ要素構造体のポインタを返します。
 *  存在しない場合は NULL を返します。
 */
static struct hash_element_t* get_element(struct hash_t* ht, const char* key)
{
    int index;
    struct hash_element_t* e;

    index = hash_calc(ht, key);
    if (index < 0 || index >= ht->capacity)
        return NULL;
    e = ht->table[index];
    while (e) {
        if (strcmp(e->key, key) == 0) {
            return e;
        }
        e = e->next;
    }
    return NULL;
}

/*
 * ハッシュテーブルから要素の数を取得します。
 *
 * ht: ハッシュテーブル構造体のポインタ
 *
 * 戻り値
 *  要素数を返します。
 */
static int element_count(struct hash_t* ht)
{
    int n = 0;
    int i;

    for (i = 0; i < ht->capacity; i++) {
        struct hash_element_t* e;

        e = ht->table[i];
        while (e) {
            n++;
            e = e->next;
        }
    }
    return n;
}

/*
 * ハッシュテーブルからキーのポインタを配列に列挙します。
 *
 * ht: ハッシュテーブル構造体のポインタ
 * list: ポインタ配列
 * count: ポインタ配列の数
 *
 * 戻り値
 *  なし
 */
static void key_list(struct hash_t* ht, char** list, int count)
{
    int i;
    int n = 0;

    for (i = 0; i < ht->capacity; i++) {
        struct hash_element_t* e;

        e = ht->table[i];
        while (e) {
            list[n++] = e->key;
            if (n >= count)
                return;
            e = e->next;
        }
    }
}

/*
 * ハッシュテーブルから要素のポインタを配列に列挙します。
 *
 * ht: ハッシュテーブル構造体のポインタ
 * list: ポインタ配列
 * count: ポインタ配列の数
 *
 * 戻り値
 *  なし
 */
static void element_list(struct hash_t* ht, void** list, int count)
{
    int i;
    int n = 0;

    for (i = 0; i < ht->capacity; i++) {
        struct hash_element_t* e;

        e = ht->table[i];
        while (e) {
            list[n++] = e->value;
            if (n >= count)
                return;
            e = e->next;
        }
    }
}

/*
 * ハッシュテーブルの初期処理を行ないます。
 * データを管理するためのメモリを確保します。
 *
 * capacity: ハッシュ要素数
 *
 * 戻り値
 *  ハッシュテーブル構造体のポインタ
 */
APIEXPORT struct hash_t* hash_initialize(int capacity)
{
    struct hash_t *ht;

    ht = (struct hash_t*)malloc(sizeof(struct hash_t));
    if (ht == NULL) {
        err_write("hash: No memory.");
        return NULL;
    }

    ht->table = (struct hash_element_t**)calloc(capacity, sizeof(struct hash_element_t*));
    if (ht->table == NULL) {
        free(ht);
        err_write("hash: No memory.");
        return NULL;
    }
    ht->capacity = capacity;

    /* クリティカルセクションの初期化 */
    CS_INIT(&ht->critical_section);

    return ht;
}

/*
 * ハッシュテーブルの使用を終了します。
 * 確保された領域は解放されます。
 *
 * ht: ハッシュテーブル構造体のポインタ
 *
 * 戻り値
 *  なし
 */
APIEXPORT void hash_finalize(struct hash_t* ht)
{
    int i;

    /* クリティカルセクションの削除 */
    CS_DELETE(&ht->critical_section);

    if (ht == NULL)
        return;

    for (i = 0; i < ht->capacity; i++) {
        if (ht->table[i] != NULL)
            free_element(ht->table[i]);
    }
    free(ht->table);
    free(ht);
}

/*
 * ハッシュテーブルに追加します。
 * キーがすでに存在する場合は値が置換されます。
 *
 * ht: ハッシュテーブル構造体のポインタ
 * key: キー値
 * value: 値
 *
 * 戻り値
 *  正常に追加された場合はゼロを返します。
 *  エラーの場合は -1 を返します。
 */
APIEXPORT int hash_put(struct hash_t* ht, const char* key, const void* value)
{
    int index;
    struct hash_element_t* e;
    struct hash_element_t* new_e;
    int result = 0;

    CS_START(&ht->critical_section);

    /* すでにキーが登録済みであれば置き換えます。*/
    e = get_element(ht, key);
    if (e) {
        e->value = (void*)value;
    } else {
        /* 存在しないので新規に登録します。 */
        new_e = (struct hash_element_t*)malloc(sizeof(struct hash_element_t));
        if (new_e == NULL) {
            err_write("hash: No memory.");
            result = -1;
            goto final;
        }
        new_e->key = malloc(strlen(key)+1);
        strcpy(new_e->key, key);
        new_e->value = (void*)value;
        new_e->next = NULL;

        index = hash_calc(ht, key);
        e = ht->table[index];

        if (e) {
            /* キーの衝突が起きたのでリストの最後に追加します。*/
            while (e->next) {
                e = e->next;
            }
            e->next = new_e;
        } else {
            ht->table[index] = new_e;
        }
    }

final:
    CS_END(&ht->critical_section);
    return result;
}

/*
 * ハッシュテーブルからキーの値を取得します。
 *
 * ht: ハッシュテーブル構造体のポインタ
 * key: キー値
 *
 * 戻り値
 *  キー値に対応した値を返します。
 *  存在しない場合は NULL を返します。
 */
APIEXPORT void* hash_get(struct hash_t* ht, const char* key)
{
    struct hash_element_t* e;
    void* v = NULL;

    CS_START(&ht->critical_section);
    e = get_element(ht, key);
    if (e)
        v = e->value;

    CS_END(&ht->critical_section);
    return v;
}

/*
 * ハッシュテーブルから要素を削除します。
 *
 * ht: ハッシュテーブル構造体のポインタ
 * key: キー値
 *
 * 戻り値
 *  削除された場合はゼロを返します。
 *  エラーの場合は -1 を返します。
 */
APIEXPORT int hash_delete(struct hash_t* ht, const char* key)
{
    int index;
    struct hash_element_t* e;
    struct hash_element_t* tmp = NULL;
    int result = -1;

    CS_START(&ht->critical_section);
    index = hash_calc(ht, key);
    e = ht->table[index];

    while (e) {
        if (strcmp(e->key, key) == 0) {
            if (tmp == NULL) {
                ht->table[index] = e->next;
            } else {
                tmp->next = e->next;
            }
            free(e->key);
            free(e);
            result = 0;
            break;
        }
        tmp = e;
        e = e->next;
    }
    CS_END(&ht->critical_section);
    return result;
}

/*
 * ハッシュテーブルから要素の数を取得します。
 *
 * ht: ハッシュテーブル構造体のポインタ
 *
 * 戻り値
 *  要素数を返します。
 */
APIEXPORT int hash_count(struct hash_t* ht)
{
    int n;

    CS_START(&ht->critical_section);
    n = element_count(ht);
    CS_END(&ht->critical_section);
    return n;
}

/*
 * ハッシュテーブルのキーをリストとして列挙して返します。
 *
 * 戻り値は以下のようなポインタ配列になります。
 * リストの最後に NULL が入ります。
 * +--------------+
 * | キーポインタ |
 * +--------------+
 * | キーポインタ |
 * +--------------+
 * | キーポインタ |
 * +--------------+
 * | NULL         |
 * +--------------+
 *
 * リスト使用後は hash_list_free()関数でメモリを解放します。
 *
 * ht: ハッシュテーブル構造体のポインタ
 *
 * 戻り値
 *  キーのポインタ配列を返します。
 *  エラーの場合は NULL を返します。
 */
APIEXPORT char** hash_keylist(struct hash_t* ht)
{
    char** list = NULL;
    int n;

    CS_START(&ht->critical_section);

    /* 要素の数を調べます。*/
    n = element_count(ht);
    list = (char**)malloc((n+1) * sizeof(void*));
    if (list != NULL) {
        if (n > 0)
            key_list(ht, list, n);
        list[n] = NULL;
    }
    CS_END(&ht->critical_section);
    return list;
}

/*
 * ハッシュテーブルの要素をリストとして列挙して返します。
 *
 * 戻り値は以下のようなポインタ配列になります。
 * リストの最後に NULL が入ります。
 * +----------+
 * | ポインタ |
 * +----------+
 * | ポインタ |
 * +----------+
 * | ポインタ |
 * +----------+
 * | NULL     |
 * +----------+
 *
 * リスト使用後は hash_list_free()関数でメモリを解放します。
 *
 * ht: ハッシュテーブル構造体のポインタ
 *
 * 戻り値
 *  要素のポインタ配列を返します。
 *  エラーの場合は NULL を返します。
 */
APIEXPORT void** hash_list(struct hash_t* ht)
{
    void** list = NULL;
    int n;

    CS_START(&ht->critical_section);

    /* 要素の数を調べます。*/
    n = element_count(ht);
    list = (void**)malloc((n+1) * sizeof(void*));
    if (list != NULL) {
        if (n > 0)
            element_list(ht, list, n);
        list[n] = NULL;
    }
    CS_END(&ht->critical_section);
    return list;
}

/*
 * ハッシュリストを解放します。
 *
 * list: ハッシュリストのポインタ
 *
 * 戻り値
 *  なし
 */
APIEXPORT void hash_list_free(void** list)
{
    if (list != NULL)
        free(list);
}

/*
//-----------------------------------------------------------------------------
// MurmurHash2A, by Austin Appleby

// This is a variant of MurmurHash2 modified to use the Merkle-Damgard
// construction. Bulk speed should be identical to Murmur2, small-key speed
// will be 10%-20% slower due to the added overhead at the end of the hash.

// This variant fixes a minor issue where null keys were more likely to
// collide with each other than expected, and also makes the algorithm
// more amenable to incremental implementations. All other caveats from
// MurmurHash2 still apply.
//
// http://sites.google.com/site/murmurhash/
*/
#define mmix(h,k) { k *= m; k ^= k >> r; k *= m; h *= m; h ^= k; }

APIEXPORT unsigned int MurmurHash2A ( const void * key, int len, unsigned int seed )
{
    const unsigned int m = 0x5bd1e995;
    const int r = 24;
    unsigned int l = len;

    const unsigned char * data = (const unsigned char *)key;

    unsigned int h = seed;
    unsigned int t = 0;

    while(len >= 4)
    {
        unsigned int k = *(unsigned int*)data;

        mmix(h,k);

        data += 4;
        len -= 4;
    }

    switch(len)
    {
    case 3: t ^= data[2] << 16;
    case 2: t ^= data[1] << 8;
    case 1: t ^= data[0];
    };

    mmix(h,t);
    mmix(h,l);

    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;

    return h;
}
