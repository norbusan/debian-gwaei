#ifndef LW_DICTIONARYLIST_INCLUDED
#define LW_DICTIONARYLIST_INCLUDED

#include <libwaei/dictionary.h>

G_BEGIN_DECLS

typedef enum {
  LW_DICTIONARYLIST_CLASS_SIGNALID_CHANGED,
  LW_DICTIONARYLIST_CLASS_SIGNALID_ADDED,
  LW_DICTIONARYLIST_CLASS_SIGNALID_REMOVED,
  TOTAL_LW_DICTIONARYLIST_CLASS_SIGNALIDS
} LwDictionaryListClassSignalId;


//Boilerplate
typedef struct _LwDictionaryList LwDictionaryList;
typedef struct _LwDictionaryListClass LwDictionaryListClass;
typedef struct _LwDictionaryListPrivate LwDictionaryListPrivate;

#define LW_TYPE_DICTIONARYLIST              (lw_dictionarylist_get_type())
#define LW_DICTIONARYLIST(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), LW_TYPE_DICTIONARYLIST, LwDictionaryList))
#define LW_DICTIONARYLIST_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), LW_TYPE_DICTIONARYLIST, LwDictionaryListClass))
#define LW_IS_DICTIONARYLIST(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), LW_TYPE_DICTIONARYLIST))
#define LW_IS_DICTIONARYLIST_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), LW_TYPE_DICTIONARYLIST))
#define LW_DICTIONARYLIST_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), LW_TYPE_DICTIONARYLIST, LwDictionaryListClass))


struct _LwDictionaryList {
  GObject object;
  LwDictionaryListPrivate *priv;
};

struct _LwDictionaryListClass {
  GObjectClass parent_class;
  guint signalid[TOTAL_LW_DICTIONARYLIST_CLASS_SIGNALIDS];

  //Signal ids
  void (*changed) (LwDictionaryList* dictionarylist, gpointer data);
  void (*added) (LwDictionaryList* dictionarylist, gpointer data);
  void (*removed) (LwDictionaryList* dictionarylist, gpointer data);
};


//Methods
LwDictionaryList* lw_dictionarylist_new (void);
GType lw_dictionarylist_get_type (void) G_GNUC_CONST;

void lw_dictionarylist_append (LwDictionaryList*, LwDictionary*);

LwDictionary* lw_dictionarylist_remove (LwDictionaryList*, LwDictionary*);
LwDictionary* lw_dictionarylist_remove_by_position (LwDictionaryList*, gint);

LwDictionary* lw_dictionarylist_get_dictionary (LwDictionaryList*, GType, const gchar*);
LwDictionary* lw_dictionarylist_get_dictionary_by_filename (LwDictionaryList*, const gchar*);
LwDictionary* lw_dictionarylist_get_dictionary_by_id (LwDictionaryList*, const gchar*);
LwDictionary* lw_dictionarylist_get_dictionary_fuzzy (LwDictionaryList*, const gchar*);
LwDictionary* lw_dictionarylist_get_dictionary_by_position (LwDictionaryList*, gint);

gint lw_dictionarylist_get_position (LwDictionaryList*, LwDictionary*);
gboolean lw_dictionarylist_dictionary_exists (LwDictionaryList*, LwDictionary*);

int lw_dictionarylist_get_total (LwDictionaryList*);

void lw_dictionarylist_load_order (LwDictionaryList*, LwPreferences*);
void lw_dictionarylist_save_order (LwDictionaryList*, LwPreferences*);

void lw_dictionarylist_load_installed (LwDictionaryList*);
void lw_dictionarylist_load_installable (LwDictionaryList*, LwPreferences*);
void lw_dictionarylist_clear (LwDictionaryList*);

gboolean lw_dictionarylist_installer_is_valid (LwDictionaryList*);

GList* lw_dictionarylist_get_list (LwDictionaryList*);
void lw_dictionarylist_sort_with_data (LwDictionaryList*, GCompareDataFunc, gpointer);

G_END_DECLS


#endif
