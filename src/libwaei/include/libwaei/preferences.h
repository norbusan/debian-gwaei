#ifndef GW_GSETTINGS_PREFERENCES_INCLUDED
#define GW_GSETTINGS_PREFERENCES_INCLUDED

#include <gio/gio.h>

//GSettings
#define GW_SCHEMA_GNOME_INTERFACE   "org.gnome.interface"
#define GW_KEY_TOOLBAR_STYLE        "toolbar-style"
#define GW_KEY_DOCUMENT_FONT_NAME   "document-font-name"
#define GW_KEY_PROGRAM_VERSION      "version"

/////////////////////////
#define GW_SCHEMA_BASE             "org.gnome.gwaei"
#define GW_KEY_TOOLBAR_SHOW        "toolbar-show"
#define GW_KEY_LESS_RELEVANT_SHOW  "less-relevant-results-show"
#define GW_KEY_HIRA_KATA           "query-hiragana-to-katakana"
#define GW_KEY_KATA_HIRA           "query-katakana-to-hiragana"
#define GW_KEY_ROMAN_KANA          "query-romanji-to-kana"
#define GW_KEY_SPELLCHECK          "query-spellcheck"
#define GW_KEY_WINDOW_POSITIONS    "window-positions"

//////////////////////////
#define GW_SCHEMA_FONT               "org.gnome.gwaei.fonts"
#define GW_KEY_FONT_USE_GLOBAL_FONT  "use-global-document-font"
#define GW_KEY_FONT_CUSTOM_FONT      "custom-document-font"
#define GW_KEY_FONT_MAGNIFICATION    "magnification"

////////////////////////////
#define GW_SCHEMA_HIGHLIGHT     "org.gnome.gwaei.highlighting"
#define GW_KEY_MATCH_FG         "match-foreground"
#define GW_KEY_MATCH_BG         "match-background"
#define GW_KEY_HEADER_FG        "header-foreground"
#define GW_KEY_HEADER_BG        "header-background"
#define GW_KEY_COMMENT_FG       "comment-foreground"

#define GW_MATCH_FG_DEFAULT       "#000000"
#define GW_MATCH_BG_DEFAULT       "#CCEECC"
#define GW_HEADER_FG_DEFAULT      "#EE1111"
#define GW_HEADER_BG_DEFAULT      "#FFDEDE"
#define GW_COMMENT_FG_DEFAULT     "#2222DD"

////////////////////////////
#define GW_SCHEMA_DICTIONARY       "org.gnome.gwaei.dictionary"
#define GW_KEY_ENGLISH_SOURCE      "english-source"
#define GW_KEY_KANJI_SOURCE        "kanji-source"
#define GW_KEY_NAMES_PLACES_SOURCE "names-places-source"
#define GW_KEY_EXAMPLES_SOURCE     "examples-source"
#define GW_KEY_LOAD_ORDER          "load-order"


void lw_pref_initialize (void);
void lw_pref_free (void);

GSettings* lw_pref_get_settings_object (const char*);

void lw_pref_reset_value (GSettings*, const char*);
void lw_pref_reset_value_by_schema (const char*, const char*);

int lw_pref_get_int (GSettings*, const char *);
int lw_pref_get_int_by_schema (const char*, const char *);

void lw_pref_set_int (GSettings*, const char*, const int);
void lw_pref_set_int_by_schema (const char*, const char*, const int);

gboolean lw_pref_get_boolean (GSettings*, const char *);
gboolean lw_pref_get_boolean_by_schema (const char*, const char*);

void lw_pref_set_boolean (GSettings*, const char*, const gboolean);
void lw_pref_set_boolean_by_schema (const char*, const char*, const gboolean);

void lw_pref_get_string (char*, GSettings*, const char*, const int);
void lw_pref_get_string_by_schema (char*, const char*, const char*, const int);

void lw_pref_set_string (GSettings*, const char*, const char*);
void lw_pref_set_string_by_schema (const char*, const char*, const char*);

gulong lw_pref_add_change_listener (GSettings*, const char*, void (*callback_function) (GSettings*, gchar*, gpointer), gpointer);
gulong lw_pref_add_change_listener_by_schema (const char*, const char*, void (*callback_function) (GSettings*, gchar*, gpointer), gpointer);

void lw_pref_remove_change_listener (GSettings*, gulong);
void lw_pref_remove_change_listener_by_schema (const char*, gulong);


#endif





