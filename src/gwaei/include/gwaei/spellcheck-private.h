#ifndef GW_SPELLCHECK_PRIVATE_INCLUDED
#define GW_SPELLCHECK_PRIVATE_INCLUDED

#include <hunspell/hunspell.h>

G_BEGIN_DECLS

typedef enum {
  GW_SPELLCHECK_SIGNALID_DRAW,
  GW_SPELLCHECK_SIGNALID_CHANGED,
  GW_SPELLCHECK_SIGNALID_POPULATE_POPUP,
  GW_SPELLCHECK_SIGNALID_BUTTON_PRESS_EVENT,
  GW_SPELLCHECK_SIGNALID_DESTROY,
  GW_SPELLCHECK_SIGNALID_RK_CONV,
  GW_SPELLCHECK_SIGNALID_DICTIONARY,
  TOTAL_GW_SPELLCHECK_SIGNALIDS
} GwSpellcheckSignalId;


typedef enum {
  GW_SPELLCHECK_TIMEOUTID_UPDATE,
  TOTAL_GW_SPELLCHECK_TIMEOUTIDS
} GwSpellcheckTimeoutid;


struct _GwSpellcheckPrivate {
  GwApplication *application;

  GtkEntry *entry;

  Hunhandle *handle;

  gchar** tolkens;           //A list of tolkens taken from the search entry
  GList *misspelled; //gchar* pointers to individual tolkens above

  guint timeout, threshold;  //Timer variables to prevent immediate spellchecks
  gint x, y; //mouse position
  guint signalid[TOTAL_GW_SPELLCHECK_SIGNALIDS];
  guint timeoutid[TOTAL_GW_SPELLCHECK_TIMEOUTIDS];
  gint rk_conv_setting;
};

#define GW_SPELLCHECK_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GW_TYPE_SPELLCHECK, GwSpellcheckPrivate))

G_END_DECLS

#endif

