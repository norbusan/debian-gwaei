#ifndef GW_SETTINGSWINDOW_CALLBACKS_INCLUDED
#define GW_SETTINGSWINDOW_CALLBACKS_INCLUDED

void gw_settingswindow_hira_kata_conv_toggled_cb (GtkWidget*, gpointer);
void gw_settingswindow_kata_hira_conv_toggled_cb (GtkWidget*, gpointer);
void gw_settingswindow_spellcheck_toggled_cb (GtkWidget*, gpointer);
void gw_settingswindow_search_as_you_type_toggled_cb (GtkWidget*, gpointer);
void gw_settingswindow_romaji_kana_conv_changed_cb (GtkWidget*, gpointer);
void gw_settingswindow_swatch_color_changed_cb (GtkWidget*, gpointer);
void gw_settingswindow_reset_all_swatches_activated_cb (GtkWidget*, gpointer);
void gw_settingswindow_use_global_document_font_toggled_cb (GtkWidget*, gpointer);
void gw_settingswindow_custom_document_font_changed_cb (GtkWidget*, gpointer);

void gw_settingswindow_close_cb (GtkWidget*, gpointer);

void gw_settingswindow_sync_global_document_font_cb (GSettings*, gchar*, gpointer);
void gw_settingswindow_sync_custom_font_cb (GSettings*, gchar*, gpointer);
void gw_settingswindow_sync_search_as_you_type_cb (GSettings*, gchar*, gpointer);
void gw_settingswindow_sync_use_global_document_font_cb (GSettings*, gchar*, gpointer);
void gw_settingswindow_sync_romaji_kana_conv_cb (GSettings*, gchar*, gpointer);
void gw_settingswindow_sync_hira_kata_conv_cb (GSettings*, gchar*, gpointer);
void gw_settingswindow_sync_kata_hira_conv_cb (GSettings*, gchar*, gpointer);
void gw_settingswindow_sync_spellcheck_cb (GSettings*, gchar*, gpointer);
void gw_settingswindow_sync_swatch_color_cb (GSettings*, gchar*, gpointer);

void gw_settingswindow_add_dictionary_cb (GSimpleAction*, GVariant*, gpointer);
void gw_settingswindow_remove_dictionary_cb (GSimpleAction*, GVariant*, gpointer);

#endif
