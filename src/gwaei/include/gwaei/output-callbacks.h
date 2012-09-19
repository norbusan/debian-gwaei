#ifndef GW_OUTPUT_CALLBACKS_INCLUDED
#define GW_OUTPUT_CALLBACKS_INCLUDED

void gw_output_append_edict_results_cb (LwSearchItem*);
void gw_output_append_kanjidict_results_cb (LwSearchItem*);
void gw_output_append_examplesdict_results_cb (LwSearchItem*);
void gw_output_append_unknowndict_results_cb (LwSearchItem*);
void gw_output_append_less_relevant_header_cb (LwSearchItem*);
void gw_output_append_more_relevant_header_cb (LwSearchItem*);
void gw_output_pre_search_prep_cb (LwSearchItem*);
void gw_output_after_search_cleanup_cb (LwSearchItem*);

#endif
