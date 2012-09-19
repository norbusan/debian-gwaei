/******************************************************************************
    AUTHOR:
    File written and Copyrighted by Zachary Dovel. All Rights Reserved.

    LICENSE:
    This file is part of gWaei.

    gWaei is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    gWaei is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with gWaei.  If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/

//!
//! @file src/gtk/main-callbacks.c
//!
//! @brief Abstraction layer for gtk callbacks
//!
//! Callbacks for activities initiated by the user. Most of the gtk code here
//! should still be abstracted to the interface C file when possible.
//!


#include <string.h>
#include <stdlib.h>
#include <libintl.h>

#include <gdk/gdkkeysyms.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <gwaei/gwaei.h>


static LwSearchItem *tooltip_item = NULL;

static gint button_press_x = 0;
static gint button_press_y = 0;
static gunichar button_character = 0;
static gulong copy_handler_id = 0;
static gulong cut_handler_id = 0;
static gulong paste_handler_id = 0;
static gulong select_all_handler_id = 0;
static gboolean start_search_in_new_window = FALSE; 
static char* hovered_word = NULL; 


//!
//! @brief Sets the cursor type depending on the character hovered
//!
//! If the character hovered is a kanji character, the hand turns into a
//! pointer in order to show that the selection is clickable. It will open
//! the kanji sidebar using gw_main_get_position_for_button_press_cb () and
//! gw_main_get_iter_for_button_release_cb ().
//! 
//! @see gw_main_get_position_for_button_press_cb ()
//! @see gw_main_get_iter_for_button_release_cb ()
//! @param widget Currently unused widget pointer
//! @param data Currently unused gpointer
//! @return Always returns false
//!
G_MODULE_EXPORT gboolean gw_main_get_iter_for_motion_cb (GtkWidget      *widget,
                                                         GdkEventButton *event,
                                                         gpointer        data   )
{
    gunichar unic;
    GtkTextIter iter, start, end;
    gint x = event->x;
    gint y = event->y;

    //get unichar
    unic = gw_main_get_hovered_character (&x, &y, &iter);

    //get word
    if (gtk_text_iter_starts_word (&iter) == FALSE)
      gtk_text_iter_backward_word_start (&iter);
    start = iter;
    if (gtk_text_iter_ends_word (&iter) == FALSE)
      gtk_text_iter_forward_word_end (&iter);
    end = iter;
    hovered_word = gtk_text_iter_get_visible_slice (&start, &end);

    LwDictInfo *di;
    di = lw_dictinfolist_get_dictinfo (GW_ENGINE_KANJI, "Kanji");
    if (di == NULL) return FALSE;
  
    // Characters above 0xFF00 represent inserted images
    if (unic > L'ー' && unic < 0xFF00)
      gw_main_set_cursor (GDK_HAND2);
    else
      gw_main_set_cursor (GDK_XTERM);

    GtkWidget *tv = GTK_WIDGET (gw_common_get_widget_by_target (GW_TARGET_RESULTS));
    GtkWidget *window = GTK_WIDGET (gtk_widget_get_tooltip_window (tv));
    if (window != NULL && button_character != unic) 
    {
      gtk_widget_destroy (window);
      gtk_widget_set_tooltip_window (tv, NULL);
    }

    return FALSE;
}


//!
//! @brief Gets the position of the cursor click and stores it
//!
//! The function stores the location of the button press, but takes no action
//! by itself.  gw_main_get_iter_for_button_release_cb () uses the saved x and y
//! coordinates and determines if an action should be taken then.
//! 
//! @see gw_main_get_iter_for_button_release_cb ()
//! @param widget Currently unused widget pointer
//! @param data Currently unused gpointer
//! @return Always returns false
//!
G_MODULE_EXPORT gboolean gw_main_get_position_for_button_press_cb (GtkWidget      *widget,
                                                                   GdkEventButton *event,
                                                                   gpointer        data    )
{
    GtkTextIter iter;
    //Window coordinates
    button_press_x = event->x;
    button_press_y = event->y;

    gw_main_get_hovered_character (&button_press_x, &button_press_y, &iter);

    return FALSE;
}


//!
//! @brief Gets the position of the cursor click then opens the kanji sidebar
//!
//! Compares the x and y coordinates fetch by gw_main_get_position_for_button_press_cb
//! for the cursor, and if the difference is below a certain threshhold,
//! decides if the user wants to open the kanji character under cursor in the
//! kanji sidebar or not.
//! 
//! @see gw_main_get_position_for_button_press_cb ()
//! @param widget Currently unused widget pointer
//! @param data Currently unused gpointer
//! @return Always returns false
//!
G_MODULE_EXPORT gboolean gw_main_get_iter_for_button_release_cb (GtkWidget      *widget,
                                                                 GdkEventButton *event,
                                                                 gpointer        data    )
{
    //Declarations
    GtkBuilder *builder;
    gint x;
    gint y;
    gint trailing;
    GtkTextIter iter;
    gunichar unic;
    LwDictInfo *di;
    GError *error;

    //Initializations
    builder = gw_common_get_builder ();
    x = event->x;
    y = event->y;
    trailing = 0;
    unic = gw_main_get_hovered_character (&x, &y, &iter);
    di = lw_dictinfolist_get_dictinfo (GW_ENGINE_KANJI, "Kanji");
    error = NULL;

    //Sanity cehck
    if (di == NULL) return FALSE;

    if (abs (button_press_x - x) < 3 && abs (button_press_y - y) < 3)
    {
      // Characters above 0xFF00 represent inserted images
      if (unic > L'ー' && unic < 0xFF00 )
      {
        //Convert the unicode character into to a utf8 string
        gchar query[7];
        gint length = g_unichar_to_utf8 (unic, query);
        query[length] = '\0'; 

        GtkWidget *tv = GTK_WIDGET (gw_common_get_widget_by_target (GW_TARGET_RESULTS));
        GtkWidget *window = GTK_WIDGET (gtk_widget_get_tooltip_window (tv));
        GtkWindow* parent = GTK_WINDOW (gtk_builder_get_object (builder, "main_window"));
        if (window == NULL) {
          button_character = unic;
          window = gtk_window_new (GTK_WINDOW_POPUP);
          gtk_window_set_skip_taskbar_hint (GTK_WINDOW (window), TRUE);
          gtk_window_set_skip_pager_hint (GTK_WINDOW (window), TRUE);
          gtk_window_set_accept_focus (GTK_WINDOW (window), FALSE);
          gtk_widget_set_tooltip_window (tv, GTK_WINDOW (window));
          gtk_window_set_transient_for (GTK_WINDOW (window), NULL);
          gtk_window_set_type_hint (GTK_WINDOW (window), GDK_WINDOW_TYPE_HINT_TOOLTIP);
          gtk_widget_set_name (GTK_WIDGET (window), "gtk-tooltip");
        }
        if (window != NULL) {
          button_character = unic;

          //Start the search
          if (tooltip_item != NULL)
          {
            gw_main_cancel_search_by_searchitem (tooltip_item);
            tooltip_item = NULL;
          }

          tooltip_item = lw_searchitem_new (query, di, GW_TARGET_KANJI, &error);
          lw_engine_get_results (tooltip_item, TRUE, FALSE);

          g_thread_join (tooltip_item->thread); 

          int winx, winy;
          gtk_window_get_position (GTK_WINDOW (window), &winx, &winy);
          gtk_window_move (GTK_WINDOW (window), event->x_root + winx - 3, event->y_root + winy - 3);
          gtk_widget_show_now (GTK_WIDGET (window));
        }
      }
      else {
        GtkWidget *tv = GTK_WIDGET (gw_common_get_widget_by_target (GW_TARGET_RESULTS));
        GtkWidget *window = GTK_WIDGET (gtk_widget_get_tooltip_window (tv));
        if (window != NULL && button_character != unic) 
        {
          gtk_widget_set_tooltip_window (tv, NULL);
          gtk_widget_destroy (window);
          window = NULL;
        }
      }

      if (error != NULL)
      {
        printf("%s\n", error->message);
        g_error_free (error);
      }
    }

    return FALSE; 
}


//!
//! @brief Closes the window passed throught the widget pointer
//!
//! This function closes the window passed through the widget pointer.
//! Depending if it is a specific window, it will save it's coordinates
//! or take other special actions before closing.
//! 
//! @param widget GtkWidget pointer to the window to close
//! @param data Currently unused gpointer
//!
G_MODULE_EXPORT void gw_main_close_cb (GtkWidget *widget, gpointer data)
{
    const char *id = gtk_buildable_get_name (GTK_BUILDABLE (widget));

    if (strcmp (id, "main_window") == 0)
    {
      gw_common_hide_window (id);
      gw_main_tab_cancel_all_searches ();
      gtk_main_quit ();
    }
    else if (strcmp (id, "radicals_window") == 0 || strcmp (id, "kanjipad_window") == 0)
    {
      gw_common_hide_window (id);
    }
    else if (strcmp (id, "settings_window") == 0)
    {
      if (lw_dictinfolist_get_total () > 0)
      {
        gw_main_update_toolbar_buttons ();
        gw_common_hide_window (id);
      }
      else
      {
        gw_main_tab_cancel_all_searches ();
        gtk_main_quit ();
      }
    }
    else
    {
      gw_common_hide_window (id);
    }
}


//!
//! @brief Preforms the action the window manager close event
//!
//! This function currently acts as a proxy for the gw_main_close_cb () function.
//! 
//! @see gw_main_close_cb ()
//! @param widget GtkWidget pointer to the window to close
//! @param data Currently unused gpointer
//! @return Always returns true
//!
G_MODULE_EXPORT gboolean gw_main_delete_event_action_cb (GtkWidget *widget, gpointer data)
{ 
    gw_main_close_cb (widget, data);
    return TRUE;
}


//!
//! @brief Closes the current window when the escape key is pressed
//!
//! Checks for a pure escape press with no modifiers then closes the window
//! passed to the function through the widget pointer is gw_main_close_cb (). This
//! function is generally only attached to windows you could picture a cancel
//! button being attached to in the right context.
//! 
//! @see gw_main_close_cb ()
//! @param widget GtkWidget pointer to the window to close
//! @param data Currently unused gpointer
//! @return Returns true when escape key is pressed
//!
G_MODULE_EXPORT gboolean gw_main_close_on_escape_cb (GtkWidget *widget,
                                                     GdkEvent  *event,
                                                     gpointer  *data   )
{
    guint keyval = ((GdkEventKey*)event)->keyval;
    guint state = ((GdkEventKey*)event)->state;
    guint modifiers = ( 
                        GDK_MOD1_MASK    |
                        GDK_CONTROL_MASK |
                        GDK_SUPER_MASK   |
                        GDK_HYPER_MASK   |
                        GDK_META_MASK      
                      );

    //Make sure no modifier keys are pressed
    if (((state & modifiers) == 0 ) && keyval == GDK_KEY_Escape)
    {
      gw_main_close_cb (widget, data);
      return TRUE;
    }

    return FALSE;
}


//!
//! @brief Quits out of the application
//!
//! This function quits gWaei out.  But before it does, it calls the gw_main_close_cb
//! function an the calling window to make sure it's current coordinates are
//! recorded for the next time the application is started.
//! 
//! @see gw_main_close_cb ()
//! @param widget GtkWidget pointer to the window to close
//! @param data Currently unused gpointer
//!
G_MODULE_EXPORT void gw_main_quit_cb (GtkWidget *widget, gpointer data)
{
    gw_main_tab_cancel_all_searches ();
    gw_main_close_cb (widget, data);
    gtk_main_quit ();
}


//!
//! @brief Preforms a search from the history.
//!
//! The function uses the gpointer data to fetch a LwSearchItem that was pased
//! to the function for the search.  It will reflow the back and forward
//! history lists so the LwSearchItem is in the current position of the
//! Historylist.
//! 
//! @see gw_main_search_cb ()
//! @param widget Unused GtkWidget pointer.
//! @param data pointer to a specially attached LwSearchItem variable
//!
G_MODULE_EXPORT void gw_main_search_from_history_cb (GtkWidget *widget, gpointer data)
{
    GtkBuilder *builder = gw_common_get_builder ();

    gw_tabs_guarantee_first ();

    LwHistoryList *hl;
    LwSearchItem *item;

    hl = lw_historylist_get_list (GW_HISTORYLIST_RESULTS);
    item = (LwSearchItem*) data;

    //Make sure searches done from the history are pointing at a valid target
    item->target_tb = (gpointer) gw_common_get_gobject_by_target (item->target);
    item->target_tv = (gpointer) gw_common_get_widget_by_target (item->target);

    //Checks to make sure everything is sane
    if (gw_main_cancel_search_for_current_tab () == FALSE)
    {
      printf("CANCEL SEARCH FOR CURRENT TAB RETURNED FALSE\n");
      return;
    }

    //Start setting things up;
    if (hl->back != NULL && g_list_find (hl->back, item))
    {
      while (hl->back != NULL && hl->current != item)
        lw_historylist_go_back_by_target (GW_HISTORYLIST_RESULTS);
    }
    else if (hl->forward != NULL && g_list_find (hl->forward, item))
    {
      while (hl->forward != NULL && hl->current != item)
        lw_historylist_go_forward_by_target (GW_HISTORYLIST_RESULTS);
    }

    //Set tab text
    gw_tabs_set_current_tab_text (item->queryline->string);

    //Add tab reference to searchitem
    gw_tabs_set_searchitem (item);
    lw_engine_get_results (item, TRUE, FALSE);
    gw_main_update_history_popups ();
    gw_main_update_toolbar_buttons ();

    //Set the search string in the GtkEntry
    gw_main_clear_search_entry ();
    gw_main_search_entry_insert (item->queryline->string);
    gw_main_text_select_all_by_target (GW_TARGET_ENTRY);
    gw_main_grab_focus_by_target (GW_TARGET_ENTRY);

    //Set the correct dictionary in the gui
    GtkWidget *combobox;
    combobox = GTK_WIDGET (gtk_builder_get_object (builder, "dictionary_combobox"));
    gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), item->dictionary->load_position);
}


//!
//! @brief Goes back one step in the search history
//! 
//! This function checks the top of the back historylist and uses the
//! LwSearchItem in it to invoke gw_main_search_from_history_cb () using it.
//!
//! @see gw_main_search_from_history_cb ()
//! @see gw_main_forward_cb ()
//! @param widget Unused GtkWidget pointer.
//! @param data pointer to a specially attached LwSearchItem variable
//!
G_MODULE_EXPORT void gw_main_back_cb (GtkWidget *widget, gpointer data)
{
    LwHistoryList *hl;
    hl = lw_historylist_get_list (GW_HISTORYLIST_RESULTS);
    if (hl->back != NULL)
    {
      gw_main_search_from_history_cb (NULL, hl->back->data);
    }
}


//!
//! @brief Goes forward one step in the search history
//! 
//! This function checks the top of the forward historylist and uses the
//! LwSearchItem in it to invoke gw_main_search_from_history_cb () using it.
//!
//! @see gw_main_search_from_history_cb ()
//! @see gw_main_back_cb ()
//! @param widget Unused GtkWidget pointer.
//! @param data pointer to a specially attached LwSearchItem variable
//!
G_MODULE_EXPORT void gw_main_forward_cb (GtkWidget *widget, gpointer data)
{
    LwHistoryList *hl;
    hl = lw_historylist_get_list (GW_HISTORYLIST_RESULTS);
    if (hl->forward != NULL)
    {
      gw_main_search_from_history_cb (NULL, hl->forward->data);
    }
}


//!
//! @brief Saves the current search results to a file
//! 
//! The function gets the current contents of the results text view and saves
//! it to a file, overwriting it if it already exists.  If part of the results
//! are highlighted, only that gets saved.
//!
//! @see gw_main_save_cb ()
//! @param widget Unused GtkWidget pointer.
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_main_save_as_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GtkBuilder *builder = gw_common_get_builder ();
    const gchar *path;
    GtkWidget *dialog, *window;
    GtkAction *edit;
    gchar *text;
    gchar *temp;
    GError *error;

    //Initializations
    path = lw_io_get_savepath ();
    temp = NULL;
    text = gw_main_buffer_get_text_by_target (GW_TARGET_RESULTS);
    window = GTK_WIDGET (gtk_builder_get_object (builder, "main_window"));
    dialog = gtk_file_chooser_dialog_new (gettext ("Save As"),
                GTK_WINDOW (window),
                GTK_FILE_CHOOSER_ACTION_SAVE,
                GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                NULL);
    gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);
    error = NULL;

    //Set the default save path if none is set
    if (path == NULL)
    {
        gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), "");
        gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), gettext ("vocabulary.txt"));
    }
    //Otherwise use the already existing one
    else
    {
        gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (dialog), path);
    }

    //Run the save as dialog
    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
        //Set the new savepath
        temp = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
        lw_io_set_savepath (temp);
        g_free (temp);
        temp = NULL;
        path = lw_io_get_savepath ();

        lw_io_write_file (path, "w", text, NULL, NULL, &error);

        edit = GTK_ACTION (gtk_builder_get_object (builder, "file_edit_action"));
        gtk_action_set_sensitive (edit, TRUE);
    }

    //Cleanup
    gtk_widget_destroy (dialog);
    g_free (text);
    text = NULL;
    gw_common_handle_error (&error, GTK_WINDOW (window), TRUE);
}


//!
//! @brief Appends the current search results to a file
//! 
//! The function gets the current contents of the results text view and appends
//! it to a file.  If the user has already saved once, it will automatically
//! keep appending to the same file. If part of the results are highlighted,
//! only that gets appended.
//!
//! @see gw_main_save_as_cb ()
//! @param widget Unused GtkWidget pointer.
//! @param data Unused gpointer
//0
G_MODULE_EXPORT void gw_main_save_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    gchar *text;
    const gchar *path;
    GError *error;
    GtkBuilder *builder;
    GtkWidget *window;

    //Initializations
    builder = gw_common_get_builder ();
    path = lw_io_get_savepath ();
    window = GTK_WIDGET (gtk_builder_get_object (builder, "main_window"));
    error = NULL;

    //Sanity check for an empty save path
    if (path == NULL || *path == '\0')
    {
      gw_main_save_as_cb (NULL, NULL);
      return;
    }

    //Carry out the save
    text = gw_main_buffer_get_text_by_target (GW_TARGET_RESULTS);
    lw_io_write_file (path, "a", text, NULL, NULL, &error);
    g_free (text);
    text = NULL;

    gw_common_handle_error (&error, GTK_WINDOW (window), TRUE);
}


//!
//! @brief Makes the text in the text buffer enlarge
//! 
//! Determines if the text size is smaller than the max possible text size,
//! and then sets the pref in gconf which will trigger the font size setting
//! function.
//!
//! @see gw_main_set_font()
//! @param widget Unused GtkWidget pointer.
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_main_zoom_in_cb (GtkWidget *widget, gpointer data)
{
    int size;
    size = lw_pref_get_int_by_schema (GW_SCHEMA_FONT, GW_KEY_FONT_MAGNIFICATION) + GW_FONT_ZOOM_STEP;
    if (size <= GW_MAX_FONT_MAGNIFICATION)
      lw_pref_set_int_by_schema (GW_SCHEMA_FONT, GW_KEY_FONT_MAGNIFICATION, size);
}


//!
//! @brief Makes the text in the text buffer shrink
//! 
//! Determines if the text size is larger than the min possible text size,
//! and then sets the pref in gconf which will trigger the font size setting
//! function.
//!
//! @see gw_main_set_font()
//! @param widget Unused GtkWidget pointer.
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_main_zoom_out_cb (GtkWidget *widget, gpointer data)
{
    int size;
    size = lw_pref_get_int_by_schema (GW_SCHEMA_FONT, GW_KEY_FONT_MAGNIFICATION) - GW_FONT_ZOOM_STEP;
    if (size >= GW_MIN_FONT_MAGNIFICATION)
      lw_pref_set_int_by_schema (GW_SCHEMA_FONT, GW_KEY_FONT_MAGNIFICATION, size);
}


//!
//! @brief Resets the text size to the default in the text buffers
//! 
//! The function acts gconf for the default font size from the schema, and then
//! sets it, which makes gconf call the font size setting function since the
//! stored value changed.
//!
//! @see gw_main_set_font()
//! @param widget Unused GtkWidget pointer.
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_main_zoom_100_cb (GtkWidget *widget, gpointer data)
{
    lw_pref_reset_value_by_schema (GW_SCHEMA_FONT, GW_KEY_FONT_MAGNIFICATION);
}


//!
//! @brief Sets the less relevant results show boolean
//! 
//! Makes the gconf pref match the current state of the triggering widget.
//! Each separate LwSearchItem stores this individually, so even if you flip
//! this, you will need to do a new search if you want to change how things are
//! displayed.
//!
//! @see gw_main_set_less_relevant_show ()
//! @param widget Unused GtkWidget pointer.
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_main_less_relevant_results_toggle_cb (GtkWidget *widget, gpointer data)
{
    gboolean state;
    state = lw_pref_get_boolean_by_schema (GW_SCHEMA_BASE, GW_KEY_LESS_RELEVANT_SHOW);
    lw_pref_set_boolean_by_schema (GW_SCHEMA_BASE, GW_KEY_LESS_RELEVANT_SHOW, !state);
}


//!
//! @brief Sets the show toolbar boolean to match the widget
//! 
//! Makes the gconf pref match the current state of the triggering widget.
//! The gconf value changed callback then updates the state of the toolbar
//! to match the pref.
//!
//! @see gw_main_set_toolbar_show ()
//! @param widget Unused GtkWidget pointer.
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_main_toolbar_toggle_cb (GtkWidget *widget, gpointer data)
{
    gboolean state;
    state = lw_pref_get_boolean_by_schema (GW_SCHEMA_BASE, GW_KEY_TOOLBAR_SHOW);
    lw_pref_set_boolean_by_schema (GW_SCHEMA_BASE, GW_KEY_TOOLBAR_SHOW, !state);
}


//!
//! @brief Changes the selected dictionary in the dictionarylist
//! 
//! This function makes the selected dictionary in the dictionarylist match
//! the dictionary of the widget that was modified.  The selected dictionary
//! in the dictionarylist acts as the central reference for the GUI.
//!
//! @param widget pointer to the GtkWidget that changed dictionaries
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_main_dictionary_changed_action_cb (GtkWidget *widget, gpointer data)
{
    GtkBuilder *builder = gw_common_get_builder ();

    //Declarations
    const char id_length = 50;
    char id[id_length];
    GList *list = NULL;
    GtkMenuShell *shell = NULL;
    strncpy (id, "dictionary_popup", id_length);
    shell = GTK_MENU_SHELL (gtk_builder_get_object (builder, id));

    //Action depending on the source
    int active = 0;
    if (strcmp (G_OBJECT_TYPE_NAME(widget), "GtkComboBox") == 0 )
    {
      active = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));
    }
    else if (strcmp (G_OBJECT_TYPE_NAME(widget), "GtkRadioMenuItem") == 0 )
    {
      list = gtk_container_get_children (GTK_CONTAINER (shell));
      GList *iter = list;
      while (iter != NULL && gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (iter->data)) == FALSE)
      {
        iter = iter->next;
        active++;
      }
      g_list_free (list);
      list = NULL;
    }

    //Finish up
    gw_main_set_dictionary (active);

    gw_main_grab_focus_by_target (GW_TARGET_ENTRY);
}


//!
//! @brief Selects all the text in the current widget
//! 
//! This function makes the selected dictionary in the dictionarylist match
//! the dictionary of the widget that was modified.  The selected dictionary
//! in the dictionarylist acts as the central reference for the GUI.
//!
//! @see gw_main_cut_cb ()
//! @see gw_main_copy_cb ()
//! @see gw_main_paste_cb ()
//! @see gw_main_update_clipboard_on_focus_change_cb ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_main_select_all_cb (GtkWidget *widget, gpointer data)
{
    guint TARGET;
    TARGET = gw_main_get_current_target_focus ("main_window");

    gw_main_text_select_all_by_target (TARGET);
}


//!
//! @brief Pastes text into the current widget
//! 
//! This function makes the selected dictionary in the dictionarylist match
//! the dictionary of the widget that was modified.  The selected dictionary
//! in the dictionarylist acts as the central reference for the GUI.
//!
//! @see gw_main_cut_cb ()
//! @see gw_main_copy_cb ()
//! @see gw_main_select_all_cb ()
//! @see gw_main_update_clipboard_on_focus_change_cb ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_main_paste_cb (GtkWidget *widget, gpointer data)
{
    guint TARGET;
    TARGET = gw_main_get_current_target_focus ("main_window");
    gw_main_paste_text (TARGET);
}


//!
//! @brief Cuts text from the current widget
//! 
//! This function makes the selected dictionary in the dictionarylist match
//! the dictionary of the widget that was modified.  The selected dictionary
//! in the dictionarylist acts as the central reference for the GUI.
//!
//! @see gw_main_paste_cb ()
//! @see gw_main_copy_cb ()
//! @see gw_main_select_all_cb ()
//! @see gw_main_update_clipboard_on_focus_change_cb ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_main_cut_cb (GtkWidget *widget, gpointer data)
{
    guint TARGET;
    TARGET = gw_main_get_current_target_focus ("main_window");
    gw_main_cut_text (TARGET);
}


//!
//! @brief Pastes text into the current widget
//! 
//! This function makes the selected dictionary in the dictionarylist match
//! the dictionary of the widget that was modified.  The selected dictionary
//! in the dictionarylist acts as the central reference for the GUI.
//!
//! @see gw_main_cut_cb ()
//! @see gw_main_paste_cb ()
//! @see gw_main_select_all_cb ()
//! @see gw_main_update_clipboard_on_focus_change_cb ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_main_copy_cb (GtkWidget *widget, gpointer data)
{
    guint TARGET;
    TARGET = gw_main_get_current_target_focus ("main_window");
    gw_main_copy_text (TARGET);
}


//!
//! @brief Manages the required changes for focus in different elements
//! 
//! Depending if the object's text is editable or not, the clipboard will
//! update button states approprately and connect the signal handlers to the
//! approprate widgets.
//!
//! @see gw_main_cut_cb ()
//! @see gw_main_copy_cb ()
//! @see gw_main_paste_cb ()
//! @see gw_main_select_all_cb ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//! @return Always returns false
//!
G_MODULE_EXPORT gboolean gw_main_update_clipboard_on_focus_change_cb (GtkWidget        *widget, 
                                                                      GtkDirectionType  arg1,
                                                                      gpointer          data   ) 
{
    GtkBuilder *builder = gw_common_get_builder ();

    gw_main_close_suggestion_box ();
    guint TARGET;
    TARGET = gw_main_get_current_target_focus ("main_window");

    //Set up the references to the actions
    GtkAction *copy_action, *cut_action, *paste_action, *select_all_action;

    char id[50];
    strncpy (id, "edit_copy_action", 50);
    copy_action       = GTK_ACTION (gtk_builder_get_object (builder, id));
    strncpy (id, "edit_cut_action", 50);
    cut_action        = GTK_ACTION (gtk_builder_get_object (builder, id));
    strncpy (id, "edit_paste_action", 50);
    paste_action      = GTK_ACTION (gtk_builder_get_object (builder, id));
    strncpy (id, "edit_select_all_action", 50);
    select_all_action = GTK_ACTION (gtk_builder_get_object (builder, id));

    //Disconnected old handlers
    if (copy_handler_id != 0)
    {
      g_signal_handler_disconnect (copy_action, copy_handler_id);
      copy_handler_id = 0;
    }
    if (cut_handler_id != 0)
    {
      g_signal_handler_disconnect (cut_action, cut_handler_id);
      cut_handler_id = 0;
    }
    if (paste_handler_id != 0)
    {
      g_signal_handler_disconnect (paste_action, paste_handler_id);
      paste_handler_id = 0;
    }
    if (select_all_handler_id != 0)
    {
      g_signal_handler_disconnect (select_all_action, select_all_handler_id);
      select_all_handler_id = 0;
    }
                                          
    //Create new ones pointed at the correct widget
    copy_handler_id = g_signal_connect       ( copy_action,
                                               "activate",
                                               G_CALLBACK (gw_main_copy_cb),
                                               widget                      );
    cut_handler_id = g_signal_connect        ( cut_action,
                                               "activate",
                                               G_CALLBACK (gw_main_cut_cb),
                                               widget                      );
    paste_handler_id = g_signal_connect      ( paste_action,
                                               "activate",
                                               G_CALLBACK (gw_main_paste_cb),
                                               widget                      );
    select_all_handler_id = g_signal_connect ( select_all_action,
                                               "activate",
                                               G_CALLBACK (gw_main_select_all_cb),
                                               widget                      );


    //Correct the sensitive state to paste
    if (gw_common_widget_equals_target (data, GW_TARGET_RESULTS) ||
        gw_common_widget_equals_target (data, GW_TARGET_KANJI)     )
      gtk_action_set_sensitive (GTK_ACTION (paste_action), FALSE);
    else
      gtk_action_set_sensitive (GTK_ACTION (paste_action), TRUE);

    return FALSE;
}


//!
//! @brief Prints the current results
//! 
//! When this function is called, the current results on screen are printed.
//! If a section of the results is selected, only that is printed.
//!
//! @see gw_print ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_main_print_cb (GtkWidget *widget, gpointer data)
{
    gw_main_tab_cancel_all_searches ();
    gw_print ();
}


//!
//! @brief Opens the saved vocab list in your default editor
//! 
//! If the user saved a vocab list using the save as or append functions, this
//! action becomes available where the file is opened in the user's default
//! text editor.
//!
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_main_edit_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    char *uri;
    GError *error;
    const char *savepath;
    GtkWidget *window;
    GtkBuilder *builder;

    //Initializations
    savepath = lw_io_get_savepath ();
    uri = g_build_filename ("file://", savepath, NULL);
    builder = gw_common_get_builder ();
    window = GTK_WIDGET (gtk_builder_get_object (builder, "main_window"));
    error = NULL;

    gtk_show_uri (NULL, uri, gtk_get_current_event_time (), &error);

    gw_common_handle_error (&error, GTK_WINDOW (window), TRUE);

    //Cleanup
    g_free (uri);
}


//!
//! @brief Sends the user to the gWaei irc channel for help
//! 
//! The IRC uri should open in the user's default IRC client.
//!
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_main_irc_channel_cb (GtkWidget *widget, gpointer data)
{
    //Initializations
    GError *error;
    GtkWidget *window;
    GtkBuilder *builder;

    //Declarations
    error = NULL;
    builder = gw_common_get_builder ();
    window = GTK_WIDGET (gtk_builder_get_object (builder, "main_window"));

    gtk_show_uri (NULL, "irc://irc.freenode.net/gWaei", gtk_get_current_event_time (), &error);

    //Cleanup
    gw_common_handle_error (&error, GTK_WINDOW (window), TRUE);
}


//!
//! @brief Sends the user to the gWaei homepage for whatever they need
//! 
//! The homepage should open in their default browser.
//!
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_main_homepage_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GError *error;
    GtkWidget *window;
    GtkBuilder *builder;

    //Initializations
    error = NULL;
    builder = gw_common_get_builder ();
    window = GTK_WIDGET (gtk_builder_get_object (builder, "main_window"));

    gtk_show_uri (NULL, "http://gwaei.sourceforge.net/", gtk_get_current_event_time (), &error);

    //Cleanup
    gw_common_handle_error (&error, GTK_WINDOW (window), TRUE);
}


//!
//! @brief Opens the gWaei help documentation
//!
//! The gWaei help documentation is opened in the user's default help program.
//!
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_main_help_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GError *error;
    GtkWidget *window;
    GtkBuilder *builder;

    //Initializations
    error = NULL;
    builder = gw_common_get_builder ();
    window = GTK_WIDGET (gtk_builder_get_object (builder, "main_window"));

    gtk_show_uri (NULL, "ghelp:gwaei", gtk_get_current_event_time (), &error);

    //Cleanup
    gw_common_handle_error (&error, GTK_WINDOW (window), TRUE);
}


//!
//! @brief Opens the gWaei dictionary glossary help documentation
//!
//! The gWaei dictionary glossary help documentation is opened in the user's
//! default help program.
//!
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_main_glossary_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    char *uri;
    GError *error;
    GtkBuilder *builder;
    GtkWidget *window;

    //Initializations
    uri = g_build_filename ("ghelp://", DATADIR2, "gnome", "help", "gwaei", "C", "glossary.xml", NULL);
    builder = gw_common_get_builder ();
    window = GTK_WIDGET (gtk_builder_get_object (builder, "main_window"));
    error = NULL;

    gtk_show_uri (NULL, uri, gtk_get_current_event_time (), &error);

    //Cleanup
    gw_common_handle_error (&error, GTK_WINDOW (window), TRUE);
    g_free (uri);
}


//!
//! @brief Opens the gWaei about dialog
//!
//! The gWaei help dialog is displayed, showing the credits of everyone who has
//! helped to make this program possible.
//!
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_main_about_cb (GtkWidget *widget, gpointer data)
{
    char *global_path = DATADIR2 G_DIR_SEPARATOR_S PACKAGE G_DIR_SEPARATOR_S "logo.png";
    char *local_path = ".." G_DIR_SEPARATOR_S "share" G_DIR_SEPARATOR_S PACKAGE G_DIR_SEPARATOR_S "logo.png";

    char *programmer_credits[] = 
    {
      "Zachary Dovel <pizzach@gmail.com>",
      "Fabrizio Sabatini",
      NULL
    };

    GdkPixbuf *logo;
    if ( (logo = gdk_pixbuf_new_from_file (global_path,    NULL)) == NULL &&
         (logo = gdk_pixbuf_new_from_file (local_path, NULL)) == NULL    )
    {
      printf ("Was unable to load the gwaei logo.\n");
    }

    GtkWidget *about = g_object_new (GTK_TYPE_ABOUT_DIALOG,
               "program-name", "gWaei", 
               "version", VERSION,
               "copyright", "gWaei (C) 2008-2010 Zachary Dovel\nKanjipad backend (C) 2002 Owen Taylor\nJStroke backend (C) 1997 Robert Wells",
               "comments", gettext("Program for Japanese translation and reference. The\ndictionaries are supplied by Jim Breen's WWWJDIC.\nSpecial thanks to the maker of GJITEN who served as an inspiration.\n Dedicated to Chuus"),
               "license", "This software is GPL Licensed.\n\ngWaei is free software: you can redistribute it and/or modify\nit under the terms of the GNU General Public License as published by\n the Free Software Foundation, either version 3 of the License, or\n(at your option) any later version.\n\ngWaei is distributed in the hope that it will be useful,\nbut WITHOUT ANY WARRANTY; without even the implied warranty of\nMERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\nGNU General Public License for more details.\n\nYou should have received a copy of the GNU General Public License\nalong with gWaei.  If not, see <http://www.gnu.org/licenses/>.",
               "logo", logo,
               // TRANSLATORS: You can add your own name to the translation of this field, it will be displayed in the "about" box when gwaei is run in your language
               "translator-credits", gettext("translator-credits"),
               "authors", programmer_credits,
               "website", "http://gwaei.sourceforge.net/",
               NULL);
    gtk_dialog_run (GTK_DIALOG (about));
    g_object_unref (logo);
    gtk_widget_destroy (about);
}


//!
//! @brief Cycles the active dictionaries down the list
//!
//! This function cycles the dictionaries down the list.  If it reaches the
//! end, it will loop back to the top.
//!
//! @see gw_main_cycle_dictionaries_backward_cb ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_main_cycle_dictionaries_forward_cb (GtkWidget *widget, gpointer data)
{
    gw_main_cycle_dictionaries (TRUE);
}


//!
//! @brief Cycles the active dictionaries up the list
//!
//! This function cycles the dictionaries up the list.  If it reaches the
//! end, it will loop back to the bottom.
//!
//! @see gw_main_cycle_dictionaries_forward_cb ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_main_cycle_dictionaries_backward_cb (GtkWidget *widget, gpointer data)
{
    gw_main_cycle_dictionaries (FALSE);
}


//!
//! @brief Update the special key press status
//!
//! Currently used to determine if a search should be opened in a new tab.
//!
//! @param widget Unused GtkWidget pointer
//! @param event the event data to get the specific key that had it's status modified
//! @param data Currently unused gpointer
//! @return Always returns FALSE
//!
G_MODULE_EXPORT gboolean gw_main_key_press_modify_status_update_cb (GtkWidget *widget,
                                                                    GdkEvent  *event,
                                                                    gpointer  *data  )
{
    GtkWidget *tv = GTK_WIDGET (gw_common_get_widget_by_target (GW_TARGET_RESULTS));
    GtkWidget *window = GTK_WIDGET (gtk_widget_get_tooltip_window (tv));
    if (window != NULL) 
    {
      gtk_widget_destroy (window);
      gtk_widget_set_tooltip_window (tv, NULL);
    }

    guint keyval = ((GdkEventKey*)event)->keyval;
    GtkWidget* search_entry = gw_common_get_widget_by_target (GW_TARGET_ENTRY);

    if ((keyval == GDK_KEY_ISO_Enter || keyval == GDK_KEY_Return) && gtk_widget_is_focus (search_entry))
    {
      gtk_widget_activate (search_entry);
      return FALSE;
    }

    if (keyval == GDK_KEY_Shift_L || keyval == GDK_KEY_Shift_R || keyval == GDK_KEY_ISO_Next_Group || keyval == GDK_KEY_ISO_Prev_Group)
    {
      start_search_in_new_window = TRUE;
    }

    return FALSE;
}


//!
//! @brief Update the special key release status
//!
//! Currently used to determine if a search should be opened in a new tab.
//!
//! @param widget Unused GtkWidget pointer
//! @param event the event data to get the specific key that had it's status modified
//! @param data Currently unused gpointer
//! @return Always returns FALSE
//!
G_MODULE_EXPORT gboolean gw_main_key_release_modify_status_update_cb (GtkWidget *widget,
                                                                      GdkEvent  *event,
                                                                      gpointer  *data  )
{
    guint keyval = ((GdkEventKey*)event)->keyval;
    if (keyval == GDK_KEY_Shift_L || keyval == GDK_KEY_Shift_R || keyval == GDK_KEY_ISO_Next_Group || keyval == GDK_KEY_ISO_Prev_Group)
    {
      start_search_in_new_window = FALSE;
    }

    return FALSE;
}


//!
//! @brief Function handles automatic focus changes on key presses
//!
//! When the user types a letter, the focus will move to the search entry and
//! auto-highlight the results so you can start typing immediately.  If the
//! user hits an arrow key or pageup/pagedown, the focus will move to the
//! search results so they can scroll them.
//!
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//! @return Always returns false
//!
G_MODULE_EXPORT gboolean gw_main_focus_change_on_key_press_cb (GtkWidget *widget,
                                                       GdkEvent  *event,
                                                       gpointer  *focus  )
{
    gw_main_close_suggestion_box ();
    guint state = ((GdkEventKey*)event)->state;
    guint keyval = ((GdkEventKey*)event)->keyval;
    guint modifiers = ( 
                        GDK_MOD1_MASK    |
                        GDK_CONTROL_MASK |
                        GDK_SUPER_MASK   |
                        GDK_HYPER_MASK   |
                        GDK_META_MASK    |
                        GDK_KEY_Meta_L       |
                        GDK_KEY_Meta_R       |
                        GDK_KEY_Alt_L        |
                        GDK_KEY_Alt_R
                      );

    //Make sure no modifier keys are pressed
    if (
          (state & modifiers) == 0   &&
          keyval != GDK_KEY_Tab          &&
          keyval != GDK_KEY_ISO_Left_Tab &&
          keyval != GDK_KEY_Shift_L      &&
          keyval != GDK_KEY_Shift_R      &&
          keyval != GDK_KEY_Control_L    &&
          keyval != GDK_KEY_Control_R    &&
          keyval != GDK_KEY_Caps_Lock    &&
          keyval != GDK_KEY_Shift_Lock   &&
          keyval != GDK_KEY_Meta_L       &&
          keyval != GDK_KEY_Meta_R       &&
          keyval != GDK_KEY_Alt_L        &&
          keyval != GDK_KEY_Alt_R        &&
          keyval != GDK_KEY_Super_L      &&
          keyval != GDK_KEY_Super_R      &&
          keyval != GDK_KEY_Hyper_L      &&
          keyval != GDK_KEY_Hyper_R      &&
          keyval != GDK_KEY_Num_Lock     &&
          keyval != GDK_KEY_Scroll_Lock  &&
          keyval != GDK_KEY_Pause        &&
          keyval != GDK_KEY_Home         &&
          keyval != GDK_KEY_End
       )
    {
      //Change focus to the text view if is an arrow key or page key
      if ( 
           ( keyval == GDK_KEY_Up        ||
             keyval == GDK_KEY_Down      ||
             keyval == GDK_KEY_Page_Up   ||
             keyval == GDK_KEY_Page_Down   
           ) &&
           (
             !gw_common_widget_equals_target (widget, GW_TARGET_RESULTS)
           )
         )
      {
        gw_main_text_select_none_by_target (GW_TARGET_ENTRY);
        gw_main_grab_focus_by_target (GW_TARGET_RESULTS);
        return TRUE;
      }

      //Change focus to the entry if other key
      else if (
                keyval != GDK_KEY_Up        &&
                keyval != GDK_KEY_Down      &&
                keyval != GDK_KEY_Page_Up   &&
                keyval != GDK_KEY_Page_Down &&
                !gw_common_widget_equals_target (widget, GW_TARGET_ENTRY)
              )
      {
        gw_main_text_select_all_by_target (GW_TARGET_ENTRY);
        gw_main_grab_focus_by_target (GW_TARGET_ENTRY);
        return TRUE;
      }
    }

    return FALSE;
}


//!
//! @brief Initiates a search on the user's typed query
//!
//! This function does the needed work to check the query for basic
//! correctness, shift the previously completed search to the history list,
//! creates the searchitem, and then initiates the search.
//!
//! @see gw_main_search_from_history_cb ()
//! @see lw_search_get_results ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_main_search_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    gchar query[100];
    LwSearchItem *item, *new_item;
    LwDictInfo *di;
    GError *error;

    //Initializations
    error = NULL;
    gw_main_strncpy_text_from_widget_by_target (query, GW_TARGET_ENTRY, 100);

    item = gw_tabs_get_searchitem ();
    di = lw_dictinfolist_get_selected_dictinfo ();

    new_item = lw_searchitem_new (query, di, GW_TARGET_RESULTS, &error);

    //Check for problems, and quit if there are
    if (error != NULL || new_item == NULL || lw_searchitem_is_equal (item, new_item) || !gw_main_cancel_search_by_searchitem (item))
    {
      lw_searchitem_increment_history_relevance_timer (item);
      lw_searchitem_free (new_item);

      if (error != NULL)
      {
        printf("%s\n", error->message);
        g_error_free (error);
      }

      return;
    }

    //Move the previous searchitem to the history or destroy it
    if (lw_searchitem_has_history_relevance (item))
      lw_historylist_add_searchitem_to_history (GW_HISTORYLIST_RESULTS, item);
    else
      lw_searchitem_free (item);

    //Add the needed references for the new search item
    gw_tabs_set_searchitem (new_item);

    //Start the search
    lw_engine_get_results (new_item, TRUE, FALSE);

    //Update the interface
    gw_main_update_toolbar_buttons ();
    gw_main_update_history_popups ();
}


//!
//! @brief Inserts an unknown regex character into the entry
//!
//! Used to help users discover regex searches.  It just insert a period
//! wherever the cursor presently is in the search entry.
//!
//! @see gw_main_insert_word_edge_cb ()
//! @see gw_main_insert_not_word_edge_cb ()
//! @see gw_main_insert_and_cb ()
//! @see gw_main_insert_or_cb ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_main_insert_unknown_character_cb (GtkWidget *widget, gpointer data)
{
    gw_main_search_entry_insert (".");
}


//!
//! @brief Inserts an a word-boundary regex character into the entry
//!
//! Used to help users discover regex searches.  It just insert \\b
//! wherever the cursor presently is in the search entry.
//!
//! @see gw_main_insert_unknown_character_cb ()
//! @see gw_main_insert_not_word_edge_cb ()
//! @see gw_main_insert_and_cb ()
//! @see gw_main_insert_or_cb ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_main_insert_word_edge_cb (GtkWidget *widget, gpointer data)
{
    gw_main_search_entry_insert ("\\b");
}


//!
//! @brief Inserts an a not-word-boundary regex character into the entry
//!
//! Used to help users discover regex searches.  It just insert \\B
//! wherever the cursor presently is in the search entry.
//!
//! @see gw_main_insert_unknown_character_cb ()
//! @see gw_main_insert_word_edge_cb ()
//! @see gw_main_insert_and_cb ()
//! @see gw_main_insert_or_cb ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_main_insert_not_word_edge_cb (GtkWidget *widget, gpointer data)
{
    gw_main_search_entry_insert ("\\B");
}


//!
//! @brief Inserts an an and regex character into the entry
//!
//! Used to help users discover regex searches.  It just insert &
//! wherever the cursor presently is in the search entry.
//!
//! @see gw_main_insert_unknown_character_cb ()
//! @see gw_main_insert_word_edge_cb ()
//! @see gw_main_insert_not_word_edge_cb ()
//! @see gw_main_insert_or_cb ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_main_insert_and_cb (GtkWidget *widget, gpointer data)
{
    gw_main_search_entry_insert ("&");
}


//!
//! @brief Inserts an an or regex character into the entry
//!
//! Used to help users discover regex searches.  It just insert |
//! wherever the cursor presently is in the search entry.
//!
//! @see gw_main_insert_unknown_character_cb ()
//! @see gw_main_insert_word_edge_cb ()
//! @see gw_main_insert_not_word_edge_cb ()
//! @see gw_main_insert_and_cb ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_main_insert_or_cb (GtkWidget *widget, gpointer data)
{
    gw_main_search_entry_insert ("|");
}


//!
//! @brief Clears the search entry and moves the focus to it
//!
//! This function acts as a quick way for the user to get back to the search
//! entry and do another search whereever they are.
//!
//! @see gw_main_clear_search_entry ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_main_clear_search_cb (GtkWidget *widget, gpointer data)
{
    gw_main_clear_search_entry ();
    gw_main_grab_focus_by_target (GW_TARGET_ENTRY);
}


//!
//! @brief Opens the dictionary folder using the user's default file browser
//!
//! The dictionary folder that is opened is generally in "~/.waei".
//!
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_main_open_dictionary_folder_cb (GtkWidget *widget, gpointer data) 
{
    //Declarations
    const char *directory;
    char *uri;
    GError *error;
    GtkBuilder *builder;
    GtkWidget *window;

    //Initializations
    directory = lw_util_get_directory (GW_PATH_DICTIONARY);
    uri = g_build_filename ("file://", directory, NULL);
    builder = gw_common_get_builder ();
    window = GTK_WIDGET (gtk_builder_get_object (builder, "main_window"));
    error = NULL;

    gtk_show_uri (NULL, uri, gtk_get_current_event_time (), &error);

    gw_common_handle_error (&error, GTK_WINDOW (window), TRUE);

    g_free (uri);
}


//!
//! @brief Sets the drag icon to the cursor if the widget is dragged over
//!
//! Part of a group of four functions to handle drag drops of text into
//! the main text buffer which will initialize a search based on that text.
//!
//! @see gw_main_search_drag_data_recieved_cb ()
//! @see gw_main_drag_leave_1_cb ()
//! @see gw_main_drag_drop_1_cb ()
//! @return Always returns true
//!
G_MODULE_EXPORT gboolean gw_main_drag_motion_1_cb (GtkWidget      *widget,
                                                   GdkDragContext *drag_context,
                                                   gint            x,
                                                   gint            y,
                                                   guint           time,
                                                   gpointer        user_data)
{
    gdk_drag_status (drag_context, GDK_ACTION_COPY, time);
    gtk_drag_highlight (widget);
    return TRUE;
}


//!
//! @brief Resets the gui when the drag leaves the widget area
//!
//! Part of a group of four functions to handle drag drops of text into
//! the main text buffer which will initialize a search based on that text.
//!
//! @see gw_main_search_drag_data_recieved_cb ()
//! @see gw_main_drag_drop_1_cb ()
//! @see gw_main_drag_motion_1_cb ()
//!
G_MODULE_EXPORT void gw_main_drag_leave_1_cb (GtkWidget      *widget,
                                      GdkDragContext *drag_context,
                                      guint           time,
                                      gpointer        user_data) 
{
    gtk_drag_unhighlight (widget);
}


//!
//! @brief Tells the widget to recieve the dragged data
//!
//! Part of a group of four functions to handle drag drops of text into
//! the main text buffer which will initialize a search based on that text.
//!
//! @see gw_main_search_drag_data_recieved_cb ()
//! @see gw_main_drag_leave_1_cb ()
//! @see gw_main_drag_motion_1_cb ()
//! @return Always returns true
//!
G_MODULE_EXPORT gboolean gw_main_drag_drop_1_cb (GtkWidget      *widget,
                                         GdkDragContext *drag_context,
                                         gint            x,
                                         gint            y,
                                         guint           time,
                                         gpointer        user_data)  
{
    GdkAtom target;
    target = gtk_drag_dest_find_target (widget, drag_context, NULL);
    gtk_drag_get_data (widget, drag_context, target, time);
    return TRUE;
}


//!
//! @brief The widget recieves the data and starts a search based on it.
//!
//! Part of a group of four functions to handle drag drops of text into
//! the main text buffer which will initialize a search based on that text.
//!
//! @see gw_main_drag_leave_1_cb ()
//! @see gw_main_drag_drop_1_cb ()
//! @see gw_main_drag_motion_1_cb ()
//!
G_MODULE_EXPORT void gw_main_search_drag_data_recieved_cb (GtkWidget        *widget,
                                                           GdkDragContext   *drag_context,
                                                           gint              x,
                                                           gint              y,
                                                           GtkSelectionData *data,
                                                           guint             info,
                                                           guint             time,
                                                           gpointer          user_data    )
{
    //Sanity checks
    if (widget == NULL) return;
    const char *name = gtk_buildable_get_name (GTK_BUILDABLE (widget));
    if (name == NULL || strcmp (name, "search_entry") == 0)
      return;

    //Declarations
    GtkWidget* entry;
    char* text;

    //Initializations
    entry = gw_common_get_widget_by_target (GW_TARGET_ENTRY);
    text = (char*) gtk_selection_data_get_text (data);   

    if (text != NULL && strlen(text) > 0)
    {
      gw_main_clear_search_cb (entry, NULL);
      gtk_entry_set_text (GTK_ENTRY (entry), text);
      gw_main_search_cb (NULL, NULL);

      gdk_drag_status (drag_context, GDK_ACTION_COPY, time);
      gtk_drag_finish (drag_context, TRUE, FALSE, time);
    }
    else
    {
      gtk_drag_finish (drag_context, FALSE, FALSE, time);
    }

    //Cleanup
    if (text != NULL) g_free (text);
}


//!
//! @brief Hides/shows buttons depending on search entry text
//!
//! Currently this function just hides and shows the clear icon depending if
//! there is any text in the entry.  Previously, this would also set the search
//! button in it's insensitive state also.
//!
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_main_update_button_states_based_on_entry_text_cb (GtkWidget *widget,
                                                                          gpointer   data   )
{
    //Declarations
    GtkWidget *entry;
    int length;

    //Initializations
    entry = gw_common_get_widget_by_target (GW_TARGET_ENTRY);
    length = gtk_entry_get_text_length (GTK_ENTRY (entry));

    //Show the clear icon when approprate
    if (length > 0)
      gtk_entry_set_icon_from_stock (GTK_ENTRY (entry), GTK_ENTRY_ICON_SECONDARY, GTK_STOCK_CLEAR);
    else
      gtk_entry_set_icon_from_stock (GTK_ENTRY (entry), GTK_ENTRY_ICON_SECONDARY, NULL);

    //Return widget colors back to normal
    gtk_widget_override_background_color (GTK_WIDGET (entry), GTK_STATE_NORMAL, NULL);
    gtk_widget_override_color (GTK_WIDGET (entry), GTK_STATE_NORMAL, NULL);
}


//!
//! @brief Populates the main contextual menu with search options
//!
//! @param entry The GtkTexView that was right clicked
//! @param menu The Popup menu to populate
//! @param data  Currently unused gpointer containing user data
//!
void gw_main_populate_popup_with_search_options_cb (GtkTextView *entry, GtkMenu *menu, gpointer data)
{
    if (hovered_word == NULL) return;

    //Declarations
    LwSearchItem *item = NULL;
    LwDictInfo *di = NULL;
    char *menu_text = NULL;
    GtkWidget *menuitem = NULL;
    GtkWidget *menuimage = NULL;
    gchar *selected_text = NULL;
    char *query_text = NULL;
    char *search_for_menuitem_text;
    char *websearch_for_menuitem_text;
    char *othersearch_for_menuitem_text;
    GObject* tb = NULL;
    GtkTextIter start_iter, end_iter;
    GList *list_selected = NULL;
    LwDictInfo *di_selected = NULL;
    int i = 0;

    //Initializations
    tb = gw_common_get_gobject_by_target (GW_TARGET_RESULTS);
    // TRANSLATORS: The first variable is the expression to look for, the second is the dictionary full name
    search_for_menuitem_text = gettext("Search for \"%s\" in the %s");
    // TRANSLATORS: The variable is the expression to look for
    othersearch_for_menuitem_text = gettext("Search for \"%s\" in a Different Dictionary");
    // TRANSLATORS: The variable is the expression to look for
    websearch_for_menuitem_text = gettext("Cross-reference \"%s\" Online");

    menuitem = gtk_separator_menu_item_new ();
    gtk_menu_shell_prepend (GTK_MENU_SHELL (menu), GTK_WIDGET (menuitem));
    gtk_widget_show (GTK_WIDGET (menuitem));
    if (gtk_text_buffer_get_has_selection (GTK_TEXT_BUFFER (tb)))
    {
      gtk_text_buffer_get_selection_bounds (GTK_TEXT_BUFFER (tb), &start_iter, &end_iter);
      query_text = gtk_text_buffer_get_text (GTK_TEXT_BUFFER (tb), &start_iter, &end_iter, FALSE);
      if (g_utf8_strchr(query_text, -1, L'\n') != NULL) query_text = NULL;
    }
    if (query_text == NULL)
    {
      query_text = hovered_word;
    }
    list_selected = lw_dictinfolist_get_selected();
    di_selected = list_selected->data;


    //Add webpage links
    GList* list =  lw_dictinfolist_get_dict_by_load_position (0);
    di = list->data;
    char *website_url_menuitems[] = {
      "Wikipedia", "http://www.wikipedia.org/wiki/%s", "wikipedia.png",
      "Goo.ne.jp", "http://dictionary.goo.ne.jp/srch/all/%s/m0u/", "goo.png",
      "Google.com", "http://www.google.com/search?q=%s", "google.png",
      NULL, NULL, NULL
    };

    //Setup the web submenu
    GtkWidget *web_menu = NULL;
    GtkWidget *web_menuitem = NULL;
    menu_text = g_strdup_printf (websearch_for_menuitem_text, query_text);
    if (menu_text != NULL)
    {
      web_menu = gtk_menu_new();
      web_menuitem = gtk_menu_item_new_with_label (menu_text);
      g_free (menu_text);
      menu_text = NULL;
    }
    else
    {
      web_menuitem = gtk_menu_item_new_with_label ("crossreference on the web");
    }
    gtk_menu_shell_prepend (GTK_MENU_SHELL (menu), GTK_WIDGET (web_menuitem));
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (web_menuitem), GTK_WIDGET (web_menu));
    gtk_widget_show (web_menuitem);
    gtk_widget_show (web_menu);

    i = 0;
    while (website_url_menuitems[i] != NULL)
    {
      if (di != NULL && (item = lw_searchitem_new (query_text, di, GW_TARGET_RESULTS, NULL)) != NULL)
      {
        //Create handy variables
        char *name = website_url_menuitems[i];
        char *url = g_strdup_printf(website_url_menuitems[i + 1], query_text);
        if (url != NULL)
        {
          g_free (item->queryline->string);
          item->queryline->string = g_strdup (url);
          g_free (url);
          url = NULL;
        }
        char *icon_path = website_url_menuitems[i + 2];

        //Start creating
        menu_text = g_strdup_printf ("%s", name);
        if (menu_text != NULL)
        {
          menuitem = GTK_WIDGET (gtk_image_menu_item_new_with_label (menu_text));
          char *path = g_build_filename (DATADIR2, PACKAGE, icon_path, NULL);
          if (path != NULL)
          {
            menuimage = gtk_image_new_from_file (path);
            gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem), GTK_WIDGET (menuimage));
            g_free (path);
            path = NULL;
          }
          g_signal_connect (G_OBJECT (menuitem), "activate", G_CALLBACK (gw_main_search_for_searchitem_online_cb), item);
          g_signal_connect (G_OBJECT (menuitem), "destroy",  G_CALLBACK (gw_tabs_destroy_tab_menuitem_searchitem_data_cb), item);
          gtk_menu_shell_append (GTK_MENU_SHELL (web_menu), GTK_WIDGET (menuitem));
          gtk_widget_show (GTK_WIDGET (menuitem));
          gtk_widget_show (GTK_WIDGET (menuimage));
          g_free (menu_text);
          menu_text = NULL;
        }
      }
      i += 3;
    }


    //Setup the submenu
    GtkWidget *dictionaries_menu = NULL;
    GtkWidget *dictionaries_menuitem = NULL;
    menu_text = g_strdup_printf (othersearch_for_menuitem_text, query_text);
    if (menu_text != NULL)
    {
      dictionaries_menu = gtk_menu_new();
      dictionaries_menuitem = gtk_menu_item_new_with_label (menu_text);
      g_free (menu_text);
      menu_text = NULL;
    }
    else
    {
      dictionaries_menuitem = gtk_menu_item_new_with_label ("Search for this in a different dictionary");
    }
    gtk_menu_shell_prepend (GTK_MENU_SHELL (menu), GTK_WIDGET (dictionaries_menuitem));
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (dictionaries_menuitem), GTK_WIDGET (dictionaries_menu));
    gtk_widget_show (dictionaries_menuitem);
    gtk_widget_show (dictionaries_menu);

    //Add internal dictionary links
    i = 0;
    while ((list = lw_dictinfolist_get_dict_by_load_position (i)) != NULL)
    {
      list = lw_dictinfolist_get_dict_by_load_position (i);
      di = list->data;
      if (di != NULL && (item = lw_searchitem_new (query_text, di, GW_TARGET_RESULTS, NULL)) != NULL)
      {
        if (di == di_selected)
        {
          menu_text = g_strdup_printf (search_for_menuitem_text, item->queryline->string, di->longname);
          if (menu_text != NULL)
          {
            menuitem = GTK_WIDGET (gtk_image_menu_item_new_with_label (menu_text));
            menuimage = gtk_image_new_from_icon_name ("stock_new-tab", GTK_ICON_SIZE_MENU);
            gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem), GTK_WIDGET (menuimage));
            g_signal_connect (G_OBJECT (menuitem), "activate", G_CALLBACK (gw_tabs_new_with_search_cb), item);
            gtk_menu_shell_prepend (GTK_MENU_SHELL (menu), GTK_WIDGET (menuitem));
            gtk_widget_show (GTK_WIDGET (menuitem));
            gtk_widget_show (GTK_WIDGET (menuimage));
            g_free (menu_text);
            menu_text = NULL;
          }
        }
        menu_text = g_strdup_printf ("%s", di->longname);
        if (menu_text != NULL)
        {
          menuitem = GTK_WIDGET (gtk_image_menu_item_new_with_label (menu_text));
          g_signal_connect (G_OBJECT (menuitem), "activate", G_CALLBACK (gw_tabs_new_with_search_cb), item);
          g_signal_connect (G_OBJECT (menuitem), "destroy",  G_CALLBACK (gw_tabs_destroy_tab_menuitem_searchitem_data_cb), item);
          gtk_menu_shell_append (GTK_MENU_SHELL (dictionaries_menu), GTK_WIDGET (menuitem));
          gtk_widget_show (GTK_WIDGET (menuitem));
          gtk_widget_show (GTK_WIDGET (menuimage));
          g_free (menu_text);
          menu_text = NULL;
        }
      }
      i++;
    }
}


//!
//! @brief Searches for the word in a webbrower in an external dictionary
//! 
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_main_search_for_searchitem_online_cb (GtkWidget *widget, gpointer data)
{
    LwSearchItem *item = (LwSearchItem*) data;
    GtkBuilder *builder;
    GtkWidget *window;
    GError *error;

    if (item != NULL)
    {
      error = NULL;
      gtk_show_uri (NULL, item->queryline->string, gtk_get_current_event_time (), &error);
      builder = gw_common_get_builder ();
      window = GTK_WIDGET (gtk_builder_get_object (builder, "main_window"));

      gw_common_handle_error (&error, GTK_WINDOW (window), TRUE);
    }
}

//!
//! @brief Emulates web browsers font size control with (ctrl + wheel)
//!
//! @param widget Unused GtkWidget pointer.
//! @param data Unused gpointer
//!
G_MODULE_EXPORT gboolean gw_main_scroll_or_zoom_cb (GtkWidget *widget, GdkEventScroll *event, gpointer data)
{
    // If "control" is being pressed
    if( (event->state & GDK_CONTROL_MASK) == GDK_CONTROL_MASK )
    {
	// On wheel direction up ~ zoom out
	if(event->direction == GDK_SCROLL_UP)
	{
	  gw_main_zoom_out_cb (widget, data);
	  return TRUE; // dont propagate event, no scroll
	}

	// On wheel direction down ~ zoom in
	if(event->direction == GDK_SCROLL_DOWN)
	{
	  gw_main_zoom_in_cb (widget, data);
	  return TRUE; // dont propagate event, no scroll
	}
    }

    // return false and propagate event for regular scroll
    return FALSE;
}


