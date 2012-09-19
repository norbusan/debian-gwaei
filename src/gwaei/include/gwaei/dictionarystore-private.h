#ifndef GW_DICTIONARYSTORE_PRIVATE_INCLUDED
#define GW_DICTIONARYSTORE_PRIVATE_INCLUDED

G_BEGIN_DECLS

struct _GwDictionaryStorePrivate {
  LwDictInfoList *dictinfolist;
  gulong list_update_handler_id;
  guint signalids[TOTAL_GW_DICTIONARYSTORE_SIGNALIDS];
};

#define GW_DICTIONARYSTORE_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GW_TYPE_DICTIONARYSTORE, GwDictionaryStorePrivate))

G_END_DECLS

#endif
