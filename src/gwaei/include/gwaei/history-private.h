#ifndef GW_HISTORY_PRIVATE_INCLUDED
#define GW_HISTORY_PRIVATE_INCLUDED

G_BEGIN_DECLS

struct _GwHistoryPrivate {
  GMenuModel *forward;
  GMenuModel *back;
  GMenuModel *combined;
};

#define GW_HISTORY_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GW_TYPE_HISTORY, GwHistoryPrivate));

G_END_DECLS

#endif
