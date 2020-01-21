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

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef HAVE_ICONV
#include <iconv.h>
#endif

#ifdef _WIN32
#include <mbstring.h>
#endif

#define API_INTERNAL
#include "common.h"

static unsigned char sjis_tbl[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 1x */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 2x */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 3x */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 4x */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 5x */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 6x */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 7x */
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 8x */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 9x */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* Ax */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* Bx */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* Cx */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* Dx */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* Ex */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, /* Fx */
};

static int iskanji(const char* p)
{
    int bytes;

#if defined(_WIN32) && defined(Shift_JIS)
    bytes = (sjis_tbl[*p & 0xff])? 2 : 1;
#else
    bytes = utf8_bytes(p);
#endif
    return bytes;
}

/*
 * 文字列を指定された文字で置換します。
 *
 * str:     検索対象文字列
 * target:  検索文字
 * rep:     置換文字
 *
 * 戻り値
 *  置換された文字列 str を返します。
 */
APIEXPORT char* chrep(char *str, const char target, const char rep)
{
    char* p;

    p = str;
    while (*p) {
        if (*p == target)
            *p = rep;
        p++;
    }
    return str;
}

/*
 * 文字列strから最初の文字列targetを検索して文字列repで置換します。
 * 置換後の文字列はdstが示すバッファに格納されます。
 * dstにはそれなりの領域が必要になります。
 *
 * srcに検索文字列targetが複数ある場合でもすべてを置換します。
 *
 * src:    検索対象文字列
 * target: 検索文字列
 * rep:    置換文字列
 * dst:    置換後の文字列が設定されるバッファ
 *
 * 戻り値
 *  置換された文字列 dst を返します。
 */
APIEXPORT char* strrep(const char *src, const char* target, const char* rep, char* dst)
{
    const char* s;
    char* d;

    s = src;
    d = dst;

    while (*s) {
        char* p;
        int rlen;

        p = strstr(s, target);
        if (p == NULL) {
            /* 検索文字列が見つからないので残りをdstにコピー */
            strcpy(d, s);
            return dst;
        }

        /* 検索文字列の前方の文字列をdstにコピー */
        while (s != p) {
            *d++ = *s++;
        }

        /* 置換文字列をdstにコピー */
        rlen = (int)strlen(rep);
        if (rlen > 0) {
            memcpy(d, rep, rlen);
            d += rlen;
        }
        *d = '\0';

        /* srcの現在位置を進めます。*/
        s = p + strlen(target);
    }
    return dst;
}

/*
 * 検索対象文字列に検索文字がいくつ含まれるか調べます。
 *
 * str:     検索対象文字列
 * target:  検索文字
 *
 * 戻り値
 *  検索文字の個数を返します。
 */
APIEXPORT int strchc(const char* str, const char target)
{
    int n = 0;

    while (*str++) {
        if (*str == target)
            n++;
    }
    return n;
}

/*
 * 検索対象文字列に検索文字列がいくつ含まれるか調べます。
 *
 * str:     検索対象文字列
 * target:  検索文字列
 *
 * 戻り値
 *  検索文字列の個数を返します。
 */
APIEXPORT int strstrc(const char* str, const char* target)
{
    char* p;
    int tlen;
    int n = 0;

    p = (char*)str;
    tlen = (int)strlen(target);

    while (*p) {
        p = strstr(p, target);
        if (p == NULL)
            break;
        p += tlen;
        n++;
    }
    return n;
}

/*
 * 文字列から指定された文字を前方から検索して該当位置のインデックスを返します。
 *
 * str:     検索対象文字列
 * target:  検索文字
 *
 * 戻り値
 *  ゼロからのインデックスを返します。
 *  対象がない場合は -1 を返します。
 */
APIEXPORT int indexof(const char* str, const char target)
{
    int i = 0;

    while (*str) {
        int bytes;

        if ((bytes = iskanji(str)) == 1) {
            if (*str == target)
                return i;
            else if (*str == '"') {
                do {
                    bytes = iskanji(str);
                    str += bytes;
                    i += bytes;
                } while (*str && *str != '"');
                bytes = (*str)? 1 : 0;
            }
        }
        str += bytes;
        i += bytes;
    }
    return -1;
}

/*
 * 文字列から指定された文字を後方から検索して該当位置のインデックスを返します。
 *
 * str:     検索対象文字列
 * target:  検索文字
 *
 * 戻り値
 *  ゼロからのインデックスを返します。
 *  対象がない場合は -1 を返します。
 */
APIEXPORT int lastindexof(const char* str, const char target)
{
    int i, len;

    len = (int)strlen(str);
    i = len - 1;
    str += i;
    while (i >= 0) {
        if (*str == target)
            return i;
        str--;
        i--;
    }
    return -1;
}

/*
 * 文字列から指定された文字列を前方から検索して該当位置のインデックスを返します。
 *
 * str:     検索対象文字列
 * target:  検索文字列
 *
 * 戻り値
 *  ゼロからのインデックスを返します。
 *  対象がない場合は -1 を返します。
 */
APIEXPORT int indexofstr(const char* str, const char* target)
{
    int i = 0;

    while (*str) {
        if (*str == *target) {
            const char* p1;
            const char* p2;

            p1 = str;
            p2 = target;

            do {
                p1++;
                p2++;
                if (*p2 == '\0')
                    return i;
            } while (*p1 == *p2);
        }
        str++;
        i++;
    }
    return -1;
}

/*
 * 文字列から指定されたインデックスからバイト数の部分文字列を作成します。
 * バイト数に -1 を指定すると文字列の最後までを対象とします。
 *
 * dst:     作成される部分文字列
 * src:     文字列
 * index:   ゼロからのインデックス
 * len:     バイト数
 *
 * 戻り値
 *  dst を返します。
 */
APIEXPORT char* substr(char* dst, const char* src, int index, int len)
{
    int n;

    if (len < 0)
        n = (int)strlen(src) - index;
    else
        n = len;

    strncpy(dst, src+index, n);
    dst[n] = '\0';
    return dst;
}

/*
 * 文字列を指定されたデリミタで区切ってポインタの配列を作成します。
 * デリミタ文字の位置は'\0'になります。
 * 文字列の最初の文字がデリミタの場合は次の文字から検索されます。
 * ポインタ配列の最後の要素には'\0'が入ります。
 *
 * マルチスレッド環境で strtok()の代わりに使用できます。
 * この関数で確保されたメモリはlist_free()関数を使用して解放する必要があります。
 *
 * src:     文字列
 * delim:   デリミタ文字
 *
 * 戻り値
 *  ポインタの配列を返します。
 *
 * str:   "aaa,bbb,ccc"
 * delim: ','
 * この例では str の内容が以下のようになります。
 * str:   "aaa\0bbb\0ccc"
 * リターン値は以下のようなポインタ配列になります。
 * +---------------+
 * | aaaのポインタ |
 * +---------------+
 * | bbbのポインタ |
 * +---------------+
 * | cccのポインタ |
 * +---------------+
 * | \0            |
 * +---------------+
 */
APIEXPORT char** split(char* str, char delim)
{
    char* p;
    int n = 0;
    char** list;
    int i;

    if (str == NULL)
        return NULL;
    if (*str == delim)
        str++;

    p = str;
    while (*p) {
        int bytes;

        if ((bytes = iskanji(p)) == 1) {
            if (*p == delim)
                n++;
            else if (*p == '"') {
                do {
                    bytes = iskanji(p);
                    p += bytes;
                } while (*p && *p != '"');
                bytes = (*p)? 1 : 0;
            }
        }
        p += bytes;
    }

    n++;
    list = (char**)malloc(sizeof(char*) * (n+1));
    if (list == NULL)
        return NULL;
    memset(list, '\0', sizeof(char*) * (n+1));

    for (i = 0; i < n && *str; i++) {
        int index;

        index = indexof(str, delim);
        if (index >= 0)
            str[index] = '\0';  /* terminate */
        list[i] = str;
        str += index + 1;
    }
    return list;
}

APIEXPORT void list_free(char** ptr)
{
    if (ptr != NULL)
        free(ptr);
}

APIEXPORT int list_count(const char** ptr)
{
    int n = 0;

    while (*ptr++)
        n++;
    return n;
}

/*
 * 文字列の両端からホワイトスペースを取り除きます。
 * ホワイトスペースは 0x20 以下の文字が対象です。
 *
 * str: 文字列
 *
 * 戻り値
 *  strのポインタを返します。
 */
APIEXPORT char* trim(char* str)
{
    int i, len;
    unsigned char* p;

    len = (int)strlen(str);
    i = len - 1;
    p = (unsigned char*)(str + i);
    while (i >= 0) {
        if (*p > 0x20)
            break;
        if (*p <= 0x20)
            *p = '\0';
        p--;
        i--;
    }

    p = (unsigned char*)str;
    i = 0;
    while (*p) {
        if (*p > 0x20) {
            if (i > 0) {
                len = (int)strlen((const char*)p);
                memmove(str, p, len);
                str[len] = '\0';
            }
            break;
        }
        i++;
        p++;
    }
    return str;
}

/*
 * 文字列からホワイトスペースをスキップします。
 * ホワイトスペースは 0x20 以下の文字が対象です。
 *
 * str: 文字列
 *
 * 戻り値
 *  スキップ後のstrのポインタを返します。
 */
APIEXPORT char* skipsp(const char* str)
{
    while (*str) {
        if ((unsigned char)*str > 0x20)
            return (char*)str;
        str++;
    }
    return (char*)str;
}

/*
 * 文字列の両端からダブルクォート文字を取り除きます。
 *
 * str: 文字列
 *
 * 戻り値
 *  strのポインタを返します。
 */
APIEXPORT char* quote(char* str)
{
    int len;
    char* p;
    
    if (*str == '\"') {
        len = (int)strlen(str);
        p = str + len  - 1;
        if (*p == '\"') {
            *p = '\0';
            len--;
            memmove(str, str+1, len);
        }
    }
    return str;
}

/*
 * 文字列の文字コードを変換します。
 *
 * エンコーディングによっては文字列の途中に'\0'がある場合がありますが、
 * 変換後の文字列バッファの最後に'\0'を付加しますので dst_size+1 の
 * 大きさのバッファが必要になります。
 *
 * サポートされている文字コードは Linux環境では以下のコマンドで調べられます。
 *    $ iconv --list
 *
 * Windows環境では iconv.dll が LGPL で公開されているものを使用している。
 *
 * src_enc:  変換元のエンコーディング名
 * src:      変換元の文字列
 * src_size: 変換元のバイト数
 * dst_enc:  変換するエンコーディング名
 * dst:      変換後の文字列バッファ
 * dst_size: 変換後の文字列バッファのバイト数
 *
 * 戻り値
 *  変換後の文字列バイト数を返します。
 *  エラーの場合は -1 を返します。
 *
 * サンプル
 *  #define BUFSIZE 1024
 *  char    str_in[BUFSIZE];
 *  char    str_out[BUFSIZE+1];
 *  strcpy(str_in, "テストの文字列。");  // Shift_JIS エンコーディング
 *  convert("Shift_JIS", str_in, strlen(str_in), "EUC-JP", str_out, BUFSIZE);
 *  printf("%s\n", str_out);
 *
 */
APIEXPORT int convert(const char* src_enc, const char* src, int src_size, const char* dst_enc, char* dst, int dst_bufsize)
{
#ifdef HAVE_ICONV
    iconv_t ic;
    char* srcbuf;
    size_t b;
    char* outp;
    size_t inbytesleft;
    size_t outbytesleft;
    size_t outsize;

    ic = iconv_open(dst_enc, src_enc);
    if (ic == (iconv_t)(-1))
        return -1;

    srcbuf = (char*)alloca(src_size+1);
    strcpy(srcbuf, src);
    
    if ((stricmp(src_enc, "UTF-8") == 0 || stricmp(src_enc, "UTF8") == 0) &&
        stricmp(dst_enc, "Shift_JIS") == 0) {
        // UTF-8からShift_JISに変換できないコードを置換
        int index = indexofstr(srcbuf, "～");
        if (index >= 0)
            memcpy(&srcbuf[index], "〜", strlen("〜"));
    }

    inbytesleft = src_size;
    outp = dst;
    outbytesleft = dst_bufsize;
    b = iconv(ic, (char**)&srcbuf, &inbytesleft, &outp, &outbytesleft);
    iconv_close(ic);
    if (b == (size_t)(-1)) {
        err_write("iconv() failed. %s(%s)\n", strerror(errno), src);
        return -1;
    }
    outsize = dst_bufsize - outbytesleft;
    dst[outsize] = '\0';
    return (int)outsize;
#else
    err_write("convert(): unsupported iconv() function.");
    return -1;
#endif  /* HAVE_ICONV */
}

/*
 * バイト列を16進数の文字列に変換します。
 *
 * dst: 16進数文字列が設定される領域
 * src: 変換するバイト列
 * size: srcのバイト数
 *
 * 戻り値
 *  dstのポインタを返します。
 */
APIEXPORT char* tohex(char* dst, const void* src, int size)
{
    const unsigned char* p = src;
    int i;

    for (i = 0; i < size; i++) {
        dst[i*2]   = "0123456789ABCDEF"[p[i] / 0x10];
        dst[i*2+1] = "0123456789ABCDEF"[p[i] % 0x10];
    }
    dst[i*2] = '\0';
    return dst;
}

/*
 * 16進数の文字列をバイト列に変換します。
 *
 * dst: バイト列が設定される領域
 * hex: 変換する16進数文字列
 *
 * 戻り値
 *  dstのポインタを返します。
 */
APIEXPORT char* tochar(char *dst, const char *hex)
{
    unsigned char* src = (unsigned char*)hex;
    unsigned char ch = 0;
    int i = 0;
    int j = 0;

    while (src[i]) {
        if (i % 2 == 0) {
            if (src[i] <= '9')
                ch = (src[i] - '0') << 4;
            else
                ch = (src[i] - 'A' + 10) << 4;
        } else {
            if (src[i] <= '9')
                dst[j] = ch + (src[i] - '0');
            else
                dst[j] = ch + (src[i] - 'A' + 10);
            j++;
        }
        i++;
    }
    /* 終端'\0'のセット */
    dst[j] = src[i];
    return dst;
}

/*
 * ワイルドカードと文字列を比較します。
 *
 * ptn: ワールドカード文字列
 * str: 文字列
 *
 * 戻り値
 *  ワイルドカードと一致する場合は真（ゼロ以外）を返します。
 *  一致しない場合はゼロを返します。
 */
APIEXPORT int strmatch(const char *ptn, const char *str)
{
    switch (*ptn) {
        case '\0':
            return (*str == '\0');
        case '*':
            return strmatch(ptn+1, str) || ((*str != '\0') && strmatch(ptn, str+1));
        case '?':
            return (*str != '\0') && strmatch(ptn+1, str+1);
        default:
            return (*ptn == *str) && strmatch(ptn+1, str+1);
    }
}

/*
 * ワイルドカードと文字列を比較します。
 * ２バイト文字が含まれていても動作します。
 *
 * Windows以外の環境では常にゼロを返します。
 *
 * ptn: ワールドカード文字列
 * str: 文字列
 *
 * 戻り値
 *  ワイルドカードと一致する場合は真（ゼロ以外）を返します。
 *  一致しない場合はゼロを返します。
 */
APIEXPORT int strmatchmb(const unsigned char *ptn, const unsigned char *str)
{
#ifdef _WIN32
    switch(*ptn) {
        case '\0':
            return (_mbsnextc(str) == '\0');
        case '*':
            return strmatchmb(_mbsinc(ptn), str) || (_mbsnextc(str) != '\0') && strmatchmb(ptn, _mbsinc(str));
        case '?':
            return (_mbsnextc(str) != '\0') && strmatchmb(_mbsinc(ptn), _mbsinc(str));
        default:
            return (_mbsnextc(ptn) == _mbsnextc(str)) && strmatchmb(_mbsinc(ptn), _mbsinc(str));
    }
#else
    return 0;
#endif
}

/*
 * 文字列がすべて半角数字か判定します。
 *
 * str: 文字列
 *
 * 戻り値
 *  半角数字の場合は 1 を返します。
 *  それ以外はゼロを返します。
 */
APIEXPORT int isdigitstr(const char *str)
{
    int c;

    while (*str) {
        c = (unsigned char)*str;
        if (! isdigit(c))
            return 0;   /* not digit */
        str++;
    }
    return 1;   /* digit string */
}

/*
 * 文字列がすべて半角英字か判定します。
 *
 * str: 文字列
 *
 * 戻り値
 *  半角英字の場合は 1 を返します。
 *  それ以外はゼロを返します。
 */
APIEXPORT int isalphastr(const char *str)
{
    int c;

    while (*str) {
        c = (unsigned char)*str;
        if (! isalpha(c))
            return 0;   /* not alpha */
        str++;
    }
    return 1;   /* alpha string */
}

/*
 * 文字列がすべて半角英数字か判定します。
 *
 * str: 文字列
 *
 * 戻り値
 *  半角英数字の場合は 1 を返します。
 *  それ以外はゼロを返します。
 */
APIEXPORT int isalnumstr(const char *str)
{
    int c;

    while (*str) {
        c = (unsigned char)*str;
        if (! isalnum(c))
            return 0;   /* not alpha-degit */
        str++;
    }
    return 1;   /* alpha-digit string */
}

static int get_skip_chars(unsigned char chr)
{
    if ((chr & 0x80) == 0x00) return 1;
    else if ((chr & 0xe0) == 0xc0) return 2;
    else if ((chr & 0xf0) == 0xe0) return 3;
    else if ((chr & 0xf8) == 0xf0) return 4;
    else if ((chr & 0xfc) == 0xf8) return 5;
    else if ((chr & 0xfe) == 0xfc) return 6;
    return 1;
}

static unsigned int get_UCS4_code(const char *ptr, int* skip)
{
    static const unsigned int base[] = {
        0x00000000, 0x00000000, 0x00003080, 0x000E2080,
        0x03C82080, 0xFA082080, 0x82082080
    };
    int size, i;
    const unsigned char *p = (const unsigned char*)ptr;
    unsigned int chr = (unsigned int)*p;
    if(chr < 0x80) {
        if (skip)
            *skip = 1;
        return chr;
    }
    
    size = get_skip_chars(chr);
    for(i = 1; i < size; i++)
        chr = (chr << 6) + p[i];
    
    if (skip)
        *skip = size;
    return chr - base[size];
}

/*
 * 指定されたutf-8の文字が何バイトで構成されているか調べます。
 *
 * c: 対象文字
 *
 * 戻り値
 *  半角英数字の場合は 1 を返します。
 *  それ以外は2以上の値を返します。
 */
APIEXPORT int utf8_bytes(const char *c)
{
    int skip;
    
    get_UCS4_code(c, &skip);
    return skip;
}

/*
 * 指定されたShift_JISの文字が何バイトか調べます。
 *
 * c: 対象文字
 *
 * 戻り値
 *  半角文字の場合は 1 を返します。
 *  全角文字の場合は 2 を返します。
 */
APIEXPORT int sjis_bytes(const char *c)
{
    return (sjis_tbl[*c & 0xff])? 2 : 1;
}

/*
 * 指定された文字列を大文字に変換します。
 *
 * s: 文字列
 *
 * 戻り値
 *  変換された文字列のポインタを返します。
 */
APIEXPORT char* toupperstr(char *s)
{
    char* p;

    for (p = s; *p; p++) {
        *p = toupper(*p);
    }
    return s;
}

/*
 * 指定された文字列を小文字に変換します。
 *
 * s: 文字列
 *
 * 戻り値
 *  変換された文字列のポインタを返します。
 */
APIEXPORT char* tolowerstr(char *s)
{
    char* p;

    for (p = s; *p; p++) {
        *p = tolower(*p);
    }
    return s;
}
