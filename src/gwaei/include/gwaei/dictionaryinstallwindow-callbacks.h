#ifndef GW_DICTIONARYINSTALLWINDOW_CALLBACKS_INCLUDED
#define GW_DICTIONARYINSTALLWINDOW_CALLBACKS_INCLUDED

void gw_dictionaryinstallwindow_source_changed_cb (GSettings*, char*, gpointer);
void gw_dictionaryinstallwindow_cursor_changed_cb (GtkTreeView*, gpointer);
void gw_dictionaryinstallwindow_select_file_cb (GtkWidget*, gpointer);
void gw_dictionaryinstallwindow_listitem_toggled_cb (GtkCellRendererToggle*, gchar*, gpointer);
void gw_dictionaryinstallwindow_reset_default_uri_cb (GtkWidget*, gpointer);
void gw_dictionaryinstallwindow_filename_entry_changed_cb (GtkWidget*, gpointer);
void gw_dictionaryinstallwindow_source_entry_changed_cb (GtkWidget*, gpointer);
void gw_dictionaryinstallwindow_engine_combobox_changed_cb (GtkWidget*, gpointer);
void gw_dictionaryinstallwindow_encoding_combobox_changed_cb (GtkWidget*, gpointer);
void gw_dictionaryinstallwindow_compression_combobox_changed_cb (GtkWidget*, gpointer);
void gw_dictionaryinstallwindow_split_checkbox_toggled_cb (GtkWidget*, gpointer);
void gw_dictionaryinstallwindow_merge_checkbox_toggled_cb (GtkWidget*, gpointer);

#endif
