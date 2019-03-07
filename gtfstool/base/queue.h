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
#ifndef _QUEUE_H_
#define _QUEUE_H_

#include "csect.h"
#include "apiexp.h"

/* queue data struct */
struct queue_data_t {
    void* data;                 /* data pointer */
    struct queue_data_t* next;  /* next queue data */
};

/* queue strcut */
struct queue_t {
    CS_DEF(critical_section);
    struct queue_data_t* top;
    struct queue_data_t* last;
};

/* prototypes */
#ifdef __cplusplus
extern "C" {
#endif

APIEXPORT struct queue_t* que_initialize(void);
APIEXPORT void que_finalize(struct queue_t* que);
APIEXPORT int que_empty(struct queue_t* que);
APIEXPORT int que_push(struct queue_t* que, void* data);
APIEXPORT void* que_pop(struct queue_t* que);
APIEXPORT int que_count(struct queue_t* que);

#ifdef __cplusplus
}
#endif

#endif /* _QUEUE_H_ */
