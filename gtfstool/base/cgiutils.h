/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* cgiutils.c
 *
 * (c) 1995-1996 William E. Weinman
 *
 */
#ifndef _CGIUTILS_H_
#define _CGIUTILS_H_

/* prototypes */
#ifdef __cplusplus
extern "C" {
#endif

void splitword(char *out, char *in, char stop);
void unescape_url(char *url);

#ifdef __cplusplus
}
#endif

#endif /* cgiutils.h */
