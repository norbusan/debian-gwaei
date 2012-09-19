#ifndef GW_VOCABULARYWINDOW_CALLBACKS_INCLUDED
#define GW_VOCABULARYWINDOW_CALLBACKS_INCLUDED

void gw_vocabularywindow_close_cb (GtkWidget*, gpointer);
gboolean gw_vocabularywindow_delete_event_cb (GtkWidget*, GdkEvent*, gpointer);
void gw_vocabularywindow_cell_edited_cb (GtkCellRendererText*, gchar*, gchar*, gpointer);
void gw_vocabularywindow_list_cell_edited_cb (GtkCellRendererText*, gchar*, gchar*, gpointer);
void gw_vocabularywindow_list_row_deleted_cb (GtkTreeModel*, GtkTreePath*, gpointer);
gboolean gw_vocabularywindow_event_after_cb (GtkWidget*, GdkEvent*, gpointer);
void gw_vocabularywindow_liststore_changed_cb (GwVocabularyListStore*, gpointer);

void gw_vocabularywindow_sync_shuffle_flashcards_cb (GSettings*, gchar*, gpointer);
void gw_vocabularywindow_sync_trim_flashcards_cb (GSettings*, gchar*, gpointer);
void gw_vocabularywindow_sync_list_order_cb (GSettings*, gchar*, gpointer);
void gw_vocabularywindow_sync_track_results_cb (GSettings*, gchar*, gpointer);
void gw_vocabularywindow_sync_toolbar_show_cb (GSettings*, gchar*, gpointer);
void gw_vocabularywindow_sync_position_column_show_cb (GSettings*, gchar*, gpointer);
void gw_vocabularywindow_sync_score_column_show_cb (GSettings*, gchar*, gpointer);
void gw_vocabularywindow_sync_timestamp_column_show_cb (GSettings*, gchar*, gpointer);

void gw_vocabularywindow_toolbar_toggled_cb (GtkAction*, gpointer);
void gw_vocabularywindow_position_column_toggled_cb (GtkAction*, gpointer);
void gw_vocabularywindow_score_column_toggled_cb (GtkAction*, gpointer);
void gw_vocabularywindow_timestamp_column_toggled_cb (GtkAction*, gpointer);

#endif
