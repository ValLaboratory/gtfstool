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
#ifndef _COMMON_H_
#define _COMMON_H_

#ifndef MAC_OSX
#if defined(__APPLE__) && defined(__MACH__)
#define MAC_OSX
#endif
#endif

#ifdef _MSC_VER
#  if _MSC_VER >= 1400 /* Visual Studio 2005 and up */
#    pragma warning(disable:4996) // unsecure
#  endif
#endif

#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#ifndef _WIN32
#define _LARGEFILE_SOURCE
#define _FILE_OFFSET_BITS 64
#endif

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <errno.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <process.h>
#include <signal.h>
#include <sys/stat.h>
#include <direct.h>
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <limits.h>
#endif

#ifndef _WIN32
#define MAX_PATH  PATH_MAX
#define INVALID_SOCKET  (-1)
#endif

#ifndef __USE_MISC
#ifndef ushort
typedef unsigned short  ushort;
#endif
#ifndef uint
typedef unsigned int    uint;
#endif
#ifndef ulong
typedef unsigned long   ulong;
#endif
#endif

#ifndef uchar
typedef unsigned char   uchar;
#endif
#ifndef int32
typedef int int32;
#endif
#ifndef uint32
typedef unsigned int uint32;
#endif
#ifndef int64
typedef long long int64;
#endif
#ifndef uint64
typedef unsigned long long   uint64;
#endif

#ifndef _WIN32
#ifndef SOCKET
typedef int SOCKET;
#endif
#endif

#ifdef _WIN32
#define FILE_OPEN _open
#define FILE_CLOSE _close
#define FILE_READ _read
#define FILE_WRITE _write
#define FILE_SEEK _lseeki64
#define FILE_TRUNCATE _chsize_s
#define SOCKET_CLOSE closesocket
#define snprintf _snprintf
#define alloca _alloca
#define sleep Sleep
#define atoi64 _atoi64
#define getpid _getpid
#define mkdir _mkdir
#else
#define FILE_OPEN open
#define FILE_CLOSE close
#define FILE_READ read
#define FILE_WRITE safe_write
#define FILE_SEEK lseek
#define FILE_TRUNCATE ftruncate
#define SOCKET_CLOSE close
#define stricmp strcasecmp
#define strnicmp strncasecmp
#define atoi64 atoll
#endif

#ifdef _WIN32
#define CREATE_MODE     S_IREAD|S_IWRITE
#define S_ISDIR(m)      (m & _S_IFDIR)
#else
#define O_BINARY        0x0
#define CREATE_MODE     S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH
#endif
#define BUF_SIZE        2048    /* send, recv buffer size */

#define RCV_TIMEOUT_NOWAIT  0
#define RCV_TIMEOUT_WAIT    -1

/* http request info. */
#define MAX_URI_LENGTH      2047
#define MAX_CONTENT_NAME    255
#define MAX_REQ_HEADER      64
#define MAX_REQ_VARIABLE    128

#define MAX_RECV_DATA_SIZE      (16*1024*1024)    /* 4MB -> 16MB at 2013/11/10 */
#define MAX_METHOD_LINE_SIZE    (MAX_URI_LENGTH+32)

#define MAX_VNAME_SIZE  64      /* variable name max size */
#define MAX_VVALUE_SIZE 2000    /* variable value max size */

struct variable_t {
    char* name;                 /* name is NULL terminated */
    char* value;                /* value is NULL terminated
                                 but attach file data
                                 is not NULL terminated */
};

struct attach_file_t {
    char filename[MAX_PATH];             /* attach file name */
    char mimetype[MAX_VNAME_SIZE+1];     /* MIME type */
    char charset[MAX_VNAME_SIZE+1];      /* charset */
    unsigned char* data;                 /* data pointer(same value in struct_variable_t) */
    int size;                            /* data size */
};

struct http_header_t {
    int count;
    struct variable_t vt[MAX_REQ_HEADER];
};

struct query_param_t {
    int count;
    struct variable_t vt[MAX_REQ_VARIABLE];
    struct attach_file_t* af[MAX_REQ_VARIABLE];
};

struct request_t {
    char method[8];                         /* GET, POST or HEAD */
    char uri[MAX_URI_LENGTH+1];             /* uri string */
    char protocol[16];                      /* HTTP/1.1 */
    struct in_addr addr;                    /* client IP addr */
    int qs_index;                           /* '?'char index, notfound is -1 */
    char content_name[MAX_CONTENT_NAME+1];  /* document or web/service name(cut starting '/') */
    struct http_header_t header;            /* header parameters */
    struct query_param_t q_param;           /* query parameters */
    struct vector_t* heap;                  /* request heap memory */
    int64 start_time;                       /* start time(usec) */
};

struct response_t {
    SOCKET socket;                          /* output socket */
    int content_size;                       /* content-length */
};

/* HTTP Status */
#define HTTP_OK                     200
#define HTTP_NOT_MODIFIED           304
#define HTTP_BADREQUEST             400
#define HTTP_NOTFOUND               404
#define HTTP_REQUEST_TIMEOUT        408
#define HTTP_REQUEST_URI_TOO_LONG   414
#define HTTP_INTERNAL_SERVER_ERROR  500
#define HTTP_NOTIMPLEMENT           501

#include "apiexp.h"
#include "csect.h"
#include "mtfunc.h"
#include "strutil.h"
#include "queue.h"
#include "memutil.h"
#include "hash.h"
#include "vector.h"
#include "syscall.h"
#include "cgiutils.h"

/* prototypes */
#ifdef __cplusplus
extern "C" {
#endif

/* datetime.c */
APIEXPORT char* todays_date(char* buf, int buf_size, char* sep);
APIEXPORT char* todays_time(char* buf, int buf_size, char* sep);
APIEXPORT char* gmtstr(char* buf, int buf_size, struct tm* gmtime);
APIEXPORT char* now_gmtstr(char* buf, int buf_size);
APIEXPORT char* jststr(char* buf, int buf_size, struct tm* jstime);
APIEXPORT char* now_jststr(char* buf, int buf_size);
APIEXPORT int64 system_time(void);
APIEXPORT uint system_seconds(void);
APIEXPORT int is_valid_dates(int year, int month, int day);

/* error.c */
APIEXPORT void err_initialize(const char* error_file);
APIEXPORT void err_finalize(void);
APIEXPORT void err_write(const char* fmt, ...);

/* sock.c */
APIEXPORT void sock_initialize(void);
APIEXPORT void sock_finalize(void);
APIEXPORT int sock_mkserver(const char* host, ushort port, struct sockaddr_in* server);
APIEXPORT SOCKET sock_connect_server(const char* host, ushort port);
APIEXPORT char* sock_local_addr(char* ip_addr);
APIEXPORT SOCKET sock_listen(ulong addr, ushort port, int backlog, struct sockaddr_in* sockaddr);

/* recv.c */
APIEXPORT int wait_recv_data(SOCKET socket, int timeout_ms);
APIEXPORT char* recv_data(SOCKET socket, int check_size, int timeout_ms, void* ssl, int* recv_size);
APIEXPORT void recv_free(const char* ptr);
APIEXPORT int recv_char(SOCKET socket, char* buf, int bufsize, int* status);
APIEXPORT int recv_nchar(SOCKET socket, char* buf, int bytes, int* status);
APIEXPORT short recv_short(SOCKET socket, int* status);
APIEXPORT int recv_int(SOCKET socket, int* status);
APIEXPORT int64 recv_int64(SOCKET socket, int* status);
APIEXPORT int recv_line(SOCKET socket, char* buf, int bufsize, const char* delimiter);
APIEXPORT char* recv_str(SOCKET socket, const char* delimiter, int delim_add_flag);
    
/* send.c */
APIEXPORT int send_data(SOCKET socket, const void* buf, int buf_size);
APIEXPORT int send_short(SOCKET socket, short data);
APIEXPORT int send_int(SOCKET socket, int data);
APIEXPORT int send_int64(SOCKET socket, int64 data);

/* http_header.c */
APIEXPORT struct http_header_t* alloc_http_header(void);
APIEXPORT void init_http_header(struct http_header_t* hdr);
APIEXPORT void free_http_header(struct http_header_t* hdr);
APIEXPORT int set_http_header(struct http_header_t* hdr, const char* name, const char* value);
APIEXPORT int set_content_type(struct http_header_t* hdr, const char* type, const char* charset);
APIEXPORT int set_content_length(struct http_header_t* hdr, int length);
APIEXPORT int set_cookie(struct http_header_t* hdr, const char* name, const char* cvalue, const char* expires, long maxage, const char* domain, const char* path, int secure);
APIEXPORT char* get_http_header(struct http_header_t* hdr, const char* name);
APIEXPORT int split_item(char* str, struct variable_t* vt, char delim);
APIEXPORT void free_item(struct variable_t* vt);
APIEXPORT char* split_header(char* msg, struct http_header_t* hdr);
APIEXPORT int get_header_length(const char* msg);
APIEXPORT char* get_cookie(struct http_header_t* hdr, const char* cname, char* cvalue);
APIEXPORT int send_header(SOCKET socket, struct http_header_t* hdr);

/* url.c */
APIEXPORT char* url_post(const char* url, struct http_header_t* header, const char* query, const char* proxy_server, ushort proxy_port, int* res_size);
APIEXPORT int url_http_status(const char* msg);

#ifdef __cplusplus
}
#endif

#endif  /* _COMMON_H_ */
