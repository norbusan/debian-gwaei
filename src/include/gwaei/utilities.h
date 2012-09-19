#ifndef GW_UTILITIES_INCLUDED
#define GW_UTILITIES_INCLUDED

typedef enum {
  GW_CONSOLE_RUNMODE,
  GW_GTK_RUNMODE,
  GW_QT_RUNMODE,
  GW_NCURSES_RUNMODE,
  GW_TOTAL_RUNMODES
} GwRunmode;

const char* gw_util_get_waei_directory ();
void initialize_gconf_schemas (void);
char* gw_util_next_hira_char_from_roma (char*);
char* gw_util_roma_to_hira (char*, char*);
gboolean gw_util_str_roma_to_hira (char*, char*, int);
gboolean gw_util_is_japanese_locale (void);
gboolean gw_util_is_japanese_ctype (void);

GwRunmode gw_util_get_runmode (void);

gboolean gw_util_is_hiragana_str (char*);
gboolean gw_util_is_util_kanji_str (char*);
gboolean gw_util_is_katakana_str (char*);
gboolean gw_util_is_romaji_str (char*);
gboolean gw_util_is_kanji_ish_str (char*);
gboolean gw_util_is_kanji_str (char*);
gboolean gw_util_is_furigana_str (char*);

void gw_util_str_shift_kata_to_hira (char*);
void gw_util_str_shift_hira_to_kata (char*);

char* gw_util_strdup_args_to_query (int, char**);

char* gw_util_sanitize_input(char*, gboolean);

gboolean gw_util_all_chars_are_in_range (char*, int, int);

gboolean gw_util_force_japanese_locale (void);
void gw_util_initialize_runmode (int, char**);

void gw_util_strncpy_fallback_from_key (char*, const char*, int);



#endif
