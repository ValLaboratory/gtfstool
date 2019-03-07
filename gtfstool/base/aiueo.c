/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * The MIT License
 *
 * Copyright (c) 2018-2019 Val Laboratory Corporation.
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

#include <ctype.h>

#include "common.h"
#include "aiueo.h"

static struct hash_t* _hiragana_katakana_htbl = NULL;
static struct hash_t* _katakana_hiragana_htbl = NULL;
static struct hash_t* _hkana_kana_htbl = NULL;
static struct hash_t* _kana_hkana_htbl = NULL;

// ひらがな・カタカナ
struct hiragana_katakana_t {
    char hiragana[8];
    char katakana[8];
} _hiragana_katakana_tbl[] = {
    { "が", "ガ" }, { "ぎ", "ギ" }, { "ぐ", "グ" }, { "げ", "ゲ" }, { "ご", "ゴ" },
    { "ざ", "ザ" }, { "じ", "ジ" }, { "ず", "ズ" }, { "ぜ", "ゼ" }, { "ぞ", "ゾ" },
    { "だ", "ダ" }, { "ぢ", "ヂ" }, { "づ", "ヅ" }, { "で", "デ" }, { "ど", "ド" },
    { "ば", "バ" }, { "び", "ビ" }, { "ぶ", "ブ" }, { "べ", "ベ" }, { "ぼ", "ボ" },
    { "ぱ", "パ" }, { "ぴ", "ピ" }, { "ぷ", "プ" }, { "ぺ", "ペ" }, { "ぽ", "ポ" },
    { "ゔ", "ヴ" },
    { "あ", "ア" }, { "い", "イ" }, { "う", "ウ" }, { "え", "エ" }, { "お", "オ" },
    { "か", "カ" }, { "き", "キ" }, { "く", "ク" }, { "け", "ケ" }, { "こ", "コ" },
    { "さ", "サ" }, { "し", "シ" }, { "す", "ス" }, { "せ", "セ" }, { "そ", "ソ" },
    { "た", "タ" }, { "ち", "チ" }, { "つ", "ツ" }, { "て", "テ" }, { "と", "ト" },
    { "な", "ナ" }, { "に", "ニ" }, { "ぬ", "ヌ" }, { "ね", "ネ" }, { "の", "ノ" },
    { "は", "ハ" }, { "ひ", "ヒ" }, { "ふ", "フ" }, { "へ", "ヘ" }, { "ほ", "ホ" },
    { "ま", "マ" }, { "み", "ミ" }, { "む", "ム" }, { "め", "メ" }, { "も", "モ" },
    { "や", "ヤ" }, { "ゆ", "ユ" }, { "よ", "ヨ" },
    { "ら", "ラ" }, { "り", "リ" }, { "る", "ル" }, { "れ", "レ" }, { "ろ", "ロ" },
    { "わ", "ワ" }, { "を", "ヲ" }, { "ん", "ン" },
    { "ぁ", "ァ" }, { "ぃ", "ィ" }, { "ぅ", "ゥ" }, { "ぇ", "ェ" }, { "ぉ", "ォ" },
    { "ゃ", "ャ" }, { "ゅ", "ュ" }, { "ょ", "ョ" },
    { "っ", "ッ" },
    { "ー", "ー" }, { "➖", "ー" }
};

// 半角カナ・カタカナ
struct hkana_kana_t {
    char hkana[8];
    char kana[8];
    int dakuten;
} _hkana_kana_tbl[] = {
    { "ｶ" "ﾞ", "ガ", 1 }, { "ｷ" "ﾞ", "ギ", 1 }, { "ｸ" "ﾞ", "グ", 1 }, { "ｹ" "ﾞ", "ゲ", 1 }, { "ｺ" "ﾞ", "ゴ", 1 },
    { "ｻ" "ﾞ", "ザ", 1 }, { "ｼ" "ﾞ", "ジ", 1 }, { "ｽ" "ﾞ", "ズ", 1 }, { "ｾ" "ﾞ", "ゼ", 1 }, { "ｿ" "ﾞ", "ゾ", 1 },
    { "ﾀ" "ﾞ", "ダ", 1 }, { "ﾁ" "ﾞ", "ヂ", 1 }, { "ﾂ" "ﾞ", "ヅ", 1 }, { "ﾃ" "ﾞ", "デ", 1 }, { "ﾄ" "ﾞ", "ド", 1 },
    { "ﾊ" "ﾞ", "バ", 1 }, { "ﾋ" "ﾞ", "ビ", 1 }, { "ﾌ" "ﾞ", "ブ", 1 }, { "ﾍ" "ﾞ", "ベ", 1 }, { "ﾎ" "ﾞ", "ボ", 1 },
    { "ﾊ" "ﾟ", "パ", 1 }, { "ﾋ" "ﾟ", "ピ", 1 }, { "ﾌ" "ﾟ", "プ", 1 }, { "ﾍ" "ﾟ", "ペ", 1 }, { "ﾎ" "ﾟ", "ポ", 1 },
    { "ｳ" "ﾞ", "ヴ", 1 },
    
    { "ｱ", "ア", 0 }, { "ｲ", "イ", 0 }, { "ｳ", "ウ", 0 }, { "ｴ", "エ", 0 }, { "ｵ", "オ", 0 },
    { "ｶ", "カ", 0 }, { "ｷ", "キ", 0 }, { "ｸ", "ク", 0 }, { "ｹ", "ケ", 0 }, { "ｺ", "コ", 0 },
    { "ｻ", "サ", 0 }, { "ｼ", "シ", 0 }, { "ｽ", "ス", 0 }, { "ｾ", "セ", 0 }, { "ｿ", "ソ", 0 },
    { "ﾀ", "タ", 0 }, { "ﾁ", "チ", 0 }, { "ﾂ", "ツ", 0 }, { "ﾃ", "テ", 0 }, { "ﾄ", "ト", 0 },
    { "ﾅ", "ナ", 0 }, { "ﾆ", "ニ", 0 }, { "ﾇ", "ヌ", 0 }, { "ﾈ", "ネ", 0 }, { "ﾉ", "ノ", 0 },
    { "ﾊ", "ハ", 0 }, { "ﾋ", "ヒ", 0 }, { "ﾌ", "フ", 0 }, { "ﾍ", "ヘ", 0 }, { "ﾎ", "ホ", 0 },
    { "ﾏ", "マ", 0 }, { "ﾐ", "ミ", 0 }, { "ﾑ", "ム", 0 }, { "ﾒ", "メ", 0 }, { "ﾓ", "モ", 0 },
    { "ﾔ", "ヤ", 0 }, { "ﾕ", "ユ", 0 }, { "ﾖ", "ヨ", 0 },
    { "ﾗ", "ラ", 0 }, { "ﾘ", "リ", 0 }, { "ﾙ", "ル", 0 }, { "ﾚ", "レ", 0 }, { "ﾛ", "ロ", 0 },
    { "ﾜ", "ワ", 0 }, { "ｦ", "ヲ", 0 }, { "ﾝ", "ン", 0 },
    { "ｧ", "ァ", 0 }, { "ｨ", "ィ", 0 }, { "ｩ", "ゥ", 0 }, { "ｪ", "ェ", 0 }, { "ｫ", "ォ", 0 },
    { "ｬ", "ャ", 0 }, { "ｭ", "ュ", 0 }, { "ｮ", "ョ", 0 },
    { "ｯ", "ッ", 0 },
    { "ｰ", "ー", 0 }, { "-", "ー", 0 }
//    { "｡", "。", 0 }, { "｢", "「", 0 }, { "｣", "」", 0 }, { "､", "、", 0 }, { "･", "・", 0 }
};

void initialize_aiueo_table()
{
    int i;

    _hiragana_katakana_htbl = hash_initialize(1009);
    _katakana_hiragana_htbl = hash_initialize(1009);

    _hkana_kana_htbl = hash_initialize(1009);
    _kana_hkana_htbl = hash_initialize(1009);

    for (i = 0; i < sizeof(_hiragana_katakana_tbl)/sizeof(struct hiragana_katakana_t); i++) {
        hash_put(_hiragana_katakana_htbl,
                 _hiragana_katakana_tbl[i].hiragana,
                 _hiragana_katakana_tbl[i].katakana);
        hash_put(_katakana_hiragana_htbl,
                 _hiragana_katakana_tbl[i].katakana,
                 _hiragana_katakana_tbl[i].hiragana);
    }

    for (i = 0; i < sizeof(_hkana_kana_tbl)/sizeof(struct hkana_kana_t); i++) {
        hash_put(_hkana_kana_htbl,
                 _hkana_kana_tbl[i].hkana,
                 _hkana_kana_tbl[i].kana);
        hash_put(_kana_hkana_htbl,
                 _hkana_kana_tbl[i].kana,
                 _hkana_kana_tbl[i].hkana);
    }
}

void finalyze_aiueo_table()
{
    hash_finalize(_hiragana_katakana_htbl);
    hash_finalize(_katakana_hiragana_htbl);
    hash_finalize(_hkana_kana_htbl);
    hash_finalize(_kana_hkana_htbl);

    _hiragana_katakana_htbl = NULL;
    _katakana_hiragana_htbl = NULL;
    _hkana_kana_htbl = NULL;
    _kana_hkana_htbl = NULL;
}

// ひらがな -> カタカナ変換
const char* hiragana_to_kana(const char* hiragana, char* kana)
{
    char* ph = (char*)hiragana;
    char* pk = kana;
    
    while (*ph) {
        char hchar[16];
        int n;
        const char* k;

        n = utf8_bytes(ph);
        strncpy(hchar, ph, n);
        hchar[n] = '\0';

        k = (const char*)hash_get(_hiragana_katakana_htbl, hchar);
        if (k) {
            int len = (int)strlen(k);
            strncpy(pk, k, len);
            pk += len;
        }
        ph += n;
    }
    *pk = '\0';
    return kana;
}

// カタカナ -> ひらがな変換
const char* kana_to_hiragana(const char* kana, char* hiragana)
{
    char* pk = (char*)kana;
    char* ph = hiragana;
    
    while (*pk) {
        char kchar[16];
        int n;
        const char* h;

        n = utf8_bytes(pk);
        strncpy(kchar, pk, n);
        kchar[n] = '\0';

        h = (const char*)hash_get(_katakana_hiragana_htbl, kchar);
        if (h) {
            int len = (int)strlen(h);
            strncpy(ph, h, len);
            ph += len;
        }
        pk += n;
    }
    *ph = '\0';
    return hiragana;
}

// 半角カナ -> カタカナ変換
const char* hkana_to_kana(const char* hkana, char* kana)
{
    char* ph = (char*)hkana;
    char* pk = kana;

    while (*ph) {
        int n;
        char hchar[16];
        const char* kch;
        
        n = utf8_bytes(ph);
        strncpy(hchar, ph, n);
        hchar[n] = '\0';

        kch = (const char*)hash_get(_hkana_kana_htbl, hchar);
        if (kch) {
            size_t len = strlen(kch);
            strncpy(pk, kch, len);
            pk += len;
        }
        ph += n;
    }
    *pk = '\0';
    return kana;
}

// カタカナ -> 半角カナ変換
const char* kana_to_hkana(const char* kana, char* hkana)
{
    char* ph = (char*)kana;
    char* pk = hkana;

    while (*ph) {
        int n;
        char hchar[16];
        const char* kch;
        
        n = utf8_bytes(ph);
        strncpy(hchar, ph, n);
        hchar[n] = '\0';

        kch = (const char*)hash_get(_kana_hkana_htbl, hchar);
        if (kch) {
            size_t len = strlen(kch);
            strncpy(pk, kch, len);
            pk += len;
        }
        ph += n;
    }
    *pk = '\0';
    return hkana;
}
