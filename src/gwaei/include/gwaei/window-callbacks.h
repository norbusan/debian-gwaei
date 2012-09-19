#ifndef GW_WINDOW_CALLBACKS_INCLUDED
#define GW_WINDOW_CALLBACKS_INCLUDED

gboolean gw_window_configure_event_cb (GtkWidget*, GdkEvent*, gpointer);
gboolean gw_window_focus_in_event_cb (GtkWidget*, GdkEvent*, gpointer);
gboolean gw_window_delete_event_cb (GtkWidget*, GdkEvent*, gpointer);

#endif
