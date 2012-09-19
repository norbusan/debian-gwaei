#ifndef GW_FLASHCARDSTORE_PRIVATE_INCLUDED
#define GW_FLASHCARDSTORE_PRIVATE_INCLUDED

#include "vocabularywordstore.h"

G_BEGIN_DECLS

struct _GwFlashCardStorePrivate {
  GwVocabularyWordStore *store;
};

#define GW_FLASHCARDSTORE_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GW_TYPE_FLASHCARDSTORE, GwFlashCardStorePrivate))

G_END_DECLS

#endif

