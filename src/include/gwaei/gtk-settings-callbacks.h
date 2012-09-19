#ifndef GW_GTK_SETTINGS_CALLBACKS_INCLUDED
#define GW_GTK_SETTINGS_CALLBACKS_INCLUDED


void do_spellcheck_toggle (GtkWidget*, gpointer);
void do_hiragana_katakana_conv_toggle (GtkWidget*, gpointer);
void do_katakana_hiragana_conv_toggle (GtkWidget*, gpointer);
void do_romaji_kana_conv_change (GtkWidget*, gpointer);
void do_set_color_to_swatch (GtkWidget*, gpointer);
void do_move_dictionary_up (GtkWidget*, gpointer);
void do_move_dictionary_down (GtkWidget*, gpointer);
void do_color_reset_for_swatches (GtkWidget*, gpointer);
void do_source_entry_changed_action (GtkWidget*, gpointer);
void do_dictionary_source_browse (GtkWidget*, gpointer);
void do_dictionary_source_reset (GtkWidget*, gpointer);
void do_toggle_advanced_source (GtkWidget*, gpointer);
void do_dictionary_install (GtkWidget*, gpointer);
void do_cancel_dictionary_install (GtkWidget*, gpointer);
void do_dictionary_remove (GtkWidget*, gpointer);
void do_toggle_use_global_document_font (GtkWidget*, gpointer);
void do_set_custom_document_font (GtkWidget*, gpointer);
void do_add_dictionary_to_order_prefs (GtkTreeModel*, GtkTreePath*, GtkTreeIter*, gpointer*);
void do_remove_dictionary_from_order_prefs (GtkTreeModel*, GtkTreePath*, gpointer*);



#endif
