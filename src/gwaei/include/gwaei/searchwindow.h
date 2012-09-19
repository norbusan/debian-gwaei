#ifndef GW_SEARCHWINDOW_INCLUDED
#define GW_SEARCHWINDOW_INCLUDED

#include <libwaei/searchitem.h>

G_BEGIN_DECLS

//Boilerplate
typedef struct _GwSearchWindow GwSearchWindow;
typedef struct _GwSearchWindowClass GwSearchWindowClass;
typedef struct _GwSearchWindowPrivate GwSearchWindowPrivate;

#define GW_TYPE_SEARCHWINDOW              (gw_searchwindow_get_type())
#define GW_SEARCHWINDOW(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GW_TYPE_SEARCHWINDOW, GwSearchWindow))
#define GW_SEARCHWINDOW_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), GW_TYPE_SEARCHWINDOW, GwSearchWindowClass))
#define GW_IS_SEARCHWINDOW(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), GW_TYPE_SEARCHWINDOW))
#define GW_IS_SEARCHWINDOW_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GW_TYPE_SEARCHWINDOW))
#define GW_SEARCHWINDOW_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), GW_TYPE_SEARCHWINDOW, GwSearchWindowClass))

#define GW_SEARCHWINDOW_KEEP_SEARCHING_MAX_DELAY 3

struct _GwSearchWindow {
  GwWindow window;
  GwSearchWindowPrivate *priv;
};

struct _GwSearchWindowClass {
  GwWindowClass parent_class;
};

GtkWindow* gw_searchwindow_new (GtkApplication *application);
GType gw_searchwindow_get_type (void) G_GNUC_CONST;

gboolean gw_searchwindow_update_progress_feedback_timeout (GwSearchWindow*);
gboolean gw_searchwindow_update_icons_for_selection_timeout (GwSearchWindow*);
gboolean gw_searchwindow_keep_searching_timeout (GwSearchWindow*);

void gw_searchwindow_update_history_popups (GwSearchWindow*);
void gw_searchwindow_update_toolbar_buttons (GwSearchWindow*);
void gw_searchwindow_update_total_results_label (GwSearchWindow*, LwSearchItem*);

char* gw_searchwindow_get_text_slice_from_buffer (GwSearchWindow*, int, int, int);
char* gw_searchwindow_get_text (GwSearchWindow*, GtkWidget*);

void gw_searchwindow_cancel_search_by_searchitem (GwSearchWindow*, LwSearchItem*);
void gw_searchwindow_cancel_search_by_tab_number (GwSearchWindow*, const int);
void gw_searchwindow_cancel_search_for_current_tab (GwSearchWindow*);
void gw_searchwindow_cancel_search_by_content (GwSearchWindow*, gpointer);
void gw_searchwindow_cancel_all_searches (GwSearchWindow*);

void gw_searchwindow_initialize_buffer_by_searchitem (GwSearchWindow*, LwSearchItem*);

void gw_searchwindow_entry_set_text (GwSearchWindow*, const gchar*);
void gw_searchwindow_entry_insert_text (GwSearchWindow*, const gchar*);
void gw_searchwindow_set_entry_text_by_searchitem (GwSearchWindow*, LwSearchItem*);
void gw_searchwindow_set_total_results_label_by_searchitem (GwSearchWindow*, LwSearchItem*);
void gw_searchwindow_set_search_progressbar_by_searchitem (GwSearchWindow*, LwSearchItem*);
char* gw_searchwindow_get_title_by_searchitem (GwSearchWindow*, LwSearchItem*);
void gw_searchwindow_set_title_by_searchitem (GwSearchWindow*, LwSearchItem*);

LwDictInfo* gw_searchwindow_get_dictionary (GwSearchWindow*);
void gw_searchwindow_set_dictionary_by_searchitem (GwSearchWindow *window, LwSearchItem*);
void gw_searchwindow_set_dictionary (GwSearchWindow*, int);

void gw_searchwindow_buffer_initialize_tags (GwSearchWindow*);
void gw_searchwindow_set_font (GwSearchWindow*);
void gw_searchwindow_buffer_initialize_marks (GtkTextBuffer*);


void gw_searchwindow_entry_insert (GwSearchWindow*, char*);
void gw_searchwindow_clear_search_entry (GwSearchWindow*);


gunichar gw_searchwindow_get_hovered_character (GwSearchWindow*, int*, int*, GtkTextIter*);
void gw_searchwindow_show_window (GwSearchWindow*, char*);
void gw_searchwindow_set_cursor (GwSearchWindow*, GdkCursorType);

void gw_searchwindow_paste_text (GwSearchWindow*, GtkWidget*);
void gw_searchwindow_cut_text (GwSearchWindow*, GtkWidget*);
void gw_searchwindow_copy_text (GwSearchWindow*, GtkWidget*);

void gw_searchwindow_cycle_dictionaries (GwSearchWindow*, gboolean);

void gw_searchwindow_select_all (GwSearchWindow*, GtkWidget*);
void gw_searchwindow_select_none (GwSearchWindow*, GtkWidget*);
gboolean gw_searchwindow_has_selection (GwSearchWindow*, GtkWidget*);

void gw_searchwindow_reload_tagtable_tags (GwSearchWindow*);

void gw_searchwindow_set_dictionary_by_searchitem (GwSearchWindow*, LwSearchItem*);

void gw_searchwindow_close_suggestion_box (GwSearchWindow*);
void gw_searchwindow_set_katakana_hiragana_conv (GwSearchWindow*, gboolean);
void gw_searchwindow_set_hiragana_katakana_conv (GwSearchWindow*, gboolean);
void gw_searchwindow_set_romaji_kana_conv (GwSearchWindow*, int);
void gw_searchwindow_set_less_relevant_show (GwSearchWindow*, gboolean);
void gw_searchwindow_set_use_global_document_font_checkbox (GwSearchWindow*, gboolean);
void gw_searchwindow_set_toolbar_show (GwSearchWindow*, gboolean*);
void gw_searchwindow_set_toolbar_style (GwSearchWindow*, const char*);
void gw_searchwindow_set_statusbar_show (GwSearchWindow*, gboolean);
void gw_searchwindow_set_color_to_swatch (GwSearchWindow*, const char*, const char*);

void gw_searchwindow_guarantee_first_tab (GwSearchWindow*);

GtkTextView* gw_searchwindow_get_current_textview (GwSearchWindow*);

void gw_searchwindow_set_tab_text_by_searchitem (GwSearchWindow*, LwSearchItem*);
void gw_searchwindow_set_current_searchitem (GwSearchWindow*, LwSearchItem*);
LwSearchItem* gw_searchwindow_get_current_searchitem (GwSearchWindow*);
void gw_searchwindow_sync_current_searchitem (GwSearchWindow*);
void gw_searchwindow_no_results_search_for_dictionary_cb (GtkWidget*, gpointer);

int gw_searchwindow_new_tab (GwSearchWindow*);
void gw_searchwindow_remove_tab (GwSearchWindow*, int);

void gw_searchwindow_start_search (GwSearchWindow*, LwSearchItem*);
void gw_searchwindow_initialize_dictionary_menu (GwSearchWindow*);
void gw_searchwindow_initialize_dictionary_combobox (GwSearchWindow*);

void gw_searchwindow_insert_resultpopup_button (GwSearchWindow*, LwSearchItem*, LwResultLine*, GtkTextIter*);

void gw_searchwindow_append_to_buffer (GwSearchWindow*, LwSearchItem*, const char *, char*, char*, int*, int*);

#include "searchwindow-callbacks.h"
#include "searchwindow-output.h"
#include "search-data.h"

G_END_DECLS

#endif
