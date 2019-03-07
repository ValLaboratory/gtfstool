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
#ifndef _STRUTIL_H_
#define _STRUTIL_H_

#include "apiexp.h"

/* prototypes */
#ifdef __cplusplus
extern "C" {
#endif

APIEXPORT char* chrep(char *str, const char target, const char rep);
APIEXPORT char* strrep(const char *src, const char* target, const char* rep, char* dst);
APIEXPORT int strchc(const char* str, const char target);
APIEXPORT int strstrc(const char* str, const char* target);
APIEXPORT int indexof(const char* str, const char target);
APIEXPORT int lastindexof(const char* str, const char target);
APIEXPORT int indexofstr(const char* str, const char* target);
APIEXPORT char* substr(char* dst, const char* src, int index, int len);
APIEXPORT char** split(char* str, char delim);
APIEXPORT void list_free(char** ptr);
APIEXPORT int list_count(const char** ptr);
APIEXPORT char* trim(char* str);
APIEXPORT char* skipsp(const char* str);
APIEXPORT char* quote(char* str);
APIEXPORT int convert(const char* src_enc, const char* src, int src_size, const char* dst_enc, char* dst, int dst_bufsize);
APIEXPORT char* tohex(char* dst, const void* src, int size);
APIEXPORT char* tochar(char *dst, const char *hex);
APIEXPORT int strmatch(const char *ptn, const char *str);
APIEXPORT int strmatchmb(const unsigned char *ptn, const unsigned char *str);
APIEXPORT int isdigitstr(const char *str);
APIEXPORT int isalphastr(const char *str);
APIEXPORT int isalnumstr(const char *str);
APIEXPORT int utf8_bytes(const char *c);
APIEXPORT int sjis_bytes(const char *c);
APIEXPORT char* toupperstr(char *s);
APIEXPORT char* tolowerstr(char *s);

#ifdef __cplusplus
}
#endif

#endif /* _STRUTIL_H_ */
