#ifndef GW_DICTINFOLIST_INCLUDED
#define GW_DICTINFOLIST_INCLUDED

#define GW_DICTINFOLIST(object) (GwDictInfoList*) object

typedef enum {
  GW_DICTINFOLIST_SIGNALID_ROW_CHANGED,
  TOTAL_GW_DICTINFOLIST_SIGNALIDS
} GwDictInfoListSignalId;


typedef enum { 
  GW_DICTINFOLIST_COLUMN_IMAGE,
  GW_DICTINFOLIST_COLUMN_POSITION,
  GW_DICTINFOLIST_COLUMN_NAME, 
  GW_DICTINFOLIST_COLUMN_LONG_NAME, 
  GW_DICTINFOLIST_COLUMN_ENGINE,
  GW_DICTINFOLIST_COLUMN_SHORTCUT,
  GW_DICTINFOLIST_COLUMN_DICT_POINTER,
  TOTAL_GW_DICTINFOLIST_COLUMNS
} GwDictInfoListColumns;


struct _GwDictInfoList {
  EXTENDS_LW_DICTINFOLIST
  GtkListStore *model;
  gulong list_update_handler_id;
  GwApplication *application;
  guint signalids[TOTAL_GW_DICTINFOLIST_SIGNALIDS];
};
typedef struct _GwDictInfoList GwDictInfoList;


GwDictInfoList* gw_dictinfolist_new (const int, GwApplication*);
void gw_dictinfolist_free (GwDictInfoList*);
void gw_dictinfolist_init (GwDictInfoList*, GwApplication*);
void gw_dictinfolist_deinit (GwDictInfoList*);

void gw_dictinfolist_reload (GwDictInfoList*);

#include <gwaei/dictinfolist-callbacks.h>

#endif
