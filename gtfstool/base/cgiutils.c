/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* cgiutils.c
 *
 * (c) 1995-1996 William E. Weinman
 *
 * 1.1 -- 先頭のスペースをスキップするように変更
 *
 */

/* splitword(char *out, char *in, char stop)
 *
 * stopの文字が最初に現れるまでinからoutに
 * すべての文字を移動する。さらにoutを終了し、
 * stopの次の文字以降をinの先頭にコピーする。
 * （これを効率よく先頭にシフトするため）
 */
void splitword(char *out, char *in, char stop)
{
    register int i, j;

    while(*in == ' ') in++; /* すべてのスペースをスキップする */

    for(i = 0; in[i] && (in[i] != stop); i++)
        out[i] = in[i];

    out[i] = '\0'; /* 終了する */
    if(in[i]) ++i; /* stopの次の文字に位置を設定 */

    while(in[i] == ' ') i++; /* すべてのスペースをスキップする */

    for(j = 0; in[i]; )  /* inの残りをシフトする */
        in[j++] = in[i++];
    in[j] = '\0'; /* 終了する */
}

static char x2c(char *x)
{
    register char c;

    /* 注意：(x & 0xdf)はxを大文字にしている */
    c  = (x[0] >= 'A' ? ((x[0] & 0xdf) - 'A') + 10 : (x[0] - '0'));
    c *= 16;
    c += (x[1] >= 'A' ? ((x[1] & 0xdf) - 'A') + 10 : (x[1] - '0'));
    return(c);
}

/* このファンクションは、URLを1文字ずつ調べ、
   “エスケープ”（16進エンコード）
   シーケンスをすべての文字に変換します

   このバージョンはプラスもスペースに変換します。
   これを別のステップで行っているものを見たことがありますが、
   この方法のほうがずっと効率的だと思います。
*/
void unescape_url(char *url)
{
    register int i, j;

    for (i = 0, j = 0; url[j]; ++i, ++j) {
        if ((url[i] = url[j]) == '%') {
            url[i] = x2c(&url[j + 1]);
            j += 2;
        } else if (url[i] == '+')
            url[i] = ' ';
    }
    url[i] = '\0';  /* 新しい長さで終了する */
}
