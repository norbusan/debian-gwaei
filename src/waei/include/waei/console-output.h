#ifndef W_CONSOLE_OUTPUT_INCLUDED
#define W_CONSOLE_OUTPUT_INCLUDED

void w_console_append_result (WApplication*, LwSearchItem*);
void w_console_no_result (WApplication*, LwSearchItem*);

int w_console_install_progress_cb (double, gpointer);
int w_console_uninstall_progress_cb (double, gpointer);


#endif
