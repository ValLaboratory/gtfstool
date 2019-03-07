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

/*
 * 受信データがあるか調べます。
 */
int wait_recv_data(SOCKET socket, int timeout_ms)
{
    struct timeval* tp = NULL;
    struct timeval timeout;
    fd_set rd;

    /* タイムアウト値設定 */
    if (timeout_ms >= 0) {
        if (timeout_ms >= 1000) {
            timeout.tv_sec  = timeout_ms / 1000;  /* 秒 */
            timeout.tv_usec = (timeout_ms % 1000) * 1000;  /* マイクロ秒 */
        } else {
            timeout.tv_sec  = 0;  /* 秒 */
            timeout.tv_usec = timeout_ms * 1000;  /* マイクロ秒 */
        }
        tp = &timeout;
    }

    FD_ZERO(&rd);
    FD_SET(socket, &rd);
    if (select(socket+1, &rd, NULL, NULL, tp) < 0)
        return 0;  /* not found */
    return FD_ISSET(socket, &rd);
}

int get_content_length(const char* p)
{
    int index;
    char lenbuf[16];

    /* Content-Length: xxxxx\r\n...
     *               |p|
     */
    index = indexof(p, '\r');
    if (index >= 0 && index < sizeof(lenbuf)) {
        substr(lenbuf, p, 0, index);
        trim(lenbuf);
        return atoi(lenbuf);
    }
    return 0;
}

/*
 * リクエストやレスポンスの受信データをすべて動的に確保したメモリに読み込みます。
 * データの最後に'\0'が設定されます。
 *
 * 受信データをすべて取り込むには、送信側がデータ送信後にソケットを
 * クローズしてくれればrecv()の戻り値がゼロになるので判定が可能になるが、
 * keep-aliveがonの場合はデータの終了を判定するのが難しい。
 * 送信側がデータを細切れで送ってきた場合もデータ到着の遅延も考慮しなければ
 * ならないため、終了判定が困難である。
 *
 * ここでは HTTPプロトコルに限定した終了判定を採用する。
 * メソッドがGET/HEADの場合は"\r\n\r\n"の文字列を受信したときに終了とする。
 * POSTやレスポンスデータの場合は、"\r\n\r\n"を受信したときに"Content-Length:"
 * ヘッダーを検索して送信バイト数を得る。"\r\n\r\n"以降のデータが送信バイト数に
 * 達したときに終了とする。
 * ただし、"Content-Length:"の値は故意に設定できるため正確性が担保されないので
 * タイムアウト時間を設定して受信できる仕様にする。
 *
 * socket: 受信するソケット
 * check_size: 最大受信サイズ（チェックしない場合は -1 を指定する）
 * timeout_ms: 受信するタイムアウト時間をミリ秒で指定します（無制限で待つ場合は -1 を指定する）
 * ssl: SSL通信を行なう場合は SSL構造体のポインタを指定します。通常は NULL を指定します。
 * recv_size: 受信したバイト数を設定します。NULLを指定した場合は設定されません。
 *
 * 戻り値
 *  受信データのアドレスを返します。
 *  呼び出し元では使用後にrecv_free()関数で解放する必要があります。
 */
char* recv_data(SOCKET socket, int check_size, int timeout_ms, void* ssl, int* recv_size)
{
    int recv_len;
    char buff[BUF_SIZE];
    char* res_ptr = NULL;
    int res_size = 0;
    int is_get = 0;
    int end_flag = 0;
    int body_index = -1;
    int content_length = 0;

    if (recv_size != NULL)
        *recv_size = 0;

    while (1) {
#ifdef HAVE_OPENSSL
        if (ssl)
            recv_len = SSL_read((SSL*)ssl, buff, sizeof(buff));
        else
#endif
        SAFE_SYSCALL(recv_len, (int)recv(socket, buff, sizeof(buff), 0));

        if (recv_len < 0) {
            if (res_ptr)
                free(res_ptr);
            if (recv_size != NULL)
                *recv_size = recv_len;
            return NULL;
        }
        if (recv_len == 0)
            break;  /* FIN受信 */

        if (res_ptr == NULL) {
            /* データが１バイトしか取得できない可能性もあるので行の先頭文字で判定する。
             * リクエスト行：GET, POST, HEAD
             * レスポンス行：HTTP1.x
             */
            is_get = (buff[0] != 'P' && buff[0] != 'H');
            res_ptr = (char*)malloc(recv_len+1);
        } else {
            char* tp;
            tp = (char*)realloc(res_ptr, res_size+recv_len+1);
            if (tp == NULL)
                free(res_ptr);
            res_ptr = tp;
        }
        if (res_ptr == NULL) {
            err_write("recv: No memory.");
            return NULL;
        }
        memcpy(&res_ptr[res_size], buff, recv_len);
        res_size += recv_len;
        res_ptr[res_size] = '\0';   /* NULL terminated. */

        /* ヘッダーの終わりを検索します。
         * 新たに読み込んだ内容だけを検索すれば効率的だが、\r\n\r\n が分断されてくる
         * 可能性もあるので毎回先頭から検索する。
         */
        if (body_index < 0) {
            body_index = indexofstr(res_ptr, "\r\n\r\n");
            if (body_index >= 0) {
                body_index += sizeof("\r\n\r\n") - 1;
                if (is_get) {
                    /* POST以外は受信完了とする。*/
                    end_flag = 1;
                    res_ptr[body_index] = '\0';  /* NULL terminated. */
                } else {
                    /* Content-Length:ヘッダーを検索してバイト数を取得する。*/
                    int len_index;
                    len_index = indexofstr(res_ptr, "Content-Length:");
                    if (len_index >= 0) {
                        len_index += sizeof("Content-Length:") - 1;
                        content_length = get_content_length(&res_ptr[len_index]);
                        /* Content-Length:分のデータを受信したかチェックします。*/
                        if (strlen(&res_ptr[body_index]) >= (size_t)content_length)
                            end_flag = 1;
                    } else {
                        /* Content-Length:ヘッダーがない場合は終了 */
                        end_flag = 1;
                    }
                }
            }
        } else {
            /* Content-Length:分のデータを受信したかチェックします。*/
            if (strlen(&res_ptr[body_index]) >= (size_t)content_length)
                end_flag = 1;
        }

        if (check_size >= 0) {
            /* 受信データサイズが最大値を超えていたらエラーとする */
            if (res_size > check_size) {
                err_write("recv: check size error.");
                free(res_ptr);
                return NULL;
            }
        }

        /* データの終了を判定する */
        if (end_flag)
            break;

        /* まだ受信データがあるかチェックする */
        if (! wait_recv_data(socket, timeout_ms))
            break;
    }
    if (recv_size != NULL)
        *recv_size = res_size;
    return res_ptr;
}

void recv_free(const char* ptr)
{
    if (ptr != NULL)
        free((void*)ptr);
}

/*
 * ソケットから最大バイト数分のchar型データを受信します。
 * 指定したバイト数分を確実に受信する場合は recv_nchar() を使用します。
 *
 * socket: ソケットハンドル
 * buf: 受信したデータが設定される領域のアドレス
 * bufsize: 領域のバイト数
 * status: 状態を示すアドレス
 *           正常に受信できた場合はゼロが設定されます。
 *           FINを受信した場合は 1 が設定されます。
 *           エラーの場合は -1 が設定されます。
 *
 * 戻り値
 *  正常に受信できた場合は受信したバイト数を返します。
 */
int recv_char(SOCKET socket, char* buf, int bufsize, int* status)
{
    int recv_len;

    *status = 0;
    SAFE_SYSCALL(recv_len, (int)recv(socket, buf, bufsize, 0));
    if (recv_len < 0) {
        *status = -1;
        return 0;
    }
    if (recv_len == 0) {
        *status = 1;  /* FIN受信 */
        return 0;
    }
    return recv_len;
}

/*
 * ソケットからバイト数分のchar型データを受信します。
 *
 * socket: ソケットハンドル
 * buf: 受信したデータが設定される領域のアドレス
 * bytes: 受信するバイト数
 * status: 状態を示すアドレス
 *           正常に受信できた場合はゼロが設定されます。
 *           FINを受信した場合は 1 が設定されます。
 *           エラーの場合は -1 が設定されます。
 *
 * 戻り値
 *  正常に受信できた場合は受信したバイト数を返します。
 */
int recv_nchar(SOCKET socket, char* buf, int bytes, int* status)
{
    int recved_bytes = 0;

    do {
        int rlen;
        int rbytes;

        rbytes = bytes - recved_bytes;
        rlen = recv_char(socket, &buf[recved_bytes], rbytes, status);
        if (rlen == 0 && status != 0)
            return 0;
        recved_bytes += rlen;
    } while (recved_bytes < bytes);
    return bytes;
}

/*
 * ソケットから short型のデータを受信します。
 * short型は符号付2バイト整数です。
 * バイト順はプラットフォームに依存します。
 *
 * socket: ソケットハンドル
 * status: 状態を示すアドレス
 *           正常に受信できた場合はゼロが設定されます。
 *           FINを受信した場合は 1 が設定されます。
 *           エラーの場合は -1 が設定されます。
 *
 * 戻り値
 *  受信したshortデータを返します。
 */
short recv_short(SOCKET socket, int* status)
{
    short data;
    int recv_len;

    *status = 0;
    SAFE_SYSCALL(recv_len, (int)recv(socket, (char*)&data, sizeof(short), 0));
    if (recv_len < 0) {
        *status = -1;
        return 0;
    }
    if (recv_len == 0) {
        *status = 1;  /* FIN受信 */
        return 0;
    }
    return data;
}

/*
 * ソケットから int型のデータを受信します。
 * int型は符号付4バイト整数です。
 * バイト順はプラットフォームに依存します。
 *
 * socket: ソケットハンドル
 * status: 状態を示すアドレス
 *           正常に受信できた場合はゼロが設定されます。
 *           FINを受信した場合は 1 が設定されます。
 *           エラーの場合は -1 が設定されます。
 *
 * 戻り値
 *  受信したintデータを返します。
 */
int recv_int(SOCKET socket, int* status)
{
    int data;
    int recv_len;

    *status = 0;
    SAFE_SYSCALL(recv_len, (int)recv(socket, (char*)&data, sizeof(int), 0));
    if (recv_len < 0) {
        *status = -1;
        return 0;
    }
    if (recv_len == 0) {
        *status = 1;  /* FIN受信 */
        return 0;
    }
    return data;
}

/*
 * ソケットから int64型のデータを受信します。
 * int64型は符号付8バイト整数です。
 * バイト順はプラットフォームに依存します。
 *
 * socket: ソケットハンドル
 * status: 状態を示すアドレス
 *           正常に受信できた場合はゼロが設定されます。
 *           FINを受信した場合は 1 が設定されます。
 *           エラーの場合は -1 が設定されます。
 *
 * 戻り値
 *  受信したint64データを返します。
 */
int64 recv_int64(SOCKET socket, int* status)
{
    int64 data;
    int recv_len;

    *status = 0;
    SAFE_SYSCALL(recv_len, (int)recv(socket, (char*)&data, sizeof(int64), 0));
    if (recv_len < 0) {
        *status = -1;
        return 0;
    }
    if (recv_len == 0) {
        *status = 1;  /* FIN受信 */
        return 0;
    }
    return data;
}

/*
 * ソケットから区切り文字列までのchar型データを受信します。
 *
 * 受信データに区切り文字列は含まれません。
 * データの最後に '\0' が付加されます。
 *
 * この関数は1バイトずつデータを読むので大量のデータを
 * 取得する場合は非効率です。
 *
 * socket: ソケットハンドル
 * data: 受信したデータが設定される領域のアドレス
 * size: 受信するデータ領域のバイト数
 * delimiter: 区切り文字列
 *
 * 戻り値
 *  正常に受信できた場合は受信したバイト数を返します。
 *  エラーの場合は -1 を返します。
 */
int recv_line(SOCKET socket, char* buf, int bufsize, const char* delimiter)
{
    int recv_size = 0;
    char* last_delimp;

    last_delimp = (char*)delimiter + strlen(delimiter) - 1;
    while (1) {
        char c;
        int recv_len;

        SAFE_SYSCALL(recv_len, (int)recv(socket, &c, sizeof(char), 0));

        if (recv_len < 0)
            return -1;
        if (recv_len == 0)
            return -1;    /* FIN受信 */

        if (recv_size+recv_len+1 > bufsize) {
            /* 受信バッファサイズ不足 */
            return -1;
        }

        buf[recv_size] = c;
        recv_size += recv_len;
        buf[recv_size] = '\0';

        if (c == *last_delimp) {
            int term_index;

            term_index = indexofstr(buf, delimiter);
            if (term_index >= 0) {
                buf[term_index] = '\0';  /* NULL terminated. */
                break;
            }
        }
    }
    return (int)strlen(buf);
}

/*
 * ソケットから区切り文字列までの文字列データを受信します。
 *
 * 文字列の領域は使用後に recv_free() で解放する必要があります。
 *
 * 受信データに区切り文字列は含まれません。
 * データの最後に '\0' が付加されます。
 *
 * socket: ソケットハンドル
 * delimiter: 区切り文字列
 * delim_add_flag: 区切り文字列をバッファの末尾に付加するフラグ
 *
 * 戻り値
 *  正常に受信できた場合は文字列領域のポインタを返します。
 *  エラーの場合は NULL を返します。
 */
char* recv_str(SOCKET socket, const char* delimiter, int delim_add_flag)
{
    char* buf;
    int alloc_size;
    int recv_size = 0;
    char* last_delimp;

    buf = (char*)malloc(BUF_SIZE+1);
    if (buf == NULL)
        return NULL;
    alloc_size = BUF_SIZE + 1;

    last_delimp = (char*)delimiter + strlen(delimiter) - 1;
    while (1) {
        char rbuf[BUF_SIZE];
        int recv_len;

        SAFE_SYSCALL(recv_len, (int)recv(socket, rbuf, sizeof(rbuf), 0));

        if (recv_len < 0) {
            free(buf);
            return NULL;
        }
        if (recv_len == 0) {
            free(buf);
            return NULL;    /* FIN受信 */
        }

        if (recv_size+recv_len+1 > alloc_size) {
            char* tp;

            /* 受信バッファサイズ不足 */
            tp = (char*)realloc(buf, alloc_size+BUF_SIZE);
            if (tp == NULL)
                return NULL;
            buf = tp;
            alloc_size += BUF_SIZE;
        }

        memcpy(&buf[recv_size], rbuf, recv_len);
        recv_size += recv_len;
        buf[recv_size] = '\0';

        if (buf[recv_size-1] == *last_delimp) {
           int term_index;

            term_index = indexofstr(buf, delimiter);
            if (term_index >= 0) {
                if (! delim_add_flag)
                    buf[term_index] = '\0';  /* NULL terminated. */
                break;
            }
        }
    }
    return buf;
}
