#ifndef GW_SEARCHWINDOW_CALLBACKS_INCLUDED
#define GW_SEARCHWINDOW_CALLBACKS_INCLUDED

void gw_searchwindow_go_back_cb (GSimpleAction*, GVariant*, gpointer);
void gw_searchwindow_go_forward_cb (GSimpleAction*, GVariant*, gpointer);
void gw_searchwindow_go_back_index_cb (GSimpleAction*, GVariant*, gpointer);
void gw_searchwindow_go_forward_index_cb (GSimpleAction*, GVariant*, gpointer);
void gw_searchwindow_save_cb  (GSimpleAction*, GVariant*, gpointer);
void gw_searchwindow_save_as_cb  (GSimpleAction*, GVariant*, gpointer);
void gw_searchwindow_print_cb (GtkWidget *widget, gpointer data);
void gw_searchwindow_zoom_in_cb (GSimpleAction*, GVariant*, gpointer);
void gw_searchwindow_zoom_out_cb (GSimpleAction*, GVariant*, gpointer);
void gw_searchwindow_zoom_100_cb (GSimpleAction*, GVariant*, gpointer);
void gw_searchwindow_statusbar_toggle_cb (GtkWidget *widget, gpointer data);
void gw_searchwindow_select_all_cb (GSimpleAction*, GVariant*, gpointer data);
void gw_searchwindow_paste_cb (GSimpleAction*, GVariant*, gpointer data);
void gw_searchwindow_cut_cb (GSimpleAction*, GVariant*, gpointer data);
void gw_searchwindow_copy_cb (GSimpleAction*, GVariant*, gpointer data);
void gw_searchwindow_destroy_cb (GObject*, gpointer);
void gw_searchwindow_search_cb (GtkWidget *widget, gpointer data);
void gw_searchwindow_search_from_history_cb (GtkWidget*, gpointer);
void gw_searchwindow_clear_search_cb (GSimpleAction*, GVariant*, gpointer);
void gw_searchwindow_update_button_states_based_on_entry_text_cb (GtkEditable*, gpointer);
void gw_searchwindow_go_menuitem_action_cb (GtkWidget*, gpointer);
void gw_searchwindow_close_kanji_results_cb (GtkWidget*, gpointer);
void gw_searchwindow_dictionary_combobox_changed_cb (GtkWidget*, gpointer);
void gw_searchwindow_dictionary_radio_changed_cb (GtkWidget*, gpointer);
void gw_searchwindow_next_dictionary_cb (GSimpleAction*, GVariant*, gpointer);
void gw_searchwindow_previous_dictionary_cb (GSimpleAction*, GVariant*, gpointer);
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

gboolean gw_searchwindow_scroll_or_zoom_cb (GtkWidget*, GdkEventScroll*, gpointer);

void gw_searchwindow_remove_tab_cb (GtkWidget*, gpointer);
void gw_searchwindow_remove_current_tab_cb (GtkWidget*, gpointer);

void gw_searchwindow_new_window_cb (GSimpleAction*, GVariant*, gpointer);
void gw_searchwindow_new_tab_cb (GSimpleAction*, GVariant*, gpointer);
void gw_searchwindow_next_tab_cb (GSimpleAction*, GVariant*, gpointer);
void gw_searchwindow_previous_tab_cb (GSimpleAction*, GVariant*, gpointer);
void gw_searchwindow_close_cb (GSimpleAction*, GVariant*, gpointer);

//Update preference callbacks
void gw_searchwindow_menubar_show_toggled_cb (GSimpleAction*, GVariant*, gpointer);
void gw_searchwindow_toolbar_show_toggled_cb (GSimpleAction*, GVariant*, gpointer);
void gw_searchwindow_tabbar_show_toggled_cb (GSimpleAction*, GVariant*, gpointer);
void gw_searchwindow_statusbar_show_toggled_cb (GSimpleAction*, GVariant*, gpointer);

//Sync to preference callbacks
void gw_searchwindow_sync_menubar_show_cb (GSettings*, gchar*, gpointer);
void gw_searchwindow_sync_toolbar_show_cb (GSettings*, gchar*, gpointer);
void gw_searchwindow_sync_tabbar_show_cb (GSettings*, gchar*, gpointer);
void gw_searchwindow_sync_statusbar_show_cb (GSettings*, gchar*, gpointer);
void gw_searchwindow_sync_font_cb (GSettings*, gchar*, gpointer);
void gw_searchwindow_sync_search_as_you_type_cb (GSettings*, gchar*, gpointer);
void gw_searchwindow_sync_spellcheck_cb (GSettings*, gchar*, gpointer);

gboolean gw_searchwindow_key_release_modify_status_update_cb (GtkWidget*, GdkEvent*, gpointer);
gboolean gw_searchwindow_key_press_modify_status_update_cb (GtkWidget*, GdkEvent*, gpointer);
gboolean gw_searchwindow_focus_in_event_cb (GtkWidget*, GdkEvent*, gpointer);
void gw_searchwindow_event_after_cb (GtkWidget*, GdkEvent*, gpointer);

gboolean gw_searchwindow_motion_notify_event_cb (GtkWidget*, GdkEventButton*, gpointer);
void gw_searchwindow_vocabulary_changed_cb (GtkWidget*, gpointer);

void gw_searchwindow_kanjipadwindow_destroy_cb (GtkWidget*, gpointer);
void gw_searchwindow_radicalswindow_destroy_cb (GtkWidget*, gpointer);

void gw_searchwindow_show_popup_menu_cb (GtkWidget *, gpointer);
void gw_searchwindow_add_vocabulary_word_cb (GSimpleAction*, GVariant*, gpointer);
void gw_searchwindow_open_vocabularywindow_cb (GSimpleAction*, GVariant*, gpointer);

void gw_searchwindow_toggle_kanjipadwindow_cb (GSimpleAction*, GVariant*, gpointer);
void gw_searchwindow_toggle_radicalswindow_cb (GSimpleAction*, GVariant*, gpointer);

void gw_searchwindow_insert_unknown_character_cb (GSimpleAction *, GVariant*, gpointer);
void gw_searchwindow_insert_not_word_edge_cb (GSimpleAction *, GVariant*, gpointer);
void gw_searchwindow_insert_word_edge_cb (GSimpleAction *, GVariant*, gpointer);
void gw_searchwindow_insert_and_cb (GSimpleAction *, GVariant*, gpointer);
void gw_searchwindow_insert_or_cb (GSimpleAction *, GVariant*, gpointer);

void gw_searchwindow_set_dictionary_cb (GSimpleAction*, GVariant*, gpointer);
void gw_searchwindow_clear_entry_button_pressed_cb (GtkEntry*, GtkEntryIconPosition, GdkEvent*, gpointer);

void gw_searchwindow_dictionarylist_changed_cb (GwSearchWindow*, GwDictionaryList*);


#endif

