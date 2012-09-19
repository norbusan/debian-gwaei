#ifndef W_CONSOLE_CALLBACKS_INCLUDED
#define W_CONSOLE_CALLBACKS_INCLUDED

gboolean w_console_append_result_timeout (gpointer);
void w_console_update_progress_cb (LwDictionary*, gpointer);
int w_console_uninstall_progress_cb (gdouble, gpointer);

#endif
