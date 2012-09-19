#ifndef LW_ENGINE_INCLUDED
#define LW_ENGINE_INCLUDED

G_BEGIN_DECLS

#define LW_MAX_HIGH_RELEVENT_RESULTS 500
#define LW_MAX_MEDIUM_IRRELEVENT_RESULTS 500
#define LW_MAX_LOW_IRRELEVENT_RESULTS    500

void lw_searchitem_start_search (LwSearchItem*, gboolean, gboolean);

G_END_DECLS

#endif
