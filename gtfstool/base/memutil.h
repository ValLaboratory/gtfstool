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
#ifndef _MEMUTIL_H_
#define _MEMUTIL_H_

#include "apiexp.h"

struct membuf_t {
    int alloc_size;
    char* buf;
    int size;
};

/* prototypes */
#ifdef __cplusplus
extern "C" {
#endif

APIEXPORT struct membuf_t* mb_alloc(int size);
APIEXPORT void mb_free(struct membuf_t* mb);
APIEXPORT int mb_append(struct membuf_t* mb, const char* buf, int size);
APIEXPORT void mb_reset(struct membuf_t* mb);
APIEXPORT char* mb_string(struct membuf_t* mb);

#ifdef __cplusplus
}
#endif

#endif /* _MEMUTIL_H_ */
