#ifndef GW_CONSOLE_INCLUDED
#define GW_CONSOLE_INCLUDED


void w_console_append_edict_results_to_buffer (LwSearchItem*);
void w_console_append_kanjidict_results_to_buffer (LwSearchItem*);
void w_console_append_examplesdict_results_to_buffer (LwSearchItem*);
void w_console_append_unknowndict_results_to_buffer (LwSearchItem*);
void w_console_update_progress_feedback (LwSearchItem*);
void w_console_no_result(LwSearchItem*);
void w_console_append_less_relevant_header_to_output (LwSearchItem*);
void w_console_append_more_relevant_header_to_output (LwSearchItem*);
void w_console_pre_search_prep (LwSearchItem*);
void w_console_after_search_cleanup (LwSearchItem*);

int w_console_install_progress_cb (double, gpointer);
int w_console_uninstall_progress_cb (double, gpointer);

void w_console_about (void);
void w_console_list (void);
void w_console_start_banner (char *, char *);
void w_console_print_available_dictionaries (void);
void w_console_print_installable_dictionaries (void);

gboolean w_console_install_dictinst (const char*, GError**);
gboolean w_console_uninstall_dictinfo (const char*, GError**);

void w_console_handle_error (GError**);
gboolean w_console_search (char*, char*, gboolean, gboolean, GError**);

#endif
