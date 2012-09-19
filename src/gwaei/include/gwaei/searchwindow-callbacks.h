#ifndef GW_SEARCHWINDOW_CALLBACKS_INCLUDED
#define GW_SEARCHWINDOW_CALLBACKS_INCLUDED

void gw_searchwindow_back_cb (GtkWidget *widget, gpointer data);
void gw_searchwindow_forward_cb (GtkWidget *widget, gpointer data);
void gw_searchwindow_save_cb (GtkWidget *widget, gpointer data);
void gw_searchwindow_print_cb (GtkWidget *widget, gpointer data);
void gw_searchwindow_zoom_in_cb (GtkWidget *widget, gpointer data);
void gw_searchwindow_zoom_out_cb (GtkWidget *widget, gpointer data);
void gw_searchwindow_zoom_100_cb (GtkWidget *widget, gpointer data);
void gw_searchwindow_statusbar_toggle_cb (GtkWidget *widget, gpointer data);
void gw_searchwindow_less_relevant_results_toggle_cb(GtkWidget *widget, gpointer data);
void gw_searchwindow_select_all_cb (GtkWidget *widget, gpointer data);
void gw_searchwindow_paste_cb (GtkAction *action, gpointer data);
void gw_searchwindow_cut_cb (GtkAction *widget, gpointer data);
void gw_searchwindow_copy_cb (GtkAction *action, gpointer data);
void gw_searchwindow_about_cb (GtkWidget *widget, gpointer data);
void gw_searchwindow_destroy_cb (GObject*, gpointer);
void gw_searchwindow_search_cb (GtkWidget *widget, gpointer data);
void gw_searchwindow_search_from_history_cb (GtkWidget*, gpointer);
void gw_searchwindow_clear_search_cb (GtkWidget*, gpointer);
void gw_searchwindow_update_button_states_based_on_entry_text_cb (GtkWidget*, gpointer);
void gw_searchwindow_go_menuitem_action_cb (GtkWidget*, gpointer);
void gw_searchwindow_close_kanji_results_cb (GtkWidget*, gpointer);
void gw_searchwindow_dictionary_combobox_changed_cb (GtkWidget*, gpointer);
void gw_searchwindow_dictionary_radio_changed_cb (GtkWidget*, gpointer);
void gw_searchwindow_cycle_dictionaries_forward_cb (GtkWidget*, gpointer);
void gw_searchwindow_cycle_dictionaries_backward_cb (GtkWidget*, gpointer);
void search_drag_data_recieved (GtkWidget*, GdkDragContext*,
                                gint, gint,
                                GtkSelectionData*, guint,
                                guint, gpointer             );

gboolean gw_searchwindow_update_clipboard_on_focus_change_cb (GtkWidget*, GtkDirectionType, gpointer);
gboolean gw_searchwindow_focus_change_on_key_press_cb (GtkWidget*, GdkEvent*, gpointer*);
gboolean gw_searchwindow_history_change_on_key_press_cb ( GtkWidget*, GdkEvent*, gpointer*);
gboolean gw_searchwindow_switch_dictionaries_quickkey_action_cb ( GtkWidget*, GdkEvent*, gpointer*);

gboolean gw_searchwindow_key_press_action_cb (GtkWidget*, GdkEvent*, gpointer*);
gboolean gw_searchwindow_close_on_escape_cb (GtkWidget*, GdkEvent*, gpointer*);

gboolean gw_searchwindow_drag_motion_1_cb (GtkWidget*, GdkDragContext*, gint, gint, guint, gpointer);
gboolean gw_searchwindow_update_clipboard_on_focus_change_cb (GtkWidget*, GtkDirectionType, gpointer);
gboolean gw_searchwindow_get_position_for_button_press_cb (GtkWidget*, GdkEventButton*, gpointer);
gboolean gw_searchwindow_drag_drop_1_cb (GtkWidget*, GdkDragContext*, gint, gint, guint, gpointer);
gboolean gw_searchwindow_get_iter_for_button_release_cb (GtkWidget*, GdkEventButton*, gpointer);
void gw_searchwindow_drag_leave_1_cb (GtkWidget*, GdkDragContext*, guint, gpointer);
void gw_searchwindow_search_drag_data_recieved_cb (GtkWidget*, GdkDragContext*, gint, gint , GtkSelectionData*, guint, guint, gpointer);
gboolean gw_searchwindow_focus_change_on_key_press_cb (GtkWidget*, GdkEvent*, gpointer*);
gboolean gw_searchwindow_update_icons_for_selection_cb (GtkWidget*, GdkEvent*, gpointer); 

//void gw_searchwindow_dictionaries_changed_cb (GtkTreeModel*, GtkTreePath*, GtkTreeIter*, gpointer);
void gw_searchwindow_dictionaries_added_cb (GtkTreeModel*, GtkTreePath*, GtkTreeIter*, gpointer);
void gw_searchwindow_dictionaries_deleted_cb (GtkTreeModel*, GtkTreePath*, gpointer);

gboolean gw_searchwindow_scroll_or_zoom_cb (GtkWidget*, GdkEventScroll*, gpointer);

void gw_searchwindow_remove_tab_cb (GtkWidget*, gpointer);
void gw_searchwindow_remove_current_tab_cb (GtkWidget*, gpointer);

void gw_searchwindow_new_window_cb (GtkWidget*, gpointer);

//Update preference callbacks
void gw_searchwindow_toolbar_show_toggled_cb (GtkWidget *widget, gpointer data);
void gw_searchwindow_statusbar_show_toggled_cb (GtkWidget *widget, gpointer data);

//Sync to preference callbacks
void gw_searchwindow_sync_toolbar_show_cb (GSettings*, gchar*, gpointer);
void gw_searchwindow_sync_statusbar_show_cb (GSettings*, gchar*, gpointer);
void gw_searchwindow_sync_font_cb (GSettings*, gchar*, gpointer);
void gw_searchwindow_sync_search_as_you_type_cb (GSettings*, gchar*, gpointer);
void gw_searchwindow_sync_spellcheck_cb (GSettings*, gchar*, gpointer);
gboolean gw_searchwindow_delete_event_action_cb (GtkWidget*, GdkEvent*, gpointer);

gboolean gw_searchwindow_key_release_modify_status_update_cb (GtkWidget*, GdkEvent*, gpointer);
gboolean gw_searchwindow_key_press_modify_status_update_cb (GtkWidget*, GdkEvent*, gpointer);
gboolean gw_searchwindow_focus_in_event_cb (GtkWidget*, GdkEvent*, gpointer);
void gw_searchwindow_event_after_cb (GtkWidget*, GdkEvent*, gpointer);

gboolean gw_searchwindow_motion_notify_event_cb (GtkWidget*, GdkEventButton*, gpointer);
void gw_searchwindow_vocabulary_changed_cb (GtkWidget*, gpointer);
void gw_searchwindow_vocabulary_menuitem_activated_cb (GtkWidget*, gpointer);

void gw_searchwindow_kanjipadwindow_destroy_cb (GtkWidget*, gpointer);
void gw_searchwindow_radicalswindow_destroy_cb (GtkWidget*, gpointer);

#endif

