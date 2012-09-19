#ifndef GW_BASE_INCLUDED
#define GW_BASE_INCLUDED

#include <libwaei/regex.h>
#include <libwaei/utilities.h>
#include <libwaei/io.h>
#include <libwaei/dictinfo.h>
#include <libwaei/dictinfolist.h>
#include <libwaei/dictinst.h>
#include <libwaei/dictinstlist.h>
#include <libwaei/preferences.h>
#include <libwaei/resultline.h>
#include <libwaei/queryline.h>
#include <libwaei/searchitem.h>
#include <libwaei/historylist.h>
#include <libwaei/engine.h>

void lw_initialize (int*, char**);
void lw_free (void);


#endif
