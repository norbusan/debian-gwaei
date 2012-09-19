#ifndef LW_UTILITIES_INCLUDED
#define LW_UTILITIES_INCLUDED

G_BEGIN_DECLS

#define HIRAGANA  "(あ)|(い)|(う)|(え)|(お)|(か)(き)|(く)|(け)|(こ)|(が)|(ぎ)|(ぐ)|(げ)|(ご)|(さ)|(し)|(す)|(せ)|(そ)|(ざ)|(じ)|(ず)|(ぜ)|(ぞ)|(た)|(ち)(つ)|(て)|(と)|(だ)|(ぢ)|(づ)|(で)|(ど)|(な)|(に)|(ぬ)|(ね)|(の)|(は)(ひ)|(ふ)|(へ)|(ほ)|(ば)|(び)(ぶ)|(べ)|(ぼ)|(ぱ)|(ぴ)|(ぷ)|(ぺ)|(ぽ)(ま)|(み)|(む)|(め)|(も)|(や)|(ゆ)|(よ)|(ら)|(り)|(る)(れ)|(ろ)|(わ)|(を)|(ん)(ぁ)|(ぃ)|(ぇ)|(ぉ)"
#define KATAKANA "(ア)|(イ)|(ウ)|(エ)|(オ)|(カ)|(キ)|(ク)|(ケ)|(コ)|(ガ)|(ギ)|(グ)|(ゲ)|(ゴ)|(サ)|(シ)|(ス)|(セ)|(ソ)|(ザ)|(ジ)|(ズ)|(ゼ)|(ゾ)|(タ)|(チ)|(ツ)|(テ)|(ト)|(ダ)|(ジ)|(ヅ)|(デ)|(ド)|(ナ)|(ニ)|(ヌ)|(ネ)|(ノ)|(ハ)|(ヒ)|(フ)|(ヘ)|(ホ)|(バ)|(ビ)|(ブ)|(ベ)|(ボ)|(パ)|(ピ)|(プ)|(ペ)|(ポ)|(マ)|(ミ)|(ム)|(メ)|(モ)|(ヤ)|(ユ)|(ヨ)|(ラ)|(リ)|(ル)|(レ)|(ロ)|(ワ)|(ヲ)|(ン)|(ァ)|(ィ)|(ェ)|(ォ)"

typedef enum {
  LW_PATH_BASE, 
  LW_PATH_DICTIONARY,
  LW_PATH_PLUGIN,
  LW_PATH_CACHE,
  LW_PATH_INDEX,
  LW_PATH_VOCABULARY,
  TOTAL_LW_PATHS
} LwFolderPath;


typedef enum {
//  LW_COMPRESSION_ZIP, //Unsupported since you can't tell what the file will be named
  LW_COMPRESSION_GZIP,
  LW_COMPRESSION_NONE,
  LW_COMPRESSION_TOTAL
} LwCompression;

typedef enum {
  LW_ENCODING_UTF8,
  LW_ENCODING_EUC_JP,
  LW_ENCODING_SHIFT_JS,
  LW_ENCODING_TOTAL
} LwEncoding;


gchar* lw_util_build_filename (const LwFolderPath, const char*);
const char* lw_util_get_compressionname (const LwCompression);
const char* lw_util_get_encodingname (const LwEncoding);


const char* lw_util_next_hira_char_from_roma (const char*);
char* lw_util_roma_to_hira (const char*, char*);
gboolean lw_util_str_roma_to_hira (const char*, char*, int);

gboolean lw_util_is_hiragana_str (const char*);
gboolean lw_util_is_util_kanji_str (const char*);
gboolean lw_util_is_katakana_str (const char*);
gboolean lw_util_is_romaji_str (const char*);
gboolean lw_util_is_kanji_ish_str (const char*);
gboolean lw_util_is_kanji_str (const char*);
gboolean lw_util_is_furigana_str (const char*);
gboolean lw_util_is_yojijukugo_str (const const char*);

void lw_util_str_shift_kata_to_hira (char*);
void lw_util_str_shift_hira_to_kata (char*);


gboolean lw_util_all_chars_are_in_range (char*, int, int);

gchar* lw_util_prepare_query (const char*, gboolean);
gchar* lw_util_sanitize_input (const char*, gboolean);
gboolean lw_util_contains_halfwidth_japanese (const gchar*);
gchar* lw_util_enlarge_halfwidth_japanese (const gchar*);

gboolean lw_util_is_japanese_locale (void);

char** lw_util_get_romaji_atoms_from_string (const char*);
char** lw_util_get_furigana_atoms_from_string (const char*);
gchar* lw_util_get_query_from_args (int, char**);

gchar* lw_strjoinv (gchar, gchar**, gint);
gchar* lw_util_collapse_string (const gchar*);

gchar* lw_util_delimit_script_changes (const gchar*, const gchar*, gboolean);
gchar* lw_util_delimit_whitespace (const gchar*, const gchar*);
gchar* lw_util_delimit_radicals (const gchar*, const gchar*);

GRegex* lw_regex_new (const gchar*, const gchar*, GError**);

G_END_DECLS

#endif
