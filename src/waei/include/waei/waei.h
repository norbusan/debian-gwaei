#ifndef GW_FRONTEND_INCLUDED
#define GW_FRONTEND_INCLUDED

#include <libwaei/libwaei.h>
#include <waei/console.h>
#include <waei/ncurses.h>

void w_initialize (int*, char**);
void w_free (void);
int w_start_console (int, char**);
int w_start_ncurses (int, char**);
gboolean w_get_color_switch (void);

#endif
