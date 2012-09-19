#ifndef LW_HISTORY_INCLUDED
#define LW_HISTORY_INCLUDED 

#include <libwaei/searchitem.h>

G_BEGIN_DECLS

#define LW_HISTORYLIST(object) (LwHistory*) object

//!
//! @brief Primitive for storing search items in intelligent ways
//!
struct _LwHistory {
    GList *back;           //!< A GList of past search items
    GList *forward;        //!< A GList where past search items get stacked when the user goes back.
    int max;
};
typedef struct _LwHistory LwHistory;

LwHistory* lw_history_new (const int);
void lw_history_free (LwHistory*);
void lw_history_init (LwHistory*, const int);
void lw_history_deinit (LwHistory*);

//Methods
GList* lw_history_get_back_list (LwHistory*);
GList* lw_history_get_forward_list (LwHistory*);
GList* lw_history_get_combined_list (LwHistory*);
void lw_history_clear_forward_list (LwHistory*);
void lw_history_clear_back_list (LwHistory*);

void lw_history_add_searchitem (LwHistory*, LwSearchItem*);

gboolean lw_history_has_back (LwHistory*);
gboolean lw_history_has_forward (LwHistory*);
LwSearchItem* lw_history_go_back (LwHistory*, LwSearchItem*);
LwSearchItem* lw_history_go_forward (LwHistory*, LwSearchItem*);

G_END_DECLS

#endif
