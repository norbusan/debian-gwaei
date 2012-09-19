#ifndef GW_GTK_SETTINGS_INTERFACE_INCLUDED
#define GW_GTK_SETTINGS_INTERFACE_INCLUDED

void gw_ui_update_settings_interface (void);

void initialize_enabled_features_list(void);
void initialize_install_dictionary_list(void);
void initialize_advanced_settings(void);

int gw_ui_update_progressbar (char*, int, gpointer);

void gw_ui_initialize_dictionary_order_list (void);
void gw_settings_initialize_installed_dictionary_list (void);
void gw_ui_set_dictionary_source (GtkWidget*, const char*);
void gw_ui_update_global_font_label (const char*);
void gw_ui_update_custom_font_button (const char*);
void gw_ui_update_dictionary_order_list (void);
void gw_settings_increment_order_list_processes (void);
void gw_settings_decrement_order_list_processes (void);

#endif
