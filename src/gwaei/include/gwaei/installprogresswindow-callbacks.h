#ifndef GW_INSTALL_PROGRESS_CALLBACKS_INCLUDED
#define GW_INSTALL_PROGRESS_CALLBACKS_INCLUDED

gboolean gw_installprogresswindow_update_ui_timeout (gpointer);
int gw_installprogresswindow_update_dictinst_cb (double, gpointer);
void gw_installprogresswindow_start_cb (GtkWidget*, gpointer);

#endif
