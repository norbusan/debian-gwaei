#ifndef GW_GTK_MAIN_INTERFACE_INCLUDED
#define GW_GTK_MAIN_INTERFACE_INCLUDED

#include <gtk/gtk.h>

#include <gwaei/searchitem-object.h>

gpointer get_gobject_by_target (GwTargetOutput);
GtkWidget* get_widget_by_target (GwTargetOutput);

void initialize_text_views (void);
void initialize_window_attributes (char* window_id);
void save_window_attributes (char* window_id);
void rebuild_history_menuitem_popup (void);
int  rebuild_combobox_dictionary_list (void);
void update_toolbar_buttons (void);
void update_history_popup (void);
void gw_ui_update_total_results_label (GwSearchItem*);
gboolean gw_load_gtk_builder_xml (const char );
char* gw_ui_get_text_slice_from_buffer (int, int, int);
const char* gw_ui_get_active_dictionary (void);
char* gw_ui_buffer_get_text_by_target (GwTargetOutput);
void initialize_gui_interface (int, char**);
void gw_ui_open_kanji_sidebar (void);
void save_window_attributes_and_hide (const char*);

gboolean gw_ui_cancel_search (gpointer);
gboolean gw_ui_cancel_search_by_searchitem (GwSearchItem*);
gboolean gw_ui_cancel_search_by_tab_number (const int);
gboolean gw_ui_cancel_search_by_target (GwTargetOutput);
gboolean gw_ui_cancel_search_for_current_tab (void);
void gw_ui_tab_cancel_all_searches (void);


void gw_ui_initialize_interface_output_generics (void);
void gw_ui_append_edict_results_to_buffer (GwSearchItem*);
void gw_ui_append_kanjidict_results_to_buffer (GwSearchItem*);
void gw_ui_append_examplesdict_results_to_buffer (GwSearchItem*);
void gw_ui_append_unknowndict_results_to_buffer (GwSearchItem*);
void gw_ui_update_progress_feedback (GwSearchItem*);
void gw_ui_no_result(GwSearchItem*);
void gw_ui_append_less_relevant_header_to_output (GwSearchItem*);
void gw_ui_append_more_relevant_header_to_output (GwSearchItem*);
void gw_ui_pre_search_prep (GwSearchItem*);
void gw_ui_after_search_cleanup (GwSearchItem*);

void gw_ui_set_query_entry_text_by_searchitem (GwSearchItem*);
void gw_ui_set_total_results_label_by_searchitem (GwSearchItem*);
void gw_ui_set_search_progressbar_by_searchitem (GwSearchItem*);
void gw_ui_set_main_window_title_by_searchitem (GwSearchItem*);

void gw_ui_grab_focus_by_target (GwTargetOutput);
void gw_ui_set_dictionary (int);
void gw_ui_buffer_initialize_tags ();
void gw_ui_set_font (char*, int*);
void gw_ui_buffer_initialize_marks (gpointer);


void gw_ui_search_entry_insert (char*);
void gw_ui_clear_search_entry (void);
void gw_ui_text_select_all_by_target (GwTargetOutput);
void gw_ui_update_history_popups (void);

void gw_ui_update_toolbar_buttons (void);

gunichar gw_ui_get_hovered_character (int*, int*, GtkTextIter*);
void gw_ui_show_window (char*);
void gw_ui_close_kanji_sidebar (void);
void gw_ui_set_cursor (GdkCursorType);
guint gw_ui_get_current_target_focus (char*);

void gw_ui_paste_text (GwTargetOutput);
void gw_ui_cut_text (GwTargetOutput);
void gw_ui_copy_text (GwTargetOutput);

void gw_ui_cycle_dictionaries (gboolean);

void gw_ui_text_select_none_by_target (GwTargetOutput);
void gw_ui_strncpy_text_from_widget_by_target (char*, GwTargetOutput, int);

gboolean gw_ui_has_selection_by_target (GwTargetOutput);

void gw_ui_buffer_reload_tagtable_tags (void);

void gw_ui_set_dictionary_by_searchitem (GwSearchItem*);


gboolean gw_ui_widget_equals_target (gpointer, GwTargetOutput);
void gw_ui_close_suggestion_box (void);
void gw_ui_set_katakana_hiragana_conv (gboolean);
void gw_ui_set_hiragana_katakana_conv (gboolean);
void gw_ui_set_romaji_kana_conv (int);
void gw_ui_set_less_relevant_show (gboolean);
void gw_ui_set_use_global_document_font_checkbox (gboolean);
void gw_ui_set_toolbar_show (gboolean);
void gw_ui_set_toolbar_style (const char*);
void gw_ui_set_color_to_swatch (const char*, const char*);


#endif
