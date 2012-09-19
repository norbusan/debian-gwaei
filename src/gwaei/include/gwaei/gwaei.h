#ifndef GW_FRONTEND_INCLUDED
#define GW_FRONTEND_INCLUDED

#include <gtk/gtk.h>

#include <libwaei/libwaei.h>

#include <gwaei/common.h>

#include <gwaei/main.h>
#include <gwaei/main-callbacks.h>
#include <gwaei/main-tabs.h>

#ifdef WITH_LIBUNIQUE
#include <gwaei/libunique.h>
#endif

#include <gwaei/printing.h>

#include <gwaei/radsearchtool.h>
#include <gwaei/radsearchtool-callbacks.h>

#include <gwaei/settings.h>
#include <gwaei/settings-callbacks.h>

#include <gwaei/dictionarymanager.h>
#include <gwaei/dictionaryinstall.h>
#include <gwaei/installprogress.h>

#include <gwaei/spellcheck.h>

#include <gwaei/kanjipad.h>
#include <gwaei/kanjipad-callbacks.h>
#include <gwaei/kanjipad-candidatearea.h>
#include <gwaei/kanjipad-drawingarea.h>


#include <gwaei/output-callbacks.h>

void gw_frontend_initialize (int*, char**);
void gw_frontend_free (void);
void gw_frontend_start_gtk (int, char**);

#endif
