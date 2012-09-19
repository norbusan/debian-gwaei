#ifndef LW_VOCABULARY_INCLUDED
#define LW_VOCABULARY_INCLUDED

#include <libwaei/word.h>

G_BEGIN_DECLS

struct _LwVocabulary {
  gchar *name;
  GList *items;
  gboolean changed;
  gdouble progress;
};

typedef struct _LwVocabulary LwVocabulary;

#define LW_VOCABULARY(obj) (LwVocabulary*)obj

gchar** lw_vocabulary_get_lists (void);
LwVocabulary* lw_vocabulary_new (const gchar*);
void lw_vocabulary_free (LwVocabulary*);

void lw_vocabulary_save (LwVocabulary*, const gchar*, LwIoProgressCallback);
void lw_vocabulary_load (LwVocabulary*, const gchar*, LwIoProgressCallback);

G_END_DECLS

#endif
