#ifndef GW_DICTIONARYLIST_PRIVATE_INCLUDED
#define GW_DICTIONARYLIST_PRIVATE_INCLUDED

G_BEGIN_DECLS

struct _GwDictionaryListPrivate {
  GtkListStore *liststore;
  GMenuModel *menumodel;
  
//  guint signalids[TOTAL_GW_DICTIONARYLIST_SIGNALIDS];
};

#define GW_DICTIONARYLIST_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GW_TYPE_DICTIONARYLIST, GwDictionaryListPrivate))

G_END_DECLS

#endif
