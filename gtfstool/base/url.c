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

#include "common.h"

#ifdef HAVE_OPENSSL
#include <openssl/ssl.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#endif

static int get_urlhost(const char* url,
                       char *host,
                       ushort* port,
                       char* path,
                       int* ssl_mode)
{
    int http_mode = 0;
    int check_len = 0;
    char host_path[MAX_URI_LENGTH];

    *ssl_mode = 0;
    if (strlen(url) > BUF_SIZE-1){
        err_write("url length too long: %s", url);
        return -1;
    }

    if (strncmp(url, "https://", 8) == 0) {
        *ssl_mode = 1;
        check_len = 8;
    } else if (strncmp(url, "http://", 7) == 0) {
        http_mode = 1;
        check_len = 7;
    }

    if (*ssl_mode || http_mode) {
        char *p;

        substr(host_path, url, check_len, -1);
        p = strchr(host_path, '/');
        if (p != NULL) {
            strcpy(path, p);
            *p = '\0';
            strcpy(host, host_path);
        } else {
            strcpy(host, host_path);
        }

        *port = (*ssl_mode)? 443 : 80;
        p = strchr(host, ':');
        if (p != NULL){
            *port = (ushort)atoi(p+1);
            if (*port <= 0)
                *port = (*ssl_mode)? 443 : 80;
            *p = '\0';
        }
    } else {
        err_write("url format is not \"http(s)://host/path\"");
        return -1;
    }
    return 0;
}

#ifdef HAVE_OPENSSL
static SSL_CTX* ssl_init()
{
    SSL_CTX* ctx;
    char err_buf[256];
    
    /* OpenSSL のエラー文字列を読み込みます。*/
    SSL_load_error_strings();
    
    /* ライブラリの初期化 */
    SSL_library_init();
    
    /* 使用するプロトコルの指定 */
    ctx = SSL_CTX_new(SSLv23_client_method());
    if (ctx == NULL) {
        ERR_error_string_n(ERR_get_error(), err_buf, sizeof(err_buf));
        err_write("url: ssl ctx error: %s", err_buf);
        return NULL;
    }
    return ctx;
}

static int ssl_connect_proxy(SOCKET c_socket,
                             const char* host,
                             ushort port)
{
    char send_buf[BUF_SIZE];
    char* recv_ptr;
    int res_size;
    char protocol[BUF_SIZE];
    char status[BUF_SIZE];
    int result = 0;
    
    /* プロキシサーバーに対してhttpsサーバの接続を要求します。*/
    sprintf(send_buf, "CONNECT %s:%d HTTP/1.0\r\n\r\n", host, port);
    if (send_data(c_socket, send_buf, (int)strlen(send_buf)) < 0) {
        err_write("url: ssl proxy connect error: %s", strerror(errno));
        return -1;
    }
    /* プロキシサーバーから応答があるまで待ちます。*/
    if (wait_recv_data(c_socket, RCV_TIMEOUT_WAIT) < 0) {
        err_write("url: ssl proxy connect recv error.");
        return -1;
    }
    /* プロキシサーバーから受信します。*/
    recv_ptr = recv_data(c_socket, -1, 0, NULL, &res_size);
    if (recv_ptr == NULL) {
        err_write("url: ssl proxy connect recv data error.");
        return -1;
    }
    /* プロキシサーバーからのステータスを確認します。*/
    if (res_size < BUF_SIZE) {
        if (sscanf(recv_ptr, "%s %s", protocol, status) == 2) {
            if (atoi(status) != HTTP_OK)
                result = -1;
        } else {
            result = -1;
        }
    } else {
        result = -1;
    }
    if (result != 0)
        err_write("url: ssl proxy connect recv data error: %s", recv_ptr);
    recv_free(recv_ptr);
    return result;
}

static SSL* ssl_connect(SOCKET c_socket, SSL_CTX* ctx)
{
    SSL* ssl;
    char err_buf[256];
    
    /* コネクションを管理する SSL 構造体を作成 */
    ssl = SSL_new(ctx);
    if (ssl == NULL) {
        ERR_error_string_n(ERR_get_error(), err_buf, sizeof(err_buf));
        err_write("url: ssl error: %s", err_buf);
        return NULL;
    }
    
    /* ソケットと SSL の構造体を結びつけます。*/
    if (SSL_set_fd(ssl, c_socket) == 0) {
        ERR_error_string_n(ERR_get_error(), err_buf, sizeof(err_buf));
        err_write("url: ssl set fd error: %s", err_buf);
        return NULL;
    }
    
    /* 乱数生成(PRNG)初期化 */
    RAND_poll();
    while (RAND_status() == 0) {
        unsigned short rand_ret = rand() % 65536;
        RAND_seed(&rand_ret, sizeof(rand_ret));
    }
    
    /* SSLで接続（ハンドシェイク） */
    if (SSL_connect(ssl) != 1) {
        ERR_error_string_n(ERR_get_error(), err_buf, sizeof(err_buf));
        err_write("url: ssl connect error: %s", err_buf);
        return NULL;
    }
    return ssl;
}
#endif  /* HAVE_OPENSSL */

/*
 * 指定されたURLにリクエストを送信します。
 * https の ssl通信にも対応しています。
 *
 * url: リクエストを送信するURL("http://foo.bar.jp/foo/bar")
 *      https://foo.bar/ の場合はsslを使用したリクエストが行なわれます。
 * header: リクエストするHTTPヘッダー構造体のポインター
 * query: クエリ文字列("api=GetVersion&type=long")
 *        NULLの場合はGETメソッドを使用します。
 * proxy_server: プロキシサーバーを指定します。使用しない場合はNULLとします。
 * proxy_port: プロキシサーバーのポート番号を指定します。proxy_serverがNULLの場合は参照されません。
 * res_size: レスポンスデータのバイト数が設定されます。NULLを指定すると設定されません。
 *
 * 戻り値
 *  レスポンスデータを動的メモリ領域に確保して返します。
 *  呼び出し元ではこの領域をrecv_free()関数で解放する必要があります。
 *  エラーの場合はNULLが返されます。
 */
char* url_post(const char* url,
               struct http_header_t* header,
               const char* query,
               const char* proxy_server,
               ushort proxy_port,
               int* res_size)
{
    SOCKET c_socket;
    struct sockaddr_in server;
    ushort port;
    char host[MAX_URI_LENGTH], path[MAX_URI_LENGTH];
    int ssl_mode;
    char* user_agent;
    char buff[BUF_SIZE];
    int send_buf_size = MAX_URI_LENGTH;
    char* send_buf = NULL;
    char* recv_ptr = NULL;
    
#ifdef HAVE_OPENSSL
    SSL* ssl = NULL;
    SSL_CTX* ctx = NULL;
    char err_buf[256];
#endif

    /* url文字列から host, port, path, ssl_mode を取得します。*/
    if (get_urlhost(url, host, &port, path, &ssl_mode) < 0)
        return NULL;
    
#ifndef HAVE_OPENSSL
    if (ssl_mode) {
        err_write("url: disable SSL mode: %s", url);
        return NULL;
    }
#endif

    /* make server */
    if (proxy_server == NULL) {
        if (sock_mkserver(host, port, &server) < 0)
            return NULL;
    } else {
        if (sock_mkserver(proxy_server, proxy_port, &server) < 0)
            return NULL;
    }

    /* make socket */
    c_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (c_socket < 0) {
        err_write("url: can't open socket: %d", strerror(errno));
        return NULL;
    }
    /* make connection */
    /* プロキシ経由の場合はプロキシサーバーに接続します。*/
    if (connect(c_socket, (struct sockaddr*)&server, sizeof(server)) < 0) {
        err_write("url: can't connect: %s", strerror(errno));
        SOCKET_CLOSE(c_socket);
        return NULL;
    }
    
#ifdef HAVE_OPENSSL
    if (ssl_mode) {
        /* OpenSSLを初期化します。*/
        ctx = ssl_init();
        if (ctx == NULL)
            goto final;
        if (proxy_server != NULL) {
            /* プロキシサーバーにSSL接続要求を行ないます。*/
            if (ssl_connect_proxy(c_socket, host, port) < 0)
                goto final;
        }
        /* SSLサーバーへ接続します。*/
        ssl = ssl_connect(c_socket, ctx);
        if (ssl == NULL)
            goto final;
    }
#endif

    /* 送信バッファの確保 */
    if (query)
        send_buf_size += strlen(query);

    send_buf = (char*)malloc(send_buf_size);
    if (send_buf == NULL) {
        err_write("url: no memory.");
        goto final;
    }

    /* HTTPの送信 */
    if (proxy_server == NULL) {
        if (query == NULL) {
            sprintf(buff, "GET %s HTTP/1.1\r\n", path);
        } else {
            sprintf(buff, "POST %s HTTP/1.1\r\n", path);
        }
        strcpy(send_buf, buff);

        sprintf(buff, "Host: %s\r\n", host);
        strcat(send_buf, buff);
    } else {
        if (query == NULL) {
            sprintf(buff, "GET %s HTTP/1.1\r\n", url);
        } else {
            sprintf(buff, "POST %s HTTP/1.1\r\n", url);
        }
        strcpy(send_buf, buff);

        sprintf(buff, "Host: %s:%d\r\n", proxy_server, proxy_port);
        strcat(send_buf, buff);
    }

    /* HTTPヘッダーの編集 */
    user_agent = get_http_header(header, "User-Agent");
    if (user_agent != NULL) {
        sprintf(buff, "User-Agent: %s\r\n", user_agent);
        strcat(send_buf, buff);
    }

    if (header != NULL) {
        int i;

        /* User-Agent, Content-Type, Content-Length は
         * 別に設定しているのでここでは無視します。
         */
        for (i = 0; i < header->count; i++) {
            if (stricmp(header->vt[i].name, "User-Agent") == 0 ||
                stricmp(header->vt[i].name, "Content-Type") == 0 ||
                stricmp(header->vt[i].name, "Content-Length") == 0)
                continue;
            sprintf(buff, "%s: %s\r\n", header->vt[i].name, header->vt[i].value);
            strcat(send_buf, buff);
        }
    }

    if (query == NULL) {
        /* GET の場合はヘッダーの終わり */
        sprintf(buff, "\r\n");
        strcat(send_buf, buff);
    } else {
        char* content_type;

        /* POST の場合だけボディを編集します。*/
        content_type = get_http_header(header, "Content-Type");
        if (content_type == NULL)
            sprintf(buff, "Content-Type: application/x-www-form-urlencoded\r\n");
        else
            sprintf(buff, "Content-Type: %s\r\n", content_type);
        strcat(send_buf, buff);

        sprintf(buff, "Content-Length: %d\r\n\r\n", (int)strlen(query));
        strcat(send_buf, buff);

        strcat(send_buf, query);
    }

    /* サーバーへ送信 */
    if (ssl_mode) {
#ifdef HAVE_OPENSSL
        if (SSL_write(ssl, send_buf, (int)strlen(send_buf)) < 0) {
            ERR_error_string_n(ERR_get_error(), err_buf, sizeof(err_buf));
            err_write("ssl write error url: %s %s [%s]", url, err_buf, send_buf);
            goto final;
        }
#endif
    } else {
        if (send_data(c_socket, send_buf, (int)strlen(send_buf)) < 0) {
            err_write("send error url: %s %s [%s]", url, strerror(errno), send_buf);
            goto final;
        }
    }

    /* サーバーから応答があるまで最大60秒待ちます。*/
    if (wait_recv_data(c_socket, 60000) > 0) {
        /* 受信サイズチェックなし、最大待ち時間3秒 */
#ifdef HAVE_OPENSSL
        recv_ptr = recv_data(c_socket, -1, 3000, ssl, res_size);
#else
        recv_ptr = recv_data(c_socket, -1, 3000, NULL, res_size);
#endif
    } else {
        err_write("url timeout: [%s]", url);
    }

final:
#ifdef HAVE_OPENSSL
    if (ssl_mode) {
        /* SSL shutdown */
        if (ssl != NULL)
            SSL_shutdown(ssl);
        
        /* 使用メモリの解放 */
        if (ssl != NULL)
            SSL_free(ssl);
        if (ctx != NULL)
            SSL_CTX_free(ctx);
        ERR_free_strings();
    }
#endif

    /* close socket */
    SOCKET_CLOSE(c_socket);

    /* 送信バッファの解放 */
    if (send_buf != NULL)
        free(send_buf);
    return recv_ptr;
}

/*
 * URLレスポンスの先頭行から HTTP status を取得します。
 *
 * msg: レスポンスデータ
 *
 * 戻り値
 *  HTTP status を整数として返します。
 *  エラーの場合はゼロを返します。
 */
int url_http_status(const char* msg)
{
    int index;
    int status;
    char* str;
    char* ver;
    char* stno;
    char* stmsg;

    /* 先頭行を取り出します。*/
    index = indexof(msg, '\r');
    if (index < 0)
        return 0;

    str = (char*)alloca(index+1);
    substr(str, msg, 0, index);

    /* 先頭行の "HTTP/1.x ステータス番号 補足メッセージ" を取得 */
    ver = (char*)alloca(index+1);
    stno = (char*)alloca(index+1);
    stmsg = (char*)alloca(index+1);
    if (sscanf(str, "%s %s %s", ver, stno, stmsg) != 3)
        return 0;
    status = atoi(stno);
    return status;
}
