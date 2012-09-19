#ifndef LW_DICTIONARYLIST_PRIVATE_INCLUDED
#define LW_DICTIONARYLIST_PRIVATE_INCLUDED

G_BEGIN_DECLS

struct _LwDictionaryListPrivate {
  GList *list;
  GMutex mutex;
};

#define LW_DICTIONARYLIST_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), LW_TYPE_DICTIONARYLIST, LwDictionaryListPrivate));

G_END_DECLS

#endif


