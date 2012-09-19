#ifndef GW_GTK_MAIN_CALLBACKS_INCLUDED
#define GW_GTK_MAIN_CALLBACKS_INCLUDED


void gw_main_back_cb (GtkWidget *widget, gpointer data);
void gw_main_forward_cb (GtkWidget *widget, gpointer data);
void gw_main_save_cb (GtkWidget *widget, gpointer data);
void gw_main_print_cb (GtkWidget *widget, gpointer data);
void gw_main_zoom_in_cb (GtkWidget *widget, gpointer data);
void gw_main_zoom_out_cb (GtkWidget *widget, gpointer data);
void gw_main_zoom_100_cb (GtkWidget *widget, gpointer data);
void gw_main_toolbar_toggle_cb (GtkWidget *widget, gpointer data);
void gw_main_less_relevant_results_toggle_cb(GtkWidget *widget, gpointer data);
void gw_main_select_all_cb (GtkWidget *widget, gpointer data);
void gw_main_paste_cb (GtkWidget *widget, gpointer data);
void gw_main_cut_cb (GtkWidget *widget, gpointer data);
void gw_main_copy_cb (GtkWidget *widget, gpointer data);
void gw_main_about_cb (GtkWidget *widget, gpointer data);
void gw_main_destroy_cb (GObject*, gpointer);
void gw_main_search_cb (GtkWidget *widget, gpointer data);
void gw_main_search_from_history_cb (GtkWidget*, gpointer);
void gw_main_clear_search_cb (GtkWidget*, gpointer);
void gw_main_update_button_states_based_on_entry_text_cb (GtkWidget*, gpointer);
void gw_main_go_menuitem_action_cb (GtkWidget*, gpointer);
void gw_main_close_kanji_results_cb (GtkWidget*, gpointer);
void gw_main_dictionary_changed_action_cb (GtkWidget*, gpointer);
void gw_main_cycle_dictionaries_forward_cb (GtkWidget*, gpointer);
void gw_main_cycle_dictionaries_backward_cb (GtkWidget*, gpointer);
void search_drag_data_recieved (GtkWidget*, GdkDragContext*,
                                gint, gint,
                                GtkSelectionData*, guint,
                                guint, gpointer             );

gboolean gw_main_update_clipboard_on_focus_change_cb (GtkWidget*, GtkDirectionType, gpointer);
gboolean gw_main_focus_change_on_key_press_cb (GtkWidget*, GdkEvent*, gpointer*);
gboolean gw_main_history_change_on_key_press_cb ( GtkWidget*, GdkEvent*, gpointer*);
gboolean gw_main_switch_dictionaries_quickkey_action_cb ( GtkWidget*, GdkEvent*, gpointer*);

gboolean gw_main_key_press_action_cb (GtkWidget*, GdkEvent*, gpointer*);
gboolean gw_main_close_on_escape_cb (GtkWidget*, GdkEvent*, gpointer*);

gboolean gw_main_drag_motion_1_cb (GtkWidget*, GdkDragContext*, gint, gint, guint, gpointer);
gboolean gw_main_update_clipboard_on_focus_change_cb (GtkWidget*, GtkDirectionType, gpointer);
gboolean gw_main_get_position_for_button_press_cb (GtkWidget*, GdkEventButton*, gpointer);
gboolean gw_main_get_iter_for_motion_cb (GtkWidget*, GdkEventButton*, gpointer);
gboolean gw_main_drag_drop_1_cb (GtkWidget*, GdkDragContext*, gint, gint, guint, gpointer);
gboolean gw_main_get_iter_for_button_release_cb (GtkWidget*, GdkEventButton*, gpointer);
void gw_main_drag_leave_1_cb (GtkWidget*, GdkDragContext*, guint, gpointer);
void gw_main_search_drag_data_recieved_cb (GtkWidget*, GdkDragContext*, gint, gint , GtkSelectionData*, guint, guint, gpointer);
gboolean gw_main_focus_change_on_key_press_cb (GtkWidget*, GdkEvent*, gpointer*);
gboolean gw_main_update_icons_for_selection_cb (GtkWidget*, GdkEvent*, gpointer); 

void gw_main_populate_popup_with_search_options_cb (GtkTextView*, GtkMenu*, gpointer);
void gw_main_search_for_searchitem_online_cb (GtkWidget*, gpointer);

gboolean gw_main_scroll_or_zoom_cb (GtkWidget*, GdkEventScroll*, gpointer);

#endif

