#ifndef GW_ENGINE_INCLUDED
#define GW_ENGINE_INCLUDED

//Starts a search.  Make sure to set target_text_buffer and target_text view

#define MAX_HIGH_RELIVENT_RESULTS 200
#define MAX_MEDIUM_IRRELIVENT_RESULTS 100
#define MAX_LOW_IRRELIVENT_RESULTS    50

typedef void(*LwOutputFunc)(LwSearchItem*);

void lw_engine_initialize (
                           LwOutputFunc,
                           LwOutputFunc,
                           LwOutputFunc,
                           LwOutputFunc,
                           LwOutputFunc,
                           LwOutputFunc,
                           LwOutputFunc,
                           LwOutputFunc
                          );
void lw_engine_free (void);

void lw_engine_get_results (LwSearchItem*, gboolean, gboolean);

LwOutputFunc lw_engine_get_append_edict_results_func (void);
LwOutputFunc lw_engine_get_append_kanjidict_results_func (void);
LwOutputFunc lw_engine_get_append_examplesdict_results_func (void);
LwOutputFunc lw_engine_get_append_unknowndict_results_func (void);
LwOutputFunc lw_engine_get_append_less_relevant_header_func (void);
LwOutputFunc lw_engine_get_append_more_relevant_header_func (void);
LwOutputFunc lw_engine_get_pre_search_prep_func (void);
LwOutputFunc lw_engine_get_after_search_cleanup_func (void);

#endif
