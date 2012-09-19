#ifndef GW_VOCABULARYLISTSTORE_PRIVATE_INCLUDED
#define GW_VOCABULARYLISTSTORE_PRIVATE_INCLUDED

G_BEGIN_DECLS

struct _GwVocabularyListStorePrivate {
  GMenuModel *menumodel;
  gboolean has_changes;
  gint list_new_index;
};

#define GW_VOCABULARYLISTSTORE_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GW_TYPE_VOCABULARYLISTSTORE, GwVocabularyListStorePrivate))

G_END_DECLS

#endif

