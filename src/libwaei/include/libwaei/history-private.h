#ifndef LW_HISTORY_PRIVATE_INCLUDED
#define LW_HISTORY_PRIVATE_INCLUDED

G_BEGIN_DECLS

struct _LwHistoryPrivate {
    GList *back;           //!< A GList of past search items
    GList *forward;        //!< A GList where past search items get stacked when the user goes back.
    gint max;
    gint time_delta;
};

#define LW_HISTORY_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), LW_TYPE_HISTORY, LwHistoryPrivate));

G_END_DECLS

#endif
