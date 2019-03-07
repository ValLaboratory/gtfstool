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

static int init_count = 0;
static CS_DEF(sock_critical_section);

void sock_initialize()
{
    if (init_count == 0) {
        CS_INIT(&sock_critical_section);
    }
    init_count++;
}

void sock_finalize()
{
    init_count--;
    if (init_count == 0) {
        CS_DELETE(&sock_critical_section);
    }
}

/*
 * サーバー構造体(struct sockaddr_in)を構築します。
 *
 * host: ホスト名または IPアドレスのポインタ
 * port: ポート番号
 * server: サーバー構造体のアドレス
 *
 * 戻り値
 *  成功した場合はゼロを返します。
 *  エラーの場合は -1 を返します。
 */
int sock_mkserver(const char* host,
                  ushort port,
                  struct sockaddr_in* server)
{
    struct hostent* servhost;
    int result = 0;

    /* gethostbyname(), gethostbyaddr()がスレッド対応でないため
     * クリティカルセクションで処理を行います。
     */
    CS_START(&sock_critical_section);
    servhost = gethostbyname(host);
    if (servhost == NULL) {
        unsigned long addr;

        addr = inet_addr(host);
        servhost = gethostbyaddr((char *)&addr, sizeof(addr), AF_INET);
        if (servhost == NULL) {
            err_write("sock_mkserver: %s can't IP address.", host);
            result = -1;
        }
    }

    if (result == 0) {
        /* make server */
        memset(server, '\0', sizeof(struct sockaddr_in)); 
        server->sin_family = AF_INET;
        memcpy(&server->sin_addr, servhost->h_addr, servhost->h_length);
        server->sin_port = htons(port);
    }
    CS_END(&sock_critical_section);
    return result;
}

/*
 * サーバーにソケットを使用して接続します。
 * ソケットには SO_REUSEADDR オプションが付加されて接続されます。
 *
 * host: ホスト名または IPアドレスのポインタ
 * port: ポート番号
 *
 * 戻り値
 *  ソケットを返します。
 *  エラーの場合は INVALID_SOCKET を返します。
 */
SOCKET sock_connect_server(const char* host, ushort port)
{
    int result;
    struct sockaddr_in server;
    SOCKET c_socket;
    int sock_optval = 1;

    result = sock_mkserver(host, port, &server);
    if (result < 0)
        return INVALID_SOCKET;

    c_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (c_socket < 0) {
        err_write("sock_connect_server: can't open socket: %d", strerror(errno));
        return INVALID_SOCKET;
    }

    setsockopt(c_socket, SOL_SOCKET, SO_REUSEADDR, &sock_optval, sizeof(sock_optval));

    if (connect(c_socket, (struct sockaddr*)&server, sizeof(server)) < 0) {
        err_write("sock_connect_server: can't connect to %s: %s", host, strerror(errno));
        SOCKET_CLOSE(c_socket);
        return INVALID_SOCKET;
    }
    return c_socket;
}

/*
 * 動作しているサーバーの IPアドレスを取得します。
 *
 * ip_addr: IPアドレスが設定される領域のポインタ
 *
 * 戻り値
 *  ip_addr のアドレスを返します。
 */
char* sock_local_addr(char* ip_addr)
{
    SOCKET s;
    struct in_addr dummy;
    struct sockaddr_in exaddr;
    struct sockaddr_in addr;
    int len;
    int status;

    *ip_addr = '\0';
    s = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (s < 0)
        return NULL;
    
    /* コネクションを確立（しようと）するためのダミーの IPアドレス */
    dummy.s_addr = inet_addr("192.0.2.1");
    /* inet_aton("192.0.2.1", &dummy); */
    memset(&exaddr, 0, sizeof(exaddr));
    exaddr.sin_family = AF_INET;
    memcpy((char*)&exaddr.sin_addr, &dummy, sizeof(dummy));
    
    if (connect(s, (struct sockaddr*)&exaddr, sizeof(exaddr)) < 0) {
        SOCKET_CLOSE(s);
        return NULL;
    }

    len = sizeof(addr);
    status = getsockname(s, (struct sockaddr*)&addr, (socklen_t*)&len);
    if (status >= 0)
        mt_inet_ntoa(addr.sin_addr, ip_addr);

    SOCKET_CLOSE(s);
    return (status >= 0)? ip_addr : NULL;
}

SOCKET sock_listen(ulong addr,
                   ushort port,
                   int backlog,
                   struct sockaddr_in* sockaddr)
{
    SOCKET sd;
    int sock_optval = 1;

    if (backlog < 1 || backlog > SOMAXCONN) {
        err_write("backlog number error: max %d", SOMAXCONN);
        backlog = SOMAXCONN;
    }

    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd == -1) {
        err_write("socket error: %s", strerror(errno));
        return INVALID_SOCKET;
    }

    /* プログラムを再起動した場合に bind で "Address already in use"に
     * なるのを防ぐために SO_REUSEADDR を指定します。
     * SO_REUSEADDR を指定しないとソケットをクローズしても一定時間（2分ぐらい）
     * TIME_WAITになり、起動時に bindエラーになるため。
     */
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &sock_optval, sizeof(sock_optval));

    memset(sockaddr, '\0', sizeof(struct sockaddr_in));
    sockaddr->sin_addr.s_addr = (uint)addr;
    sockaddr->sin_port        = htons(port);
    sockaddr->sin_family      = AF_INET;

    if (bind(sd, (struct sockaddr*)sockaddr, sizeof(struct sockaddr_in)) < 0) {
        err_write("bind error: %s", strerror(errno));
        SOCKET_CLOSE(sd);
        return INVALID_SOCKET;
    }

    if (listen(sd, backlog) < 0) {
        err_write("listen error: %s", strerror(errno));
        SOCKET_CLOSE(sd);
        return INVALID_SOCKET;
    }
    return sd;
}
