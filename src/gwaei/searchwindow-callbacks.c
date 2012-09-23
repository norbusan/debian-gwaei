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


#include <string.h>
#include <stdlib.h>

#include <gdk/gdkkeysyms.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include <gwaei/gwaei.h>
#include <gwaei/gettext.h>
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
          data = g_object_get_data (G_OBJECT (tag), "word-data");
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
    LwDictionaryList *dictionarylist;
    LwDictionary *dictionary;
    GtkWidget *tooltip_window;
    gboolean is_hovering_kanji_character;
    GtkTextWindowType type;

    //Initializations
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_val_if_fail (window != NULL, FALSE);
    application = gw_window_get_application (GW_WINDOW (window));
    view = gw_searchwindow_get_current_textview (window);
    if (view == NULL) return FALSE;
    type = gtk_text_view_get_window_type (view, event->window);
    gtk_text_view_window_to_buffer_coords (view, type, (gint) event->x, (gint) event->y, &x, &y);
    gtk_text_view_get_iter_at_position (view, &iter, NULL, x, y);
    dictionarylist = LW_DICTIONARYLIST (gw_application_get_installed_dictionarylist (application));
    dictionary = lw_dictionarylist_get_dictionary (dictionarylist, LW_TYPE_KANJIDICTIONARY, "Kanji");
    tooltip_window = GTK_WIDGET (gtk_widget_get_tooltip_window (GTK_WIDGET (view)));

    is_hovering_kanji_character = gw_searchwindow_hovering_kanji_character (&iter);
    vocabulary_data = gw_searchwindow_hovering_vocabulary_data_tag (&iter);
 
    // Characters above 0xFF00 represent inserted images
    if ((is_hovering_kanji_character && dictionary != NULL) || vocabulary_data)
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
    if (view == NULL) return FALSE;
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
    GtkTextView *view;
    GtkTextIter iter;
    gint x;
    gint y;
    const gchar *vocabulary_data;
    LwDictionaryList *dictionarylist;
    LwDictionary *dictionary;
    GtkTextWindowType type;
    gboolean within_movement_threshold;
    gunichar character;

    //Initializations
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_val_if_fail (window != NULL, FALSE);
    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));
    view = gw_searchwindow_get_current_textview (window);
    if (view == NULL) return FALSE;
    type = gtk_text_view_get_window_type (view, event->window);
    gtk_text_view_window_to_buffer_coords (view, type, (gint) event->x, (gint) event->y, &x, &y);
    gtk_text_view_get_iter_at_position (view, &iter, NULL, x, y);
    dictionarylist = LW_DICTIONARYLIST (gw_application_get_installed_dictionarylist (application));
    dictionary = lw_dictionarylist_get_dictionary (dictionarylist, LW_TYPE_KANJIDICTIONARY, "Kanji");
    within_movement_threshold = (abs (priv->mouse_button_press_x - x) < 3 && abs (priv->mouse_button_press_y - y) < 3);

    character = gw_searchwindow_hovering_kanji_character (&iter);
    vocabulary_data = gw_searchwindow_hovering_vocabulary_data_tag (&iter);

    if (!within_movement_threshold) return FALSE;
 
    // Characters above 0xFF00 represent inserted images
    if (character && dictionary != NULL)
    {
      //Convert the unicode character into to a utf8 string
      gchar query[7];
      gint length = g_unichar_to_utf8 (character, query);
      query[length] = '\0'; 
      LwSearch *search;
      GError *error;

      //Start the search
      if (priv->mouse_item != NULL)
      {
        lw_search_cancel (priv->mouse_item);
        lw_search_free (priv->mouse_item);
        priv->mouse_item = NULL;
      }
      priv->mouse_button_press_x = event->x;
      priv->mouse_button_press_y = event->y;
      priv->mouse_button_press_root_x = event->x_root; //x position of the tooltip
      priv->mouse_button_press_root_y = event->y_root; //y position of the tooltip
      priv->mouse_button_character = character;
      error = NULL;
      search = lw_search_new (dictionary, query, 0, &error);
      if (search != NULL && error == NULL)
      {
        lw_search_start (search, TRUE);
        priv->mouse_item = search;
      }
      gw_application_handle_error (application, NULL, FALSE, &error);
    }
    else if (vocabulary_data)
    {
      GtkWindow *avw = gw_addvocabularywindow_new (GTK_APPLICATION (application));
      LwWord *vi = lw_word_new_from_string (vocabulary_data);
      gtk_window_set_transient_for (avw, GTK_WINDOW (window));
      gw_addvocabularywindow_set_kanji (GW_ADDVOCABULARYWINDOW (avw), lw_word_get_kanji (vi));
      gw_addvocabularywindow_set_furigana (GW_ADDVOCABULARYWINDOW (avw), lw_word_get_furigana (vi));
      gw_addvocabularywindow_set_definitions (GW_ADDVOCABULARYWINDOW (avw), lw_word_get_definitions (vi));
      lw_word_free (vi); vi = NULL;
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
gw_searchwindow_close_cb (GSimpleAction *action, 
                          GVariant      *variant, 
                          gpointer       data)
{
    //Declarations
    GwSearchWindow *window;
    GwSearchWindowPrivate *priv;
    GwApplication *application;
    gint pages;
    
    //Initializations
    window = GW_SEARCHWINDOW (data);
    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));
    pages = gtk_notebook_get_n_pages (priv->notebook);

    if (pages == 1) 
      gtk_widget_destroy (GTK_WIDGET (window));
    else
      gw_searchwindow_remove_current_tab_cb (GTK_WIDGET (window), data);

    if (gw_application_should_quit (application))
      gw_application_quit (application);
}


//!
//! @brief Goes back one step in the search history
//! @see gw_searchwindow_search_from_history_cb ()
//! @see gw_searchwindow_forward_cb ()
//! @param widget Unused GtkWidget pointer.
//! @param data pointer to a specially attached LwSearch variable
//!
G_MODULE_EXPORT void 
gw_searchwindow_go_back_cb (GSimpleAction *action,
                            GVariant      *parameter,
                            gpointer       data)
{
    //Declarations
    GwSearchWindow *window;
    
    //Initializations
    window = GW_SEARCHWINDOW (data);

    gw_searchwindow_go_back (window, 1);
}


//!
//! @brief Goes forward one step in the search history
//! @see gw_searchwindow_search_from_history_cb ()
//! @see gw_searchwindow_back_cb ()
//! @param widget Unused GtkWidget pointer.
//! @param data pointer to a specially attached LwSearch variable
//!
G_MODULE_EXPORT void 
gw_searchwindow_go_forward_cb (GSimpleAction *action,
                               GVariant      *parameter,
                               gpointer       data)
{
    //Declarations
    GwSearchWindow *window;
    
    //Initializations
    window = GW_SEARCHWINDOW (data);

    gw_searchwindow_go_forward (window, 1);
}


//!
//! @brief Goes back one step in the search history
//! @see gw_searchwindow_search_from_history_cb ()
//! @see gw_searchwindow_forward_cb ()
//! @param widget Unused GtkWidget pointer.
//! @param data pointer to a specially attached LwSearch variable
//!
G_MODULE_EXPORT void 
gw_searchwindow_go_back_index_cb (GSimpleAction *action,
                                  GVariant      *parameter,
                                  gpointer       data)
{
    //Declarations
    GwSearchWindow *window;
    const gchar *value;
    gint i;
    
    //Initializations
    window = GW_SEARCHWINDOW (data);
    value = g_variant_get_string (parameter, NULL);
    i = (gint) g_ascii_strtoll (value, NULL, 10);

    gw_searchwindow_go_back (window, i);
}


//!
//! @brief Goes forward one step in the search history
//! @see gw_searchwindow_search_from_history_cb ()
//! @see gw_searchwindow_back_cb ()
//! @param widget Unused GtkWidget pointer.
//! @param data pointer to a specially attached LwSearch variable
//!
G_MODULE_EXPORT void 
gw_searchwindow_go_forward_index_cb (GSimpleAction *action,
                                     GVariant      *parameter,
                                     gpointer       data)
{
    //Declarations
    GwSearchWindow *window;
    const gchar *value;
    gint i;
    
    //Initializations
    window = GW_SEARCHWINDOW (data);
    value = g_variant_get_string (parameter, NULL);
    i = (gint) g_ascii_strtoll (value, NULL, 10);

    gw_searchwindow_go_forward (window, i);
}


//!
//! @brief Saves the current search results to a file
//! @see gw_searchwindow_save_cb ()
//! @param widget Unused GtkWidget pointer.
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_searchwindow_save_as_cb (GSimpleAction *action,
                            GVariant      *parameter,
                            gpointer       data)
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
        gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), gettext ("word.txt"));
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
gw_searchwindow_save_cb (GSimpleAction *action,
                         GVariant      *parameter,
                         gpointer       data)
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
    window = GW_SEARCHWINDOW (data);
    application = gw_window_get_application (GW_WINDOW (window));
    gw_application_block_searches (application);
    error = NULL;

    //Sanity check for an empty save path
    if (path == NULL || *path == '\0')
    {
      gw_searchwindow_save_as_cb (action, parameter, data);
      gw_application_unblock_searches (application);
      return;
    }

    //Carry out the save
    view = gw_searchwindow_get_current_textview (window);
    if (view == NULL) return;
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
gw_searchwindow_zoom_in_cb (GSimpleAction *action,
                            GVariant      *parameter,
                            gpointer       data)
{
    GwApplication *application;
    GwSearchWindow *window;
    LwPreferences *preferences;
    gint size;

    window = GW_SEARCHWINDOW (data);
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
gw_searchwindow_zoom_out_cb (GSimpleAction *action,
                             GVariant      *parameter,
                             gpointer       data)
{
    GwApplication *application;
    GwSearchWindow *window;
    LwPreferences *preferences;
    gint size;

    window = GW_SEARCHWINDOW (data);
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
gw_searchwindow_zoom_100_cb (GSimpleAction *action,
                             GVariant      *parameter,
                             gpointer       data)
{
    //Declarations
    GwApplication *application;
    GwSearchWindow *window;
    LwPreferences *preferences;

    //Initializations
    window = GW_SEARCHWINDOW (data);
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
    gint active;

    //Initializations
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (widget), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    active = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));

    gw_searchwindow_set_dictionary (window, active);

    gtk_widget_grab_focus (GTK_WIDGET (priv->entry));
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
gw_searchwindow_select_all_cb (GSimpleAction *widget, 
                               GVariant      *parameter,
                               gpointer       data)
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
gw_searchwindow_paste_cb (GSimpleAction *action, 
                          GVariant      *parameter,
                          gpointer       data)
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
void 
gw_searchwindow_cut_cb (GSimpleAction *action, 
                        GVariant      *parameter,
                        gpointer       data)
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
void 
gw_searchwindow_copy_cb (GSimpleAction *action, 
                         GVariant      *parameter,
                         gpointer       data)
{
    GwSearchWindow *window;
    GtkWidget *focus;

    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    focus = gtk_window_get_focus (GTK_WINDOW (window));

    gw_searchwindow_copy_text (window, focus);
}
 

//!
//! @brief Cycles the active dictionaries down the list
//! @see gw_searchwindow_cycle_dictionaries_backward_cb ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_searchwindow_next_dictionary_cb (GSimpleAction *action,
                                    GVariant      *parameter,
                                    gpointer       data)
{
    GwSearchWindow *window;
    window = GW_SEARCHWINDOW (data);

    gw_searchwindow_cycle_dictionaries (window, FALSE);
}


//!
//! @brief Cycles the active dictionaries up the list
//! @see gw_searchwindow_cycle_dictionaries_forward_cb ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_searchwindow_previous_dictionary_cb (GSimpleAction *action,
                                        GVariant      *parameter,
                                        gpointer       data)
{
    GwSearchWindow *window;
    window = GW_SEARCHWINDOW (data);

    gw_searchwindow_cycle_dictionaries (window, TRUE);
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
    if (view == NULL) return FALSE;
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
    if (view == NULL) return FALSE;
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
    gchar query[50];
    LwSearch *search;
    LwSearch *new_item;
    LwDictionary *dictionary;
    GError *error;
    GwSearchData *sdata;
    GtkTextView *view;
    gint index;
    LwHistory *history;
    LwSearchFlags flags;

    //Initializations
    error = NULL;
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));
    history = LW_HISTORY (priv->history);

    if (!gw_application_can_start_search (application)) return;

    preferences = gw_application_get_preferences (application);
    flags = lw_search_get_flags_from_preferences (preferences);
    strncpy (query, gtk_entry_get_text (priv->entry), 50);
    index = gw_searchwindow_get_current_tab_index (window);
    search = gw_searchwindow_get_searchitem_by_index (window, index);
    dictionary = gw_searchwindow_get_dictionary (window);
    gw_searchwindow_guarantee_first_tab (window);

    //Cancel all searches if the search bar is empty
    if (strlen(query) == 0 || dictionary == NULL) 
    {
      lw_search_cancel (search);
      return;
    }

    view = gw_searchwindow_get_current_textview (window);
    new_item = lw_search_new (dictionary, query, flags, &error);
    if (new_item == NULL)
    {
      gw_application_handle_error (application, NULL, FALSE, &error);
      return;
    }
    sdata = gw_searchdata_new (view, window);
    lw_search_set_data (new_item, sdata, LW_SEARCH_DATA_FREE_FUNC (gw_searchdata_free));

    //Check for problems, and quit if there are
    if (error != NULL ||
        new_item == NULL ||
        lw_search_is_equal (search, new_item)
       )
    {
      if (new_item != NULL)
      {
        lw_search_free (new_item);
        new_item = NULL;
      }

      gw_application_handle_error (application, NULL, FALSE, &error);

      return;
    }

    if (priv->new_tab)
    {
      gw_searchwindow_new_tab (window);
      search = NULL;
    }
    else
    {
      lw_search_cancel (search);
    }

    //Push the previous searchitem or replace it with the new one
    if (lw_history_has_relevance (history, search, priv->keep_searching_enabled))
    {
      search = gw_searchwindow_steal_searchitem_by_index (window, index);
      if (search != NULL) 
      {
        lw_history_add_search (history, search);
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
gw_searchwindow_insert_unknown_character_cb (GSimpleAction *action,
                                             GVariant      *parameter,
                                             gpointer       data)
{
    GwSearchWindow *window;
    window = GW_SEARCHWINDOW (data);
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
gw_searchwindow_insert_word_edge_cb (GSimpleAction *action, 
                                     GVariant      *parameter,
                                     gpointer       data)
{
    GwSearchWindow *window;
    window = GW_SEARCHWINDOW (data);
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
gw_searchwindow_insert_not_word_edge_cb (GSimpleAction *action, 
                                         GVariant      *parameter, 
                                         gpointer       data)
{
    GwSearchWindow *window;
    window = GW_SEARCHWINDOW (data);
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
gw_searchwindow_insert_and_cb (GSimpleAction *action,
                               GVariant      *parameter,
                               gpointer       data)
{
    GwSearchWindow *window;
    window = GW_SEARCHWINDOW (data);
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
gw_searchwindow_insert_or_cb (GSimpleAction *action, 
                              GVariant      *parameter,
                              gpointer       data)
{
    GwSearchWindow *window;
    window = GW_SEARCHWINDOW (data);
    gw_searchwindow_entry_insert_text (window, "|");
}


//!
//! @brief Clears the search entry and moves the focus to it
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_searchwindow_clear_search_cb (GSimpleAction *action, 
                                 GVariant      *parameter, 
                                 gpointer       data)
{
    GwSearchWindow *window;
    GwSearchWindowPrivate *priv;

    window = GW_SEARCHWINDOW (data);
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
      gtk_entry_set_text (priv->entry, text);
      gtk_widget_grab_focus (GTK_WIDGET (priv->entry));
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
gw_searchwindow_update_button_states_based_on_entry_text_cb (GtkEditable *editable,
                                                             gpointer     data   )
{
    //Declarations
    GtkIconTheme *theme;
    GtkEntry *entry;
    const gchar* NAME;
    gint length;

    //Initializations
    entry = GTK_ENTRY (editable);
    length = gtk_entry_get_text_length (entry);
    NAME = NULL;

    //Show the clear icon when approprate
    if (length > 0) 
    {
      theme = gtk_icon_theme_get_default ();
      if (gtk_icon_theme_has_icon (theme, "edit-clear-symbolic")) NAME = "edit-clear-symbolic";
      else NAME = "edit-clear";
    }

    gtk_entry_set_icon_from_icon_name (entry, GTK_ENTRY_ICON_SECONDARY, NAME);
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
        //gw_searchwindow_zoom_out_cb (widget, data);
        return TRUE; // dont propagate event, no scroll
      }

      if(event->direction == GDK_SCROLL_DOWN)
      {
        //gw_searchwindow_zoom_in_cb (widget, data);
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
gw_searchwindow_new_tab_cb (GSimpleAction *action,
                            GVariant      *parameter,
                            gpointer       data)
{
    gw_searchwindow_new_tab (GW_SEARCHWINDOW (data));
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
    gint page_num;

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
                               gint         page_num, 
                               gpointer     data)
{
    //Declarations
    GwSearchWindow *window;
    GwSearchWindowPrivate *priv;
    LwSearch *search;
    GtkWidget *container;

    //Initializations
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    search = NULL;

    container = gtk_notebook_get_nth_page (priv->notebook, page_num);
    if (container != NULL)
    {
      search = LW_SEARCH (g_object_get_data (G_OBJECT (container), "searchitem"));
      gw_searchwindow_set_dictionary_by_searchitem (window, search);

      gw_searchwindow_set_entry_text_by_searchitem (window, search);
      gw_searchwindow_set_title_by_searchitem (window, search);
      gw_searchwindow_set_total_results_label_by_searchitem (window, search);
      gw_searchwindow_set_search_progressbar_by_searchitem (window, search);

      gboolean enabled = (search != NULL);
      GActionMap *map = G_ACTION_MAP (window);
      GSimpleAction *action = NULL;

      //Update Save sensitivity state
      action = G_SIMPLE_ACTION (g_action_map_lookup_action (map, "save"));
      g_simple_action_set_enabled (action, enabled);

      //Update Save as sensitivity state
      action = G_SIMPLE_ACTION (g_action_map_lookup_action (map, "save-as"));
      g_simple_action_set_enabled (action, enabled);

      //Update Print sensitivity state
      action = G_SIMPLE_ACTION (g_action_map_lookup_action (map, "print"));
      g_simple_action_set_enabled (action, enabled);

      //Update Print preview sensitivity state
      action = G_SIMPLE_ACTION (g_action_map_lookup_action (map, "print-preview"));
      g_simple_action_set_enabled (action, enabled);
    }
}


//!
//! @brief Cycles to the next tab 
//! @param widget Currently unused widget pointer
//! @param data Currently unused gpointer
//!
G_MODULE_EXPORT void 
gw_searchwindow_next_tab_cb (GSimpleAction *action, 
                             GVariant      *parameter,
                             gpointer       data)
{
    GwSearchWindow *window;
    GwSearchWindowPrivate *priv;

    window = GW_SEARCHWINDOW (data);
    priv = window->priv;

    gtk_notebook_next_page (priv->notebook);
}


//!
//! @brief Cycles to the previous tab 
//! @param widget Currently unused widget pointer
//! @param data Currently unused gpointer
//!
G_MODULE_EXPORT void 
gw_searchwindow_previous_tab_cb (GSimpleAction *action,
                                 GVariant      *parameter,
                                 gpointer       data)
{
    GwSearchWindow *window;
    GwSearchWindowPrivate *priv;

    window = GW_SEARCHWINDOW (data);
    priv = window->priv;

    gtk_notebook_prev_page (priv->notebook);
}


G_MODULE_EXPORT void 
gw_searchwindow_no_results_search_for_dictionary_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwSearchWindow *window;
    gint position;

    //Initializations
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    position = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (widget), "load-position"));

    gw_searchwindow_set_dictionary (window, position);

    gw_searchwindow_search_cb (widget, data);
}


//!
//! @brief Sets the show menu boolean to match the widget
//! @see gw_searchwindow_set_menu_show ()
//! @param widget Unused GtkWidget pointer.
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_searchwindow_menubar_show_toggled_cb (GSimpleAction *action, 
                                         GVariant      *parameter, 
                                         gpointer       data)
{
    //Declarations
    GwApplication *application;
    GwSearchWindow *window;
    LwPreferences *preferences;
    gboolean show;

    //Initializations
    window = GW_SEARCHWINDOW (data);
    application = gw_window_get_application (GW_WINDOW (window));
    preferences = gw_application_get_preferences (application);
    show = lw_preferences_get_boolean_by_schema (preferences, LW_SCHEMA_BASE, LW_KEY_MENUBAR_SHOW);

    lw_preferences_set_boolean_by_schema (preferences, LW_SCHEMA_BASE, LW_KEY_MENUBAR_SHOW, !show);
}


//!
//! @brief Syncs the gui to the preference settinging.  It should be attached to the gsettings object
//!
G_MODULE_EXPORT void 
gw_searchwindow_sync_menubar_show_cb (GSettings *settings, 
                                      gchar     *key, 
                                      gpointer   data)
{
    //Declarations
    GwSearchWindow *window;
    GwSearchWindowPrivate *priv;
    gboolean show;
    GAction *action;
    GtkSettings *gtk_settings;
    gboolean shell_shows_menubar;

    //Initializations
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    gtk_settings = gtk_settings_get_default ();
    g_object_get (G_OBJECT (gtk_settings), "gtk-shell-shows-menubar", &shell_shows_menubar, NULL);
    show = (lw_preferences_get_boolean (settings, key));
    action = g_action_map_lookup_action (G_ACTION_MAP (window), "toggle-menubar-show");

    gw_window_show_menubar (GW_WINDOW (window), show && !shell_shows_menubar);
    gtk_window_set_hide_titlebar_when_maximized (GTK_WINDOW (window), !show || shell_shows_menubar);
    g_simple_action_set_state (G_SIMPLE_ACTION (action), g_variant_new_boolean (show));

    if (priv->menu_toolbutton != NULL)
    {
      if (show == TRUE || shell_shows_menubar)
        gtk_widget_hide (GTK_WIDGET (priv->menu_toolbutton));
      else
        gtk_widget_show (GTK_WIDGET (priv->menu_toolbutton));
    }
}


//!
//! @brief Sets the show toolbar boolean to match the widget
//! @see gw_searchwindow_set_toolbar_show ()
//! @param widget Unused GtkWidget pointer.
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_searchwindow_toolbar_show_toggled_cb (GSimpleAction *action, 
                                         GVariant      *parameter, 
                                         gpointer       data)
{
    //Declarations
    GwApplication *application;
    GwSearchWindow *window;
    LwPreferences *preferences;
    gboolean show;

    //Initializations
    window = GW_SEARCHWINDOW (data);
    application = gw_window_get_application (GW_WINDOW (window));
    preferences = gw_application_get_preferences (application);
    show = lw_preferences_get_boolean_by_schema (preferences, LW_SCHEMA_BASE, LW_KEY_TOOLBAR_SHOW);

    lw_preferences_set_boolean_by_schema (preferences, LW_SCHEMA_BASE, LW_KEY_TOOLBAR_SHOW, !show);
}


//!
//! @brief Syncs the gui to the preference settinging.  It should be attached to the gsettings object
//!
G_MODULE_EXPORT void 
gw_searchwindow_sync_toolbar_show_cb (GSettings *settings, 
                                      gchar     *key, 
                                      gpointer   data)
{
    //Declarations
    GwSearchWindow *window;
    GwSearchWindowPrivate *priv;
    gboolean show;
    GtkToolbar *primary_toolbar;
    GAction *action;

    //Initializations
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    show = lw_preferences_get_boolean (settings, key);
    primary_toolbar = GTK_TOOLBAR (priv->primary_toolbar);
    action = g_action_map_lookup_action (G_ACTION_MAP (window), "toggle-toolbar-show");

    if (show == TRUE)
      gtk_widget_show (GTK_WIDGET (primary_toolbar));
    else
      gtk_widget_hide (GTK_WIDGET (primary_toolbar));

    g_simple_action_set_state (G_SIMPLE_ACTION (action), g_variant_new_boolean (show));
}


//!
//! @brief Sets the show menu boolean to match the widget
//! @see gw_searchwindow_set_menu_show ()
//! @param widget Unused GtkWidget pointer.
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_searchwindow_tabbar_show_toggled_cb (GSimpleAction *action, 
                                        GVariant      *parameter, 
                                        gpointer       data)
{
    //Declarations
    GwApplication *application;
    GwSearchWindow *window;
    LwPreferences *preferences;
    gboolean show;

    //Initializations
    window = GW_SEARCHWINDOW (data);
    application = gw_window_get_application (GW_WINDOW (window));
    preferences = gw_application_get_preferences (application);
    show = lw_preferences_get_boolean_by_schema (preferences, LW_SCHEMA_BASE, LW_KEY_TABBAR_SHOW);

    lw_preferences_set_boolean_by_schema (preferences, LW_SCHEMA_BASE, LW_KEY_TABBAR_SHOW, !show);
}


//!
//! @brief Syncs the gui to the preference settinging.  It should be attached to the gsettings object
//!
G_MODULE_EXPORT void 
gw_searchwindow_sync_tabbar_show_cb (GSettings *settings, 
                                      gchar     *key, 
                                      gpointer   data)
{
    //Declarations
    GwSearchWindow *window;
    GwSearchWindowPrivate *priv;
    gboolean show;
    GAction *action;

    //Initializations
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    show = lw_preferences_get_boolean (settings, key);
    action = g_action_map_lookup_action (G_ACTION_MAP (window), "toggle-tabbar-show");
    priv->always_show_tabbar = show;

    g_simple_action_set_state (G_SIMPLE_ACTION (action), g_variant_new_boolean (show));
    gw_searchwindow_sync_tabbar_show (window);
}

//!
//! @brief Sets the show toolbar boolean to match the widget
//! @see gw_searchwindow_set_toolbar_show ()
//! @param widget Unused GtkWidget pointer.
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_searchwindow_statusbar_show_toggled_cb (GSimpleAction *action, 
                                           GVariant      *parameter, 
                                           gpointer       data)
{
    //Declarations
    GwApplication *application;
    GwSearchWindow *window;
    LwPreferences *preferences;
    gboolean show;

    //Initializations
    window = GW_SEARCHWINDOW (data);
    application = gw_window_get_application (GW_WINDOW (window));
    preferences = gw_application_get_preferences (application);
    show = lw_preferences_get_boolean_by_schema (preferences, LW_SCHEMA_BASE, LW_KEY_STATUSBAR_SHOW);

    lw_preferences_set_boolean_by_schema (preferences, LW_SCHEMA_BASE, LW_KEY_STATUSBAR_SHOW, !show);
}


//!
//! @brief Sets the checkbox to show or hide the statusbar
//! @param show How to set the statusbar
//!
G_MODULE_EXPORT void 
gw_searchwindow_sync_statusbar_show_cb (GSettings *settings, 
                                        gchar     *key, 
                                        gpointer   data)
{
    //Declarations
    GwSearchWindow *window;
    GwSearchWindowPrivate *priv;
    gboolean show;
    GAction *action;

    //Initializations
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    show = lw_preferences_get_boolean (settings, key);
    action = g_action_map_lookup_action (G_ACTION_MAP (window), "toggle-statusbar-show");

    if (show == TRUE)
      gtk_widget_show (priv->statusbar);
    else
      gtk_widget_hide (priv->statusbar);

    g_simple_action_set_state (G_SIMPLE_ACTION (action), g_variant_new_boolean (show));
}


//!
//! @brief Sets the requested font with magnification applied
//!
G_MODULE_EXPORT void 
gw_searchwindow_sync_font_cb (GSettings *settings, 
                              gchar     *KEY, 
                              gpointer   data)
{
    //Declarations
    GwSearchWindow *window;

    //Initializations
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);

    gw_searchwindow_set_font (window);
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
gw_searchwindow_toggle_kanjipadwindow_cb (GSimpleAction *action, 
                                          GVariant      *parameter, 
                                          gpointer       data)
{
    //Declarations
    GwSearchWindow *window;
    GwSearchWindowPrivate *priv;
    GwApplication *application;
    gboolean show;

    //Initializations
    window = GW_SEARCHWINDOW (data);
    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));
    show = g_variant_get_boolean (parameter);

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

        priv->signalid[GW_SEARCHWINDOW_SIGNALID_KANJIPADWINDOW_CLOSED] = g_signal_connect (
          G_OBJECT (priv->kanjipadwindow),
          "hide",
          G_CALLBACK (gw_searchwindow_kanjipadwindow_destroy_cb),
          window
        );

        gtk_widget_show (GTK_WIDGET (priv->kanjipadwindow));
        gtk_window_present (GTK_WINDOW (priv->kanjipadwindow));
        g_object_add_weak_pointer (G_OBJECT (priv->kanjipadwindow), (gpointer) &(priv->kanjipadwindow));
      }
    }
    else
    {
      if (priv->kanjipadwindow != NULL) gtk_widget_hide (GTK_WIDGET (priv->kanjipadwindow));
    }

    g_simple_action_set_state (action, parameter);
}


void
gw_searchwindow_kanjipadwindow_destroy_cb (GtkWidget *widget, gpointer data)
{
    //Sanity checks
    g_return_if_fail (data != NULL);

    //Declarations
    GwSearchWindow *window;
    GSimpleAction *action;
    GActionMap *map;

    //Initializations
    window = GW_SEARCHWINDOW (data);
    map = G_ACTION_MAP (window);
    action = G_SIMPLE_ACTION (g_action_map_lookup_action (map, "toggle-kanjipad-show"));

    g_simple_action_set_state (action, g_variant_new_boolean (FALSE));
}



//!
//! @brief Sets the query created in the radicalwindow to the searchwindow
//!
static void
gw_searchwindow_radicalswindow_query_changed_cb (GwRadicalsWindow *window, gpointer data)
{
    //Sanity checks
    g_return_if_fail (window != NULL); 

    //Declarations
    GwSearchWindow *searchwindow;
    GwApplication *application;
    LwDictionaryList *dictionarylist;
    LwDictionary *dictionary;
    gchar *text_query;
    gchar *text_radicals;
    gchar *text_strokes;
    gint position;

    //Initializations
    searchwindow = GW_SEARCHWINDOW (gtk_window_get_transient_for (GTK_WINDOW (window)));
    g_assert (searchwindow != NULL);
    application = gw_window_get_application (GW_WINDOW (window));
    dictionarylist = LW_DICTIONARYLIST (gw_application_get_installed_dictionarylist (application));
    dictionary = lw_dictionarylist_get_dictionary (dictionarylist, LW_TYPE_KANJIDICTIONARY, "Kanji");
    position = lw_dictionarylist_get_position (dictionarylist, dictionary);
    if (dictionary == NULL) return;

    text_radicals = gw_radicalswindow_strdup_selected (window);
    text_strokes = gw_radicalswindow_strdup_prefered_stroke_count (window);
    text_query = g_strdup_printf ("%s%s", text_radicals, text_strokes);

    //Sanity checks
    if (text_query != NULL && strlen(text_query) > 0)
    {
      gw_searchwindow_entry_set_text (searchwindow, text_query);
      gw_searchwindow_set_dictionary (searchwindow, position);

      gw_searchwindow_search_cb (GTK_WIDGET (searchwindow), searchwindow);
    }

    //Cleanup
    if (text_query != NULL) g_free (text_query);
    if (text_strokes != NULL) g_free (text_strokes);
    if (text_radicals != NULL) g_free (text_radicals);
}


G_MODULE_EXPORT void 
gw_searchwindow_toggle_radicalswindow_cb (GSimpleAction *action, 
                                          GVariant      *parameter,
                                          gpointer       data)
{
    //Declarations
    GwSearchWindow *window;
    GwSearchWindowPrivate *priv;
    GwApplication *application;
    gboolean show;

    //Initializations
    window = GW_SEARCHWINDOW (data);
    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));
    show = g_variant_get_boolean (parameter);

    if (show)
    {
      if (priv->radicalswindow != NULL)
      {
        gtk_window_set_transient_for (GTK_WINDOW (priv->radicalswindow), GTK_WINDOW (window));
        gw_radicalswindow_deselect (GW_RADICALSWINDOW (priv->radicalswindow));
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

        priv->signalid[GW_SEARCHWINDOW_SIGNALID_RADICALSWINDOW_CLOSED] = g_signal_connect (
          G_OBJECT (priv->radicalswindow),
          "hide",
          G_CALLBACK (gw_searchwindow_radicalswindow_destroy_cb),
          window
        );

        gtk_widget_show (GTK_WIDGET (priv->radicalswindow));
        gtk_window_present (GTK_WINDOW (priv->radicalswindow));
        g_object_add_weak_pointer (G_OBJECT (priv->radicalswindow), (gpointer) &(priv->radicalswindow));
      }
    }
    else
    {
      if (priv->radicalswindow != NULL) gtk_widget_hide (GTK_WIDGET (priv->radicalswindow));
    }

    g_simple_action_set_state (action, parameter);
}


void
gw_searchwindow_radicalswindow_destroy_cb (GtkWidget *widget, gpointer data)
{
    //Sanity checks
    g_return_if_fail (data != NULL);

    //Declarations
    GwSearchWindow *window;
    GSimpleAction *action;
    GActionMap *map;

    //Initializations
    window = GW_SEARCHWINDOW (data);
    map = G_ACTION_MAP (window);
    action = G_SIMPLE_ACTION (g_action_map_lookup_action (map, "toggle-radicals-show"));

    g_simple_action_set_state (action, g_variant_new_boolean (FALSE));
}


//!
//! @brief Disables portions of the interface depending on the currently queued jobs.
//!
G_MODULE_EXPORT void 
gw_searchwindow_dictionaries_changed_cb (GwSearchWindow   *window,
                                         LwDictionaryList *dictionarylist)
{
    //Declarations
    GwSearchWindowPrivate *priv;
    gboolean enabled;
    GActionMap *map;
    GSimpleAction *action;
    LwHistory *history;

    //Initializations
    g_return_if_fail (window != NULL);
    priv = window->priv;
    map = G_ACTION_MAP (window);
    history = LW_HISTORY (priv->history);

    //Update radicals window tool menuitem
    action = G_SIMPLE_ACTION (g_action_map_lookup_action (map, "toggle-radicals-show"));
    enabled = (lw_dictionarylist_get_dictionary (dictionarylist, LW_TYPE_KANJIDICTIONARY, "Kanji") != NULL);
    g_simple_action_set_enabled (action, enabled);

    //Set the show state of the dictionaries required message
    if (lw_dictionarylist_get_total (dictionarylist) > 0)
      gw_searchwindow_set_dictionary (window, 0);

    //Reset history and searchitems
    lw_history_clear_forward_list (history);
    lw_history_clear_back_list (history);

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
}


G_MODULE_EXPORT void 
gw_searchwindow_total_tab_pages_changed_cb (GtkNotebook *notebook, 
                                            GtkWidget   *child, 
                                            guint        page_num, 
                                            gpointer     data     )
{
    //Declarations
    GwSearchWindow *window;

    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    if (window == NULL) return;

    gw_searchwindow_sync_tabbar_show (window);
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
gw_searchwindow_open_vocabularywindow_cb (GSimpleAction *action, 
                                          GVariant      *parameter,
                                          gpointer       data)
{
    //Declarations
    GwSearchWindow *window;
    GwApplication *application;
    GtkWindow *vocabularywindow;

    //Initializations
    window = GW_SEARCHWINDOW (data);
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
    GActionMap *map;
    GSimpleAction *action_cut, *action_paste, *action_copy, *action_select_all;
    gboolean has_selection;
    GtkWidget *selectable;
  
    //Initializations
    window = GW_SEARCHWINDOW (widget);
    map = G_ACTION_MAP (window);
    selectable = gtk_window_get_focus (GTK_WINDOW (window));
    if (selectable == NULL) return;
    action_cut = G_SIMPLE_ACTION (g_action_map_lookup_action (map, "cut"));
    action_copy = G_SIMPLE_ACTION (g_action_map_lookup_action (map, "copy"));
    action_paste = G_SIMPLE_ACTION (g_action_map_lookup_action (map, "paste"));
    action_select_all = G_SIMPLE_ACTION (g_action_map_lookup_action (map, "select-all"));
    has_selection = (gw_searchwindow_has_selection (window, selectable));

    //Set the sensitivity states
    if (GTK_IS_ENTRY (selectable))
    {
      g_simple_action_set_enabled (action_cut, has_selection);
      g_simple_action_set_enabled (action_copy, has_selection);
      g_simple_action_set_enabled (action_paste, TRUE);
      g_simple_action_set_enabled (action_select_all, TRUE);
    }
    else if (GTK_IS_TEXT_VIEW (selectable))
    {
      g_simple_action_set_enabled (action_cut, FALSE);
      g_simple_action_set_enabled (action_copy, has_selection);
      g_simple_action_set_enabled (action_paste, FALSE);
      g_simple_action_set_enabled (action_select_all, TRUE);
    }
    else
    {
      g_simple_action_set_enabled (action_cut, FALSE);
      g_simple_action_set_enabled (action_copy, FALSE);
      g_simple_action_set_enabled (action_paste, FALSE);
      g_simple_action_set_enabled (action_select_all, FALSE);
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
gw_searchwindow_add_vocabulary_word_cb (GSimpleAction *action, 
                                        GVariant      *parameter,
                                        gpointer       data)
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
gw_searchwindow_set_dictionary_cb (GSimpleAction *action,
                                   GVariant      *parameter,
                                   gpointer       data)
{
    //Declarations
    GwSearchWindow *window;
    const gchar *position;
    gint index;

    //Initializations
    window = GW_SEARCHWINDOW (data);
    position = g_variant_get_string (parameter, NULL);
    index = (gint) g_ascii_strtoll (position, NULL, 10) - 1;

    gw_searchwindow_set_dictionary (window, index);
}


static void
gw_searchwindow_detach_popup (GwSearchWindow *window, GtkMenu *menu)
{
    g_return_if_fail (menu != NULL);
    gtk_widget_destroy (GTK_WIDGET (menu));
}


GtkMenu*
gw_searchwindow_get_toolbar_menu (GwSearchWindow *window)
{
    //Sanity checks
    g_return_val_if_fail (window != NULL, NULL);

    //Declarations
    GwSearchWindowPrivate *priv;
    GMenuModel *menumodel;
    GtkBuilder *builder;

    //Initializations
    priv = window->priv;

    if (priv->toolbar_menu == NULL)
    {
      builder = gtk_builder_new ();
      if (builder != NULL)
      {
        gw_application_load_xml (builder, "searchwindow-menumodel-toolbar.ui");
        menumodel = G_MENU_MODEL (gtk_builder_get_object (builder, "menu"));
        priv->toolbar_menu = GTK_MENU (gtk_menu_new_from_model (menumodel));
        gtk_menu_attach_to_widget (priv->toolbar_menu, GTK_WIDGET (window), (GtkMenuDetachFunc) gw_searchwindow_detach_popup);
        
        g_object_unref (builder); builder = NULL;
      }
    }

    return priv->toolbar_menu;
}



G_MODULE_EXPORT gboolean
gw_searchwindow_show_toolbar_popup_cb (GtkToolbar *toolbar, 
                                       gint        x, 
                                       gint        y,
                                       gint        button, 
                                       gpointer    data)
{
    //Declarations
    GwSearchWindow *window;
    GtkMenu *menu;

    //Initializations
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_val_if_fail (window != NULL, FALSE);
    menu = gw_searchwindow_get_toolbar_menu (window);

    gtk_menu_popup (menu, NULL, NULL, NULL, NULL, button, gtk_get_current_event_time ());
    gtk_widget_show_all (GTK_WIDGET (menu));

    return TRUE;
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
    gboolean request;

    //Initializations
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));
    preferences = gw_application_get_preferences (application);
    request = lw_preferences_get_boolean_by_schema (preferences, LW_SCHEMA_BASE, LW_KEY_SPELLCHECK);

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


//!
//! @brief Makes sure the selected combobox item is sane when the dictionary list is updated
//!
G_MODULE_EXPORT void
gw_searchwindow_dictionarylist_changed_cb (GwSearchWindow *window, GwDictionaryList *dictionarylist)
{
    //Sanity checks
    g_return_if_fail (window != NULL);
    g_return_if_fail (dictionarylist != NULL);

    //Declarations
    GtkTreeModel *treemodel;
    gint children;
    GwSearchWindowPrivate *priv;
    GtkComboBox *combobox;
    gint active;

    //Initializations
    treemodel = GTK_TREE_MODEL (gw_dictionarylist_get_liststore (dictionarylist));
    children = gtk_tree_model_iter_n_children (treemodel, NULL);
    priv = window->priv;
    combobox = priv->combobox;
    active = gtk_combo_box_get_active (combobox);

    if (active < 0 && children > 0) gw_searchwindow_set_dictionary (window, 0);
}

