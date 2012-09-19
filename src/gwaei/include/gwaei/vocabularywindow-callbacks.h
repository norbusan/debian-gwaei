#ifndef GW_VOCABULARYWINDOW_CALLBACKS_INCLUDED
#define GW_VOCABULARYWINDOW_CALLBACKS_INCLUDED

void gw_vocabularywindow_close_cb (GSimpleAction*, GVariant*, gpointer);
void gw_vocabularywindow_cell_edited_cb (GtkCellRendererText*, gchar*, gchar*, gpointer);
void gw_vocabularywindow_list_cell_edited_cb (GtkCellRendererText*, gchar*, gchar*, gpointer);
void gw_vocabularywindow_list_row_deleted_cb (GtkTreeModel*, GtkTreePath*, gpointer);
gboolean gw_vocabularywindow_event_after_cb (GtkWidget*, GdkEvent*, gpointer);
void gw_vocabularywindow_liststore_changed_cb (GwVocabularyListStore*, gpointer);

void gw_vocabularywindow_sync_menubar_show_cb (GSettings*, gchar*, gpointer);
void gw_vocabularywindow_sync_shuffle_flashcards_cb (GSettings*, gchar*, gpointer);
void gw_vocabularywindow_sync_trim_flashcards_cb (GSettings*, gchar*, gpointer);
void gw_vocabularywindow_sync_list_order_cb (GSettings*, gchar*, gpointer);
void gw_vocabularywindow_sync_track_results_cb (GSettings*, gchar*, gpointer);
void gw_vocabularywindow_sync_toolbar_show_cb (GSettings*, gchar*, gpointer);
void gw_vocabularywindow_sync_position_column_show_cb (GSettings*, gchar*, gpointer);
void gw_vocabularywindow_sync_score_column_show_cb (GSettings*, gchar*, gpointer);
void gw_vocabularywindow_sync_timestamp_column_show_cb (GSettings*, gchar*, gpointer);

void gw_vocabularywindow_editable_toggled_cb (GSimpleAction*, GVariant*, gpointer);
void gw_vocabularywindow_toolbar_show_toggled_cb (GSimpleAction*, GVariant*, gpointer);
void gw_vocabularywindow_menubar_show_toggled_cb (GSimpleAction*, GVariant*, gpointer);
void gw_vocabularywindow_position_column_toggled_cb (GSimpleAction*, GVariant*, gpointer);
void gw_vocabularywindow_score_column_toggled_cb (GSimpleAction*, GVariant*, gpointer);
void gw_vocabularywindow_timestamp_column_toggled_cb (GSimpleAction*, GVariant*, gpointer);
void gw_vocabularywindow_revert_cb (GSimpleAction*, GVariant*, gpointer);
void gw_vocabularywindow_save_cb (GSimpleAction*, GVariant*, gpointer);
void gw_vocabularywindow_import_cb (GSimpleAction*, GVariant*, gpointer);
void gw_vocabularywindow_export_cb (GSimpleAction*, GVariant*, gpointer);

void gw_vocabularywindow_cut_cb (GSimpleAction*, GVariant*, gpointer);
void gw_vocabularywindow_copy_cb (GSimpleAction*, GVariant*, gpointer);
void gw_vocabularywindow_paste_cb (GSimpleAction*, GVariant*, gpointer);
void gw_vocabularywindow_delete_cb (GSimpleAction*, GVariant*, gpointer);

void gw_vocabularywindow_new_word_cb (GSimpleAction*, GVariant*, gpointer);
void gw_vocabularywindow_remove_word_cb (GSimpleAction*, GVariant*, gpointer);
void gw_vocabularywindow_new_list_cb (GSimpleAction*, GVariant*, gpointer);
void gw_vocabularywindow_remove_list_cb (GSimpleAction*, GVariant*, gpointer);

void gw_vocabularywindow_kanji_definition_flashcards_cb (GSimpleAction*, GVariant*, gpointer);
void gw_vocabularywindow_definition_kanji_flashcards_cb (GSimpleAction*, GVariant*, gpointer);
void gw_vocabularywindow_furigana_definition_flashcards_cb (GSimpleAction*, GVariant*, gpointer);
void gw_vocabularywindow_definition_furigana_flashcards_cb (GSimpleAction*, GVariant*, gpointer);
void gw_vocabularywindow_kanji_furigana_flashcards_cb(GSimpleAction*, GVariant*, gpointer);
void gw_vocabularywindow_furigana_kanji_flashcards_cb (GSimpleAction*, GVariant*, gpointer);

void gw_vocabularywindow_shuffle_toggled_cb (GSimpleAction*,  GVariant*, gpointer);
void gw_vocabularywindow_trim_toggled_cb (GSimpleAction*,  GVariant*, gpointer);
void gw_vocabularywindow_track_results_toggled_cb (GSimpleAction*,  GVariant*, gpointer);

void gw_vocabularywindow_select_new_word_from_dialog_cb (GtkWidget*, gpointer);

#endif
