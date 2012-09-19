#ifndef GW_NCURSES_INCLUDED
#define GW_NCURSES_INCLUDED


typedef enum {
  GW_NCCOLORS_GREENONBLACK,
  GW_NCCOLORS_BLUEONBLACK,
  GW_NCCOLORS_REDONBLACK,
  GW_NCCOLORS_TOTAL
} GwNcursesColorPair;


void initialize_ncurses_interface (int, char**);
void w_ncurses_initialize_interface_output_generics (void);
void w_ncurses_append_edict_results_to_buffer (LwSearchItem*);
void w_ncurses_append_kanjidict_results_to_buffer (LwSearchItem*);
void w_ncurses_append_examplesdict_results_to_buffer (LwSearchItem*);
void w_ncurses_append_unknowndict_results_to_buffer (LwSearchItem*);

void w_ncurses_update_progress_feedback (LwSearchItem*);
void w_ncurses_no_result(LwSearchItem*);
void w_ncurses_append_less_relevant_header_to_output (LwSearchItem*);
void w_ncurses_append_more_relevant_header_to_output (LwSearchItem*);
void w_ncurses_pre_search_prep (LwSearchItem*);
void w_ncurses_after_search_cleanup (LwSearchItem*);

void w_ncurses_start ();

#endif
