#ifndef GW_MAIN_INCLUDED
#define GW_MAIN_INCLUDED

#include <gtk/gtk.h>

#include <libwaei/searchitem.h>

void gw_main_initialize ();
void gw_main_free (void);

gboolean gw_main_update_progress_feedback_timeout (gpointer);
void gw_main_update_total_results_label (LwSearchItem*);
char* gw_main_get_text_slice_from_buffer (int, int, int);
const char* gw_main_get_active_dictionary (void);
char* gw_main_buffer_get_text_by_target (LwTargetOutput);

gboolean gw_main_cancel_search (gpointer);
gboolean gw_main_cancel_search_by_searchitem (LwSearchItem*);
gboolean gw_main_cancel_search_by_tab_number (const int);
gboolean gw_main_cancel_search_by_target (LwTargetOutput);
gboolean gw_main_cancel_search_for_current_tab (void);
void gw_main_tab_cancel_all_searches (void);

void gw_main_display_no_results_found_page (LwSearchItem*);
void gw_main_initialize_buffer_by_searchitem (LwSearchItem*);

void gw_main_set_entry_text_by_searchitem (LwSearchItem*);
void gw_main_set_total_results_label_by_searchitem (LwSearchItem*);
void gw_main_set_search_progressbar_by_searchitem (LwSearchItem*);
void gw_main_set_main_window_title_by_searchitem (LwSearchItem*);

void gw_main_grab_focus_by_target (LwTargetOutput);
void gw_main_set_dictionary (int);
void gw_main_buffer_initialize_tags ();
void gw_main_set_font (char*, int*);
void gw_main_buffer_initialize_marks (gpointer);


void gw_main_search_entry_insert (char*);
void gw_main_clear_search_entry (void);
void gw_main_text_select_all_by_target (LwTargetOutput);
void gw_main_update_history_popups (void);

void gw_main_update_toolbar_buttons (void);

gunichar gw_main_get_hovered_character (int*, int*, GtkTextIter*);
void gw_main_show_window (char*);
void gw_main_set_cursor (GdkCursorType);
guint gw_main_get_current_target_focus (char*);

void gw_main_paste_text (LwTargetOutput);
void gw_main_cut_text (LwTargetOutput);
void gw_main_copy_text (LwTargetOutput);

void gw_main_cycle_dictionaries (gboolean);

void gw_main_text_select_none_by_target (LwTargetOutput);
void gw_main_strncpy_text_from_widget_by_target (char*, LwTargetOutput, int);

gboolean gw_main_has_selection_by_target (LwTargetOutput);

void gw_main_buffer_reload_tagtable_tags (void);

void gw_main_set_dictionary_by_searchitem (LwSearchItem*);

void gw_main_close_suggestion_box (void);
void gw_main_set_katakana_hiragana_conv (gboolean);
void gw_main_set_hiragana_katakana_conv (gboolean);
void gw_main_set_romaji_kana_conv (int);
void gw_main_set_less_relevant_show (gboolean);
void gw_main_set_use_global_document_font_checkbox (gboolean);
void gw_main_set_toolbar_show (gboolean);
void gw_main_set_toolbar_style (const char*);
void gw_main_set_color_to_swatch (const char*, const char*);

gboolean gw_update_icons_for_selection (gpointer);
gboolean gw_main_keep_searching_timeout (gpointer);

const char* gw_main_get_program_name (void);
char* gw_main_get_window_title_by_searchitem (LwSearchItem*);


#endif
