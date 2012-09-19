#ifndef LW_HISTORY_INCLUDED
#define LW_HISTORY_INCLUDED 

#include <libwaei/search.h>

G_BEGIN_DECLS

typedef enum {
  LW_HISTORY_CLASS_SIGNALID_CHANGED,
  LW_HISTORY_CLASS_SIGNALID_BACK,
  LW_HISTORY_CLASS_SIGNALID_FORWARD,
  LW_HISTORY_CLASS_SIGNALID_ADDED,
  TOTAL_LW_HISTORY_CLASS_SIGNALIDS
} LwHistoryClassSignalId;


//Boilerplate
typedef struct _LwHistory LwHistory;
typedef struct _LwHistoryClass LwHistoryClass;
typedef struct _LwHistoryPrivate LwHistoryPrivate;

#define LW_TYPE_HISTORY              (lw_history_get_type())
#define LW_HISTORY(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), LW_TYPE_HISTORY, LwHistory))
#define LW_HISTORY_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), LW_TYPE_HISTORY, LwHistoryClass))
#define LW_IS_HISTORY(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), LW_TYPE_HISTORY))
#define LW_IS_HISTORY_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), LW_TYPE_HISTORY))
#define LW_HISTORY_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), LW_TYPE_HISTORY, LwHistoryClass))


struct _LwHistory {
  GObject object;
  LwHistoryPrivate *priv;
};

struct _LwHistoryClass {
  GObjectClass parent_class;
  guint signalid[TOTAL_LW_HISTORY_CLASS_SIGNALIDS];

  //Signal ids
  void (*changed) (LwHistory* history, gpointer data);
  void (*back) (LwHistory* history, gpointer data);
  void (*forward) (LwHistory* history, gpointer data);
  void (*added) (LwHistory* history, gpointer data);
};


//Methods
GType lw_history_get_type (void) G_GNUC_CONST;
LwHistory* lw_history_new (const gint);

GList* lw_history_get_back_list (LwHistory*);
GList* lw_history_get_forward_list (LwHistory*);
GList* lw_history_get_combined_list (LwHistory*);
void lw_history_clear_forward_list (LwHistory*);
void lw_history_clear_back_list (LwHistory*);

void lw_history_add_search (LwHistory*, LwSearch*);

gboolean lw_history_has_back (LwHistory*);
gboolean lw_history_has_forward (LwHistory*);
LwSearch* lw_history_go_back (LwHistory*, LwSearch*);
LwSearch* lw_history_go_forward (LwHistory*, LwSearch*);

gboolean lw_history_has_relevance (LwHistory*, LwSearch*, gboolean);

G_END_DECLS

#endif
