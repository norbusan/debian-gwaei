#ifndef LW_PREFERENCES_INCLUDED
#define LW_PREFERENCES_INCLUDED

#include <gio/gio.h>
#define G_SETTINGS_ENABLE_BACKEND
#include <gio/gsettingsbackend.h>

//GSettings
#define LW_SCHEMA_GNOME_INTERFACE   "org.gnome.desktop.interface"
#define LW_KEY_TOOLBAR_STYLE        "toolbar-style"
#define LW_KEY_DOCUMENT_FONT_NAME   "font-name"
#define LW_KEY_PROGRAM_VERSION      "version"

/////////////////////////
#define LW_SCHEMA_BASE             "org.gnome.gwaei"
#define LW_KEY_TOOLBAR_SHOW        "toolbar-show"
#define LW_KEY_STATUSBAR_SHOW      "statusbar-show"
#define LW_KEY_LESS_RELEVANT_SHOW  "less-relevant-results-show"
#define LW_KEY_HIRA_KATA           "query-hiragana-to-katakana"
#define LW_KEY_KATA_HIRA           "query-katakana-to-hiragana"
#define LW_KEY_ROMAN_KANA          "query-romanji-to-kana"
#define LW_KEY_SPELLCHECK          "query-spellcheck"
#define LW_KEY_SEARCH_AS_YOU_TYPE  "search-as-you-type"
#define LW_KEY_WINDOW_POSITIONS    "window-positions"

//////////////////////////
#define LW_SCHEMA_FONT               "org.gnome.gwaei.fonts"
#define LW_KEY_FONT_USE_GLOBAL_FONT  "use-global-document-font"
#define LW_KEY_FONT_CUSTOM_FONT      "custom-document-font"
#define LW_KEY_FONT_MAGNIFICATION    "magnification"

////////////////////////////
#define LW_SCHEMA_HIGHLIGHT     "org.gnome.gwaei.highlighting"
#define LW_KEY_MATCH_FG         "match-foreground"
#define LW_KEY_MATCH_BG         "match-background"
#define LW_KEY_HEADER_FG        "header-foreground"
#define LW_KEY_HEADER_BG        "header-background"
#define LW_KEY_COMMENT_FG       "comment-foreground"

#define LW_MATCH_FG_DEFAULT       "#000000"
#define LW_MATCH_BG_DEFAULT       "#CCEECC"
#define LW_HEADER_FG_DEFAULT      "#EE1111"
#define LW_HEADER_BG_DEFAULT      "#FFDEDE"
#define LW_COMMENT_FG_DEFAULT     "#2222DD"

////////////////////////////
#define LW_SCHEMA_DICTIONARY       "org.gnome.gwaei.dictionary"
#define LW_KEY_ENGLISH_SOURCE      "english-source"
#define LW_KEY_KANJI_SOURCE        "kanji-source"
#define LW_KEY_NAMES_PLACES_SOURCE "names-places-source"
#define LW_KEY_EXAMPLES_SOURCE     "examples-source"
#define LW_KEY_LOAD_ORDER          "load-order"

#define LW_PREFMANAGER(object) (LwPreferences*) object

struct _LwPreferences {
  GList *settingslist;
  GMutex *mutex;
  GSettingsBackend *backend;

  gboolean toolbar_show; 
  gboolean statusbar_show;
  gboolean query_katakana_to_hiragana;
  gboolean query_hiragana_to_katakana;
  gint query_romaji_to_kana;
  gboolean query_spellcheck;
  gboolean search_as_you_type;
  gchar* window_positions;

  gchar *dictionary_load_order;
  gchar *dictionary_english_source;
  gchar *dictionary_kanji_source;
  gchar *dictionary_names_places_source;
  gchar *dictionary_examples_source;

  gboolean use_global_document_font;
  gchar *custom_document_font;
  gint magnification;
  
  gchar *comment_foreground;
  gchar *comment_background;
  gchar *match_foreground;
  gchar *match_background;
  gchar *header_foreground;
  gchar *header_background;
};
typedef struct _LwPreferences LwPreferences;


LwPreferences* lw_preferences_new (GSettingsBackend*);
void lw_preferences_free (LwPreferences*);
void lw_preferences_init (LwPreferences*, GSettingsBackend*);
void lw_preferences_deinit (LwPreferences*);

void lw_preferences_free_settings (LwPreferences*);

gboolean lw_preferences_schema_is_installed (const char*);
GSettings* lw_preferences_get_settings_object (LwPreferences*, const char*);

void lw_preferences_reset_value (GSettings*, const char*);
void lw_preferences_reset_value_by_schema (LwPreferences*, const char*, const char*);

int lw_preferences_get_int (GSettings*, const char *);
int lw_preferences_get_int_by_schema (LwPreferences*, const char*, const char *);

void lw_preferences_set_int (GSettings*, const char*, const int);
void lw_preferences_set_int_by_schema (LwPreferences*, const char*, const char*, const int);

gboolean lw_preferences_get_boolean (GSettings*, const char *);
gboolean lw_preferences_get_boolean_by_schema (LwPreferences*, const char*, const char*);

void lw_preferences_set_boolean (GSettings*, const char*, const gboolean);
void lw_preferences_set_boolean_by_schema (LwPreferences*, const char*, const char*, const gboolean);

void lw_preferences_get_string (char*, GSettings*, const char*, const int);
void lw_preferences_get_string_by_schema (LwPreferences*, char*, const char*, const char*, const int);

void lw_preferences_set_string (GSettings*, const char*, const char*);
void lw_preferences_set_string_by_schema (LwPreferences*, const char*, const char*, const char*);

gulong lw_preferences_add_change_listener (GSettings*, const char*, void (*callback_function) (GSettings*, gchar*, gpointer), gpointer);
gulong lw_preferences_add_change_listener_by_schema (LwPreferences*, const char*, const char*, void (*callback_function) (GSettings*, gchar*, gpointer), gpointer);

void lw_preferences_remove_change_listener (GSettings*, gulong);
void lw_preferences_remove_change_listener_by_schema (LwPreferences*, const char*, gulong);


#endif





