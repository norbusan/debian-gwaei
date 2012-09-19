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
//! @file src/gtk-main-callbacks.c
//!
//! @brief Abstraction layer for gtk callbacks
//!
//! Callbacks for activities initiated by the user. Most of the gtk code here
//! should still be abstracted to the interface C file when possible.
//!


#include <string.h>
#include <regex.h>
#include <stdlib.h>
#include <libintl.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include <gwaei/definitions.h>
#include <gwaei/dictionary-objects.h>
#include <gwaei/regex.h>
#include <gwaei/utilities.h>
#include <gwaei/io.h>
#include <gwaei/search-objects.h>
#include <gwaei/preferences.h>

#include <gwaei/engine.h>

#include <gwaei/gtk.h>
#include <gwaei/gtk-printing.h>
#include <gwaei/gtk-main-interface.h>
#include <gwaei/gtk-main-interface-tabs.h>
#include <gwaei/gtk-main-callbacks.h>
#include <gwaei/gtk-radicals-callbacks.h>
#include <gwaei/gtk-settings-interface.h>


static gint button_press_x = 0;
static gint button_press_y = 0;
static gulong copy_handler_id = 0;
static gulong cut_handler_id = 0;
static gulong paste_handler_id = 0;
static gulong select_all_handler_id = 0;
static gboolean start_search_in_new_window = FALSE; 
static char* hovered_word = NULL; 


//!
//! @brief Brings up the preferences dialog to change settings
//!
//! This function sets up the dialog window, makes sure no searches are
//! currently running, then makes the window appear.
//!
//! @param widget Currently unused widget pointer
//! @param data Currently unused gpointer
//!
G_MODULE_EXPORT void do_settings (GtkWidget *widget, gpointer data)
{
    gw_ui_tab_cancel_all_searches ();
    gw_ui_cancel_search_by_target (GW_TARGET_KANJI);

    //Setup please install dictionary message and notebook page
    GtkWidget *notebook;
    notebook = GTK_WIDGET (gtk_builder_get_object (builder, "settings_notebook"));
    if (data != NULL)
    {
      gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), GPOINTER_TO_INT (data));
    }
    else
    {
      gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), 0);
    }

    //get some needed window references
    gw_ui_update_settings_interface ();
    gw_ui_show_window ("settings_window");
}


//!
//! @brief Brings up the kanjipad dialog to change settings
//!
//! Sets kanjipad to paste the selected result to the search entry
//! and then makes the dialog to appear.
//!
//! @param widget Currently unused widget pointer
//! @param data Currently unused gpointer
//!
G_MODULE_EXPORT void do_kanjipad (GtkWidget *widget, gpointer data)
{
    gw_ui_show_window ("kanjipad_window");
}


//!
//! @brief Closes the kanjipad sidebar
//!
//! Calls the gwaei ui function to close the kanji results sidebar.
//!
//! @param widget Currently unused widget pointer
//! @param data Currently unused gpointer
//!
G_MODULE_EXPORT void do_close_kanji_results (GtkWidget *widget, gpointer data)
{ 
    gw_ui_close_kanji_sidebar ();
}


//!
//! @brief Sets the cursor type depending on the character hovered
//!
//! If the character hovered is a kanji character, the hand turns into a
//! pointer in order to show that the selection is clickable. It will open
//! the kanji sidebar using do_get_position_for_button_press () and
//! do_get_iter_for_button_release ().
//! 
//! @see do_get_position_for_button_press ()
//! @see do_get_iter_for_button_release ()
//! @param widget Currently unused widget pointer
//! @param data Currently unused gpointer
//! @return Always returns false
//!
G_MODULE_EXPORT gboolean do_get_iter_for_motion (GtkWidget      *widget,
                                                 GdkEventButton *event,
                                                 gpointer        data   )
{
    gunichar unic;
    GtkTextIter iter, start, end;
    gint x = event->x;
    gint y = event->y;

    //get unichar
    unic = gw_ui_get_hovered_character (&x, &y, &iter);

    //get word
    if (gtk_text_iter_starts_word (&iter) == FALSE)
      gtk_text_iter_backward_word_start (&iter);
    start = iter;
    if (gtk_text_iter_ends_word (&iter) == FALSE)
      gtk_text_iter_forward_word_end (&iter);
    end = iter;
    hovered_word = gtk_text_iter_get_visible_slice (&start, &end);

    GwDictInfo *di;
    di = gw_dictlist_get_dictinfo_by_alias ("Kanji");
  
    // Characters above 0xFF00 represent inserted images
    if (unic > L'ー' && unic < 0xFF00 && di->status == GW_DICT_STATUS_INSTALLED)
      gw_ui_set_cursor (GDK_HAND2);
    else
      gw_ui_set_cursor (GDK_XTERM);

    return FALSE;
}


//!
//! @brief Gets the position of the cursor click and stores it
//!
//! The function stores the location of the button press, but takes no action
//! by itself.  do_get_iter_for_button_release () uses the saved x and y
//! coordinates and determines if an action should be taken then.
//! 
//! @see do_get_iter_for_button_release ()
//! @param widget Currently unused widget pointer
//! @param data Currently unused gpointer
//! @return Always returns false
//!
G_MODULE_EXPORT gboolean do_get_position_for_button_press (GtkWidget      *widget,
                                                           GdkEventButton *event,
                                                           gpointer        data    )
{
    GtkTextIter iter;
    //Window coordinates
    button_press_x = event->x;
    button_press_y = event->y;

    gw_ui_get_hovered_character (&button_press_x, &button_press_y, &iter);

    return FALSE;
}


//!
//! @brief Gets the position of the cursor click then opens the kanji sidebar
//!
//! Compares the x and y coordinates fetch by do_get_position_for_button_press
//! for the cursor, and if the difference is below a certain threshhold,
//! decides if the user wants to open the kanji character under cursor in the
//! kanji sidebar or not.
//! 
//! @see do_get_position_for_button_press ()
//! @param widget Currently unused widget pointer
//! @param data Currently unused gpointer
//! @return Always returns false
//!
G_MODULE_EXPORT gboolean do_get_iter_for_button_release (GtkWidget      *widget,
                                                         GdkEventButton *event,
                                                         gpointer        data    )
{
    //Window coordinates
    gint x = event->x;
    gint y = event->y;
    gint trailing = 0;
    GtkTextIter iter;

    gunichar unic;
    unic = gw_ui_get_hovered_character (&x, &y, &iter);

    GwDictInfo *di;
    di = gw_dictlist_get_dictinfo_by_alias ("Kanji");

    if (di->status == GW_DICT_STATUS_INSTALLED     &&
        abs (button_press_x - x) < 3 &&
        abs (button_press_y - y) < 3   )
    {
      // Characters above 0xFF00 represent inserted images
      if (unic > L'ー' && unic < 0xFF00 )
      {
        gw_ui_open_kanji_sidebar ();

        //Convert the unicode character into to a utf8 string
        gchar query[7];
        gint length = g_unichar_to_utf8 (unic, query);
        query[length] = '\0'; 

        //Start the search
        GwSearchItem *item;
        item = gw_searchitem_new (query, di, GW_TARGET_KANJI);

        gw_search_get_results (item);
      }
      else
      {
        gw_ui_close_kanji_sidebar ();
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
G_MODULE_EXPORT void do_close (GtkWidget *widget, gpointer data)
{
    const char *id = gtk_buildable_get_name (GTK_BUILDABLE (widget));

    if (strcmp (id, "main_window") == 0)
    {
      save_window_attributes_and_hide (id);
      gw_ui_tab_cancel_all_searches ();
      gtk_main_quit ();
    }
    else if (strcmp (id, "radicals_window") == 0 || strcmp (id, "kanjipad_window") == 0)
    {
      save_window_attributes_and_hide (id);
    }
    else if (strcmp (id, "settings_window") == 0)
    {
      if (rebuild_combobox_dictionary_list () > 0)
      {
        gtk_widget_hide (widget);
        gw_ui_update_toolbar_buttons ();
      }
      else
      {
        gw_ui_tab_cancel_all_searches ();
        gtk_main_quit ();
      }
    }
}


//!
//! @brief Preforms the action the window manager close event
//!
//! This function currently acts as a proxy for the do_close () function.
//! 
//! @see do_close ()
//! @param widget GtkWidget pointer to the window to close
//! @param data Currently unused gpointer
//! @return Always returns true
//!
G_MODULE_EXPORT gboolean do_delete_event_action (GtkWidget *widget, gpointer data)
{ 
    do_close (widget, data);
    return TRUE;
}


//!
//! @brief Closes the current window when the escape key is pressed
//!
//! Checks for a pure escape press with no modifiers then closes the window
//! passed to the function through the widget pointer is do_close(). This
//! function is generally only attached to windows you could picture a cancel
//! button being attached to in the right context.
//! 
//! @see do_close ()
//! @param widget GtkWidget pointer to the window to close
//! @param data Currently unused gpointer
//! @return Returns true when escape key is pressed
//!
G_MODULE_EXPORT gboolean do_close_on_escape (GtkWidget *widget,
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
    if (((state & modifiers) == 0 ) && keyval == GDK_Escape)
    {
      do_close (widget, data);
      return TRUE;
    }
    else
    {
      return FALSE;
    }
}


//!
//! @brief Quits out of the application
//!
//! This function quits gWaei out.  But before it does, it calls the do_close
//! function an the calling window to make sure it's current coordinates are
//! recorded for the next time the application is started.
//! 
//! @see do_close ()
//! @param widget GtkWidget pointer to the window to close
//! @param data Currently unused gpointer
//!
G_MODULE_EXPORT void do_quit (GtkWidget *widget, gpointer data)
{
    gw_ui_tab_cancel_all_searches ();
    do_close (widget, data);
    gtk_main_quit ();
}


//!
//! @brief Preforms a search from the history.
//!
//! The function uses the gpointer data to fetch a GwSearchItem that was pased
//! to the function for the search.  It will reflow the back and forward
//! history lists so the GwSearchItem is in the current position of the
//! Historylist.
//! 
//! @see do_search ()
//! @param widget Unused GtkWidget pointer.
//! @param data pointer to a specially attached GwSearchItem variable
//!
G_MODULE_EXPORT void do_search_from_history (GtkWidget *widget, gpointer data)
{
    gw_guarantee_first_tab ();

    GwHistoryList *hl;
    hl = gw_historylist_get_list (GW_HISTORYLIST_RESULTS);
    GwSearchItem *item;
    item = (GwSearchItem*) data;

    //Make sure searches done from the history are pointing at a valid target
    item->target_tb = (gpointer) get_gobject_by_target (item->target);
    item->target_tv = (gpointer) get_widget_by_target (item->target);

    //Checks to make sure everything is sane
    if (gw_ui_cancel_search_for_current_tab () == FALSE)
      return;
    if (item->dictionary->status != GW_DICT_STATUS_INSTALLED) return;

    //Start setting things up;
    if (hl->back != NULL && g_list_find (hl->back, item))
    {
      while (hl->back != NULL && hl->current != item)
        gw_historylist_go_back_by_target (GW_HISTORYLIST_RESULTS);
    }
    else if (hl->forward != NULL && g_list_find (hl->forward, item))
    {
      while (hl->forward != NULL && hl->current != item)
        gw_historylist_go_forward_by_target (GW_HISTORYLIST_RESULTS);
    }

    //Set tab text
    gw_tab_set_current_tab_text (hl->current->queryline->string);

    //Add tab reference to searchitem
    GtkWidget *notebook = GTK_WIDGET (gtk_builder_get_object (builder, "notebook"));
    int page_num = gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));
    gw_tab_set_searchitem_by_page_num (hl->current, page_num);

    gw_search_get_results (hl->current);
    gw_ui_update_history_popups ();
    gw_ui_update_toolbar_buttons ();

    //Set the search string in the GtkEntry
    gw_ui_clear_search_entry ();
    gw_ui_search_entry_insert ((hl->current)->queryline->string);
    gw_ui_text_select_all_by_target (GW_TARGET_ENTRY);
    gw_ui_grab_focus_by_target (GW_TARGET_ENTRY);

    //Set the correct dictionary in the gui
    const int id_length = 50;
    char id[id_length];
    GtkWidget *combobox;
    strncpy (id, "dictionary_combobox", id_length);
    combobox = GTK_WIDGET (gtk_builder_get_object (builder, id));
    gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), hl->current->dictionary->load_position);
}


//!
//! @brief Goes back one step in the search history
//! 
//! This function checks the top of the back historylist and uses the
//! GwSearchItem in it to invoke do_search_from_history () using it.
//!
//! @see do_search_from_history ()
//! @see do_forward ()
//! @param widget Unused GtkWidget pointer.
//! @param data pointer to a specially attached GwSearchItem variable
//!
G_MODULE_EXPORT void do_back (GtkWidget *widget, gpointer data)
{
    GwHistoryList *hl;
    hl = gw_historylist_get_list (GW_HISTORYLIST_RESULTS);
    if (hl->back != NULL)
    {
      do_search_from_history (NULL, hl->back->data);
    }
}


//!
//! @brief Goes forward one step in the search history
//! 
//! This function checks the top of the forward historylist and uses the
//! GwSearchItem in it to invoke do_search_from_history () using it.
//!
//! @see do_search_from_history ()
//! @see do_back ()
//! @param widget Unused GtkWidget pointer.
//! @param data pointer to a specially attached GwSearchItem variable
//!
G_MODULE_EXPORT void do_forward (GtkWidget *widget, gpointer data)
{
    GwHistoryList *hl;
    hl = gw_historylist_get_list (GW_HISTORYLIST_RESULTS);
    if (hl->forward != NULL)
    {
      do_search_from_history (NULL, hl->forward->data);
    }
}


//!
//! @brief Saves the current search results to a file
//! 
//! The function gets the current contents of the results text view and saves
//! it to a file, overwriting it if it already exists.  If part of the results
//! are highlighted, only that gets saved.
//!
//! @see do_save ()
//! @param widget Unused GtkWidget pointer.
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void do_save_as (GtkWidget *widget, gpointer data)
{
    //Declarations
    GtkWidget *dialog, *window;
    window = GTK_WIDGET (gtk_builder_get_object (builder, "main_window"));
    dialog = gtk_file_chooser_dialog_new (gettext ("Save As"),
                GTK_WINDOW (window),
                GTK_FILE_CHOOSER_ACTION_SAVE,
                GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                NULL);
    gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);

    //See if the user has already saved.  If they did, reuse the path
    if (save_path[0] == '\0')
    {
        gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), "");
        gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), gettext ("vocabulary.txt"));
    }
    else
    {
        gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (dialog), save_path);
    }

    //Prepare the text
    gchar *text;
    //Get the region of text to be saved if some text is highlighted
    text = gw_ui_buffer_get_text_by_target (GW_TARGET_RESULTS);

    //Run the save as dialog
    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *filename;
        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
        strncpy (save_path,
                 gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog)),
                 FILENAME_MAX                                               );
        g_free (filename);
        filename = NULL;

        gw_io_write_file ("w", text);

        GtkAction *edit;
        edit = GTK_ACTION (gtk_builder_get_object (builder, "file_edit_action"));
        gtk_action_set_sensitive (edit, TRUE);
    }

    gtk_widget_destroy (dialog);
    g_free (text);
    text = NULL;
}


//!
//! @brief Appends the current search results to a file
//! 
//! The function gets the current contents of the results text view and appends
//! it to a file.  If the user has already saved once, it will automatically
//! keep appending to the same file. If part of the results are highlighted,
//! only that gets appended.
//!
//! @see do_save_as ()
//! @param widget Unused GtkWidget pointer.
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void do_save (GtkWidget *widget, gpointer data)
{
    GtkWidget *dialog, *window;
    window = GTK_WIDGET (gtk_builder_get_object (builder, "main_window"));

    gchar *text;
    text = gw_ui_buffer_get_text_by_target (GW_TARGET_RESULTS);

    //Pop up a save dialog if the user hasn't saved before
    if (save_path[0] == '\0')
    {
      //Setup the save dialog
      dialog = gtk_file_chooser_dialog_new (gettext ("Save"),
                  GTK_WINDOW (window),
                  GTK_FILE_CHOOSER_ACTION_SAVE,
                  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                  GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                  NULL);
      gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);

      //Set the default save path
      gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), "");
      gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), gettext ("vocabulary.txt"));

      //Run the save dialog
      if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
      {
          char *filename;
          filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
          strncpy (save_path,
                   gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog)),
                   FILENAME_MAX                                              );
          g_free (filename);
          filename = NULL;

          gw_io_write_file ("a", text);

          GtkAction *edit;
          edit = GTK_ACTION (gtk_builder_get_object (builder, "file_edit_action"));
          gtk_action_set_sensitive (edit, TRUE);
      }

      gtk_widget_destroy (dialog);
    }

    //Write the file without opening a dialog
    else
    {
        gw_io_write_file ("a", text);
    }

    g_free (text);
    text = NULL;
}


//!
//! @brief Makes the text in the text buffer enlarge
//! 
//! Determines if the text size is smaller than the max possible text size,
//! and then sets the pref in gconf which will trigger the font size setting
//! function.
//!
//! @see gw_ui_set_font()
//! @param widget Unused GtkWidget pointer.
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void do_zoom_in (GtkWidget *widget, gpointer data)
{
    int size;
    size = gw_pref_get_int (GCKEY_GW_FONT_MAGNIFICATION, GW_DEFAULT_FONT_MAGNIFICATION) + GW_FONT_ZOOM_STEP;
    if (size <= GW_MAX_FONT_MAGNIFICATION)
      gw_pref_set_int (GCKEY_GW_FONT_MAGNIFICATION, size);
}


//!
//! @brief Makes the text in the text buffer shrink
//! 
//! Determines if the text size is larger than the min possible text size,
//! and then sets the pref in gconf which will trigger the font size setting
//! function.
//!
//! @see gw_ui_set_font()
//! @param widget Unused GtkWidget pointer.
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void do_zoom_out (GtkWidget *widget, gpointer data)
{
    int size;
    size = gw_pref_get_int (GCKEY_GW_FONT_MAGNIFICATION, GW_DEFAULT_FONT_MAGNIFICATION) - GW_FONT_ZOOM_STEP;
    if (size >= GW_MIN_FONT_MAGNIFICATION)
      gw_pref_set_int (GCKEY_GW_FONT_MAGNIFICATION, size);
}


//!
//! @brief Resets the text size to the default in the text buffers
//! 
//! The function acts gconf for the default font size from the schema, and then
//! sets it, which makes gconf call the font size setting function since the
//! stored value changed.
//!
//! @see gw_ui_set_font()
//! @param widget Unused GtkWidget pointer.
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void do_zoom_100 (GtkWidget *widget, gpointer data)
{
    int size;
    size = gw_pref_get_default_int (GCKEY_GW_FONT_MAGNIFICATION, GW_DEFAULT_FONT_MAGNIFICATION);
    gw_pref_set_int (GCKEY_GW_FONT_MAGNIFICATION, size);
}


//!
//! @brief Sets the less relevant results show boolean
//! 
//! Makes the gconf pref match the current state of the triggering widget.
//! Each separate GwSearchItem stores this individually, so even if you flip
//! this, you will need to do a new search if you want to change how things are
//! displayed.
//!
//! @see gw_ui_set_less_relevant_show ()
//! @param widget Unused GtkWidget pointer.
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void do_less_relevant_results_toggle (GtkWidget *widget, gpointer data)
{
    gboolean state;
    state = gw_pref_get_boolean (GCKEY_GW_LESS_RELEVANT_SHOW, TRUE);
    gw_pref_set_boolean (GCKEY_GW_LESS_RELEVANT_SHOW, !state);
}


//!
//! @brief Sets the show toolbar boolean to match the widget
//! 
//! Makes the gconf pref match the current state of the triggering widget.
//! The gconf value changed callback then updates the state of the toolbar
//! to match the pref.
//!
//! @see gw_ui_set_toolbar_show ()
//! @param widget Unused GtkWidget pointer.
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void do_toolbar_toggle (GtkWidget *widget, gpointer data)
{
    gboolean state;
    state = gw_pref_get_boolean (GCKEY_GW_TOOLBAR_SHOW, TRUE);
    gw_pref_set_boolean (GCKEY_GW_TOOLBAR_SHOW, !state);
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
G_MODULE_EXPORT void do_dictionary_changed_action (GtkWidget *widget, gpointer data)
{
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
      while (list != NULL && gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (list->data)) == FALSE)
      {
        list = g_list_next (list);
        active++;
      }
    }

    //Finish up
    gw_ui_set_dictionary (active);
    //gw_ui_grab_focus_by_target (GW_TARGET_ENTRY);
}


//!
//! @brief Selects all the text in the current widget
//! 
//! This function makes the selected dictionary in the dictionarylist match
//! the dictionary of the widget that was modified.  The selected dictionary
//! in the dictionarylist acts as the central reference for the GUI.
//!
//! @see do_cut ()
//! @see do_copy ()
//! @see do_paste ()
//! @see do_update_clipboard_on_focus_change ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void do_select_all (GtkWidget *widget, gpointer data)
{
    guint TARGET;
    TARGET = gw_ui_get_current_target_focus ("main_window");

    gw_ui_text_select_all_by_target (TARGET);
}


//!
//! @brief Pastes text into the current widget
//! 
//! This function makes the selected dictionary in the dictionarylist match
//! the dictionary of the widget that was modified.  The selected dictionary
//! in the dictionarylist acts as the central reference for the GUI.
//!
//! @see do_cut ()
//! @see do_copy ()
//! @see do_select_all ()
//! @see do_update_clipboard_on_focus_change ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void do_paste (GtkWidget *widget, gpointer data)
{
    guint TARGET;
    TARGET = gw_ui_get_current_target_focus ("main_window");
    gw_ui_paste_text (TARGET);
}


//!
//! @brief Cuts text from the current widget
//! 
//! This function makes the selected dictionary in the dictionarylist match
//! the dictionary of the widget that was modified.  The selected dictionary
//! in the dictionarylist acts as the central reference for the GUI.
//!
//! @see do_paste ()
//! @see do_copy ()
//! @see do_select_all ()
//! @see do_update_clipboard_on_focus_change ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void do_cut (GtkWidget *widget, gpointer data)
{
    guint TARGET;
    TARGET = gw_ui_get_current_target_focus ("main_window");
    gw_ui_cut_text (TARGET);
}


//!
//! @brief Pastes text into the current widget
//! 
//! This function makes the selected dictionary in the dictionarylist match
//! the dictionary of the widget that was modified.  The selected dictionary
//! in the dictionarylist acts as the central reference for the GUI.
//!
//! @see do_cut ()
//! @see do_paste ()
//! @see do_select_all ()
//! @see do_update_clipboard_on_focus_change ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void do_copy (GtkWidget *widget, gpointer data)
{
    guint TARGET;
    TARGET = gw_ui_get_current_target_focus ("main_window");
    gw_ui_copy_text (TARGET);
}


//!
//! @brief Manages the required changes for focus in different elements
//! 
//! Depending if the object's text is editable or not, the clipboard will
//! update button states approprately and connect the signal handlers to the
//! approprate widgets.
//!
//! @see do_cut ()
//! @see do_copy ()
//! @see do_paste ()
//! @see do_select_all ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//! @return Always returns false
//!
G_MODULE_EXPORT gboolean do_update_clipboard_on_focus_change (GtkWidget        *widget, 
                                                              GtkDirectionType  arg1,
                                                              gpointer          data   ) 
{
    gw_ui_close_suggestion_box ();
    guint TARGET;
    TARGET = gw_ui_get_current_target_focus ("main_window");

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
                                               G_CALLBACK (do_copy),
                                               widget                      );
    cut_handler_id = g_signal_connect        ( cut_action,
                                               "activate",
                                               G_CALLBACK (do_cut),
                                               widget                      );
    paste_handler_id = g_signal_connect      ( paste_action,
                                               "activate",
                                               G_CALLBACK (do_paste),
                                               widget                      );
    select_all_handler_id = g_signal_connect ( select_all_action,
                                               "activate",
                                               G_CALLBACK (do_select_all),
                                               widget                      );


    //Correct the sensitive state to paste
    if (gw_ui_widget_equals_target (data, GW_TARGET_RESULTS) ||
        gw_ui_widget_equals_target (data, GW_TARGET_KANJI)     )
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
G_MODULE_EXPORT void do_print (GtkWidget *widget, gpointer data)
{
    gw_ui_tab_cancel_all_searches ();
    gw_print ();
}


//!
//! @brief Brings up the search tool dialog
//! 
//! This function sets up the radical search tool dialog as needed, then makes
//! it appear.  Before showing it, it makes sure that the strokes spinner is 
//! visable if the needed dictionaries are available and sets the spinner to
//! default to 1 stroke.
//!
//! @see gw_print ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void do_radical_search_tool (GtkWidget *widget, gpointer data)
{
    GtkWidget *hbox;
    hbox = GTK_WIDGET (gtk_builder_get_object (builder, "strokes_hbox"));

    GtkWidget *spinbutton;
    spinbutton = GTK_WIDGET (gtk_builder_get_object (builder, "strokes_spinbutton"));
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (spinbutton), 1.0);

    do_radical_clear (NULL, NULL);

    //Hide the togglebox if the Mix dictionary is not present
    if (gw_dictlist_dictionary_get_status_by_id (GW_DICT_ID_MIX) != GW_DICT_STATUS_INSTALLED)
      gtk_widget_hide (hbox); 
    else
      gtk_widget_show (hbox); 

    //Show the window
    gw_ui_show_window ("radicals_window");
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
G_MODULE_EXPORT void do_edit (GtkWidget *widget, gpointer data)
{
    char *uri = g_build_filename ("file://", save_path, NULL);

    GError *err = NULL;
    gtk_show_uri (NULL, uri, gtk_get_current_event_time (), &err);
    if (err != NULL)
      g_error_free (err);

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
G_MODULE_EXPORT void do_irc_channel (GtkWidget *widget, gpointer data)
{
    GError *err = NULL;
    gtk_show_uri (NULL, "irc://irc.freenode.net/gWaei", gtk_get_current_event_time (), &err);
    if (err != NULL)
      g_error_free (err);
}


//!
//! @brief Sends the user to the gWaei homepage for whatever they need
//! 
//! The homepage should open in their default browser.
//!
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void do_homepage (GtkWidget *widget, gpointer data)
{
    GError *err = NULL;
    gtk_show_uri (NULL, "http://gwaei.sourceforge.net/", gtk_get_current_event_time (), &err);
    if (err != NULL)
      g_error_free (err);
}


//!
//! @brief Opens the gWaei help documentation
//!
//! The gWaei help documentation is opened in the user's default help program.
//!
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void do_help (GtkWidget *widget, gpointer data)
{
    GError *err = NULL;
    gtk_show_uri (NULL, "ghelp:gwaei", gtk_get_current_event_time (), &err);
    if (err != NULL)
      g_error_free (err);
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
G_MODULE_EXPORT void do_glossary (GtkWidget *widget, gpointer data)
{
    char *uri = g_build_filename ("ghelp://", DATADIR, "gnome", "help", "gwaei", "C", "glossary.xml", NULL);

    GError *err = NULL;
    gtk_show_uri (NULL, uri, gtk_get_current_event_time (), &err);
    if (err != NULL)
      g_error_free (err);

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
G_MODULE_EXPORT void do_about (GtkWidget *widget, gpointer data)
{
    char *pixbuf_path = DATADIR G_DIR_SEPARATOR_S PACKAGE G_DIR_SEPARATOR_S "logo.png";

    char *programmer_credits[] = 
    {
      "Zachary Dovel <pizzach@gmail.com>",
      "Fabrizio Sabatini",
      NULL
    };

    GdkPixbuf *logo;
    if ( (logo = gdk_pixbuf_new_from_file ( pixbuf_path,    NULL)) == NULL &&
         (logo = gdk_pixbuf_new_from_file ( "img" G_DIR_SEPARATOR_S "logo.png", NULL)) == NULL    )
    {
      printf ("Was unable to load the gwaei logo.\n");
    }

    GtkWidget *about = g_object_new (GTK_TYPE_ABOUT_DIALOG,
               "program-name", "gWaei", 
               "version", VERSION,
               "copyright", "gWaei (C) 2008-2010 Zachary Dovel\nKanjipad backend (C) 2002 Owen Taylor\nJStroke backend (C) 1997 Robert Wells",
               "comments", gettext("Program for Japanese translation and reference. The\ndictionaries are supplied by Jim Breen's WWWJDIC.\nSpecial thanks to the maker of GJITEN who served as an inspiration."),
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
//! @see do_cycle_dictionaries_backward ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void do_cycle_dictionaries_forward (GtkWidget *widget, gpointer data)
{
    gw_ui_cycle_dictionaries (TRUE);
}


//!
//! @brief Cycles the active dictionaries up the list
//!
//! This function cycles the dictionaries up the list.  If it reaches the
//! end, it will loop back to the bottom.
//!
//! @see do_cycle_dictionaries_forward ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void do_cycle_dictionaries_backward (GtkWidget *widget, gpointer data)
{
    gw_ui_cycle_dictionaries (FALSE);
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
G_MODULE_EXPORT gboolean do_key_press_modify_status_update (GtkWidget *widget,
                                                            GdkEvent  *event,
                                                            gpointer  *data  )
{
    guint keyval = ((GdkEventKey*)event)->keyval;
    GtkWidget* search_entry = get_widget_by_target (GW_TARGET_ENTRY);

    if ((keyval == GDK_ISO_Enter || keyval == GDK_Return) && gtk_widget_is_focus (search_entry))
    {
      gtk_widget_activate (search_entry);
      return FALSE;
    }

    if (keyval == GDK_Shift_L || keyval == GDK_Shift_R || keyval == GDK_ISO_Next_Group || keyval == GDK_ISO_Prev_Group)
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
G_MODULE_EXPORT gboolean do_key_release_modify_status_update (GtkWidget *widget,
                                                              GdkEvent  *event,
                                                              gpointer  *data  )
{
    guint keyval = ((GdkEventKey*)event)->keyval;
    if (keyval == GDK_Shift_L || keyval == GDK_Shift_R || keyval == GDK_ISO_Next_Group || keyval == GDK_ISO_Prev_Group)
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
G_MODULE_EXPORT gboolean do_focus_change_on_key_press (GtkWidget *widget,
                                                       GdkEvent  *event,
                                                       gpointer  *focus  )
{
    gw_ui_close_suggestion_box ();
    guint state = ((GdkEventKey*)event)->state;
    guint keyval = ((GdkEventKey*)event)->keyval;
    guint modifiers = ( 
                        GDK_MOD1_MASK    |
                        GDK_CONTROL_MASK |
                        GDK_SUPER_MASK   |
                        GDK_HYPER_MASK   |
                        GDK_META_MASK      |
                        GDK_Meta_L    |
                        GDK_Meta_R    |
                        GDK_Alt_L     |
                        GDK_Alt_R
                      );

    //Make sure no modifier keys are pressed
    if (
          (state & modifiers) == 0   &&
          keyval != GDK_Tab          &&
          keyval != GDK_ISO_Left_Tab &&
          keyval != GDK_Shift_L      &&
          keyval != GDK_Shift_R      &&
          keyval != GDK_Control_L    &&
          keyval != GDK_Control_R    &&
          keyval != GDK_Caps_Lock    &&
          keyval != GDK_Shift_Lock   &&
          keyval != GDK_Meta_L       &&
          keyval != GDK_Meta_R       &&
          keyval != GDK_Alt_L        &&
          keyval != GDK_Alt_R        &&
          keyval != GDK_Super_L      &&
          keyval != GDK_Super_R      &&
          keyval != GDK_Hyper_L      &&
          keyval != GDK_Hyper_R      &&
          keyval != GDK_Num_Lock     &&
          keyval != GDK_Scroll_Lock  &&
          keyval != GDK_Pause        &&
          keyval != GDK_Home         &&
          keyval != GDK_End
       )
    {
      //Change focus to the text view if is an arrow key or page key
      if ( 
           ( keyval == GDK_Up        ||
             keyval == GDK_Down      ||
             keyval == GDK_Page_Up   ||
             keyval == GDK_Page_Down   
           ) &&
           (
             !gw_ui_widget_equals_target (widget, GW_TARGET_RESULTS)
           )
         )
      {
        gw_ui_text_select_none_by_target (GW_TARGET_ENTRY);
        gw_ui_grab_focus_by_target (GW_TARGET_RESULTS);
        return FALSE;
        return TRUE;
      }

      //Change focus to the entry if other key
      else if (
                keyval != GDK_Up        &&
                keyval != GDK_Down      &&
                keyval != GDK_Page_Up   &&
                keyval != GDK_Page_Down &&
                !gw_ui_widget_equals_target (widget, GW_TARGET_ENTRY)
              )
      {
        gw_ui_text_select_all_by_target (GW_TARGET_ENTRY);
        gw_ui_grab_focus_by_target (GW_TARGET_ENTRY);
        return FALSE;
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
//! @see do_search_from_history ()
//! @see gw_search_get_results ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void do_search (GtkWidget *widget, gpointer data)
{
    gchar query[MAX_QUERY];
    gw_ui_strncpy_text_from_widget_by_target (query, GW_TARGET_ENTRY, MAX_QUERY);

    gw_guarantee_first_tab ();

    if (start_search_in_new_window == TRUE)
      do_new_tab (NULL, NULL);

    GwHistoryList* hl = gw_historylist_get_list (GW_HISTORYLIST_RESULTS);

    GtkWidget *notebook = GTK_WIDGET (gtk_builder_get_object (builder, "notebook"));
    int page_num = gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));
    GwSearchItem *item = g_list_nth_data (gw_tab_get_searchitem_list (), page_num);

    GList *list = gw_dictlist_get_selected ();
    GwDictInfo *dictionary = list->data;

    char *gckey = GCKEY_GW_LESS_RELEVANT_SHOW; 
    gboolean show_less_relevant = gw_pref_get_boolean (gckey, TRUE);

    //Stop empty searches
    if (strlen (query) == 0)
      return;

    //Stop duplicate searches
    if (item != NULL &&
        item->queryline != NULL &&
        strcmp (query, item->queryline->string) == 0 &&
        dictionary->id == item->dictionary->id &&
        show_less_relevant == item->show_less_relevant_results)
      return;

    if (gw_ui_cancel_search_for_current_tab () == FALSE)
      return;

    if (hl->current != NULL && (hl->current)->total_results) 
    {
      gw_historylist_add_searchitem_to_history (GW_HISTORYLIST_RESULTS, hl->current);
      hl->current = NULL;
      gw_ui_update_history_popups ();
    }
    else if (hl->current != NULL)
    {
      gw_ui_cancel_search_by_searchitem (item);
      gw_searchitem_free (hl->current);
      hl->current = NULL;
    }

    //in add_to_history() rather than here
    hl->current = gw_searchitem_new (query, dictionary, GW_TARGET_RESULTS);
    if (hl->current == NULL)
    {
      g_warning ("There was an error creating the searchitem variable.  I will cancel this search.  Please eat some cheese.\n");
      return;
    }

    //Add tab reference to searchitem
    gw_tab_set_searchitem_by_page_num (hl->current, page_num);

    //Start the search
    //Set tab text
    gw_guarantee_first_tab ();
    gw_tab_set_current_tab_text (query);
    gw_ui_set_query_entry_text_by_searchitem (hl->current);
    gw_ui_close_kanji_sidebar ();
    gw_search_get_results (hl->current);

    //Update the toolbar buttons
    gw_ui_update_toolbar_buttons ();
}


//!
//! @brief Inserts an unknown regex character into the entry
//!
//! Used to help users discover regex searches.  It just insert a period
//! wherever the cursor presently is in the search entry.
//!
//! @see do_insert_word_edge ()
//! @see do_insert_not_word_edge ()
//! @see do_insert_and ()
//! @see do_insert_or ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void do_insert_unknown_character (GtkWidget *widget, gpointer data)
{
    gw_ui_search_entry_insert (".");
}


//!
//! @brief Inserts an a word-boundary regex character into the entry
//!
//! Used to help users discover regex searches.  It just insert \\b
//! wherever the cursor presently is in the search entry.
//!
//! @see do_insert_unknown_character ()
//! @see do_insert_not_word_edge ()
//! @see do_insert_and ()
//! @see do_insert_or ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void do_insert_word_edge (GtkWidget *widget, gpointer data)
{
    gw_ui_search_entry_insert ("\\b");
}


//!
//! @brief Inserts an a not-word-boundary regex character into the entry
//!
//! Used to help users discover regex searches.  It just insert \\B
//! wherever the cursor presently is in the search entry.
//!
//! @see do_insert_unknown_character ()
//! @see do_insert_word_edge ()
//! @see do_insert_and ()
//! @see do_insert_or ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void do_insert_not_word_edge (GtkWidget *widget, gpointer data)
{
    gw_ui_search_entry_insert ("\\B");
}


//!
//! @brief Inserts an an and regex character into the entry
//!
//! Used to help users discover regex searches.  It just insert &
//! wherever the cursor presently is in the search entry.
//!
//! @see do_insert_unknown_character ()
//! @see do_insert_word_edge ()
//! @see do_insert_not_word_edge ()
//! @see do_insert_or ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void do_insert_and (GtkWidget *widget, gpointer data)
{
    gw_ui_search_entry_insert ("&");
}


//!
//! @brief Inserts an an or regex character into the entry
//!
//! Used to help users discover regex searches.  It just insert |
//! wherever the cursor presently is in the search entry.
//!
//! @see do_insert_unknown_character ()
//! @see do_insert_word_edge ()
//! @see do_insert_not_word_edge ()
//! @see do_insert_and ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void do_insert_or (GtkWidget *widget, gpointer data)
{
    gw_ui_search_entry_insert ("|");
}


//!
//! @brief Clears the search entry and moves the focus to it
//!
//! This function acts as a quick way for the user to get back to the search
//! entry and do another search whereever they are.
//!
//! @see gw_ui_clear_search_entry ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void do_clear_search (GtkWidget *widget, gpointer data)
{
    gw_ui_clear_search_entry ();
    gw_ui_grab_focus_by_target (GW_TARGET_ENTRY);
}


//!
//! @brief Makes the background color of the kanji sidebar max the theme
//!
//! Keeps the kanji sidebar backgound looking as if it is part of the window
//! even if the gtk theme changes.  The look helps to keep the central focus
//! of the window on the main results, and it looks nice, too.
//!
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void do_update_styles (GtkWidget *widget, gpointer data)
{
    GtkWidget *window, *view;
    GdkColor color;

    window = GTK_WIDGET (gtk_builder_get_object (builder, "main_window"));
    view   = GTK_WIDGET (gtk_builder_get_object (builder, "kanji_text_view"));
    color  = window->style->bg[GTK_STATE_NORMAL];

    g_signal_handlers_block_by_func (widget, do_update_styles, NULL);
    gtk_widget_modify_base (view, GTK_STATE_NORMAL, &color);
    g_signal_handlers_unblock_by_func (widget, do_update_styles, NULL);
}


//!
//! @brief Opens the dictionary folder using the user's default file browser
//!
//! The dictionary folder that is opened is generally in "~/.waei".
//!
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void do_open_dictionary_folder (GtkWidget *widget, gpointer data) 
{
    char directory[FILENAME_MAX];
    gw_util_get_waei_directory (directory);

    char *uri = g_build_filename ("file://", directory, NULL);

    GError *err = NULL;
    gtk_show_uri (NULL, uri, gtk_get_current_event_time (), &err);
    if (err != NULL)
      g_error_free (err);

    g_free (uri);
}


//!
//! @brief Sets the drag icon to the cursor if the widget is dragged over
//!
//! Part of a group of four functions to handle drag drops of text into
//! the main text buffer which will initialize a search based on that text.
//!
//! @see do_search_drag_data_recieved ()
//! @see do_drag_leave_1 ()
//! @see do_drag_drop_1 ()
//! @return Always returns true
//!
G_MODULE_EXPORT gboolean do_drag_motion_1 (GtkWidget      *widget,
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
//! @see do_search_drag_data_recieved ()
//! @see do_drag_drop_1 ()
//! @see do_drag_motion_1 ()
//!
G_MODULE_EXPORT void do_drag_leave_1 (GtkWidget      *widget,
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
//! @see do_search_drag_data_recieved ()
//! @see do_drag_leave_1 ()
//! @see do_drag_motion_1 ()
//! @return Always returns true
//!
G_MODULE_EXPORT gboolean do_drag_drop_1 (GtkWidget      *widget,
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
//! @see do_drag_leave_1 ()
//! @see do_drag_drop_1 ()
//! @see do_drag_motion_1 ()
//!
G_MODULE_EXPORT void do_search_drag_data_recieved (GtkWidget        *widget,
                                                   GdkDragContext   *drag_context,
                                                   gint              x,
                                                   gint              y,
                                                   GtkSelectionData *data,
                                                   guint             info,
                                                   guint             time,
                                                   gpointer          user_data    )
{
    if (widget == NULL) 
      return;
    const char *name = gtk_buildable_get_name (GTK_BUILDABLE (widget));
    if (name == NULL || strcmp (name, "search_entry") == 0)
      return;

    GtkWidget* entry = get_widget_by_target (GW_TARGET_ENTRY);
   
    char* text = (char*) gtk_selection_data_get_text (data);   
    if (text != NULL)
    {
      // Sanitizes text : when drag/dropping text from external sources it 
      // might contains some invalid utf8 data that we should clean.
      // (ex: from the anki tool, it has some trailing unicode control char).
      char* sane_text = gw_util_sanitize_input (text, TRUE);
      g_free (text);
      text = sane_text;
      sane_text = NULL;
    }

    if ((data->length >= 0) && (data->format == 8) && text != NULL)
    {
      do_clear_search (entry, NULL);
      gtk_entry_set_text (GTK_ENTRY (entry), text);
      do_search (NULL, NULL);

      drag_context->action = GDK_ACTION_COPY;
      gtk_drag_finish (drag_context, TRUE, FALSE, time);
    }
    else
    {
      gtk_drag_finish (drag_context, FALSE, FALSE, time);
    }

    if (text != NULL)
    {
      g_free (text);
      text = NULL;
    }

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
G_MODULE_EXPORT void do_update_button_states_based_on_entry_text (GtkWidget *widget,
                                                                  gpointer   data   )
{
    GtkWidget *search_entry = get_widget_by_target (GW_TARGET_ENTRY);
    int length = gtk_entry_get_text_length (GTK_ENTRY (search_entry));

    //Show the clear icon when approprate
    if (length > 0)
      gtk_entry_set_icon_from_stock (GTK_ENTRY (search_entry), GTK_ENTRY_ICON_SECONDARY, GTK_STOCK_CLEAR);
    else
      gtk_entry_set_icon_from_stock (GTK_ENTRY (search_entry), GTK_ENTRY_ICON_SECONDARY, NULL);

    //Return widget colors back to normal
    gtk_widget_modify_base (GTK_WIDGET (search_entry), GTK_STATE_NORMAL, NULL);
    gtk_widget_modify_text (GTK_WIDGET (search_entry), GTK_STATE_NORMAL, NULL);
}


//!
//! @brief Finds out if some text is selected and updates the buttons accordingly
//!
//! When text is found selected, some buttons become sensitive and some have the
//! label change.  This tells the user they can save/print sections of the
//! results.
//!
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT gboolean do_update_icons_for_selection (GtkWidget *widget, 
                                                        GdkEvent  *event,
                                                        gpointer   data   ) 
{
    GtkAction *action;
    action = GTK_ACTION (gtk_builder_get_object (builder, "file_print_action"));
    //Set the special buttons
    if ((event->type == GDK_MOTION_NOTIFY || event->type == GDK_BUTTON_RELEASE) && gw_ui_has_selection_by_target (GW_TARGET_RESULTS))
    {
      gw_ui_update_toolbar_buttons();
      action = GTK_ACTION (gtk_builder_get_object (builder, "file_append_action"));
      gtk_action_set_label (action, gettext("A_ppend Selected"));
      action = GTK_ACTION (gtk_builder_get_object (builder, "file_save_as_action"));
      gtk_action_set_label (action, gettext("Save Selected _As"));
      action = GTK_ACTION (gtk_builder_get_object (builder, "file_print_action"));
      gtk_action_set_label (action, gettext("_Print Selected"));
    }
    //Reset the buttons to their normal states
    else if ((event->type == GDK_FOCUS_CHANGE || event->type == GDK_BUTTON_RELEASE || event->type == GDK_KEY_RELEASE || event->type == GDK_LEAVE_NOTIFY) &&  !gw_ui_has_selection_by_target (GW_TARGET_RESULTS))
    {
      gw_ui_update_toolbar_buttons();
      action = GTK_ACTION (gtk_builder_get_object (builder, "file_append_action"));
      gtk_action_set_label (action, gettext("A_ppend"));
      action = GTK_ACTION (gtk_builder_get_object (builder, "file_save_as_action"));
      gtk_action_set_label (action, NULL);
      action = GTK_ACTION (gtk_builder_get_object (builder, "file_print_action"));
      gtk_action_set_label (action, NULL);
    }
    return FALSE; 
}


//!
//! @brief Populates the main contextual menu with search options
//!
//! @param entry The GtkTexView that was right clicked
//! @param menu The Popup menu to populate
//! @param data  Currently unused gpointer containing user data
//!
void do_populate_popup_with_search_options (GtkTextView *entry, GtkMenu *menu, gpointer data)
{
    if (hovered_word == NULL) return;

    //Declarations
    GwSearchItem *item = NULL;
    GwDictInfo *di = NULL;
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
    GwDictInfo *di_selected = NULL;
    int i = 0;

    //Initializations
    tb = get_gobject_by_target (GW_TARGET_RESULTS);
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
    list_selected = gw_dictlist_get_selected();
    di_selected = list_selected->data;


    //Add webpage links
    GList* list =  gw_dictlist_get_dict_by_load_position (0);
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
      if (di != NULL && (item = gw_searchitem_new (query_text, di, GW_TARGET_RESULTS)) != NULL)
      {
        //Create handy variables
        char *name = website_url_menuitems[i];
        char *url = g_strdup_printf(website_url_menuitems[i + 1], query_text);
        if (url != NULL)
        {
          strncpy(item->queryline->string, url, MAX_QUERY);
          g_free (url);
          url = NULL;
        }
        char *icon_path = website_url_menuitems[i + 2];

        //Start creating
        menu_text = g_strdup_printf ("%s", name);
        if (menu_text != NULL)
        {
          menuitem = GTK_WIDGET (gtk_image_menu_item_new_with_label (menu_text));
          char *path = g_build_filename (DATADIR, PACKAGE, icon_path, NULL);
          if (path != NULL)
          {
            menuimage = gtk_image_new_from_file (path);
            gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem), GTK_WIDGET (menuimage));
            g_free (path);
            path = NULL;
          }
          g_signal_connect (G_OBJECT (menuitem), "activate", G_CALLBACK (do_search_for_searchitem_online), item);
          g_signal_connect (G_OBJECT (menuitem), "destroy",  G_CALLBACK (do_destroy_tab_menuitem_searchitem_data), item);
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
    while ((list = gw_dictlist_get_dict_by_load_position(i)) != NULL)
    {
      list = gw_dictlist_get_dict_by_load_position(i);
      di = list->data;
      if (di != NULL && (item = gw_searchitem_new (query_text, di, GW_TARGET_RESULTS)) != NULL)
      {
        if (di->id == di_selected->id)
        {
          menu_text = g_strdup_printf (search_for_menuitem_text, item->queryline->string, di->long_name);
          if (menu_text != NULL)
          {
            menuitem = GTK_WIDGET (gtk_image_menu_item_new_with_label (menu_text));
            menuimage = gtk_image_new_from_icon_name ("stock_new-tab", GTK_ICON_SIZE_MENU);
            gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem), GTK_WIDGET (menuimage));
            g_signal_connect (G_OBJECT (menuitem), "activate", G_CALLBACK (do_prep_and_start_search_in_new_tab), item);
            gtk_menu_shell_prepend (GTK_MENU_SHELL (menu), GTK_WIDGET (menuitem));
            gtk_widget_show (GTK_WIDGET (menuitem));
            gtk_widget_show (GTK_WIDGET (menuimage));
            g_free (menu_text);
            menu_text = NULL;
          }
        }
        menu_text = g_strdup_printf ("%s", di->long_name);
        if (menu_text != NULL)
        {
          menuitem = GTK_WIDGET (gtk_image_menu_item_new_with_label (menu_text));
          g_signal_connect (G_OBJECT (menuitem), "activate", G_CALLBACK (do_prep_and_start_search_in_new_tab), item);
          g_signal_connect (G_OBJECT (menuitem), "destroy",  G_CALLBACK (do_destroy_tab_menuitem_searchitem_data), item);
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
G_MODULE_EXPORT void do_search_for_searchitem_online (GtkWidget *widget, gpointer data)
{
    GwSearchItem *item = (GwSearchItem*) data;
    if (item != NULL)
    {
      GError *error = NULL;
      gtk_show_uri (NULL, item->queryline->string, gtk_get_current_event_time (), &error);
      if (error != NULL)
      {
        g_error_free (error);
        error = NULL;
      }
    }
}

//!
//! @brief Emulates web browsers font size control with (ctrl + wheel)
//!
//! @param widget Unused GtkWidget pointer.
//! @param data Unused gpointer
//!
G_MODULE_EXPORT gboolean do_scroll_or_zoom(GtkWidget *widget, GdkEventScroll *event, gpointer data)
{
    // If "control" is being pressed
    if( (event->state & GDK_CONTROL_MASK) == GDK_CONTROL_MASK )
    {
	// On wheel direction up ~ zoom out
	if(event->direction == GDK_SCROLL_UP)
	{
	  do_zoom_out(widget, data);
	  return TRUE; // dont propagate event, no scroll
	}

	// On wheel direction down ~ zoom in
	if(event->direction == GDK_SCROLL_DOWN)
	{
	  do_zoom_in(widget, data);
	  return TRUE; // dont propagate event, no scroll
	}
    }

    // return false and propagate event for regular scroll
    return FALSE;
}
