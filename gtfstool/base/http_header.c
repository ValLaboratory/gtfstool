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

static int get_header_index(struct http_header_t* hdr, const char* name)
{
    int i;

    for (i = 0; i < hdr->count; i++) {
        if (stricmp(hdr->vt[i].name, name) == 0)
            return i;
    }
    return -1;
}

/*
 * HTTPヘッダー構造体を確保します。
 *
 * 戻り値
 *  HTTPヘッダー構造体のポインタ
 */
struct http_header_t* alloc_http_header()
{
    struct http_header_t* hdr;

    hdr = (struct http_header_t*)calloc(1, sizeof(struct http_header_t));
    if (hdr == NULL) {
        err_write("header: no memory.");
        return NULL;
    }
    return hdr;
}

/*
 * HTTPヘッダー構造体を初期化します。
 *
 * この関数を実行すると以下のヘッダーが設定されます。
 *   Date: 現在日時
 *   Connection: close
 *
 * 戻り値
 *  HTTPヘッダー構造体のポインタ
 */
void init_http_header(struct http_header_t* hdr)
{
    char now_date[256]; 

    hdr->count = 0;

    /* 現在時刻をGMTで取得 */
    now_gmtstr(now_date, sizeof(now_date));

    /* Date */
    set_http_header(hdr, "Date", now_date);

    /* Connection */
    set_http_header(hdr, "Connection", "close");
}

/*
 * HTTPヘッダー構造体の動的に確保されたメモリを解放します。
 *
 * alloc_http_header()で確保したメモリをこの関数で解放します。
 *
 * hdr: HTTPヘッダー構造体のポインタ
 *
 * 戻り値
 *  なし
 */
void free_http_header(struct http_header_t* hdr)
{
    int i;

    for (i = 0; i < hdr->count; i++) {
        if (hdr->vt[i].name != NULL)
            free(hdr->vt[i].name);
    }
    free(hdr);
}

/*
 * HTTPヘッダー構造体にヘッダーを設定します。
 *
 * ヘッダー名がすでに存在している場合は置換されます。
 * ただし、Set-Cookieヘッダーは置換されずに追加されます。
 *
 * hdr: HTTPヘッダー構造体のポインター
 * name: ヘッダー名
 * value: ヘッダー値
 *
 * 戻り値
 *  ヘッダーの個数を返します。
 *  エラーの場合は -1 を返します。
 */
int set_http_header(struct http_header_t* hdr, const char* name, const char* value)
{
    int index = -1;
    char* tp;

    if (hdr == NULL)
        return -1;

    if (strlen(name) > MAX_VNAME_SIZE)
        return -1;
    if (strlen(value) > MAX_VVALUE_SIZE)
        return -1;

    /* ヘッダーがすでに存在するか調べます。*/
    if (stricmp(name, "Set-Cookie") != 0)
        index = get_header_index(hdr, name);

    if (index >= 0) {
        /* 置換します。*/
        tp = (char*)realloc(hdr->vt[index].name, strlen(name) + strlen(value) + 2);
        if (tp == NULL)
            return -1;
    } else {
        /* 追加します。*/
        if (hdr->count >= MAX_REQ_HEADER)
            return -1;
        tp = (char*)malloc(strlen(name) + strlen(value) + 2);
        if (tp == NULL)
            return -1;
        index = hdr->count++;
    }
    hdr->vt[index].name = tp;
    strcpy(hdr->vt[index].name, name);
    hdr->vt[index].value = tp + strlen(name) + 1;
    strcpy(hdr->vt[index].value, value);
    return hdr->count;
}

/*
 * HTTPヘッダー構造体に"Content-type"ヘッダーを設定します。
 *
 * "Content-type"ヘッダーがすでに存在している場合は置換されます。
 * charset に NULL を指定した場合は "charset=XXXXXX" は付加されません。
 *
 * hdr: HTTPヘッダー構造体のポインター
 * type: コンテンツタイプ
 * charset: 文字エンコーディング名
 *
 * 戻り値
 *  設定されているヘッダーの個数を返します。
 *  エラーの場合は -1 を返します。
 */
int set_content_type(struct http_header_t* hdr, const char* type, const char* charset)
{
    if (charset == NULL) {
        return set_http_header(hdr, "Content-type", type);
    } else {
        char buf[1024];

        snprintf(buf, sizeof(buf), "%s; charset=%s", type, charset);
        return set_http_header(hdr, "Content-type", buf);
    }
}

/*
 * HTTPヘッダー構造体に"Content-length"ヘッダーを追加します。
 *
 * "Content-length"ヘッダーがすでに存在している場合は置換されます。
 *
 * hdr: HTTPヘッダー構造体のポインター
 * length: バイト数
 *
 * 戻り値
 *  設定されているヘッダーの個数を返します。
 *  エラーの場合は -1 を返します。
 */
int set_content_length(struct http_header_t* hdr, int length)
{
    char value[128];

    snprintf(value, sizeof(value), "%d", length);
    return set_http_header(hdr, "Content-length", value);
}

/*
 * HTTPヘッダー構造体に"Set-Cookie"ヘッダーを追加します。
 *
 * Set-Cookie: NAME=VALUE; expires=DATE; path=PATH; domain=DOMAIN_NAME; secure
 * Set-Cookie: CUSTOMER=WILE_E_COYOTE; path=/; expires=Wednesday, 09-Nov-99 23:12:40 GMT
 *
 * hdr: HTTPヘッダー構造体のポインター
 * name: Cookie名称
 * cvalue: 値
 * expires: 有効期限（NULLは指定なし）
 * maxage: 有効秒数（ゼロは指定なし、expiresが指定されている場合は無視される）
 * domain: ドメイン（NULLは指定なし）
 * path: パス（NULLは指定なし）
 * secure: セキュアの場合はゼロ以外を指定
 *
 * 戻り値
 *  設定されているヘッダーの個数を返します。
 *  エラーの場合は -1 を返します。
 */
int set_cookie(struct http_header_t* hdr,
               const char* name,
               const char* cvalue,
               const char* expires,
               long maxage,
               const char* domain,
               const char* path,
               int secure)
{
    char* value;

    if (strlen(name) > MAX_VNAME_SIZE) {
        err_write("set_cookie: name is too large: %s", name);
        return -1;
    }
    if (strlen(cvalue) > MAX_VVALUE_SIZE) {
        err_write("set_cookie: value is too large: %s", cvalue);
        return -1;
    }

    value = (char*)alloca(4096);  /* Cookie 仕様(4KBまで) */
    *value = '\0';
    sprintf(value, "%s=%s;", name, cvalue);
    if (expires != NULL) {
        if (strlen(expires) > 50) {
            err_write("set_cookie: illegal expires format: %s", expires);
            return -1;
        }
        strcat(value, " expires=");
        strcat(value, expires);
        strcat(value, ";");
    } else {
        if (maxage > 0L) {
            char tbuf[20];
            strcat(value, " max-age=");
            snprintf(tbuf, sizeof(tbuf), "%ld", maxage);
            strcat(value, tbuf);
            strcat(value, ";");
        }
    }
    if (domain != NULL) {
        if (strlen(domain) > 250) {
            err_write("set_cookie: domain is too large: %s", domain);
            return -1;
        }
        strcat(value, " domain=");
        strcat(value, domain);
        strcat(value, ";");
    }
    if (path != NULL) {
        if (strlen(path) > MAX_PATH) {
            err_write("set_cookie: path is too large: %s", path);
            return -1;
        }
        strcat(value, " path=");
        strcat(value, path);
        strcat(value, ";");
    }
    if (secure != 0)
        strcat(value, " secure");

    return set_http_header(hdr, "Set-Cookie", value);
}


/*
 * HTTPヘッダー構造体から指定されたヘッダー値を取得します。
 * ヘッダー名は大文字と小文字を識別しません。
 *
 * hdr: HTTPヘッダー構造体のポインター
 * name: 取得するヘッダー名
 *
 * 戻り値
 *  ヘッダー値（文字列）のポインターを返します。
 *  ヘッダー名が存在しない場合はNULLを返します。
 */
char* get_http_header(struct http_header_t* hdr, const char* name)
{
    int index;

    if (hdr == NULL)
        return NULL;
    index = get_header_index(hdr, name);
    if (index >= 0)
        return hdr->vt[index].value;
    return NULL;    // notfound
}

/*
 * 文字列 str を区切り文字 delim で分割して name と value に設定します。
 *
 * 戻り値
 *  成功した場合はゼロを返します。
 *  エラーの場合は -1 を返します。
 */
int split_item(char* str, struct variable_t* vt, char delim)
{
    char tbuf[MAX_VNAME_SIZE+MAX_VVALUE_SIZE+2];

    /* '+1' is delim char*/
    if (strlen(str) > MAX_VNAME_SIZE+MAX_VVALUE_SIZE+1)
        return -1;  // size error
    splitword(tbuf, str, delim);
    if (strlen(tbuf) > MAX_VNAME_SIZE)
        return -1;
    if (strlen(str) > MAX_VVALUE_SIZE)
        return -1;

    vt->name = (char*)malloc(strlen(tbuf) + strlen(str) + 2);
    if (vt->name == NULL) {
        err_write("split_item: no memory");
        return -1;
    }

    strcpy(vt->name, tbuf);
    vt->value = vt->name + strlen(tbuf) + 1;
    strcpy(vt->value, str);
    return 0;
}

/*
 * split_item()で確保されたメモリを解放します。
 *
 * 戻り値
 *  なし
 */
void free_item(struct variable_t* vt)
{
    if (vt != NULL && vt->name != NULL)
        free(vt->name);
}

/*
 * レスポンスデータからヘッダーをHTTPヘッダー構造体に設定します。
 * msgの内容は更新されますので注意してください。
 * hdrは初期化されるので項目が存在する場合はメモリリークを起こします。
 * hdrの項目がないHTTPヘッダー構造体を使用してください。
 *
 * msg: レスポンスデータ
 * hdr: HTTPヘッダー構造体のポインター
 *
 * 戻り値
 *  ボディデータのポインターを返します。
 *  エラーの場合は NULL を返します。
 */
char* split_header(char* msg, struct http_header_t* hdr)
{
    char* delim_ptr;
    char* body_ptr;
    char** list_ptr;
    int n;
    char* tp;

    /* ヘッダー初期化 */
    memset(hdr, '\0', sizeof(struct http_header_t));

    delim_ptr = strstr(msg, "\r\n\r\n");
    if (delim_ptr == NULL)
        return msg;     /* ヘッダーなし */
    body_ptr = delim_ptr + sizeof("\r\n\r\n") - 1;

    /* ヘッダー名と文字列に分解して設定します。*/
    *delim_ptr = '\0';  // NULL terminated.
    list_ptr = split(msg, '\n');
    if (list_ptr == NULL)
        return msg;

    n = 0;
    while ((tp = list_ptr[n]) != NULL) {
        char* last_p;

        last_p = tp + strlen(tp) - 1;
        if (*last_p == '\r')
            *last_p = '\0';  /* 最後の'\r'を取る */
        if (*tp == '\0') {
            /* ヘッダーの終わり */
            break;
        }
        if (n >= MAX_REQ_HEADER) {
            err_write("split_header: request header count is over: %d", n);
            break;
        }
        if (split_item(tp, &hdr->vt[n], ':') < 0) {
            err_write("split_header: request header is too long: %s", tp);
            n++;
            continue;
        }
        n++;
    }
    hdr->count = n;

    list_free(list_ptr);
    return body_ptr;
}

/*
 * HTTPヘッダーのバイト数を取得します。
 *
 * msg: レスポンスデータ
 *
 * 戻り値
 *   ヘッダーのバイト数を返します。
 *   ヘッダーが見つからない場合はゼロを返します。
 */
int get_header_length(const char* msg)
{
    int index;

    index = indexofstr(msg, "\r\n\r\n");
    if (index < 0)
        return 0;
    return index + sizeof("\r\n\r\n") - 1;
}

/*
 * HTTPヘッダーの Cookieヘッダーから名称を検索して値を取得します。
 *
 * Cookie: NAME1=OPAQUE_STRING1; NAME2=OPAQUE_STRING2 ...
 *
 * hdr: HTTPヘッダー構造体のポインター
 * cname: Cookie名称
 * cvalue: 値が設定される領域のポインタ
 *
 * 戻り値
 *   Cookie値のポインタを返します。
 *   見つからない場合は NULL を返します。
 */
char* get_cookie(struct http_header_t* hdr, const char* cname, char* cvalue)
{
    char* value;
    char* vstr;
    char** list_ptr;
    char* tp;
    int n;
    char* retval = NULL;

    value = get_http_header(hdr, "Cookie");
    if (value == NULL)
        return NULL;

    vstr = alloca(strlen(value)+1);
    strcpy(vstr, value);
    list_ptr = split(vstr, ';');
    if (list_ptr == NULL)
        return NULL;

    n = 0;
    while ((tp = list_ptr[n]) != NULL) {
        if (strstr(tp, cname)) {
            struct variable_t vt;

            if (split_item(tp, &vt, '=') < 0) {
                err_write("get_cookie: cookie header is too long: %s", tp);
                break;
            }
            trim(vt.name);
            if (stricmp(vt.name, cname) == 0) {
                trim(vt.value);
                strcpy(cvalue, vt.value);
                retval = cvalue;
                free_item(&vt);
                break;
            }
            free_item(&vt);
        }
        n++;
    }
    list_free(list_ptr);
    return retval;
}

/*
 * HTTPヘッダー構造体の内容をクライアントに送信します。
 *
 * 先頭に"HTTP/1.1 200 OK\r\n"が送信されます。
 * ヘッダーの終わりを示す"\r\n"が自動的に付加されて送信されます。
 *
 * socket: 送信するソケット
 * hdr: HTTPヘッダー構造体のポインター
 *
 * 戻り値
 *  送信したバイト数を返します。
 *  エラーの場合は -1 を返します。
 */
int send_header(SOCKET socket, struct http_header_t* hdr)
{
    struct membuf_t* mb;
    int i;
    int n;

    mb = mb_alloc(1024);
    if (mb == NULL)
        return -1;

    if (mb_append(mb, "HTTP/1.1 200 OK\r\n", sizeof("HTTP/1.1 200 OK\r\n")-1) < 0) {
        mb_free(mb);
        return -1;
    }

    for (i = 0; i < hdr->count; i++) {
        if (mb_append(mb, hdr->vt[i].name, (int)strlen(hdr->vt[i].name)) < 0) {
            mb_free(mb);
            return -1;
        }
        if (mb_append(mb, ": ", sizeof(": ")-1) < 0) {
            mb_free(mb);
            return -1;
        }
        if (mb_append(mb, hdr->vt[i].value, (int)strlen(hdr->vt[i].value)) < 0) {
            mb_free(mb);
            return -1;
        }
        if (mb_append(mb, "\r\n", sizeof("\r\n")-1) < 0) {
            mb_free(mb);
            return -1;
        }
    }

    if (mb_append(mb, "\r\n", sizeof("\r\n")-1) < 0) {
        mb_free(mb);
        return -1;
    }
    n = send_data(socket, mb->buf, mb->size);
    mb_free(mb);
    return n;
}
