#ifndef GW_DICTIONARYSTORE_INCLUDED
#define GW_DICTIONARYSTORE_INCLUDED

G_BEGIN_DECLS

typedef enum {
  GW_DICTIONARYSTORE_SIGNALID_ROW_CHANGED,
  TOTAL_GW_DICTIONARYSTORE_SIGNALIDS
} GwDictionaryStoreSignalId;


typedef enum { 
  GW_DICTIONARYSTORE_COLUMN_IMAGE,
  GW_DICTIONARYSTORE_COLUMN_POSITION,
  GW_DICTIONARYSTORE_COLUMN_NAME, 
  GW_DICTIONARYSTORE_COLUMN_LONG_NAME, 
  GW_DICTIONARYSTORE_COLUMN_ENGINE,
  GW_DICTIONARYSTORE_COLUMN_SHORTCUT,
  GW_DICTIONARYSTORE_COLUMN_DICT_POINTER,
  TOTAL_GW_DICTIONARYSTORE_COLUMNS
} GwDictionaryStoreColumns;


//Boilerplate
typedef struct _GwDictionaryStore GwDictionaryStore;
typedef struct _GwDictionaryStoreClass GwDictionaryStoreClass;
typedef struct _GwDictionaryStorePrivate GwDictionaryStorePrivate;

#define GW_TYPE_DICTIONARYSTORE              (gw_dictionarystore_get_type())
#define GW_DICTIONARYSTORE(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GW_TYPE_DICTIONARYSTORE, GwDictionaryStore))
#define GW_DICTIONARYSTORE_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), GW_TYPE_DICTIONARYSTORE, GwDictionaryStoreClass))
#define GW_IS_DICTIONARYSTORE(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), GW_TYPE_DICTIONARYSTORE))
#define GW_IS_DICTIONARYSTORE_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GW_TYPE_DICTIONARYSTORE))
#define GW_DICTIONARYSTORE_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), GW_TYPE_DICTIONARYSTORE, GwDictionaryStoreClass))

struct _GwDictionaryStore {
  GtkListStore store;
  GwDictionaryStorePrivate *priv;
};

struct _GwDictionaryStoreClass {
  GtkListStoreClass parent_class;
  guint signalid[TOTAL_GW_DICTIONARYSTORE_SIGNALIDS];
  void (*changed) (GwDictionaryStore *store);
};

//Methods
GtkListStore* gw_dictionarystore_new (void);
GType gw_dictionarystore_get_type (void) G_GNUC_CONST;

void gw_dictionarystore_reload (GwDictionaryStore*, LwPreferences*);
LwDictInfoList* gw_dictionarystore_get_dictinfolist (GwDictionaryStore*);
void gw_dictionarystore_save_order (GwDictionaryStore*, LwPreferences*);
void gw_dictionarystore_load_order (GwDictionaryStore*, LwPreferences*);
void gw_dictionarystore_update (GwDictionaryStore*);
void gw_dictionarystore_normalize (GwDictionaryStore*);

#include <gwaei/dictionarystore-callbacks.h>

G_END_DECLS

#endif
