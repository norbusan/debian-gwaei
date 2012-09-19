#ifndef GW_FLASHCARDSTORE_INCLUDED
#define GW_FLASHCARDSTORE_INCLUDED


#include "vocabularywordstore.h"

G_BEGIN_DECLS

typedef enum {
  GW_FLASHCARDSTORE_COLUMN_QUESTION,
  GW_FLASHCARDSTORE_COLUMN_ANSWER,
  GW_FLASHCARDSTORE_COLUMN_IS_COMPLETED,
  GW_FLASHCARDSTORE_COLUMN_TREE_PATH,
  GW_FLASHCARDSTORE_COLUMN_WEIGHT,
  GW_FLASHCARDSTORE_COLUMN_ORDER,
  GW_FLASHCARDSTORE_COLUMN_CORRECT_GUESSES,
  GW_FLASHCARDSTORE_COLUMN_INCORRECT_GUESSES,
  TOTAL_GW_FLASHCARDSTORE_COLUMNS
} GwFlashCardStoreColumn;

//Boilerplate
typedef struct _GwFlashCardStore GwFlashCardStore;
typedef struct _GwFlashCardStoreClass GwFlashCardStoreClass;
typedef struct _GwFlashCardStorePrivate GwFlashCardStorePrivate;

#define GW_TYPE_FLASHCARDSTORE              (gw_flashcardstore_get_type())
#define GW_FLASHCARDSTORE(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GW_TYPE_FLASHCARDSTORE, GwFlashCardStore))
#define GW_FLASHCARDSTORE_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), GW_TYPE_FLASHCARDSTORE, GwFlashCardStoreClass))
#define GW_IS_FLASHCARDSTORE(obj)                    (G_TYPE_CHECK_INSTANCE_TYPE((obj), GW_TYPE_FLASHCARDSTORE))
#define GW_IS_FLASHCARDSTORE_CLASS(klass)            (G_TYPE_CHECK_CLASS_TYPE ((klass), GW_TYPE_FLASHCARDSTORE))
#define GW_FLASHCARDSTORE_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), GW_TYPE_FLASHCARDSTORE, GwFlashCardStoreClass))

struct _GwFlashCardStore {
  GtkListStore model;
  GwFlashCardStorePrivate *priv;
};

struct _GwFlashCardStoreClass {
  GtkListStoreClass parent_class;
  //guint signalid[TOTAL_GW_FLASHCARDSTORE_CLASS_SIGNALIDS];
  //void (*changed) (GwFlashCardStore *store);
};

//Methods
GtkListStore* gw_flashcardstore_new (void);
GType gw_flashcardstore_get_type (void) G_GNUC_CONST;

void gw_flashcardstore_set_vocabularywordstore (GwFlashCardStore*, GwVocabularyWordStore*, gint, gint);
void gw_flashcardstore_trim (GwFlashCardStore*, gint);
void gw_flashcardstore_shuffle (GwFlashCardStore*);

void gw_flashcardstore_set_correct_guesses (GwFlashCardStore*, GtkTreeIter*, gint, gboolean);
gint gw_flashcardstore_get_correct_guesses (GwFlashCardStore*, GtkTreeIter*);

void gw_flashcardstore_set_incorrect_guesses (GwFlashCardStore*, GtkTreeIter*, gint, gboolean);
gint gw_flashcardstore_get_incorrect_guesses (GwFlashCardStore*, GtkTreeIter*);

void gw_flashcardstore_set_completed (GwFlashCardStore*, GtkTreeIter*, gboolean);
gboolean gw_flashcardstore_is_completed (GwFlashCardStore*, GtkTreeIter*);


G_END_DECLS

#endif
