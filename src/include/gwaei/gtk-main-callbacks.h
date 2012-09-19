#ifndef GW_GTK_MAIN_CALLBACKS_INCLUDED
#define GW_GTK_MAIN_CALLBACKS_INCLUDED


void do_back (GtkWidget *widget, gpointer data);
void do_forward (GtkWidget *widget, gpointer data);
void do_save (GtkWidget *widget, gpointer data);
void do_print (GtkWidget *widget, gpointer data);
void do_zoom_in (GtkWidget *widget, gpointer data);
void do_zoom_out (GtkWidget *widget, gpointer data);
void do_zoom_100 (GtkWidget *widget, gpointer data);
void do_toolbar_toggle (GtkWidget *widget, gpointer data);
void do_less_relevant_results_toggle(GtkWidget *widget, gpointer data);
void do_select_all (GtkWidget *widget, gpointer data);
void do_paste (GtkWidget *widget, gpointer data);
void do_cut (GtkWidget *widget, gpointer data);
void do_copy (GtkWidget *widget, gpointer data);
void do_about(GtkWidget *widget, gpointer data);
void do_settings (GtkWidget *widget, gpointer data);
void do_destroy(GtkObject*, gpointer);
void do_search (GtkWidget *widget, gpointer data);
void do_search_from_history (GtkWidget*, gpointer);
void do_clear_search(GtkWidget*, gpointer);
void do_update_button_states_based_on_entry_text (GtkWidget*, gpointer);
void do_go_menuitem_action (GtkWidget*, gpointer);
void do_close_kanji_results(GtkWidget*, gpointer);
void do_dictionary_changed_action (GtkWidget*, gpointer);
void do_cycle_dictionaries_forward (GtkWidget*, gpointer);
void do_cycle_dictionaries_backward (GtkWidget*, gpointer);
void search_drag_data_recieved (GtkWidget*, GdkDragContext*,
                                gint, gint,
                                GtkSelectionData*, guint,
                                guint, gpointer             );

gboolean do_update_clipboard_on_focus_change (GtkWidget*, GtkDirectionType, gpointer);
gboolean do_focus_change_on_key_press (GtkWidget*, GdkEvent*, gpointer*);
gboolean do_history_change_on_key_press ( GtkWidget*, GdkEvent*, gpointer*);
gboolean do_switch_dictionaries_quickkey_action ( GtkWidget*, GdkEvent*, gpointer*);

gboolean do_key_press_action (GtkWidget*, GdkEvent*, gpointer*);
gboolean do_close_on_escape (GtkWidget*, GdkEvent*, gpointer*);

gboolean do_drag_motion_1 (GtkWidget*, GdkDragContext*, gint, gint, guint, gpointer);
gboolean do_update_clipboard_on_focus_change (GtkWidget*, GtkDirectionType, gpointer);
gboolean do_get_position_for_button_press (GtkWidget*, GdkEventButton*, gpointer);
gboolean do_get_iter_for_motion (GtkWidget*, GdkEventButton*, gpointer);
gboolean do_drag_drop_1 (GtkWidget*, GdkDragContext*, gint, gint, guint, gpointer);
gboolean do_get_iter_for_button_release (GtkWidget*, GdkEventButton*, gpointer);
void do_drag_leave_1 (GtkWidget*, GdkDragContext*, guint, gpointer);
void do_search_drag_data_recieved (GtkWidget*, GdkDragContext*, gint, gint , GtkSelectionData*, guint, guint, gpointer);
gboolean do_focus_change_on_key_press (GtkWidget*, GdkEvent*, gpointer*);
gboolean do_update_icons_for_selection (GtkWidget*, GdkEvent*, gpointer); 

void do_populate_popup_with_search_options (GtkTextView*, GtkMenu*, gpointer);
void do_search_for_searchitem_online (GtkWidget*, gpointer);

gboolean do_scroll_or_zoom (GtkWidget*, GdkEventScroll*, gpointer);

#endif

