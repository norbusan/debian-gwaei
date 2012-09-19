#ifndef LW_DICTIONARY_INCLUDED
#define LW_DICTIONARY_INCLUDED

#include <stdio.h>
#include <libwaei/result.h>
#include <libwaei/query.h>

G_BEGIN_DECLS

typedef enum {
  LW_DICTIONARY_CLASS_SIGNALID_PROGRESS_CHANGED,
  TOTAL_LW_DICTIONARY_CLASS_SIGNALIDS
} LwDictionaryClassSignalId;

//Boilerplate
typedef struct _LwDictionary LwDictionary;
typedef struct _LwDictionaryClass LwDictionaryClass;
typedef struct _LwDictionaryPrivate LwDictionaryPrivate;
typedef struct _LwDictionaryInstall LwDictionaryInstall;

#define LW_TYPE_DICTIONARY              (lw_dictionary_get_type())
#define LW_DICTIONARY(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), LW_TYPE_DICTIONARY, LwDictionary))
#define LW_DICTIONARY_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), LW_TYPE_DICTIONARY, LwDictionaryClass))
#define LW_IS_DICTIONARY(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), LW_TYPE_DICTIONARY))
#define LW_IS_DICTIONARY_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), LW_TYPE_DICTIONARY))
#define LW_DICTIONARY_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), LW_TYPE_DICTIONARY, LwDictionaryClass))


struct _LwDictionary {
  GObject object;
  LwDictionaryPrivate *priv;
};

struct _LwDictionaryClass {
  GObjectClass parent_class;
  guint signalid[TOTAL_LW_DICTIONARY_CLASS_SIGNALIDS];

  //Signal ids
  void (*progress_changed) (LwDictionary* dictionary, gpointer data);

  //Virtual methods
  gboolean (*parse_query) (LwDictionary *dictionary, LwQuery *query, const gchar *TEXT, GError **error);
  gint (*parse_result) (LwDictionary *dictionary, LwResult *result, FILE *fd);
  gboolean (*compare) (LwDictionary *dictionary, LwQuery *query, LwResult *result, const LwRelevance relevance);
  gboolean (*installer_postprocess) (LwDictionary *dictionary, gchar** sourcelist, gchar** targetlist, LwIoProgressCallback cb, gpointer data, GCancellable *cancellable, GError **error);
  gchar ***patterns;  
};

//Methods
GType lw_dictionary_get_type (void) G_GNUC_CONST;
gboolean lw_dictionary_install (LwDictionary*, GCancellable*, GError**);
gboolean lw_dictionary_uninstall (LwDictionary*, LwIoProgressCallback, GError**);
gchar* lw_dictionary_get_directory (GType);
gchar* lw_dictionary_get_path (LwDictionary*);
gboolean lw_dictionary_compare (LwDictionary*, LwQuery*, LwResult*, const LwRelevance);

FILE* lw_dictionary_open (LwDictionary*);

const gchar* lw_dictionary_get_filename (LwDictionary*);
const gchar* lw_dictionary_get_name (LwDictionary*);

gboolean lw_dictionary_parse_query (LwDictionary*, LwQuery*, const gchar*, GError**);
gboolean lw_dictionary_parse_result (LwDictionary*, LwResult*, FILE*);
size_t lw_dictionary_get_length (LwDictionary*);
void lw_dictionary_cancel (LwDictionary*);

gboolean lw_dictionary_equals (LwDictionary*, LwDictionary*);
gchar* lw_dictionary_build_id (LwDictionary*);
gboolean lw_dictionary_is_selected (LwDictionary*);
void lw_dictionary_set_selected (LwDictionary*, gboolean);
gchar* lw_dictionary_directoryname_to_typename (const gchar*);
gchar* lw_dictionary_get_directoryname (GType);

gchar** lw_dictionary_get_installed_idlist (GType);

void lw_dictionary_build_regex (LwDictionary*, LwQuery*, GError**);

LwDictionaryInstall* lw_dictionary_steal_installer (LwDictionary*);
void lw_dictionary_set_installer (LwDictionary*, LwDictionaryInstall*);
void lw_dictionary_set_installer_full (LwDictionary*, const gchar*, const gchar*, const gchar*, LwEncoding, gboolean);
void lw_dictionary_set_builtin_installer_full (LwDictionary*, const gchar*, LwPreferences*, const gchar*, const gchar*, LwEncoding, gboolean);

G_END_DECLS

#include <libwaei/dictionary-installer.h>
#include <libwaei/dictionary-callbacks.h>

#endif
