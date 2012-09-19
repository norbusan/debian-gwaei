#ifndef GW_SPELLCHECK_CALLBACKS_INCLUDED
#define GW_SPELLCHECK_CALLBACKS_INCLUDED

void gw_spellcheck_add_menuitem_activated_cb (GtkWidget*, gpointer);
void gw_spellcheck_queue_cb (GtkEditable*, gpointer);
void gw_spellcheck_menuitem_activated_cb (GtkWidget*, gpointer);
gboolean gw_spellcheck_draw_underline_cb (GtkWidget*, cairo_t*, gpointer);
gboolean gw_spellcheck_update_timeout (gpointer);
void gw_spellcheck_free_menuitem_data_cb (GtkWidget*, gpointer);
gboolean gw_spellcheck_button_press_event_cb (GtkWidget*, GdkEvent*, gpointer);
void gw_spellcheck_populate_popup_cb (GtkEntry*, GtkMenu*, gpointer);
void gw_spellcheck_sync_rk_conv_cb (GSettings*, gchar*, gpointer);
void gw_spellcheck_sync_dictionary_cb (GSettings*, gchar*, gpointer);

#endif
