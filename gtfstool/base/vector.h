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
#ifndef _VECTOR_H_
#define _VECTOR_H_

#include "csect.h"
#include "apiexp.h"

struct vector_t {
    CS_DEF(critical_section);
    int capacity;   /* number of allocate */
    int count;      /* count */
    void** ptr;     /* pointer array */
};

/* prototypes */
#ifdef __cplusplus
extern "C" {
#endif

APIEXPORT struct vector_t* vect_initialize(int init_capacity);
APIEXPORT int vect_append(struct vector_t* vt, const void* ptr);
APIEXPORT int vect_delete(struct vector_t* vt, const void* ptr);
APIEXPORT int vect_insert(struct vector_t* vt, int index, const void* ptr);
APIEXPORT int vect_update(struct vector_t* vt, const void* ptr, const void* new_ptr);
APIEXPORT int vect_count(struct vector_t* vt);
APIEXPORT void* vect_get(struct vector_t* vt, int index);
APIEXPORT int vect_list(struct vector_t* vt, void** list, int count);
APIEXPORT void vect_finalize(struct vector_t* vt);

#ifdef __cplusplus
}
#endif

#endif /* _VECTOR_H_ */
