#ifndef W_CONSOLE_INCLUDED
#define W_CONSOLE_INCLUDED

void w_console_about (WApplication*);
void w_console_list (WApplication*);
void w_console_start_banner (WApplication*);
void w_console_print_available_dictionaries (WApplication*);
void w_console_print_installable_dictionaries (WApplication*);

int w_console_install_dictionary (WApplication*, GError**);
int w_console_uninstall_dictionary (WApplication*, GError**);
int w_console_search (WApplication*, GError**);

#include "console-output.h"
#include "console-callbacks.h"

#endif
