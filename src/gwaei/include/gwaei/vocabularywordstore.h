#ifndef GW_VOCABULARYWORDSTORE_INCLUDED
#define GW_VOCABULARYWORDSTORE_INCLUDED

G_BEGIN_DECLS

typedef enum {
  GW_VOCABULARYWORDSTORE_CLASS_SIGNALID_CHANGED,
  TOTAL_GW_VOCABULARYWORDSTORE_CLASS_SIGNALIDS
} GwVocabularyWordStoreClassSignalId;

typedef enum { 
  GW_VOCABULARYWORDSTORE_COLUMN_POSITION_INTEGER,
  GW_VOCABULARYWORDSTORE_COLUMN_POSITION_STRING,
  GW_VOCABULARYWORDSTORE_COLUMN_KANJI,
  GW_VOCABULARYWORDSTORE_COLUMN_FURIGANA,
  GW_VOCABULARYWORDSTORE_COLUMN_DEFINITIONS,
  GW_VOCABULARYWORDSTORE_COLUMN_CORRECT_GUESSES,
  GW_VOCABULARYWORDSTORE_COLUMN_INCORRECT_GUESSES,
  GW_VOCABULARYWORDSTORE_COLUMN_SCORE,
  GW_VOCABULARYWORDSTORE_COLUMN_TIMESTAMP,
  GW_VOCABULARYWORDSTORE_COLUMN_DAYS,
  GW_VOCABULARYWORDSTORE_COLUMN_WEIGHT,
  TOTAL_GW_VOCABULARYWORDSTORE_COLUMNS
} GwVocabularyWordStoreColumn;

//Boilerplate
typedef struct _GwVocabularyWordStore GwVocabularyWordStore;
typedef struct _GwVocabularyWordStoreClass GwVocabularyWordStoreClass;
typedef struct _GwVocabularyWordStorePrivate GwVocabularyWordStorePrivate;

#define GW_TYPE_VOCABULARYWORDSTORE              (gw_vocabularywordstore_get_type())
#define GW_VOCABULARYWORDSTORE(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GW_TYPE_VOCABULARYWORDSTORE, GwVocabularyWordStore))
#define GW_VOCABULARYWORDSTORE_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), GW_TYPE_VOCABULARYWORDSTORE, GwVocabularyWordStoreClass))
#define GW_IS_VOCABULARYWORDSTORE(obj)                    (G_TYPE_CHECK_INSTANCE_TYPE((obj), GW_TYPE_VOCABULARYWORDSTORE))
#define GW_IS_VOCABULARYWORDSTORE_CLASS(klass)            (G_TYPE_CHECK_CLASS_TYPE ((klass), GW_TYPE_VOCABULARYWORDSTORE))
#define GW_VOCABULARYWORDSTORE_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), GW_TYPE_VOCABULARYWORDSTORE, GwVocabularyWordStoreClass))

struct _GwVocabularyWordStore {
  GtkListStore model;
  GwVocabularyWordStorePrivate *priv;
};

struct _GwVocabularyWordStoreClass {
  GtkListStoreClass parent_class;
  guint signalid[TOTAL_GW_VOCABULARYWORDSTORE_CLASS_SIGNALIDS];
  void (*changed) (GwVocabularyWordStore *store);
};

//Methods
GtkListStore* gw_vocabularywordstore_new (const gchar*);
GType gw_vocabularywordstore_get_type (void) G_GNUC_CONST;

void gw_vocabularywordstore_save (GwVocabularyWordStore*, const gchar*);
void gw_vocabularywordstore_load (GwVocabularyWordStore*, const gchar*);
gboolean gw_vocabularywordstore_loaded (GwVocabularyWordStore*);
const gchar* gw_vocabularywordstore_get_name (GwVocabularyWordStore*);
gchar* gw_vocabularywordstore_get_filename (GwVocabularyWordStore*);
gboolean gw_vocabularywordstore_file_exists (GwVocabularyWordStore*);
void gw_vocabularywordstore_reset (GwVocabularyWordStore*);
gboolean gw_vocabularywordstore_has_changes (GwVocabularyWordStore*);
void gw_vocabularywordstore_set_has_changes (GwVocabularyWordStore*, gboolean);
void gw_vocabularywordstore_set_name (GwVocabularyWordStore*, const gchar*);

gchar* gw_vocabularywordstore_path_list_to_string (GwVocabularyWordStore*, GList*);
void gw_vocabularywordstore_remove_path_list (GwVocabularyWordStore*, GList*);
void gw_vocabularywordstore_append_text (GwVocabularyWordStore*, GtkTreeIter*, gboolean, const gchar*);
void gw_vocabularywordstore_new_word (GwVocabularyWordStore*, GtkTreeIter*, GtkTreeIter*, const gchar*, const gchar*, const gchar*);
void gw_vocabularywordstore_set_string (GwVocabularyWordStore*, GtkTreeIter*, gint, const gchar*);
gchar* gw_vocabularywordstore_iter_to_string (GwVocabularyWordStore*, GtkTreeIter*);

void gw_vocabularywordstore_update_timestamp_by_iter (GwVocabularyWordStore*, GtkTreeIter*);

gint gw_vocabularywordstore_get_correct_guesses_by_iter (GwVocabularyWordStore*, GtkTreeIter*);
void gw_vocabularywordstore_set_correct_guesses_by_iter (GwVocabularyWordStore*, GtkTreeIter*, gint);

gint gw_vocabularywordstore_get_incorrect_guesses_by_iter (GwVocabularyWordStore*, GtkTreeIter*);
void gw_vocabularywordstore_set_incorrect_guesses_by_iter (GwVocabularyWordStore*, GtkTreeIter*, gint);

gint gw_vocabularywordstore_calculate_weight (GwVocabularyWordStore*, GtkTreeIter*);

G_END_DECLS

#endif
