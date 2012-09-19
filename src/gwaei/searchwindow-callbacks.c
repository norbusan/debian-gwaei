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
//! @file searchwindow-callbacks.c
//!
//! @brief To be written
//!


#include "../private.h"

#include <string.h>
#include <stdlib.h>

#include <gdk/gdkkeysyms.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <gwaei/gwaei.h>
#include <gwaei/searchwindow-private.h>

static void gw_searchwindow_add_vocabulary_destroy_cb (GwAddVocabularyWindow*, gpointer);

static const gchar*
gw_searchwindow_hovering_vocabulary_data_tag (GtkTextIter *iter)
{
    GSList *taglist;
    GSList *link;
    GtkTextTag *tag;
    gchar *data;

    taglist = gtk_text_iter_get_tags (iter);
    data = NULL;

    if (taglist != NULL)
    {
      for (link = taglist;  link != NULL && data == NULL;  link = link->next)
      {
        tag = GTK_TEXT_TAG (link->data);
        if (tag != NULL) 
        {
          data = g_object_get_data (G_OBJECT (tag), "vocabulary-data");
        }
      }
      g_slist_free (taglist); taglist = NULL;
    }

    return data;
}


static gunichar
gw_searchwindow_hovering_kanji_character (GtkTextIter *iter)
{
    gunichar character;

    character = gtk_text_iter_get_char (iter);

    if (!g_unichar_validate (character))
      character = 0;
    else if (g_unichar_get_script (character) != G_UNICODE_SCRIPT_HAN)
      character = 0;

    return character;
}




G_MODULE_EXPORT gboolean
gw_searchwindow_motion_notify_event_cb (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         data   )
{
    //Declarations
    GwSearchWindow *window;
    GwApplication *application;
    GtkTextView *view;
    GtkTextIter iter;
    gint x;
    gint y;
    const gchar *vocabulary_data;
    GtkListStore *dictionarystore;
    LwDictInfoList *dictinfolist;
    LwDictInfo *dictinfo;
    GtkWidget *tooltip_window;
    gboolean is_hovering_kanji_character;
    GtkTextWindowType type;

    //Initializations
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_val_if_fail (window != NULL, FALSE);
    application = gw_window_get_application (GW_WINDOW (window));
    view = gw_searchwindow_get_current_textview (window);
    type = gtk_text_view_get_window_type (view, event->window);
    gtk_text_view_window_to_buffer_coords (view, type, (gint) event->x, (gint) event->y, &x, &y);
    gtk_text_view_get_iter_at_position (view, &iter, NULL, x, y);
    dictionarystore = gw_application_get_dictionarystore (application);
    dictinfolist = gw_dictionarystore_get_dictinfolist (GW_DICTIONARYSTORE (dictionarystore));
    dictinfo = lw_dictinfolist_get_dictinfo (dictinfolist, LW_DICTTYPE_KANJI, "Kanji");
    tooltip_window = GTK_WIDGET (gtk_widget_get_tooltip_window (GTK_WIDGET (view)));

    is_hovering_kanji_character = gw_searchwindow_hovering_kanji_character (&iter);
    vocabulary_data = gw_searchwindow_hovering_vocabulary_data_tag (&iter);
 
    // Characters above 0xFF00 represent inserted images
    if ((is_hovering_kanji_character && dictinfo != NULL) || vocabulary_data)
    {
      GdkWindow* gdkwindow;
      GdkCursor* cursor;
      gdkwindow = gtk_text_view_get_window (view, GTK_TEXT_WINDOW_TEXT);
      cursor = gdk_cursor_new (GDK_HAND1);
      gdk_window_set_cursor (gdkwindow, cursor);
      g_object_unref (cursor); cursor = NULL;
    }
    else
    {
      GdkWindow* gdkwindow;
      gdkwindow = gtk_text_view_get_window (view, GTK_TEXT_WINDOW_TEXT);
      gdk_window_set_cursor (gdkwindow, NULL);
    }

    if (tooltip_window != NULL && !is_hovering_kanji_character) 
    {
      gtk_widget_destroy (tooltip_window);
      gtk_widget_set_tooltip_window (GTK_WIDGET (view), NULL);
    }

    return FALSE;
}


//!
//! @brief Gets the position of the cursor click and stores it
//! @see gw_searchwindow_get_iter_for_button_release_cb ()
//! @param widget Currently unused widget pointer
//! @param data Currently unused gpointer
//! @return Always returns false
//!
G_MODULE_EXPORT gboolean 
gw_searchwindow_get_position_for_button_press_cb (GtkWidget      *widget,
                                                  GdkEventButton *event,
                                                  gpointer        data    )
{
    //Declarations
    GwSearchWindow *window;
    GwSearchWindowPrivate *priv;
    GtkTextWindowType type;
    GtkTextView *view;
    gint x, y;

    //Window coordinates
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_val_if_fail (window != NULL, FALSE);
    priv = window->priv;
    view = gw_searchwindow_get_current_textview (window);
    type = gtk_text_view_get_window_type (view, event->window);
    gtk_text_view_window_to_buffer_coords (view, type, (gint) event->x, (gint) event->y, &x, &y);

    priv->mouse_button_press_x = x;
    priv->mouse_button_press_y = y;

    return FALSE;
}


//!
//! @brief Gets the position of the cursor click then opens the kanji sidebar
//! @see gw_searchwindow_get_position_for_button_press_cb ()
//! @param widget Currently unused widget pointer
//! @param data Currently unused gpointer
//! @return Always returns false
//!
G_MODULE_EXPORT gboolean 
gw_searchwindow_get_iter_for_button_release_cb (GtkWidget      *widget,
                                                GdkEventButton *event,
                                                gpointer        data    )
{
    //Declarations
    GwSearchWindow *window;
    GwSearchWindowPrivate *priv;
    GwApplication *application;
    LwPreferences *preferences;
    GtkTextView *view;
    GtkTextIter iter;
    gint x;
    gint y;
    const gchar *vocabulary_data;
    GtkListStore *dictionarystore;
    LwDictInfoList *dictinfolist;
    LwDictInfo *dictinfo;
    GtkTextWindowType type;
    gboolean within_movement_threshold;
    gunichar character;

    //Initializations
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_val_if_fail (window != NULL, FALSE);
    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));
    preferences = gw_application_get_preferences (application);
    view = gw_searchwindow_get_current_textview (window);
    type = gtk_text_view_get_window_type (view, event->window);
    gtk_text_view_window_to_buffer_coords (view, type, (gint) event->x, (gint) event->y, &x, &y);
    gtk_text_view_get_iter_at_position (view, &iter, NULL, x, y);
    dictionarystore = gw_application_get_dictionarystore (application);
    dictinfolist = gw_dictionarystore_get_dictinfolist (GW_DICTIONARYSTORE (dictionarystore));
    dictinfo = lw_dictinfolist_get_dictinfo (dictinfolist, LW_DICTTYPE_KANJI, "Kanji");
    within_movement_threshold = (abs (priv->mouse_button_press_x - x) < 3 && abs (priv->mouse_button_press_y - y) < 3);

    character = gw_searchwindow_hovering_kanji_character (&iter);
    vocabulary_data = gw_searchwindow_hovering_vocabulary_data_tag (&iter);

    if (!within_movement_threshold) return FALSE;
 
    // Characters above 0xFF00 represent inserted images
    if (character && dictinfo != NULL)
    {
      //Convert the unicode character into to a utf8 string
      gchar query[7];
      gint length = g_unichar_to_utf8 (character, query);
      query[length] = '\0'; 

      //Start the search
      if (priv->mouse_item != NULL)
      {
        lw_searchitem_cancel_search (priv->mouse_item);
        lw_searchitem_free (priv->mouse_item);
        priv->mouse_item = NULL;
      }
      priv->mouse_button_press_x = event->x;
      priv->mouse_button_press_y = event->y;
      priv->mouse_button_press_root_x = event->x_root; //x position of the tooltip
      priv->mouse_button_press_root_y = event->y_root; //y position of the tooltip
      priv->mouse_button_character = character;
      priv->mouse_item = lw_searchitem_new (query, dictinfo, preferences, NULL);
      lw_searchitem_start_search (priv->mouse_item, TRUE, FALSE);
    }
    else if (vocabulary_data)
    {
      GtkWindow *avw = gw_addvocabularywindow_new (GTK_APPLICATION (application));
      LwVocabularyItem *vi = lw_vocabularyitem_new_from_string (vocabulary_data);
      gtk_window_set_transient_for (avw, GTK_WINDOW (window));
      gw_addvocabularywindow_set_kanji (GW_ADDVOCABULARYWINDOW (avw), lw_vocabularyitem_get_kanji (vi));
      gw_addvocabularywindow_set_furigana (GW_ADDVOCABULARYWINDOW (avw), lw_vocabularyitem_get_furigana (vi));
      gw_addvocabularywindow_set_definitions (GW_ADDVOCABULARYWINDOW (avw), lw_vocabularyitem_get_definitions (vi));
      lw_vocabularyitem_free (vi); vi = NULL;
      gtk_widget_show (GTK_WIDGET (avw));
      gw_addvocabularywindow_set_focus (GW_ADDVOCABULARYWINDOW (avw), GW_ADDVOCABULARYWINDOW_FOCUS_LIST);
      g_signal_connect (G_OBJECT (avw), "word-added", G_CALLBACK (gw_searchwindow_add_vocabulary_destroy_cb), NULL);
    }

    return FALSE; 
}


//!
//! @brief Closes the window passed throught the widget pointer
//! @param widget GtkWidget pointer to the window to close
//! @param data Currently unused gpointer
//!
G_MODULE_EXPORT void 
gw_searchwindow_close_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwSearchWindow *window;
    GwSearchWindowPrivate *priv;
    GwApplication *application;
    int pages;
    
    //Initializations
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));
    pages = gtk_notebook_get_n_pages (priv->notebook);

    if (pages == 1) 
      gtk_widget_destroy (GTK_WIDGET (window));
    else
      gw_searchwindow_remove_current_tab_cb (widget, data);

    if (gw_application_should_quit (application))
      gw_application_quit (application);
}


//!
//! @brief Preforms the action the window manager close event
//! @see gw_searchwindow_close_cb ()
//! @param widget GtkWidget pointer to the window to close
//! @param data Currently unused gpointer
//! @return Always returns true
//!
G_MODULE_EXPORT gboolean 
gw_searchwindow_delete_event_action_cb (GtkWidget *widget, GdkEvent *event, gpointer data)
{ 
    GwApplication *application;
    GwSearchWindow *window;

    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_val_if_fail (window != NULL, FALSE);
    application = gw_window_get_application (GW_WINDOW (window));

    gtk_widget_destroy (GTK_WIDGET (window));

    if (gw_application_should_quit (application))
      gw_application_quit (application);

    return TRUE;
}


//!
//! @brief Quits out of the application
//! @see gw_application_quit ()
//! @param widget GtkWidget pointer to the window to close
//! @param data Currently unused gpointer
//!
G_MODULE_EXPORT void 
gw_searchwindow_quit_cb (GtkWidget *widget, gpointer data)
{
    GwApplication *application;
    GwSearchWindow *window;

    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    application = gw_window_get_application (GW_WINDOW (window));

    gw_application_quit (application);
}


//!
//! @brief Preforms a search from the history.
//! @see gw_searchwindow_search_cb ()
//! @param widget Should be a pointer to the GtkMenuItem to go back to
//! @param data A pointer to the GwSearchWindow object
//!
G_MODULE_EXPORT void 
gw_searchwindow_search_from_history_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwApplication *application;
    GwSearchWindow *window;
    GwSearchWindowPrivate *priv;
    GwSearchData *sdata;
    gboolean is_in_back_index;
    gboolean is_in_forward_index;
    LwSearchItem *current;
    GtkMenuShell *shell;
    GList *children;
    GList *list;
    gint pre_menu_items;
    gint i;
    LwSearchItem *item;
    gint index;

    //Initializations
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));
    if (!gw_application_can_start_search (application)) return;
    shell = GTK_MENU_SHELL (priv->history_popup);
    pre_menu_items = 3;

    children = gtk_container_get_children (GTK_CONTAINER (shell));
    i = 0;
    if (children != NULL)
    {
      i = g_list_index (children, widget) - pre_menu_items;
      g_list_free (children); children = NULL;
    }

    list = lw_history_get_combined_list (priv->history);
    item = NULL;
    if (list != NULL)
    {
      item = LW_SEARCHITEM (g_list_nth_data (list, i));
      g_list_free (list); list = NULL;
      if (item == NULL) return;
    }

    list = lw_history_get_back_list (priv->history);
    is_in_back_index = (g_list_index (list, item) != -1);

    list = lw_history_get_forward_list (priv->history);
    is_in_forward_index = (g_list_index (list, item) != -1);

    if (!is_in_back_index && !is_in_forward_index) return;

    sdata = lw_searchitem_get_data (item);
    sdata->view = gw_searchwindow_get_current_textview (window);
    
    //Checks to make sure everything is sane
    gw_searchwindow_cancel_search_for_current_tab (window);

    index = gw_searchwindow_get_current_tab_index (window);
    current = gw_searchwindow_steal_searchitem_by_index (window, index);
    if (item != NULL && !lw_searchitem_has_history_relevance (current, priv->keep_searching_enabled))
    {
      lw_searchitem_free (current); current = NULL;
    }

    //Cycle the history
    if (is_in_back_index)
    {
      while (current != item) current = lw_history_go_back (priv->history, current);
    }
    else if (is_in_forward_index)
    {
      while (current != item) current = lw_history_go_forward (priv->history, current);
    }
    else
    {
      g_assert_not_reached ();
      return;
    }

    //Add tab reference to searchitem
    gw_searchwindow_start_search (window, item);

    //Set the search string in the GtkEntry
    gtk_widget_grab_focus (GTK_WIDGET (priv->entry));
    gw_searchwindow_select_all (window, GTK_WIDGET (priv->entry));
}


//!
//! @brief Goes back one step in the search history
//! @see gw_searchwindow_search_from_history_cb ()
//! @see gw_searchwindow_forward_cb ()
//! @param widget Unused GtkWidget pointer.
//! @param data pointer to a specially attached LwSearchItem variable
//!
G_MODULE_EXPORT void 
gw_searchwindow_back_cb (GtkWidget *widget, gpointer data)
{
    GwSearchWindow *window;
    GwSearchWindowPrivate *priv;
    GtkWidget *menuitem;
    GtkMenuShell *shell;
    GList *link;
    GList *list;
    GList *children;
    LwSearchItem *item;
    int pre_menu_items;
    int i;
    
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    
    pre_menu_items = 3;
    shell = GTK_MENU_SHELL (priv->history_popup);
    link = g_list_last (priv->history->back);
    item = LW_SEARCHITEM (link->data);

    list = lw_history_get_combined_list (priv->history);
    i = g_list_index (list, item) + pre_menu_items;
    g_list_free (list); list = NULL;

    children = gtk_container_get_children (GTK_CONTAINER (shell));
    menuitem = GTK_WIDGET (g_list_nth_data (children, i));
    g_list_free (children); children = NULL;

    if (lw_history_has_back (priv->history))
    {
      gw_searchwindow_search_from_history_cb (menuitem, data);
    }
}


//!
//! @brief Goes forward one step in the search history
//! @see gw_searchwindow_search_from_history_cb ()
//! @see gw_searchwindow_back_cb ()
//! @param widget Unused GtkWidget pointer.
//! @param data pointer to a specially attached LwSearchItem variable
//!
G_MODULE_EXPORT void 
gw_searchwindow_forward_cb (GtkWidget *widget, gpointer data)
{
    GwSearchWindow *window;
    GwSearchWindowPrivate *priv;
    GtkWidget *menuitem;
    GtkMenuShell *shell;
    GList *link;
    GList *list;
    GList *children;
    LwSearchItem *item;
    int pre_menu_items;
    int i;
     
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;

    pre_menu_items = 3;
    shell = GTK_MENU_SHELL (priv->history_popup);
    link = g_list_last (priv->history->forward);
    item = LW_SEARCHITEM (link->data);

    list = lw_history_get_combined_list (priv->history);
    i = g_list_index (list, item) + pre_menu_items;;
    g_list_free (list);
    list = NULL;

    children = gtk_container_get_children (GTK_CONTAINER (shell));
    menuitem = GTK_WIDGET (g_list_nth_data (children, i));
    g_list_free (children);
    children = NULL;

    if (lw_history_has_forward (priv->history))
    {
      gw_searchwindow_search_from_history_cb (menuitem, window);
    }
}


//!
//! @brief Saves the current search results to a file
//! @see gw_searchwindow_save_cb ()
//! @param widget Unused GtkWidget pointer.
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_searchwindow_save_as_cb (GtkWidget *widget, gpointer data)
{

    //Declarations
    GwApplication *application;
    GwSearchWindow *window;
    GtkTextView *view;
    const gchar *path;
    GtkWidget *dialog;
    gchar *text;
    gchar *temp;
    GError *error;

    //Initializations
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    application = gw_window_get_application (GW_WINDOW (window));
    gw_application_block_searches (application);
    path = lw_io_get_savepath ();
    temp = NULL;
    view = gw_searchwindow_get_current_textview (window);
    text = gw_searchwindow_get_text (window, GTK_WIDGET (view));
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
    }

    //Cleanup
    gtk_widget_destroy (dialog);
    g_free (text);
    text = NULL;
    gw_application_handle_error (application, GTK_WINDOW (window), TRUE, &error);

    gw_application_unblock_searches (application);
}


//!
//! @brief Appends the current search results to a file
//! @see gw_searchwindow_save_as_cb ()
//! @param widget Unused GtkWidget pointer.
//! @param data Unused gpointer
//0
G_MODULE_EXPORT void 
gw_searchwindow_save_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    gchar *text;
    const gchar *path;
    GError *error;
    GwSearchWindow* window;
    GwApplication *application;
    GtkTextView *view;

    //Initializations
    path = lw_io_get_savepath ();
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    application = gw_window_get_application (GW_WINDOW (window));
    gw_application_block_searches (application);
    error = NULL;

    //Sanity check for an empty save path
    if (path == NULL || *path == '\0')
    {
      gw_searchwindow_save_as_cb (widget, data);
      gw_application_unblock_searches (application);
      return;
    }

    //Carry out the save
    view = gw_searchwindow_get_current_textview (window);
    text = gw_searchwindow_get_text (window, GTK_WIDGET (view));
    lw_io_write_file (path, "a", text, NULL, NULL, &error);
    g_free (text);
    text = NULL;

    gw_application_handle_error (application, GTK_WINDOW (window), FALSE, &error);

    gw_application_unblock_searches (application);
}


//!
//! @brief Makes the text in the text buffer enlarge
//! @see gw_searchwindow_set_font()
//! @param widget Unused GtkWidget pointer.
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_searchwindow_zoom_in_cb (GtkWidget *widget, gpointer data)
{
    GwApplication *application;
    GwSearchWindow *window;
    LwPreferences *preferences;
    int size;

    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    application = gw_window_get_application (GW_WINDOW (window));
    preferences = gw_application_get_preferences (application);
    size = lw_preferences_get_int_by_schema (preferences, LW_SCHEMA_FONT, LW_KEY_FONT_MAGNIFICATION) + GW_APPLICATION_FONT_ZOOM_STEP;

    if (size <= GW_APPLICATION_MAX_FONT_MAGNIFICATION)
    {
      lw_preferences_set_int_by_schema (preferences, LW_SCHEMA_FONT, LW_KEY_FONT_MAGNIFICATION, size);
    }
}


//!
//! @brief Makes the text in the text buffer shrink
//! @see gw_searchwindow_set_font()
//! @param widget Unused GtkWidget pointer.
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_searchwindow_zoom_out_cb (GtkWidget *widget, gpointer data)
{
    GwApplication *application;
    GwSearchWindow *window;
    LwPreferences *preferences;
    int size;

    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    application = gw_window_get_application (GW_WINDOW (window));
    preferences = gw_application_get_preferences (application);
    size = lw_preferences_get_int_by_schema (preferences, LW_SCHEMA_FONT, LW_KEY_FONT_MAGNIFICATION) - GW_APPLICATION_FONT_ZOOM_STEP;
    if (size >= GW_APPLICATION_MIN_FONT_MAGNIFICATION)
    {
      lw_preferences_set_int_by_schema (preferences, LW_SCHEMA_FONT, LW_KEY_FONT_MAGNIFICATION, size);
    }
}


//!
//! @brief Resets the text size to the default in the text buffers
//! @see gw_searchwindow_set_font()
//! @param widget Unused GtkWidget pointer.
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_searchwindow_zoom_100_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwApplication *application;
    GwSearchWindow *window;
    LwPreferences *preferences;

    //Initializations
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    application = gw_window_get_application (GW_WINDOW (window));
    preferences = gw_application_get_preferences (application);

    lw_preferences_reset_value_by_schema (preferences, LW_SCHEMA_FONT, LW_KEY_FONT_MAGNIFICATION);
}


//!
//! @brief Sets the less relevant results show boolean
//! @see gw_searchwindow_set_less_relevant_show ()
//! @param widget Unused GtkWidget pointer.
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_searchwindow_less_relevant_results_toggle_cb (GtkWidget *widget, gpointer data)
{
    GwSearchWindow *window;
    GwApplication *application;
    LwPreferences *preferences;
    gboolean state;

    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (widget), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    application = gw_window_get_application (GW_WINDOW (window));
    preferences = gw_application_get_preferences (application);
    state = lw_preferences_get_boolean_by_schema (preferences, LW_SCHEMA_BASE, LW_KEY_LESS_RELEVANT_SHOW);

    lw_preferences_set_boolean_by_schema (preferences, LW_SCHEMA_BASE, LW_KEY_LESS_RELEVANT_SHOW, !state);
}


G_MODULE_EXPORT void 
gw_searchwindow_dictionary_combobox_changed_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwSearchWindow *window;
    GwSearchWindowPrivate *priv;
    int active;

    //Initializations
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (widget), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    active = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));

    gw_searchwindow_set_dictionary (window, active);

    gtk_widget_grab_focus (GTK_WIDGET (priv->entry));
}

//!
//! @brief Changes the selected dictionary in the dictionarylist
//! @param widget pointer to the GtkWidget that changed dictionaries
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_searchwindow_dictionary_radio_changed_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwSearchWindow *window;
    GwSearchWindowPrivate *priv;
    GList *list;
    GList *iter;
    GtkMenuShell *shell;
    GtkCheckMenuItem *check_menu_item;
    int active;

    //Initializations
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    shell = GTK_MENU_SHELL (priv->dictionary_popup);
    active = 0;

    list = gtk_container_get_children (GTK_CONTAINER (shell));
    for (iter = list; iter != NULL; iter = iter->next)
    {
      check_menu_item = GTK_CHECK_MENU_ITEM (iter->data);
      if (gtk_check_menu_item_get_active (check_menu_item) == TRUE)
        break;
      active++;
    }
    g_list_free (list);

    //Finish up
    gw_searchwindow_set_dictionary (window, active);
}


//!
//! @brief Selects all the text in the current widget
//! @see gw_searchwindow_cut_cb ()
//! @see gw_searchwindow_copy_cb ()
//! @see gw_searchwindow_paste_cb ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_searchwindow_select_all_cb (GtkWidget *widget, gpointer data)
{
    GwSearchWindow *window;
    GtkWidget *focus;

    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    focus = gtk_window_get_focus (GTK_WINDOW (window));

    gw_searchwindow_select_all (window, focus);
}


//!
//! @brief Pastes text into the current widget
//! @see gw_searchwindow_cut_cb ()
//! @see gw_searchwindow_copy_cb ()
//! @see gw_searchwindow_select_all_cb ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_searchwindow_paste_cb (GtkAction *action, gpointer data)
{
    GwSearchWindow *window;
    GtkWidget *focus;

    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    focus = gtk_window_get_focus (GTK_WINDOW (window));

    gw_searchwindow_paste_text (window, focus);
}


//!
//! @brief Cuts text from the current widget
//! @see gw_searchwindow_paste_cb ()
//! @see gw_searchwindow_copy_cb ()
//! @see gw_searchwindow_select_all_cb ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_searchwindow_cut_cb (GtkAction *action, gpointer data)
{
    GwSearchWindow *window;
    GtkWidget *focus;

    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    focus = gtk_window_get_focus (GTK_WINDOW (window));

    gw_searchwindow_cut_text (window, focus);
}


//!
//! @brief Pastes text into the current widget
//! @see gw_searchwindow_cut_cb ()
//! @see gw_searchwindow_paste_cb ()
//! @see gw_searchwindow_select_all_cb ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_searchwindow_copy_cb (GtkAction *action, gpointer data)
{
    GwSearchWindow *window;
    GtkWidget *focus;

    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    focus = gtk_window_get_focus (GTK_WINDOW (window));

    gw_searchwindow_copy_text (window, focus);
}
 

//!
//! @brief Sends the user to the gWaei irc channel for help
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_searchwindow_irc_channel_cb (GtkWidget *widget, gpointer data)
{
    //Initializations
    GError *error;
    GwSearchWindow *window;
    GwApplication *application;

    //Declarations
    error = NULL;
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    application = gw_window_get_application (GW_WINDOW (window));

    gtk_show_uri (NULL, "irc://irc.freenode.net/gWaei", gtk_get_current_event_time (), &error);

    //Cleanup
    gw_application_handle_error (application, GTK_WINDOW (window), TRUE, &error);
}


//!
//! @brief Sends the user to the gWaei homepage for whatever they need
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_searchwindow_homepage_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GError *error;
    GwSearchWindow *window;
    GwApplication *application;

    //Initializations
    error = NULL;
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    application = gw_window_get_application (GW_WINDOW (window));

    gtk_show_uri (NULL, "http://gwaei.sourceforge.net/", gtk_get_current_event_time (), &error);

    //Cleanup
    gw_application_handle_error (application, GTK_WINDOW (window), TRUE, &error);
}


//!
//! @brief Opens the gWaei help documentation
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_searchwindow_show_help_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GtkWindow *window;

    //Initializations
    window = GTK_WINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GTK_TYPE_WINDOW));
    g_return_if_fail (window != NULL);

    gtk_show_uri (NULL, "ghelp:gwaei", gtk_get_current_event_time (), NULL);
}


//!
//! @brief Opens the gWaei dictionary glossary help documentation
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_searchwindow_glossary_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    char *uri;
    GError *error;
    GwSearchWindow *window;
    GwApplication *application;

    //Initializations
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    application = gw_window_get_application (GW_WINDOW (window));
    uri = g_build_filename ("ghelp://", DATADIR2, "gnome", "help", "gwaei", "C", "glossary.xml", NULL);
    error = NULL;

    gtk_show_uri (NULL, uri, gtk_get_current_event_time (), &error);

    //Cleanup
    gw_application_handle_error (application, GTK_WINDOW (window), TRUE, &error);
    g_free (uri);
}


//!
//! @brief Opens the gWaei about dialog
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_searchwindow_about_cb (GtkWidget *widget, gpointer data)
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
               "copyright", "gWaei (C) 2008-2012 Zachary Dovel\n" 
                            "Kanjipad backend (C) 2002 Owen Taylor\n"
                            "JStroke backend (C) 1997 Robert Wells",
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
//! @see gw_searchwindow_cycle_dictionaries_backward_cb ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_searchwindow_cycle_dictionaries_forward_cb (GtkWidget *widget, gpointer data)
{
    GwSearchWindow *window;
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    gw_searchwindow_cycle_dictionaries (window, TRUE);
}


//!
//! @brief Cycles the active dictionaries up the list
//! @see gw_searchwindow_cycle_dictionaries_forward_cb ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_searchwindow_cycle_dictionaries_backward_cb (GtkWidget *widget, gpointer data)
{
    GwSearchWindow *window;
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    gw_searchwindow_cycle_dictionaries (window, FALSE);
}


//!
//! @brief Update the special key press status
//! @param widget Unused GtkWidget pointer
//! @param event the event data to get the specific key that had it's status modified
//! @param data Currently unused gpointer
//! @return Always returns FALSE
//!
G_MODULE_EXPORT gboolean 
gw_searchwindow_key_press_modify_status_update_cb (GtkWidget *widget,
                                                   GdkEvent  *event,
                                                   gpointer   data  )
{
    //Declarations
    GwSearchWindow *window;
    GwSearchWindowPrivate *priv;
    GtkTextView *view;
    GtkWidget *tooltip_window;

    //Initializations
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_val_if_fail (window != NULL, FALSE);
    priv = window->priv;
    view = gw_searchwindow_get_current_textview (window);
    tooltip_window = GTK_WIDGET (gtk_widget_get_tooltip_window (GTK_WIDGET (view)));

    if (tooltip_window != NULL) 
    {
      gtk_widget_destroy (tooltip_window);
      gtk_widget_set_tooltip_window (GTK_WIDGET (view), NULL);
    }

    guint keyval = ((GdkEventKey*)event)->keyval;

    if ((keyval == GDK_KEY_ISO_Enter || 
         keyval == GDK_KEY_Return) 
         && gtk_widget_is_focus (GTK_WIDGET (priv->entry)) && priv->new_tab == TRUE)
    {
      gtk_widget_activate (GTK_WIDGET (priv->entry));
      return TRUE;
    }

    if (keyval == GDK_KEY_Shift_L || 
        keyval == GDK_KEY_Shift_R || 
        keyval == GDK_KEY_ISO_Next_Group || 
        keyval == GDK_KEY_ISO_Prev_Group)
    {
      priv->new_tab = TRUE;
    }

    return FALSE;
}


//!
//! @brief Update the special key release status
//! @param widget Unused GtkWidget pointer
//! @param event the event data to get the specific key that had it's status modified
//! @param data Currently unused gpointer
//! @return Always returns FALSE
//!
G_MODULE_EXPORT gboolean 
gw_searchwindow_key_release_modify_status_update_cb (GtkWidget *widget,
                                                     GdkEvent  *event,
                                                     gpointer   data  )
{
    //Declarations
    GwSearchWindow *window;
    GwSearchWindowPrivate *priv;
    guint keyval;

    //Initializations
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_val_if_fail (window != NULL, FALSE);
    priv = window->priv;
    keyval = ((GdkEventKey*)event)->keyval;

    if (keyval == GDK_KEY_Shift_L || 
        keyval == GDK_KEY_Shift_R || 
        keyval == GDK_KEY_ISO_Next_Group || 
        keyval == GDK_KEY_ISO_Prev_Group)
    {
      priv->new_tab = FALSE;
    }

    return FALSE;
}


//!
//! @brief Function handles automatic focus changes on key presses
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//! @return Always returns false
//!
G_MODULE_EXPORT gboolean 
gw_searchwindow_focus_change_on_key_press_cb (GtkWidget *widget,
                                              GdkEvent  *event,
                                              gpointer  *data  )
{
    GwSearchWindow *window;
    GwSearchWindowPrivate *priv;
    guint state;
    guint keyval;
    guint modifiers;
    GtkTextView *view;
    gint page_num;
    GtkScrolledWindow *scrolledwindow;

    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_val_if_fail (window != NULL, FALSE);
    priv = window->priv;
    state = ((GdkEventKey*)event)->state;
    keyval = ((GdkEventKey*)event)->keyval;
    modifiers = ( 
       GDK_MOD1_MASK    |
       GDK_CONTROL_MASK |
       GDK_SUPER_MASK   |
       GDK_HYPER_MASK   |
       GDK_META_MASK    |
       GDK_KEY_Meta_L   |
       GDK_KEY_Meta_R   |
       GDK_KEY_Alt_L    |
       GDK_KEY_Alt_R
    );
    view = gw_searchwindow_get_current_textview (window);
    page_num = gtk_notebook_get_current_page (priv->notebook);
    scrolledwindow = GTK_SCROLLED_WINDOW (gtk_notebook_get_nth_page (priv->notebook, page_num));

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
             widget != GTK_WIDGET (view)
           )
         )
      {
        gw_searchwindow_select_none (window, GTK_WIDGET (view));
        gtk_widget_grab_focus (GTK_WIDGET (view));
        return TRUE;
      }

      //A manual reimplimentation of scroll for the textview/scollbox
      else if ( 
           ( keyval == GDK_KEY_Up        ||
             keyval == GDK_KEY_Down      ||
             keyval == GDK_KEY_Page_Up   ||
             keyval == GDK_KEY_Page_Down   
           ) &&
           (
             widget == GTK_WIDGET (view)
           )
         )
      {
        GtkAdjustment *adjustment = gtk_scrolled_window_get_vadjustment (scrolledwindow);
        gdouble increment = 0.0;
        gdouble value = gtk_adjustment_get_value (adjustment);

        switch (keyval)
        {
          case GDK_KEY_Up:
            increment = -gtk_adjustment_get_step_increment (adjustment);
            break;
          case GDK_KEY_Down:
            increment = gtk_adjustment_get_step_increment (adjustment);
            break;
          case GDK_KEY_Page_Up:
            increment = -gtk_adjustment_get_page_increment (adjustment);
            break;
          case GDK_KEY_Page_Down:
            increment = gtk_adjustment_get_page_increment (adjustment);
            break;
          default:
            break;
        }
        gtk_adjustment_set_value (adjustment, increment + value);
        gtk_adjustment_value_changed (adjustment);
        return TRUE;
      }

      //Change focus to the entry if other key
      else if (
                keyval != GDK_KEY_Up        &&
                keyval != GDK_KEY_Down      &&
                keyval != GDK_KEY_Page_Up   &&
                keyval != GDK_KEY_Page_Down &&
                widget != GTK_WIDGET (priv->entry)
              )
      {
        gw_searchwindow_select_all (window, GTK_WIDGET (priv->entry));
        gtk_widget_grab_focus (GTK_WIDGET (priv->entry));
        return TRUE;
      }
    }

    return FALSE;
}


//!
//! @brief Initiates a search on the user's typed query
//! @see gw_searchwindow_search_from_history_cb ()
//! @see lw_search_get_results ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_searchwindow_search_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwApplication *application;
    GwSearchWindow *window;
    GwSearchWindowPrivate *priv;
    LwPreferences *preferences;
    char query[50];
    LwSearchItem *item;
    LwSearchItem *new_item;
    LwDictInfo *di;
    GError *error;
    GwSearchData *sdata;
    GtkTextView *view;
    gint index;

    //Initializations
    error = NULL;
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));

    if (!gw_application_can_start_search (application)) return;

    preferences = gw_application_get_preferences (application);
    strncpy (query, gtk_entry_get_text (priv->entry), 50);
    index = gw_searchwindow_get_current_tab_index (window);
    item = gw_searchwindow_get_searchitem_by_index (window, index);
    di = gw_searchwindow_get_dictionary (window);
    gw_searchwindow_guarantee_first_tab (window);

    //Cancel all searches if the search bar is empty
    if (strlen(query) == 0 || di == NULL) 
    {
      lw_searchitem_cancel_search (item);
      return;
    }

    view = gw_searchwindow_get_current_textview (window);
    new_item = lw_searchitem_new (query, di, preferences, &error);
    if (new_item == NULL)
    {
      gw_application_handle_error (application, NULL, FALSE, &error);
      return;
    }
    sdata = gw_searchdata_new (view, window);
    lw_searchitem_set_data (new_item, sdata, LW_SEARCHITEM_DATA_FREE_FUNC (gw_searchdata_free));

    //Check for problems, and quit if there are
    if (error != NULL ||
        new_item == NULL ||
        lw_searchitem_is_equal (item, new_item)
       )
    {
      lw_searchitem_increment_history_relevance_timer (item);

      if (new_item != NULL)
      {
        lw_searchitem_free (new_item);
        new_item = NULL;
      }

      gw_application_handle_error (application, NULL, FALSE, &error);

      return;
    }

    if (priv->new_tab)
    {
      gw_searchwindow_new_tab (window);
      item = NULL;
    }
    else
    {
      lw_searchitem_cancel_search (item);
    }

    //Push the previous searchitem or replace it with the new one
    if (item != NULL && lw_searchitem_has_history_relevance (item, priv->keep_searching_enabled))
    {
      item = gw_searchwindow_steal_searchitem_by_index (window, index);
      if (item != NULL) 
      {
        lw_history_add_searchitem (priv->history, item);
      }
    }

    gw_searchwindow_start_search (window, new_item);
}


//!
//! @brief Inserts an unknown regex character into the entry
//! @see gw_searchwindow_insert_word_edge_cb ()
//! @see gw_searchwindow_insert_not_word_edge_cb ()
//! @see gw_searchwindow_insert_and_cb ()
//! @see gw_searchwindow_insert_or_cb ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_searchwindow_insert_unknown_character_cb (GtkWidget *widget, gpointer data)
{
    GwSearchWindow *window;
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    gw_searchwindow_entry_insert_text (window, ".");
}


//!
//! @brief Inserts an a word-boundary regex character into the entry
//! @see gw_searchwindow_insert_unknown_character_cb ()
//! @see gw_searchwindow_insert_not_word_edge_cb ()
//! @see gw_searchwindow_insert_and_cb ()
//! @see gw_searchwindow_insert_or_cb ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_searchwindow_insert_word_edge_cb (GtkWidget *widget, gpointer data)
{
    GwSearchWindow *window;
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    gw_searchwindow_entry_insert_text (window, "\\b");
}


//!
//! @brief Inserts an a not-word-boundary regex character into the entry
//! @see gw_searchwindow_insert_unknown_character_cb ()
//! @see gw_searchwindow_insert_word_edge_cb ()
//! @see gw_searchwindow_insert_and_cb ()
//! @see gw_searchwindow_insert_or_cb ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_searchwindow_insert_not_word_edge_cb (GtkWidget *widget, gpointer data)
{
    GwSearchWindow *window;
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    gw_searchwindow_entry_insert_text (window, "\\B");
}


//!
//! @brief Inserts an an and regex character into the entry
//! @see gw_searchwindow_insert_unknown_character_cb ()
//! @see gw_searchwindow_insert_word_edge_cb ()
//! @see gw_searchwindow_insert_not_word_edge_cb ()
//! @see gw_searchwindow_insert_or_cb ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_searchwindow_insert_and_cb (GtkWidget *widget, gpointer data)
{
    GwSearchWindow *window;
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    gw_searchwindow_entry_insert_text (window, "&");
}


//!
//! @brief Inserts an an or regex character into the entry
//! @see gw_searchwindow_insert_unknown_character_cb ()
//! @see gw_searchwindow_insert_word_edge_cb ()
//! @see gw_searchwindow_insert_not_word_edge_cb ()
//! @see gw_searchwindow_insert_and_cb ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_searchwindow_insert_or_cb (GtkWidget *widget, gpointer data)
{
    GwSearchWindow *window;
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    gw_searchwindow_entry_insert_text (window, "|");
}


//!
//! @brief Clears the search entry and moves the focus to it
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_searchwindow_clear_search_cb (GtkWidget *widget, gpointer data)
{
    GwSearchWindow *window;
    GwSearchWindowPrivate *priv;

    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;

    gtk_entry_set_text (priv->entry, "");
    gtk_widget_grab_focus (GTK_WIDGET (priv->entry));
}


//!
//! @brief Clears the search entry and moves the focus to it
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_searchwindow_clear_entry_button_pressed_cb (GtkEntry             *entry, 
                                               GtkEntryIconPosition  icon_pos, 
                                               GdkEvent             *event, 
                                               gpointer              data    )
{
    GwSearchWindow *window;
    GwSearchWindowPrivate *priv;

    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;

    gtk_entry_set_text (priv->entry, "");
    gtk_widget_grab_focus (GTK_WIDGET (priv->entry));
}


//!
//! @brief Sets the drag icon to the cursor if the widget is dragged over
//! @see gw_searchwindow_search_drag_data_recieved_cb ()
//! @see gw_searchwindow_drag_leave_1_cb ()
//! @see gw_searchwindow_drag_drop_1_cb ()
//! @return Always returns true
//!
G_MODULE_EXPORT gboolean 
gw_searchwindow_drag_motion_1_cb (GtkWidget      *widget,
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
//! @see gw_searchwindow_search_drag_data_recieved_cb ()
//! @see gw_searchwindow_drag_drop_1_cb ()
//! @see gw_searchwindow_drag_motion_1_cb ()
//!
G_MODULE_EXPORT void 
gw_searchwindow_drag_leave_1_cb (GtkWidget      *widget,
                                 GdkDragContext *drag_context,
                                 guint           time,
                                 gpointer        user_data) 
{
    gtk_drag_unhighlight (widget);
}


//!
//! @brief Tells the widget to recieve the dragged data
//! @see gw_searchwindow_search_drag_data_recieved_cb ()
//! @see gw_searchwindow_drag_leave_1_cb ()
//! @see gw_searchwindow_drag_motion_1_cb ()
//! @return Always returns true
//!
G_MODULE_EXPORT gboolean 
gw_searchwindow_drag_drop_1_cb (GtkWidget      *widget,
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
//! @see gw_searchwindow_drag_leave_1_cb ()
//! @see gw_searchwindow_drag_drop_1_cb ()
//! @see gw_searchwindow_drag_motion_1_cb ()
//!
G_MODULE_EXPORT void 
gw_searchwindow_search_drag_data_recieved_cb (GtkWidget        *widget,
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
    GwSearchWindow *window;
    GwSearchWindowPrivate *priv;
    char* text;

    //Initializations
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);

    priv = window->priv;
    text = (char*) gtk_selection_data_get_text (data);   

    if (text != NULL && strlen(text) > 0)
    {
      gw_searchwindow_clear_search_cb (widget, data);
      gtk_entry_set_text (priv->entry, text);
      gw_searchwindow_search_cb (widget, data);

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
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_searchwindow_update_button_states_based_on_entry_text_cb (GtkWidget *widget,
                                                             gpointer   data   )
{
    //Declarations
    GwSearchWindow *window;
    GwSearchWindowPrivate *priv;
    const gchar* NAME;
    gint length;

    //Initializations
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);

    priv = window->priv;
    length = gtk_entry_get_text_length (GTK_ENTRY (priv->entry));

    //Show the clear icon when approprate
    if (length > 0)
      NAME = "edit-clear-symbolic";
    else
      NAME = NULL;

    gtk_entry_set_icon_from_icon_name (GTK_ENTRY (priv->entry), GTK_ENTRY_ICON_SECONDARY, NAME);
}


//!
//! @brief Emulates web browsers font size control with (ctrl + wheel)
//! @param widget Unused GtkWidget pointer.
//! @param data Unused gpointer
//!
G_MODULE_EXPORT gboolean 
gw_searchwindow_scroll_or_zoom_cb (GtkWidget *widget, GdkEventScroll *event, gpointer data)
{
    if( (event->state & GDK_CONTROL_MASK) == GDK_CONTROL_MASK )
    {
      if(event->direction == GDK_SCROLL_UP)
      {
        gw_searchwindow_zoom_out_cb (widget, data);
        return TRUE; // dont propagate event, no scroll
      }

      if(event->direction == GDK_SCROLL_DOWN)
      {
        gw_searchwindow_zoom_in_cb (widget, data);
        return TRUE; // dont propagate event, no scroll
      }
    }

    return FALSE;
}


//!
//! @brief Append a tag to the end of the tags
//! @param widget Currently unused widget pointer
//! @param data Currently unused gpointer
//!
G_MODULE_EXPORT void 
gw_searchwindow_new_tab_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwSearchWindow *window;

    //Initializations
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    
    gw_searchwindow_new_tab (window);
}


G_MODULE_EXPORT void 
gw_searchwindow_new_window_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwApplication *application;
    GwSearchWindow *window;
    GtkWindow *new_window;

    //Initializations
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    application = gw_window_get_application (GW_WINDOW (window));
    new_window = gw_searchwindow_new (GTK_APPLICATION (application));

    gtk_widget_show (GTK_WIDGET (new_window));
}


//!
//! @brief Remove the tab where the close button is clicked
//! @param widget Currently unused widget pointer
//! @param data Currently unused gpointer
//!
G_MODULE_EXPORT void 
gw_searchwindow_remove_tab_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwSearchWindow *window;
    GwSearchWindowPrivate *priv;
    int page_num;

    //Initializations
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    page_num = gtk_notebook_page_num (priv->notebook, GTK_WIDGET (data));

    if (page_num != -1)
      gw_searchwindow_remove_tab_by_index (window, page_num);
}


//!
//! @brief Remove the tab where the close button is clicked
//! @param widget Currently unused widget pointer
//! @param data Currently unused gpointer
//!
G_MODULE_EXPORT void 
gw_searchwindow_remove_current_tab_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwSearchWindow *window;
    GwSearchWindowPrivate *priv;
    int page_num;

    //Initializations
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    page_num = gtk_notebook_get_current_page (priv->notebook);

    if (page_num != -1)
      gw_searchwindow_remove_tab_by_index (window, page_num);
}


//!
//! @brief Do the side actions required when a tab switch takes place
//! @param widget Currently unused widget pointer
//! @param data Currently unused gpointer
//!
G_MODULE_EXPORT void 
gw_searchwindow_switch_tab_cb (GtkNotebook *notebook, 
                               GtkWidget   *page, 
                               int          page_num, 
                               gpointer     data)
{
    //Declarations
    GwSearchWindow *window;
    GwSearchWindowPrivate *priv;
    LwSearchItem *item;
    GtkWidget *container;

    //Initializations
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    item = NULL;

    container = gtk_notebook_get_nth_page (priv->notebook, page_num);
    if (container != NULL)
    {
      item = LW_SEARCHITEM (g_object_get_data (G_OBJECT (container), "searchitem"));

      gw_searchwindow_set_dictionary_by_searchitem (window, item);
      gw_searchwindow_set_entry_text_by_searchitem (window, item);
      gw_searchwindow_set_title_by_searchitem (window, item);
      gw_searchwindow_set_total_results_label_by_searchitem (window, item);
      gw_searchwindow_set_search_progressbar_by_searchitem (window, item);
    }
}


//!
//! @brief Cycles to the next tab 
//! @param widget Currently unused widget pointer
//! @param data Currently unused gpointer
//!
G_MODULE_EXPORT void 
gw_searchwindow_next_tab_cb (GtkWidget *widget, gpointer data)
{
    GwSearchWindow *window;
    GwSearchWindowPrivate *priv;

    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;

    gtk_notebook_next_page (priv->notebook);
}


//!
//! @brief Cycles to the previous tab 
//! @param widget Currently unused widget pointer
//! @param data Currently unused gpointer
//!
G_MODULE_EXPORT void 
gw_searchwindow_previous_tab_cb (GtkWidget *widget, gpointer data)
{
    GwSearchWindow *window;
    GwSearchWindowPrivate *priv;

    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;

    gtk_notebook_prev_page (priv->notebook);
}


G_MODULE_EXPORT void 
gw_searchwindow_no_results_search_for_dictionary_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwSearchWindow *window;
    gint load_position;

    //Initializations
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    load_position = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (widget), "load-position"));

    gw_searchwindow_set_dictionary (window, load_position);

    gw_searchwindow_search_cb (widget, data);
}


//!
//! @brief Sets the show toolbar boolean to match the widget
//! @see gw_searchwindow_set_toolbar_show ()
//! @param widget Unused GtkWidget pointer.
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_searchwindow_toolbar_show_toggled_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwApplication *application;
    GwSearchWindow *window;
    LwPreferences *preferences;
    gboolean request;

    //Initializations
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    application = gw_window_get_application (GW_WINDOW (window));
    preferences = gw_application_get_preferences (application);
    request = lw_preferences_get_boolean_by_schema (preferences, LW_SCHEMA_BASE, LW_KEY_TOOLBAR_SHOW);

    lw_preferences_set_boolean_by_schema (preferences, LW_SCHEMA_BASE, LW_KEY_TOOLBAR_SHOW, !request);
}


//!
//! @brief Syncs the gui to the preference settinging.  It should be attached to the gsettings object
//!
G_MODULE_EXPORT void 
gw_searchwindow_sync_toolbar_show_cb (GSettings *settings, gchar *key, gpointer data)
{
    //Declarations
    GwSearchWindow *window;
    GwSearchWindowPrivate *priv;
    GtkWidget *toplevel;
    GtkAction *action;
    gboolean request;
    GtkToolbar *primary_toolbar;
    GtkToolbar *search_toolbar;
    GtkStyleContext *context;

    //Initializations
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    toplevel = gw_window_get_toplevel (GW_WINDOW (window));
    action = GTK_ACTION (priv->show_toolbar_toggleaction);
    request = lw_preferences_get_boolean (settings, key);
    primary_toolbar = GTK_TOOLBAR (priv->primary_toolbar);
    search_toolbar = GTK_TOOLBAR (priv->search_toolbar);

    if (request == TRUE)
    {
      context = gtk_widget_get_style_context (GTK_WIDGET (search_toolbar));
      gtk_style_context_add_class (context, GTK_STYLE_CLASS_PRIMARY_TOOLBAR);
      gtk_widget_reset_style (GTK_WIDGET (search_toolbar));

      gtk_widget_show (GTK_WIDGET (primary_toolbar));
    }
    else
    {
      context = gtk_widget_get_style_context (GTK_WIDGET (search_toolbar));
      gtk_style_context_add_class (context, GTK_STYLE_CLASS_PRIMARY_TOOLBAR);
      gtk_widget_reset_style (GTK_WIDGET (search_toolbar));

      gtk_widget_hide (GTK_WIDGET (primary_toolbar));
    }


    G_GNUC_EXTENSION g_signal_handlers_block_by_func (action, gw_searchwindow_toolbar_show_toggled_cb, toplevel);
    gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action), request);
    G_GNUC_EXTENSION g_signal_handlers_unblock_by_func (action, gw_searchwindow_toolbar_show_toggled_cb, toplevel);
}


//!
//! @brief Sets the show toolbar boolean to match the widget
//! @see gw_searchwindow_set_toolbar_show ()
//! @param widget Unused GtkWidget pointer.
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_searchwindow_statusbar_show_toggled_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwApplication *application;
    GwSearchWindow *window;
    LwPreferences *preferences;
    gboolean request;

    //Initializations
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    application = gw_window_get_application (GW_WINDOW (window));
    preferences = gw_application_get_preferences (application);
    request = lw_preferences_get_boolean_by_schema (preferences, LW_SCHEMA_BASE, LW_KEY_STATUSBAR_SHOW);

    lw_preferences_set_boolean_by_schema (preferences, LW_SCHEMA_BASE, LW_KEY_STATUSBAR_SHOW, !request);
}


//!
//! @brief Sets the checkbox to show or hide the statusbar
//! @param request How to set the statusbar
//!
G_MODULE_EXPORT void 
gw_searchwindow_sync_statusbar_show_cb (GSettings *settings, gchar *key, gpointer data)
{
    //Declarations
    GwSearchWindow *window;
    GwSearchWindowPrivate *priv;
    GtkWidget *toplevel;
    GtkAction *action;
    gboolean request;

    //Initializations
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    toplevel = gw_window_get_toplevel (GW_WINDOW (window));
    action = GTK_ACTION (priv->show_statusbar_toggleaction);
    request = lw_preferences_get_boolean (settings, key);

    if (request == TRUE)
      gtk_widget_show (priv->statusbar);
    else
      gtk_widget_hide (priv->statusbar);

    G_GNUC_EXTENSION g_signal_handlers_block_by_func (action, gw_searchwindow_statusbar_show_toggled_cb, toplevel);
    gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action), request);
    G_GNUC_EXTENSION g_signal_handlers_unblock_by_func (action, gw_searchwindow_statusbar_show_toggled_cb, toplevel);
}


//!
//! @brief Sets the requested font with magnification applied
//!
G_MODULE_EXPORT void 
gw_searchwindow_sync_font_cb (GSettings *settings, gchar *KEY, gpointer data)
{
    //Declarations
    GwSearchWindow *window;

    //Initializations
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);

    gw_searchwindow_set_font (window);
}


//!
//! @brief Callback to toggle spellcheck in the search entry
//! @param widget Unused pointer to a GtkWidget
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_searchwindow_spellcheck_toggled_cb (GtkWidget *widget, gpointer data)
{
#ifdef WITH_HUNSPELL
    //Declarations
    GwApplication *application;
    GwSearchWindow *window;
    LwPreferences *preferences;
    gboolean state;

    //Initializations
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    application = gw_window_get_application (GW_WINDOW (window));
    preferences = gw_application_get_preferences (application);
    state = lw_preferences_get_boolean_by_schema (preferences, LW_SCHEMA_BASE, LW_KEY_SPELLCHECK);

    lw_preferences_set_boolean_by_schema (preferences, LW_SCHEMA_BASE, LW_KEY_SPELLCHECK, !state);
#endif
}


//!
//! @brief Sets the gui widgets consistently to the requested state
//! @param request the requested state for spellchecking widgets
//!
G_MODULE_EXPORT void 
gw_searchwindow_sync_spellcheck_cb (GSettings *settings, gchar *KEY, gpointer data)
{
#ifdef WITH_HUNSPELL
    //Declarations
    GwApplication *application;
    GwSearchWindow *window;
    GwSearchWindowPrivate *priv;
    LwPreferences *preferences;
    GtkWidget *toplevel;
    GtkToggleToolButton *toolbutton;
    gboolean request;

    //Initializations
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));
    preferences = gw_application_get_preferences (application);
    toplevel = gw_window_get_toplevel (GW_WINDOW (window));
    toolbutton = GTK_TOGGLE_TOOL_BUTTON (priv->spellcheck_toolbutton);
    request = lw_preferences_get_boolean_by_schema (preferences, LW_SCHEMA_BASE, LW_KEY_SPELLCHECK);

    G_GNUC_EXTENSION g_signal_handlers_block_by_func (toolbutton, gw_searchwindow_spellcheck_toggled_cb, toplevel);
    gtk_toggle_tool_button_set_active (toolbutton, request);
    G_GNUC_EXTENSION g_signal_handlers_unblock_by_func (toolbutton, gw_searchwindow_spellcheck_toggled_cb, toplevel);

    if (request == TRUE && priv->spellcheck == NULL)
    {
      priv->spellcheck =  gw_spellcheck_new_with_entry (application, priv->entry);
      g_object_add_weak_pointer (G_OBJECT (priv->spellcheck), (gpointer*) &(priv->spellcheck));
    }
    else if (request == FALSE && priv->spellcheck != NULL)
    {
      if (priv->spellcheck != NULL)
        g_object_unref (G_OBJECT (priv->spellcheck));
    }
#endif
}


G_MODULE_EXPORT void 
gw_searchwindow_sync_search_as_you_type_cb (GSettings *settings, 
                                            gchar     *KEY, 
                                            gpointer   data     )
{
    //Declarations
    GwSearchWindow *window;
    GwSearchWindowPrivate *priv;
    GtkWidget *toolbutton;
    gboolean request;

    //Initializations
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    toolbutton = GTK_WIDGET (priv->submit_toolbutton);
    request = lw_preferences_get_boolean (settings, KEY);

    priv->keep_searching_enabled = request;
    if (request) gtk_widget_hide (toolbutton);
    else gtk_widget_show (toolbutton);
}


static void
gw_searchwindow_kanjipadwindow_kanji_selected_cb (GwKanjipadWindow *window, const gchar *text, gpointer data)
{
    GwSearchWindow *searchwindow;
    searchwindow = GW_SEARCHWINDOW (data);
    gw_searchwindow_entry_insert_text (searchwindow, text);
}


G_MODULE_EXPORT void 
gw_searchwindow_toggle_kanjipadwindow_cb (GtkAction *action, gpointer data)
{
    //Declarations
    GwSearchWindow *window;
    GwSearchWindowPrivate *priv;
    GwApplication *application;
    gboolean show;

    //Initializations
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));
    show = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));

    if (show)
    {
      if (priv->kanjipadwindow != NULL)
      {
        gtk_window_set_transient_for (GTK_WINDOW (priv->kanjipadwindow), GTK_WINDOW (window));
        gtk_window_present (GTK_WINDOW (priv->kanjipadwindow));
      }
      else
      {
        priv->kanjipadwindow = GW_KANJIPADWINDOW (gw_kanjipadwindow_new (GTK_APPLICATION (application)));
        gtk_window_set_transient_for (GTK_WINDOW (priv->kanjipadwindow), GTK_WINDOW (window));
        g_signal_connect (
          G_OBJECT (priv->kanjipadwindow), 
          "kanji-selected", 
          G_CALLBACK (gw_searchwindow_kanjipadwindow_kanji_selected_cb), 
          window
        );

        g_signal_connect (
          G_OBJECT (priv->kanjipadwindow),
          "hide",
          G_CALLBACK (gw_searchwindow_kanjipadwindow_destroy_cb),
          window
        );

        gtk_widget_show (GTK_WIDGET (priv->kanjipadwindow));
        g_object_add_weak_pointer (G_OBJECT (priv->kanjipadwindow), (gpointer) &(priv->kanjipadwindow));
      }
    }
    else
    {
      if (priv->kanjipadwindow != NULL) gtk_widget_hide (GTK_WIDGET (priv->kanjipadwindow));
    }
}


void
gw_searchwindow_kanjipadwindow_destroy_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwSearchWindow *window;
    GwSearchWindowPrivate *priv;
    GtkWidget *toplevel;
    GtkToggleAction *action;

    //Initializations
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    toplevel = gw_window_get_toplevel (GW_WINDOW (window));
    action = priv->show_kanjipad_toggleaction;

    G_GNUC_EXTENSION g_signal_handlers_block_by_func (action, gw_searchwindow_toggle_kanjipadwindow_cb, toplevel);
    gtk_toggle_action_set_active (action, FALSE);
    G_GNUC_EXTENSION g_signal_handlers_unblock_by_func (action, gw_searchwindow_toggle_kanjipadwindow_cb, toplevel);
}



//!
//! @brief Sets the query created in the radicalwindow to the searchwindow
//!
static void
gw_searchwindow_radicalswindow_query_changed_cb (GwRadicalsWindow *window, gpointer data)
{
    //Declarations
    GwSearchWindow *searchwindow;
    GwApplication *application;
    GtkListStore *dictionarystore;
    LwDictInfoList *dictinfolist;
    LwDictInfo *di;
    char *text_query;
    char *text_radicals;
    char *text_strokes;

    //Initializations
    searchwindow = GW_SEARCHWINDOW (gtk_window_get_transient_for (GTK_WINDOW (window)));
    g_assert (searchwindow != NULL);
    application = gw_window_get_application (GW_WINDOW (window));
    dictionarystore = gw_application_get_dictionarystore (application);
    dictinfolist = gw_dictionarystore_get_dictinfolist (GW_DICTIONARYSTORE (dictionarystore));
    di = lw_dictinfolist_get_dictinfo (dictinfolist, LW_DICTTYPE_KANJI, "Kanji");
    if (di == NULL) return;

    text_radicals = gw_radicalswindow_strdup_all_selected (window);
    text_strokes = gw_radicalswindow_strdup_prefered_stroke_count (window);
    text_query = g_strdup_printf ("%s%s", text_radicals, text_strokes);

    //Sanity checks
    if (text_query != NULL && strlen(text_query) > 0)
    {
      gw_searchwindow_entry_set_text (searchwindow, text_query);
      gw_searchwindow_set_dictionary (searchwindow, di->load_position);

      gw_searchwindow_search_cb (GTK_WIDGET (searchwindow), searchwindow);
    }

    //Cleanup
    if (text_query != NULL) g_free (text_query);
    if (text_strokes != NULL) g_free (text_strokes);
    if (text_radicals != NULL) g_free (text_radicals);
}


G_MODULE_EXPORT void 
gw_searchwindow_toggle_radicalswindow_cb (GtkAction *action, gpointer data)
{
    //Declarations
    GwSearchWindow *window;
    GwSearchWindowPrivate *priv;
    GwApplication *application;
    gboolean show;

    //Initializations
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));
    show = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));

    if (show)
    {
      if (priv->radicalswindow != NULL)
      {
        gtk_window_set_transient_for (GTK_WINDOW (priv->radicalswindow), GTK_WINDOW (window));
        gw_radicalswindow_deselect_all_radicals (GW_RADICALSWINDOW (priv->radicalswindow));
        gtk_window_present (GTK_WINDOW (priv->radicalswindow));
      }
      else
      {
        priv->radicalswindow = GW_RADICALSWINDOW (gw_radicalswindow_new (GTK_APPLICATION (application)));
        gtk_window_set_transient_for (GTK_WINDOW (priv->radicalswindow), GTK_WINDOW (window));

        g_signal_connect (
          G_OBJECT (priv->radicalswindow), 
          "query-changed", 
          G_CALLBACK (gw_searchwindow_radicalswindow_query_changed_cb), 
          window
        );

        g_signal_connect (
          G_OBJECT (priv->radicalswindow),
          "hide",
          G_CALLBACK (gw_searchwindow_radicalswindow_destroy_cb),
          window
        );

        gtk_widget_show (GTK_WIDGET (priv->radicalswindow));
        g_object_add_weak_pointer (G_OBJECT (priv->radicalswindow), (gpointer) &(priv->radicalswindow));
      }
    }
    else
    {
      if (priv->radicalswindow != NULL) gtk_widget_hide (GTK_WIDGET (priv->radicalswindow));
    }
}


void
gw_searchwindow_radicalswindow_destroy_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwSearchWindow *window;
    GwSearchWindowPrivate *priv;
    GtkWidget *toplevel;
    GtkAction *action;

    //Initializations
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    toplevel = gw_window_get_toplevel (GW_WINDOW (window));
    action = GTK_ACTION (priv->show_radicals_toggleaction);

    G_GNUC_EXTENSION g_signal_handlers_block_by_func (action, gw_searchwindow_toggle_radicalswindow_cb, toplevel);
    gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action), FALSE);
    G_GNUC_EXTENSION g_signal_handlers_unblock_by_func (action, gw_searchwindow_toggle_radicalswindow_cb, toplevel);
}


G_MODULE_EXPORT void 
gw_searchwindow_open_settingswindow_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwApplication *application;
    GwSearchWindow *window;
    GtkWindow *settingswindow;
    GList *iter;

    //Initializations
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    application = gw_window_get_application (GW_WINDOW (window));
    iter = gtk_application_get_windows (GTK_APPLICATION (application));

    while (iter != NULL && !GW_IS_SETTINGSWINDOW (iter->data)) iter = iter->next;

    if (iter != NULL)
    {
      settingswindow = GTK_WINDOW (iter->data);
      gtk_window_set_transient_for (GTK_WINDOW (settingswindow), GTK_WINDOW (window));
      gtk_window_present (GTK_WINDOW (settingswindow));
    }
    else
    {
      settingswindow = gw_settingswindow_new (GTK_APPLICATION (application));
      gtk_window_set_transient_for (GTK_WINDOW (settingswindow), GTK_WINDOW (window));
      gtk_widget_show (GTK_WIDGET (settingswindow));
    }
}


G_MODULE_EXPORT void 
gw_searchwindow_dictionaries_added_cb (GtkTreeModel *model, 
                                       GtkTreePath  *path, 
                                       GtkTreeIter  *iter, 
                                       gpointer      data  )
{
    //Lazy implimenation
    gw_searchwindow_dictionaries_deleted_cb (model, path, data);
}


//!
//! @brief Disables portions of the interface depending on the currently queued jobs.
//!
G_MODULE_EXPORT void 
gw_searchwindow_dictionaries_deleted_cb (GtkTreeModel *model, 
                                         GtkTreePath  *path, 
                                         gpointer      data  )
{
    //Declarations
    GwSearchWindow *window;
    GwSearchWindowPrivate *priv;
    GwApplication *application;
    GtkListStore *dictionarystore;
    LwDictInfoList *dictinfolist;
    GtkAction *action;
    gboolean enable;

    //Initializations
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));
    dictionarystore = gw_application_get_dictionarystore (application);
    dictinfolist = gw_dictionarystore_get_dictinfolist (GW_DICTIONARYSTORE (dictionarystore));

    //Update radicals window tool menuitem
    action = GTK_ACTION (priv->show_radicals_toggleaction);
    enable = (lw_dictinfolist_get_dictinfo (dictinfolist, LW_DICTTYPE_KANJI, "Kanji") != NULL);
    gtk_action_set_sensitive (action, enable);

    gw_searchwindow_initialize_dictionary_menu (window);
    gw_searchwindow_initialize_dictionary_combobox (window);

    //Set the show state of the dictionaries required message
    if (lw_dictinfolist_get_total (dictinfolist) > 0)
      gw_searchwindow_set_dictionary (window, 0);

    //Reset history and searchitems
    if (priv->history != NULL) lw_history_free (priv->history);
    priv->history = lw_history_new (20);

    GList *children, *link;
    children = link = gtk_container_get_children (GTK_CONTAINER (priv->notebook));
    if (children != NULL)
    {
      while (link != NULL)
      {
        if (link->data != NULL) g_object_set_data (G_OBJECT (link->data), "searchitem", NULL);
        link = link->next;
      }
      g_list_free (children); children = NULL;
    }

    gw_searchwindow_update_history_popups (window);
}


G_MODULE_EXPORT void 
gw_searchwindow_total_tab_pages_changed_cb (GtkNotebook *notebook, 
                                            GtkWidget   *child, 
                                            guint        page_num, 
                                            gpointer     data     )
{
    //Declarations
    GwSearchWindow *window;
    GwSearchWindowPrivate *priv;
    int pages;
    const char *label_text;

    pages = gtk_notebook_get_n_pages (notebook);
    if (page_num > 0)
    {
      //Initializations
      window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
      g_return_if_fail (window != NULL);
      priv = window->priv;

      gtk_action_set_sensitive (priv->previous_tab_action, (pages > 1));
      gtk_action_set_sensitive (priv->next_tab_action, (pages > 1));

      if (pages > 1)
        label_text = gettext("_Close Tab");
      else
        label_text = gettext("_Close");

      gtk_action_set_label (priv->close_action, label_text);
    }

    gtk_notebook_set_show_tabs (notebook, (pages > 1));
}


G_MODULE_EXPORT gboolean 
gw_searchwindow_focus_in_event_cb (GtkWidget *widget, 
                                   GdkEvent  *event, 
                                   gpointer   data   )
{
    GwSearchWindow *window;
    GwApplication *application;

    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_val_if_fail (window != NULL, FALSE);
    application = gw_window_get_application (GW_WINDOW (window));

    gw_application_set_last_focused_searchwindow (application, window);

    return FALSE;
}


G_MODULE_EXPORT void 
gw_searchwindow_open_vocabularywindow_cb (GtkWidget *widget, 
                                          gpointer   data   )
{
    //Declarations
    GwSearchWindow *window;
    GwApplication *application;
    GtkWindow *vocabularywindow;

    //Initializations
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    application = gw_window_get_application (GW_WINDOW (window));

    vocabularywindow = gw_vocabularywindow_new (GTK_APPLICATION (application));
    gtk_widget_show (GTK_WIDGET (vocabularywindow));
}


G_MODULE_EXPORT void 
gw_searchwindow_event_after_cb (GtkWidget *widget, 
                                GdkEvent  *event, 
                                gpointer   data)
{
    //Declarations
    GwSearchWindow *window;
    GwSearchWindowPrivate *priv;
    GtkAction *action_cut, *action_paste, *action_copy, *action_select_all;
    gboolean has_selection;
    GtkWidget *selectable;
  
    //Initializations
    window = GW_SEARCHWINDOW (widget);
    priv = window->priv;
    selectable = gtk_window_get_focus (GTK_WINDOW (window));
    if (selectable == NULL) return;
    action_cut = GTK_ACTION (priv->cut_action);
    action_copy = GTK_ACTION (priv->copy_action);
    action_paste = GTK_ACTION (priv->paste_action);
    action_select_all = GTK_ACTION (priv->select_all_action);
    has_selection = (gw_searchwindow_has_selection (window, selectable));

    //Set the sensitivity states
    if (GTK_IS_ENTRY (selectable))
    {
      gtk_action_set_sensitive (action_cut, has_selection);
      gtk_action_set_sensitive (action_copy, has_selection);
      gtk_action_set_sensitive (action_paste, TRUE);
      gtk_action_set_sensitive (action_select_all, TRUE);
    }
    else if (GTK_IS_TEXT_VIEW (selectable))
    {
      gtk_action_set_sensitive (action_cut, FALSE);
      gtk_action_set_sensitive (action_copy, has_selection);
      gtk_action_set_sensitive (action_paste, FALSE);
      gtk_action_set_sensitive (action_select_all, TRUE);
    }
    else
    {
      gtk_action_set_sensitive (action_cut, FALSE);
      gtk_action_set_sensitive (action_copy, FALSE);
      gtk_action_set_sensitive (action_paste, FALSE);
      gtk_action_set_sensitive (action_select_all, FALSE);
    }
}

static void
gw_searchwindow_add_vocabulary_destroy_cb (GwAddVocabularyWindow *window, gpointer data)
{
  printf("word added\n");
  GtkListStore *wordstore;

  wordstore = gw_addvocabularywindow_get_wordstore (window);
  if (wordstore != NULL)
  {
    gw_vocabularywordstore_save (GW_VOCABULARYWORDSTORE (wordstore), NULL);
    printf("save\n");
  }
}


G_MODULE_EXPORT void
gw_searchwindow_add_vocabulary_word_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwSearchWindow *window;
    GwApplication *application;
    GtkWindow *addvocabularywindow;

    //Initializations
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    application = gw_window_get_application (GW_WINDOW (window));

    addvocabularywindow = gw_addvocabularywindow_new (GTK_APPLICATION (application));
    g_signal_connect (G_OBJECT (addvocabularywindow), "word-added", G_CALLBACK (gw_searchwindow_add_vocabulary_destroy_cb), NULL);
    gtk_window_set_transient_for (addvocabularywindow, GTK_WINDOW (window));
    gtk_widget_show (GTK_WIDGET (addvocabularywindow));
}


G_MODULE_EXPORT void
gw_searchwindow_vocabulary_changed_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwSearchWindow *window;

    //Initializations
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);

    gw_searchwindow_update_vocabulary_menuitems (window);
}


G_MODULE_EXPORT void
gw_searchwindow_vocabulary_menuitem_activated_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwSearchWindow *window;
    GwApplication *application;
    GtkWindow *vocabularywindow;
    GtkMenuItem *menuitem;
    GtkTreePath *path;

    //Initializations
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    application = gw_window_get_application (GW_WINDOW (window));
    menuitem = GTK_MENU_ITEM (widget);
    path = (GtkTreePath*) g_object_get_data (G_OBJECT (menuitem), "tree-path");

    vocabularywindow = gw_vocabularywindow_new (GTK_APPLICATION (application));

    gw_vocabularywindow_set_selected_list (GW_VOCABULARYWINDOW (vocabularywindow), path);
    gw_vocabularywindow_show_vocabulary_list (GW_VOCABULARYWINDOW (vocabularywindow), FALSE);

    gtk_widget_show (GTK_WIDGET (vocabularywindow));

}


