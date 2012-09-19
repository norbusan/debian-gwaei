#ifndef GW_HISTORY_INCLUDED
#define GW_HISTORY_INCLUDED 

//Boilerplate
typedef struct _GwHistory GwHistory;
typedef struct _GwHistoryClass GwHistoryClass;
typedef struct _GwHistoryPrivate GwHistoryPrivate;

#define GW_TYPE_HISTORY              (gw_history_get_type())
#define GW_HISTORY(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GW_TYPE_HISTORY, GwHistory))
#define GW_HISTORY_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), GW_TYPE_HISTORY, GwHistoryClass))
#define GW_IS_HISTORY(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), GW_TYPE_HISTORY))
#define GW_IS_HISTORY_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GW_TYPE_HISTORY))
#define GW_HISTORY_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), GW_TYPE_HISTORY, GwHistoryClass))


struct _GwHistory {
  LwHistory history;
  GwHistoryPrivate *priv;
};

struct _GwHistoryClass {
  LwHistoryClass parent_class;
};

//Methods
GType gw_history_get_type (void) G_GNUC_CONST;
GwHistory* gw_history_new (const gint);

void gw_history_sync_menumodels (GwHistory*);

GMenuModel* gw_history_get_back_menumodel (GwHistory*);
GMenuModel* gw_history_get_forward_menumodel (GwHistory*);
GMenuModel* gw_history_get_combined_menumodel (GwHistory*);


#endif

