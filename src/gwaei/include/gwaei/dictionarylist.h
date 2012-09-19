#ifndef GW_DICTIONARYLIST_INCLUDED
#define GW_DICTIONARYLIST_INCLUDED

G_BEGIN_DECLS

typedef enum {
  GW_DICTIONARYLIST_SIGNALID_ROW_CHANGED,
  TOTAL_GW_DICTIONARYLIST_SIGNALIDS
} GwDictionaryListSignalId;


typedef enum { 
  GW_DICTIONARYLIST_COLUMN_IMAGE,
  GW_DICTIONARYLIST_COLUMN_POSITION,
  GW_DICTIONARYLIST_COLUMN_NAME, 
  GW_DICTIONARYLIST_COLUMN_LONG_NAME, 
  GW_DICTIONARYLIST_COLUMN_ENGINE,
  GW_DICTIONARYLIST_COLUMN_SHORTCUT,
  GW_DICTIONARYLIST_COLUMN_SELECTED,
  GW_DICTIONARYLIST_COLUMN_DICT_POINTER,
  TOTAL_GW_DICTIONARYLIST_COLUMNS
} GwDictionaryListColumns;


//Boilerplate
typedef struct _GwDictionaryList GwDictionaryList;
typedef struct _GwDictionaryListClass GwDictionaryListClass;
typedef struct _GwDictionaryListPrivate GwDictionaryListPrivate;

#define GW_TYPE_DICTIONARYLIST              (gw_dictionarylist_get_type())
#define GW_DICTIONARYLIST(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GW_TYPE_DICTIONARYLIST, GwDictionaryList))
#define GW_DICTIONARYLIST_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), GW_TYPE_DICTIONARYLIST, GwDictionaryListClass))
#define GW_IS_DICTIONARYLIST(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), GW_TYPE_DICTIONARYLIST))
#define GW_IS_DICTIONARYLIST_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GW_TYPE_DICTIONARYLIST))
#define GW_DICTIONARYLIST_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), GW_TYPE_DICTIONARYLIST, GwDictionaryListClass))

struct _GwDictionaryList {
  LwDictionaryList store;
  GwDictionaryListPrivate *priv;
};

struct _GwDictionaryListClass {
  LwDictionaryListClass parent_class;
  guint signalid[TOTAL_GW_DICTIONARYLIST_SIGNALIDS];
};

//Methods
GwDictionaryList* gw_dictionarylist_new (void);
GType gw_dictionarylist_get_type (void) G_GNUC_CONST;

void gw_dictionarylist_reload (GwDictionaryList*, LwPreferences*);
LwDictionaryList* gw_dictionarylist_get_dictionarylist (GwDictionaryList*);
void gw_dictionarylist_save_order (GwDictionaryList*, LwPreferences*);
void gw_dictionarylist_load_order (GwDictionaryList*, LwPreferences*);
void gw_dictionarylist_update (GwDictionaryList*);
void gw_dictionarylist_normalize (GwDictionaryList*);
void gw_dictionarylist_sync_menumodel (GwDictionaryList*);
GMenuModel* gw_dictionarylist_get_menumodel (GwDictionaryList*);
GtkListStore* gw_dictionarylist_get_liststore (GwDictionaryList*);
void gw_dictionarylist_sync_treestore (GwDictionaryList*);

#include <gwaei/dictionarylist-callbacks.h>

G_END_DECLS

#endif
