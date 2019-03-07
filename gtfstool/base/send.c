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

/*
 * データを送信します。
 *
 * socket: 送信するソケット
 * buf: 送信バッファ
 * bufsize: 送信するサイズ
 *
 * 戻り値
 *  送信したバイト数を返します。
 *  エラーの場合は -1 を返します。
 */
int send_data(SOCKET socket, const void* buf, int buf_size)
{
    return safe_send(socket, buf, buf_size, 0);
}

/*
 * 2バイト整数値を送信します。
 *
 * socket: 送信するソケット
 * data: 送信2バイト整数
 *
 * 戻り値
 *  送信したバイト数を返します。
 *  エラーの場合は -1 を返します。
 */
int send_short(SOCKET socket, short data)
{
    return safe_send(socket, (const void*)&data, sizeof(short), 0);
}

/*
 * 4バイト整数値を送信します。
 *
 * socket: 送信するソケット
 * data: 送信4バイト整数
 *
 * 戻り値
 *  送信したバイト数を返します。
 *  エラーの場合は -1 を返します。
 */
int send_int(SOCKET socket, int data)
{
    return safe_send(socket, (const void*)&data, sizeof(int), 0);
}

/*
 * 8バイト整数値を送信します。
 *
 * socket: 送信するソケット
 * data: 送信8バイト整数
 *
 * 戻り値
 *  送信したバイト数を返します。
 *  エラーの場合は -1 を返します。
 */
int send_int64(SOCKET socket, int64 data)
{
    return safe_send(socket, (const void*)&data, sizeof(int64), 0);
}
