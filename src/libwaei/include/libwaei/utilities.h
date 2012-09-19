#ifndef GW_UTILITIES_INCLUDED
#define GW_UTILITIES_INCLUDED

#define HIRAGANA  "(あ)|(い)|(う)|(え)|(お)|(か)(き)|(く)|(け)|(こ)|(が)|(ぎ)|(ぐ)|(げ)|(ご)|(さ)|(し)|(す)|(せ)|(そ)|(ざ)|(じ)|(ず)|(ぜ)|(ぞ)|(た)|(ち)(つ)|(て)|(と)|(だ)|(ぢ)|(づ)|(で)|(ど)|(な)|(に)|(ぬ)|(ね)|(の)|(は)(ひ)|(ふ)|(へ)|(ほ)|(ば)|(び)(ぶ)|(べ)|(ぼ)|(ぱ)|(ぴ)|(ぷ)|(ぺ)|(ぽ)(ま)|(み)|(む)|(め)|(も)|(や)|(ゆ)|(よ)|(ら)|(り)|(る)(れ)|(ろ)|(わ)|(を)|(ん)(ぁ)|(ぃ)|(ぇ)|(ぉ)"
#define KATAKANA "(ア)|(イ)|(ウ)|(エ)|(オ)|(カ)|(キ)|(ク)|(ケ)|(コ)|(ガ)|(ギ)|(グ)|(ゲ)|(ゴ)|(サ)|(シ)|(ス)|(セ)|(ソ)|(ザ)|(ジ)|(ズ)|(ゼ)|(ゾ)|(タ)|(チ)|(ツ)|(テ)|(ト)|(ダ)|(ジ)|(ヅ)|(デ)|(ド)|(ナ)|(ニ)|(ヌ)|(ネ)|(ノ)|(ハ)|(ヒ)|(フ)|(ヘ)|(ホ)|(バ)|(ビ)|(ブ)|(ベ)|(ボ)|(パ)|(ピ)|(プ)|(ペ)|(ポ)|(マ)|(ミ)|(ム)|(メ)|(モ)|(ヤ)|(ユ)|(ヨ)|(ラ)|(リ)|(ル)|(レ)|(ロ)|(ワ)|(ヲ)|(ン)|(ァ)|(ィ)|(ェ)|(ォ)"

typedef enum {
  GW_PATH_BASE, 
  GW_PATH_DICTIONARY,
  GW_PATH_DICTIONARY_EDICT,
  GW_PATH_DICTIONARY_KANJI,
  GW_PATH_DICTIONARY_EXAMPLES,
  GW_PATH_DICTIONARY_UNKNOWN,
  GW_PATH_PLUGIN,
  GW_PATH_CACHE,
  GW_PATH_TOTAL
} LwFolderPath;

//!
//! @brief Dictionary type assigned by the program.  It determines the parsing algorithm
//!
typedef enum {  
  GW_ENGINE_EDICT,         //!< Standard edict format dictionary
  GW_ENGINE_KANJI,         //!< Kanjidic format dictionary
  GW_ENGINE_EXAMPLES,      //!< Examples format dictionary
  GW_ENGINE_UNKNOWN,          //!< Unkown format which should use safe parsing
  GW_ENGINE_TOTAL
} LwEngine;

typedef enum {
//  GW_COMPRESSION_ZIP, //Unsupported since you can't tell what the file will be named
  GW_COMPRESSION_GZIP,
  GW_COMPRESSION_NONE,
  GW_COMPRESSION_TOTAL
} LwCompression;

typedef enum {
  GW_ENCODING_UTF8,
  GW_ENCODING_EUC_JP,
  GW_ENCODING_SHIFT_JS,
  GW_ENCODING_TOTAL
} LwEncoding;


const char* lw_util_get_directory (const LwFolderPath);
const char* lw_util_get_directory_for_engine (const LwEngine);
const char* lw_util_get_engine_name (const LwEngine ENGINE);
LwEngine lw_util_get_engine_from_enginename (const char*);
const char* lw_util_get_compression_name (const LwCompression);
const char* lw_util_get_encoding_name (const LwEncoding);


char* lw_util_next_hira_char_from_roma (char*);
char* lw_util_roma_to_hira (char*, char*);
gboolean lw_util_str_roma_to_hira (char*, char*, int);

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

#endif
