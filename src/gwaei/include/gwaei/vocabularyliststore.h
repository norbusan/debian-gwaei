#ifndef GW_VOCABULARYLISTSTORE_INCLUDED
#define GW_VOCABULARYLISTSTORE_INCLUDED

#include <gwaei/vocabularywordstore.h>

G_BEGIN_DECLS

typedef enum {
  GW_VOCABULARYLISTSTORE_CLASS_SIGNALID_CHANGED,
  TOTAL_GW_VOCABULARYLISTSTORE_CLASS_SIGNALIDS
} GwVocabularyListStoreClassSignalId;

typedef enum { 
  GW_VOCABULARYLISTSTORE_COLUMN_NAME,
  GW_VOCABULARYLISTSTORE_COLUMN_CHANGED,
  GW_VOCABULARYLISTSTORE_COLUMN_OBJECT,
  TOTAL_GW_VOCABULARYLISTSTORE_COLUMNS
} GwVocabularyListStoreColumn;

//Boilerplate
typedef struct _GwVocabularyListStore GwVocabularyListStore;
typedef struct _GwVocabularyListStoreClass GwVocabularyListStoreClass;
typedef struct _GwVocabularyListStorePrivate GwVocabularyListStorePrivate;

#define GW_TYPE_VOCABULARYLISTSTORE              (gw_vocabularyliststore_get_type())
#define GW_VOCABULARYLISTSTORE(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GW_TYPE_VOCABULARYLISTSTORE, GwVocabularyListStore))
#define GW_VOCABULARYLISTSTORE_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), GW_TYPE_VOCABULARYLISTSTORE, GwVocabularyListStoreClass))
#define GW_IS_VOCABULARYLISTSTORE(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), GW_TYPE_VOCABULARYLISTSTORE))
#define GW_IS_VOCABULARYLISTSTORE_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GW_TYPE_VOCABULARYLISTSTORE))
#define GW_VOCABULARYLISTSTORE_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), GW_TYPE_VOCABULARYLISTSTORE, GwVocabularyListStoreClass))

struct _GwVocabularyListStore {
  GtkListStore store;
  GwVocabularyListStorePrivate *priv;
};

struct _GwVocabularyListStoreClass {
  GtkListStoreClass parent_class;
  guint signalid[TOTAL_GW_VOCABULARYLISTSTORE_CLASS_SIGNALIDS];
  void (*changed) (GwVocabularyListStore *store);
};

//Methods
GtkListStore* gw_vocabularyliststore_new (void);
GType gw_vocabularyliststore_get_type (void) G_GNUC_CONST;

void gw_vocabularyliststore_save (GwVocabularyListStore*, GtkTreeIter*);
void gw_vocabularyliststore_save_all (GwVocabularyListStore*);

void gw_vocabularyliststore_revert (GwVocabularyListStore*, GtkTreeIter*);
void gw_vocabularyliststore_revert_all (GwVocabularyListStore*);

void gw_vocabularyliststore_save_list_order (GwVocabularyListStore*, LwPreferences*);
void gw_vocabularyliststore_load_list_order (GwVocabularyListStore*, LwPreferences*);

GtkListStore* gw_vocabularyliststore_get_wordstore_by_iter (GwVocabularyListStore*, GtkTreeIter*);
GtkListStore* gw_vocabularyliststore_get_wordstore_by_index (GwVocabularyListStore*, gint);
GtkListStore* gw_vocabularyliststore_get_wordstore_by_name (GwVocabularyListStore*, const gchar*);
gchar* gw_vocabularyliststore_get_name_by_iter (GwVocabularyListStore*, GtkTreeIter*);

void gw_vocabularyliststore_remove_path_list (GwVocabularyListStore*, GList*);
void gw_vocabularyliststore_new_list (GwVocabularyListStore*, GtkTreeIter*);
gboolean gw_vocabularyliststore_list_exists (GwVocabularyListStore*, const gchar*);
gboolean gw_vocabularyliststore_has_changes (GwVocabularyListStore*);
void gw_vocabularyliststore_set_has_changes (GwVocabularyListStore*, gboolean);
GMenuModel* gw_vocabularyliststore_get_menumodel (GwVocabularyListStore*);

void gw_vocabularyliststore_load (GwVocabularyListStore*);

G_END_DECLS

#endif
