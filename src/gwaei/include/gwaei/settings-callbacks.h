#ifndef GW_GTK_SETTINGS_CALLBACKS_INCLUDED
#define GW_GTK_SETTINGS_CALLBACKS_INCLUDED


void gw_settings_show_cb (GtkWidget *widget, gpointer data);
void gw_settings_spellcheck_toggled_cb (GtkWidget*, gpointer);
void gw_settings_hira_kata_conv_toggled_cb (GtkWidget*, gpointer);
void gw_settings_kata_hira_conv_toggled_cb (GtkWidget*, gpointer);
void gw_settings_romaji_kana_conv_changed_cb (GtkWidget*, gpointer);
void gw_settings_swatch_color_set_cb (GtkWidget*, gpointer);
void gw_settings_swatch_colors_reset_cb (GtkWidget*, gpointer);
void gw_settings_use_global_document_font_toggled_cb (GtkWidget*, gpointer);
void gw_settings_custom_document_font_set_cb (GtkWidget*, gpointer);



#endif
