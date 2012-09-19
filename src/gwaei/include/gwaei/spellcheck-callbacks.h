#ifndef GW_SPELLCHECK_CALLBACKS_INCLUDED
#define GW_SPELLCHECK_CALLBACKS_INCLUDED

void gw_spellcheck_queue_cb (GtkEditable*, gpointer);
void gw_spellcheck_menuitem_activated_cb (GtkWidget*, gpointer);
gboolean gw_spellcheck_draw_underline_cb (GtkWidget*, cairo_t*, gpointer);
void gw_spellcheck_populate_cb (GtkEntry*, GtkMenu*, gpointer);
gboolean gw_spellcheck_update_timeout (gpointer);
void gw_spellcheck_free_menuitem_data_cb (GtkWidget*, gpointer);

#endif
