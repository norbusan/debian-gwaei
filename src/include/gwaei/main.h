#ifndef GW_MAIN_INCLUDED
#define GW_MAIN_INCLUDED

#include <gwaei/searchitem-object.h>

void (*gw_output_generic_append_edict_results)(GwSearchItem*);
void (*gw_output_generic_append_kanjidict_results)(GwSearchItem*);
void (*gw_output_generic_append_examplesdict_results)(GwSearchItem*);
void (*gw_output_generic_append_unknowndict_results)(GwSearchItem*);

void (*gw_output_generic_update_progress_feedback)(GwSearchItem*);
void (*gw_output_generic_append_less_relevant_header_to_output)(GwSearchItem*);
void (*gw_output_generic_append_more_relevant_header_to_output)(GwSearchItem*);
void (*gw_output_generic_pre_search_prep)(GwSearchItem*);
void (*gw_output_generic_after_search_cleanup)(GwSearchItem*);

gboolean gw_main_verify_output_generic_functions (void);
void gw_main_initialize_generic_output_functions_to_null (void);

#endif
