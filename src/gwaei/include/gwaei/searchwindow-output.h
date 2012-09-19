#ifndef GW_SEARCHWINDOW_OUTPUT_INCLUDED
#define GW_SEARCHWINDOW_OUTPUT_INCLUDED

void gw_searchwindow_append_result (GwSearchWindow*, LwSearchItem*);
void gw_searchwindow_append_kanjidict_tooltip_result (GwSearchWindow*, LwSearchItem*);
void gw_searchwindow_display_no_results_found_page (GwSearchWindow*, LwSearchItem*);

#endif
