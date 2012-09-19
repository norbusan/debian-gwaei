#ifndef GW_DEFINITIONS_INCLUDED
#define GW_DEFINITIONS_INCLUDED

#define MAX_QUERY        250
#define MAX_DICTIONARIES 20
#define MAX_DICTIONARY   100
#define MAX_RESULTS      1000000
#define MAX_LINE         5000 
#define MAX_NMATCH       5
#define MAX_CHUNK        500
#define MAX_GCONF_KEY    100
#define MAX_MEDIUM_IRRELIVENT_RESULTS 100
#define MAX_LOW_IRRELIVENT_RESULTS    50

#define GW_MAX_FONT_MAGNIFICATION  6
#define GW_MIN_FONT_MAGNIFICATION -6
#define GW_DEFAULT_FONT_MAGNIFICATION 0

#define GW_FONT_ZOOM_STEP 2

#define GW_MAX_FONT_SIZE 100
#define GW_MIN_FONT_SIZE 6
#define GW_DEFAULT_FONT_SIZE 12

#define GW_DEFAULT_FONT "Sans 12"

//GConfig
#define GCPATH_INTERFACE           "/desktop/gnome/interface"
#define GCKEY_TOOLBAR_STYLE        "/desktop/gnome/interface/toolbar_style"
#define GCKEY_DOCUMENT_FONT_NAME   "/desktop/gnome/interface/document_font_name"

#define GCPATH_GW               "/apps/gwaei"
#define GCKEY_GW_TOOLBAR_SHOW   "/apps/gwaei/toolbar_show"
#define GCKEY_GW_LESS_RELEVANT_SHOW   "/apps/gwaei/less_relevant_results_show"
#define GCKEY_GW_FONT_USE_GLOBAL_FONT  "/apps/gwaei/fonts/use_global_document_font"
#define GCKEY_GW_FONT_CUSTOM_FONT      "/apps/gwaei/fonts/custom_document_font"
#define GCKEY_GW_FONT_MAGNIFICATION    "/apps/gwaei/fonts/magnification"

#define GCKEY_GW_HIRA_KATA      "/apps/gwaei/query_hiragana_to_katakana"
#define GCKEY_GW_KATA_HIRA      "/apps/gwaei/query_katakana_to_hiragana"
#define GCKEY_GW_ROMAN_KANA     "/apps/gwaei/query_romanji_to_kana"
#define GCKEY_GW_SPELLCHECK     "/apps/gwaei/query_spellcheck"

#define GCPATH_GW_HIGHLIGHT     "/apps/gwaei/highlighting/"
#define GCKEY_GW_MATCH_FG       "/apps/gwaei/highlighting/match_foreground"
#define GCKEY_GW_MATCH_BG       "/apps/gwaei/highlighting/match_background"
#define GCKEY_GW_HEADER_FG      "/apps/gwaei/highlighting/header_foreground"
#define GCKEY_GW_HEADER_BG      "/apps/gwaei/highlighting/header_background"
#define GCKEY_GW_COMMENT_FG     "/apps/gwaei/highlighting/comment_foreground"

#define GW_MATCH_FG_FALLBACK       "#000000"
#define GW_MATCH_BG_FALLBACK       "#CCEECC"
#define GW_HEADER_FG_FALLBACK      "#EE1111"
#define GW_HEADER_BG_FALLBACK      "#FFDEDE"
#define GW_COMMENT_FG_FALLBACK     "#2222DD"

//Location to install dictionaries from
#define GCKEY_GW_ENGLISH_SOURCE  "/apps/gwaei/dictionary/english_source"
#define GCKEY_GW_KANJI_SOURCE    "/apps/gwaei/dictionary/kanji_source"
#define GCKEY_GW_NAMES_SOURCE    "/apps/gwaei/dictionary/names_source"
#define GCKEY_GW_PLACES_SOURCE   "/apps/gwaei/dictionary/places_source"
#define GCKEY_GW_RADICALS_SOURCE "/apps/gwaei/dictionary/radicals_source"
#define GCKEY_GW_EXAMPLES_SOURCE "/apps/gwaei/dictionary/examples_source"
#define GCKEY_GW_LOAD_ORDER "/apps/gwaei/dictionary/load_order"

#define GCKEY_GW_FRENCH_SOURCE "/apps/gwaei/dictionary/french_source"
#define GCKEY_GW_GERMAN_SOURCE "/apps/gwaei/dictionary/german_source"
#define GCKEY_GW_SPANISH_SOURCE "/apps/gwaei/dictionary/spanish_source"

#define GW_ENGLISH_URI_FALLBACK  "ftp://ftp.monash.edu.au/pub/nihongo/edict.gz"
#define GW_KANJI_URI_FALLBACK    "ftp://ftp.monash.edu.au/pub/nihongo/kanjidic.gz"
#define GW_NAMES_URI_FALLBACK    "ftp://ftp.monash.edu.au/pub/nihongo/enamdict.gz"
#define GW_RADICALS_URI_FALLBACK "ftp://ftp.monash.edu.au/pub/nihongo/kradfile.gz"
#define GW_EXAMPLES_URI_FALLBACK "http://www.csse.monash.edu.au/~jwb/examples.gz"

#define GW_FRENCH_URI_FALLBACK "http://dico.fj.free.fr/fj.utf"
#define GW_GERMAN_URI_FALLBACK "http://www.bibiko.de/WdJTEUC.zip"
#define GW_SPANISH_URI_FALLBACK "http://hispadic.googlepages.com/hispamix_euc.zip"
#define GW_LOAD_ORDER_FALLBACK "English,Mix,Kanji,Radicals,Names,Places,Examples"


#define GCKEY_DEFAULT_BROWSER "/desktop/gnome/url-handlers/http/command"

#define HIRAGANA  "(あ)|(い)|(う)|(え)|(お)|(か)(き)|(く)|(け)|(こ)|(が)|(ぎ)|(ぐ)|(げ)|(ご)|(さ)|(し)|(す)|(せ)|(そ)|(ざ)|(じ)|(ず)|(ぜ)|(ぞ)|(た)|(ち)(つ)|(て)|(と)|(だ)|(ぢ)|(づ)|(で)|(ど)|(な)|(に)|(ぬ)|(ね)|(の)|(は)(ひ)|(ふ)|(へ)|(ほ)|(ば)|(び)(ぶ)|(べ)|(ぼ)|(ぱ)|(ぴ)|(ぷ)|(ぺ)|(ぽ)(ま)|(み)|(む)|(め)|(も)|(や)|(ゆ)|(よ)|(ら)|(り)|(る)(れ)|(ろ)|(わ)|(を)|(ん)(ぁ)|(ぃ)|(ぇ)|(ぉ)"
#define KATAKANA "(ア)|(イ)|(ウ)|(エ)|(オ)|(カ)|(キ)|(ク)|(ケ)|(コ)|(ガ)|(ギ)|(グ)|(ゲ)|(ゴ)|(サ)|(シ)|(ス)|(セ)|(ソ)|(ザ)|(ジ)|(ズ)|(ゼ)|(ゾ)|(タ)|(チ)|(ツ)|(テ)|(ト)|(ダ)|(ジ)|(ヅ)|(デ)|(ド)|(ナ)|(ニ)|(ヌ)|(ネ)|(ノ)|(ハ)|(ヒ)|(フ)|(ヘ)|(ホ)|(バ)|(ビ)|(ブ)|(ベ)|(ボ)|(パ)|(ピ)|(プ)|(ペ)|(ポ)|(マ)|(ミ)|(ム)|(メ)|(モ)|(ヤ)|(ユ)|(ヨ)|(ラ)|(リ)|(ル)|(レ)|(ロ)|(ワ)|(ヲ)|(ン)|(ァ)|(ィ)|(ェ)|(ォ)"

#define DELIMITOR_STR     ";"
#define DELIMITOR_CHR     ';'

#define GW_GENERIC_ERROR "gwaei generic error"

#define GW_FILE_ERROR 1


typedef enum
{
  GW_TARGET_RESULTS,
  GW_TARGET_KANJI,
  GW_TARGET_ENTRY,
  GW_TARGET_CONSOLE
} GwTargetOutput;

#endif
