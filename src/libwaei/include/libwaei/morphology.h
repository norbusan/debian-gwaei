#ifndef LW_MORPHOLOGY_INCLUDED
#define LW_MORPHOLOGY_INCLUDED 

#include <mecab.h>

G_BEGIN_DECLS

struct _LwMorphologyEngine {
  GMutex mutex;
  mecab_t *mecab;
};
typedef struct _LwMorphologyEngine LwMorphologyEngine;
#define LW_MORPHOLOGYENGINE(obj) (LwMorphologyEngine*)obj

LwMorphologyEngine* lw_morphologyengine_new (void);
LwMorphologyEngine* lw_morphologyengine_get_default (void);
void lw_morphologyengine_free (LwMorphologyEngine*);

gboolean lw_morphologyengine_has_default (void);


//!
//! @brief Morphological analysis of input
//!
struct _LwMorphology {
  gchar *word;           //!< Original word
  gchar *base_form;      //!< Deduced (most likely) dictionary form of the word. NULL if no result.
  gchar *explanation;    //!< Free-form explanation of the morphological analysis. NULL if none.
};
typedef struct _LwMorphology LwMorphology;
#define LW_MORPHOLOGY(obj) (LwMorphology*)obj

GList* lw_morphologyengine_analyze (LwMorphologyEngine *engine, const gchar*);

void lw_morphologylist_free (GList*);

G_END_DECLS

#endif
