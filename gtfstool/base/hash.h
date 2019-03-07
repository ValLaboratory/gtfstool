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
#ifndef _HASH_H_
#define _HASH_H_

#include "csect.h"
#include "apiexp.h"

#define MAX_HASH_KEYSIZE 255

struct hash_element_t {
    char key[MAX_HASH_KEYSIZE+1];
    void* value;
    struct hash_element_t* next;
};

struct hash_t {
    CS_DEF(critical_section);
    int capacity;
    struct hash_element_t** table;
};

/* prototypes */
#ifdef __cplusplus
extern "C" {
#endif

APIEXPORT struct hash_t* hash_initialize(int capacity);
APIEXPORT void hash_finalize(struct hash_t* ht);
APIEXPORT int hash_put(struct hash_t* ht, const char* key, const void* value);
APIEXPORT void* hash_get(struct hash_t* ht, const char* key);
APIEXPORT int hash_delete(struct hash_t* ht, const char* key);
APIEXPORT int hash_count(struct hash_t* ht);
APIEXPORT char** hash_keylist(struct hash_t* ht);
APIEXPORT void** hash_list(struct hash_t* ht);
APIEXPORT void hash_list_free(void** list);
APIEXPORT unsigned int MurmurHash2A(const void * key, int len, unsigned int seed);

#ifdef __cplusplus
}
#endif

#endif /* _HASH_H_ */
