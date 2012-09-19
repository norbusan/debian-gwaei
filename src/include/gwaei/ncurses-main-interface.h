#ifndef GW_NCURSES_MAIN_INTERFACE_INCLUDED
#define GW_NCURSES_MAIN_INTERFACE_INCLUDED


typedef enum {
  GW_NCCOLORS_GREENONBLACK,
  GW_NCCOLORS_BLUEONBLACK,
  GW_NCCOLORS_REDONBLACK,
  GW_NCCOLORS_TOTAL
} GwNcursesColorPair;


void initialize_ncurses_interface (int, char**);
void gw_ncurses_initialize_interface_output_generics (void);
void gw_ncurses_append_edict_results (GwSearchItem*);
void gw_ncurses_append_kanjidict_results (GwSearchItem*);
void gw_ncurses_append_examplesdict_results (GwSearchItem*);
void gw_ncurses_append_unknowndict_results (GwSearchItem*);
void gw_ncurses_update_progress_feedback (GwSearchItem*);
void gw_ncurses_no_result(GwSearchItem*);
void gw_ncurses_append_less_relevant_header_to_output (GwSearchItem*);
void gw_ncurses_append_more_relevant_header_to_output (GwSearchItem*);
void gw_ncurses_pre_search_prep (GwSearchItem*);
void gw_ncurses_after_search_cleanup (GwSearchItem*);

#endif
