#ifndef GW_VOCABULARYWORDSTORE_PRIVATE_INCLUDED
#define GW_VOCABULARYWORDSTORE_PRIVATE_INCLUDED

G_BEGIN_DECLS

struct _GwVocabularyWordStorePrivate {
  gchar* name;
  gchar* filename;
  LwVocabularyList *vocabulary_list;
  gboolean has_changes;
  gboolean loaded;
};

#define GW_VOCABULARYWORDSTORE_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GW_TYPE_VOCABULARYWORDSTORE, GwVocabularyWordStorePrivate))

G_END_DECLS

#endif

