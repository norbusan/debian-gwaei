#ifndef GW_CONSOLE_MAIN_INTERFACE_INCLUDED
#define GW_CONSOLE_MAIN_INTERFACE_INCLUDED

void initialize_console_interface (int, char**);

void gw_console_initialize_interface_output_generics (void);

void gw_console_append_edict_results (GwSearchItem*);
void gw_console_append_kanjidict_results (GwSearchItem*);
void gw_console_append_examplesdict_results (GwSearchItem*);
void gw_console_append_unknowndict_results (GwSearchItem*);
void gw_console_update_progress_feedback (GwSearchItem*);
void gw_console_no_result(GwSearchItem*);
void gw_console_append_less_relevant_header_to_output (GwSearchItem*);
void gw_console_append_more_relevant_header_to_output (GwSearchItem*);
void gw_console_pre_search_prep (GwSearchItem*);
void gw_console_after_search_cleanup (GwSearchItem*);

#endif
