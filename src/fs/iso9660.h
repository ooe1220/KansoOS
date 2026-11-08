#ifndef ISO9660_H
#define ISO9660_H

#include <stdint.h>

/*
 * ISO9660 ディレクトリレコードの「固定部」
 * ECMA-119 / ISO9660 準拠。
 * 固定長 33 バイトの直後に「ファイル識別子(id_len バイト)」が続く。
 * （そのさらに後ろに Rock Ridge 拡張フィールドが来ることがあるので、
 *   名前を取るときは id_len だけ読んで止めること）
 */
typedef struct __attribute__((packed)) {
    uint8_t  len;           /* このレコード全体のバイト数。0 なら終端 */
    uint8_t  ext_attr_len;  /* 拡張属性のバイト数（普通 0） */
    uint32_t lba_le;        /* extent 先頭 LBA（リトルエンディアン） */
    uint32_t lba_be;        /* 同上（ビッグエンディアン版、読まなくて OK） */
    uint32_t size_le;       /* データサイズ（リトル） */
    uint32_t size_be;       /* 同上（ビッグ） */
    uint8_t  date[7];       /* 年 月 日 時 分 秒 タイムゾーン */
    uint8_t  flags;         /* bit0=1 → ディレクトリ */
    uint8_t  unit_size;     /* インターリーブ用（普通 0） */
    uint8_t  gap_size;      /* 同上 */
    uint16_t vol_seq_le;    /* ボリューム列番号（リトル） */
    uint16_t vol_seq_be;    /* 同上（ビッグ） */
    uint8_t  id_len;        /* ★ファイル識別子のバイト数 */
} iso_dir_rec_t;

/* 固定部のバイト数（33） */
#define ISO_DIR_REC_FIXED_SZ 33

/* flags */
#define ISO_FILE_DIR  0x01   /* ディレクトリ */
#define ISO_FILE_SELF 0x02   /* （内部フラグとしては使わないが目印） */

/* セミコロンの扱い：そのまま表示する */
/* #define ISO_STRIP_SEMICOLON 1 を定義すれば ";1" を消す */

/* ---- 公開 API ---- */

/*
 * 1 ブロック(2048バイト)を読む関数の型。
 * I/O 層(g_fs_blk など)を知らなくていいように、外から注入する。
 * 戻り値 0 = 成功、buf に 2048 バイトを書くこと。
 */
typedef int (*iso_blk_read_fn)(uint32_t lba, void *buf);

/*
 * buf/bufsz のディレクトリレコード列を走査して「名前だけ」表示する。
 * I/O は一切しない（テスト・スタブでも動く）。
 */
void iso9660_list_names_only(const void *buf, int bufsz);

/*
 * PVD(論理ブロック 16) を読んでルートを解決し、
 * ルートの中身を iso9660_list_names_only で表示する。
 */
void iso9660_list_root(void);

#endif