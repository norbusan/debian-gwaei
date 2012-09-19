#ifndef GW_GTK_SETTINGS_INTERFACE_INCLUDED
#define GW_GTK_SETTINGS_INTERFACE_INCLUDED

#include <gwaei/settings-callbacks.h>
#include <gwaei/settings-listeners.h>

void gw_settings_initialize (void);
void gw_settings_free (void);

void gw_settings_update_interface (void);

int gw_settings_update_progressbar (char*, int, gpointer);

void gw_settings_initialize_dictionary_order_list (void);
void gw_settings_set_dictionary_source (GtkWidget*, const char*);
void gw_settings_update_global_font_label (const char*);
void gw_settings_update_custom_font_button (const char*);
void gw_settings_update_dictionary_order_list (void);
void gw_settings_increment_order_list_processes (void);
void gw_settings_decrement_order_list_processes (void);
void gw_settings_set_use_global_document_font_checkbox (gboolean);

#endif
