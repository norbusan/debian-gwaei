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
//! @file searchwindow.c
//!
//! @brief To be written
//!

#include <stdlib.h>
#include <string.h>

#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include <gwaei/gwaei.h>
#include <gwaei/searchwindow-private.h>


//Static declarations
static void gw_searchwindow_attach_signals (GwSearchWindow*);
static void gw_searchwindow_remove_signals (GwSearchWindow*);

G_DEFINE_TYPE (GwSearchWindow, gw_searchwindow, GW_TYPE_WINDOW)

//!
//! @brief Sets up the variables in main-interface.c and main-callbacks.c for use
//!
GtkWindow* 
gw_searchwindow_new (GtkApplication *application)
{
    g_assert (application != NULL);

    //Declarations
    GwSearchWindow *window;

    //Initializations
    window = GW_SEARCHWINDOW (g_object_new (GW_TYPE_SEARCHWINDOW,
                                            "type",        GTK_WINDOW_TOPLEVEL,
                                            "application", GW_APPLICATION (application),
                                            "ui-xml",      "searchwindow.ui",
                                            NULL));

    return GTK_WINDOW (window);
}


static void 
gw_searchwindow_init (GwSearchWindow *window)
{
    window->priv = GW_SEARCHWINDOW_GET_PRIVATE (window);
    memset(window->priv, 0, sizeof(GwSearchWindowPrivate));

    GwSearchWindowPrivate *priv;
    priv = window->priv;

    priv->feedback_status = LW_SEARCHSTATUS_IDLE;
}


static void 
gw_searchwindow_finalize (GObject *object)
{
    GwSearchWindow *window;
    GwSearchWindowPrivate *priv;

    window = GW_SEARCHWINDOW (object);
    priv = window->priv;

    gw_searchwindow_cancel_all_searches (window);

    if (priv->spellcheck) gw_spellcheck_free (priv->spellcheck); priv->spellcheck = NULL;
    if (priv->history) lw_history_free (priv->history); priv->history = NULL;
    if (priv->tablist) g_list_free (priv->tablist); priv->tablist = NULL;
    if (priv->mouse_hovered_word) g_free (priv->mouse_hovered_word); priv->mouse_hovered_word = NULL;
    if (priv->keep_searching_query) g_free (priv->keep_searching_query); priv->keep_searching_query = NULL;

    G_OBJECT_CLASS (gw_searchwindow_parent_class)->finalize (object);
}


static void 
gw_searchwindow_constructed (GObject *object)
{
    //Declarations
    GwSearchWindow *window;
    GwSearchWindowPrivate *priv;
    GtkToolButton *toolbutton;
    gboolean enchant_exists;
    GtkWidget *widget;
    GtkAccelGroup *accelgroup;

    //Chain the parent class
    {
      G_OBJECT_CLASS (gw_searchwindow_parent_class)->constructed (object);
    }

    //Initializations
    window = GW_SEARCHWINDOW (object);
    priv = window->priv;

    //Set up the gtkbuilder links
    priv->entry = GTK_ENTRY (gw_window_get_object (GW_WINDOW (window), "search_entry"));
    priv->notebook = GTK_NOTEBOOK (gw_window_get_object (GW_WINDOW (window), "notebook"));
    priv->toolbar = GTK_TOOLBAR (gw_window_get_object (GW_WINDOW (window), "toolbar"));
    priv->statusbar = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "statusbar"));
    priv->combobox = GTK_COMBO_BOX (gw_window_get_object (GW_WINDOW (window), "dictionary_combobox"));
    priv->history = lw_history_new (20);

    //Set up the gtk window
    gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_MOUSE);
    gtk_window_set_default_size (GTK_WINDOW (window), 620, 500);
    gtk_window_set_icon_name (GTK_WINDOW (window), "gwaei");

    gtk_widget_grab_focus (GTK_WIDGET (priv->entry));
    gw_searchwindow_set_dictionary (window, 0);
    gw_searchwindow_guarantee_first_tab (window);

    //We are going to lazily update the sensitivity of the spellcheck buttons only when the window is created
    toolbutton = GTK_TOOL_BUTTON (gw_window_get_object (GW_WINDOW (window), "spellcheck_toolbutton")); 
    enchant_exists = g_file_test (ENCHANT, G_FILE_TEST_IS_REGULAR);
    accelgroup = gw_window_get_accel_group (GW_WINDOW (window));

    //This code should probalby be moved to when the window is realized
    gw_searchwindow_initialize_dictionary_combobox (window);
    gw_searchwindow_initialize_dictionary_menu (window);
    gw_searchwindow_update_history_popups (window);

    gtk_widget_set_sensitive (GTK_WIDGET (priv->entry), enchant_exists);
    gtk_widget_set_sensitive (GTK_WIDGET (toolbutton), enchant_exists);
    if (!enchant_exists) g_warning ("Enchant is not installed or support wasn't compiled in.  Spellcheck will be disabled.");

    //Set menu accelerators
    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "new_window_menuitem"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_N), GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "new_tab_menuitem"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_T), GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "append_menuitem"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_S), GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "print_menuitem"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_P), GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "close_menuitem"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_W), GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "quit_menuitem"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_Q), GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    //Edit popup
    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "cut_menuitem"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_X), GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "copy_menuitem"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_C), GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "paste_menuitem"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_P), GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "select_all_menuitem"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_A), GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    //View popup
    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "zoom_in_menuitem"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_plus), GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_KP_Add), GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "zoom_out_menuitem"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_minus), GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_KP_Subtract), GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "zoom_100_menuitem"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_0), GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_KP_0), GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "previous_tab_menuitem"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_Page_Up), GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "next_tab_menuitem"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_Page_Down), GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    //Insert popup
    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "kanjipad_menuitem"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_K), GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "radicals_menuitem"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_R), GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "word_edge_menuitem"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_B), GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "not_word_edge_menuitem"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_B), GDK_SHIFT_MASK | GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "clear_menuitem"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_L), GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    //History popup
    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "back_menuitem"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_Left), GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);

    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "forward_menuitem"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_Right), GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);

    //Help popup
    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "help_menuitem"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_F1), 0, GTK_ACCEL_VISIBLE);

    gw_searchwindow_attach_signals (window);
}


static void
gw_searchwindow_class_init (GwSearchWindowClass *klass)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = gw_searchwindow_constructed;
  object_class->finalize = gw_searchwindow_finalize;

  g_type_class_add_private (object_class, sizeof (GwSearchWindowPrivate));
}


//!
//! @brief Updates the progress information based on the LwSearchItem info
//! @param item A LwSearchItem pointer to gleam information from.
//! @returns Currently always returns TRUE
//!
gboolean 
gw_searchwindow_update_progress_feedback_timeout (GwSearchWindow *window)
{
    //Sanity checks
    if (gtk_widget_get_visible (GTK_WIDGET (window)) == FALSE) return TRUE;

    //Declarations
    GwSearchWindowPrivate *priv;
    LwSearchItem *item;

    //Initializations
    priv = window->priv;
    item = gw_searchwindow_get_current_searchitem (window);

    if (item != NULL) 
    {
      lw_searchitem_lock_mutex (item);
        if (
            item->status != LW_SEARCHSTATUS_CANCELING &&
            (item != priv->feedback_item ||
             item->current != priv->feedback ||
             item->status != priv->feedback_status       )
            )
        {
          gw_searchwindow_set_search_progressbar_by_searchitem (window, item);
          gw_searchwindow_set_total_results_label_by_searchitem (window, item);
          gw_searchwindow_set_title_by_searchitem (window, item);

          priv->feedback_item = item;
          priv->feedback = item->current;
          priv->feedback_status = item->status;
        }
      lw_searchitem_unlock_mutex (item);
    }

   return TRUE;
}


gboolean 
gw_searchwindow_append_result_timeout (GwSearchWindow *window)
{
    //Sanity check
    if (gtk_widget_get_visible (GTK_WIDGET (window)) == FALSE) return TRUE;

    //Declarations
    GwSearchWindowPrivate *priv;
    LwSearchItem *item;
    int chunk;
    int max_chunk;

    //Initializations
    priv = window->priv;
    item = gw_searchwindow_get_current_searchitem (window);
    chunk = 0;
    max_chunk = 10;
    
    if (item != NULL && lw_searchitem_should_check_results (item))
    {
      while (item != NULL && lw_searchitem_should_check_results (item) && chunk < max_chunk)
      {
        gw_searchwindow_append_result (window, item);
        chunk++;
      }
    }
    else
    {
        gw_searchwindow_display_no_results_found_page (window, item);
    }

    
    if (priv->mouse_item != NULL)
    {
      gw_searchwindow_append_kanjidict_tooltip_result (window, priv->mouse_item);
    }

    return TRUE;
}


void 
gw_searchwindow_entry_set_text (GwSearchWindow *window, const gchar *text)
{
    GwSearchWindowPrivate *priv;

    priv = window->priv;

    if (text != NULL) gtk_entry_set_text (priv->entry, text);
}


void 
gw_searchwindow_entry_insert_text (GwSearchWindow *window, const gchar *TEXT)
{
    //Sanity checks
    g_assert (window != NULL);
    if (TEXT == NULL) return;

    //Declarations
    GwSearchWindowPrivate *priv;
    gint start, end;

    //Initializations
    priv = window->priv;

    gtk_editable_get_selection_bounds (GTK_EDITABLE (priv->entry), &start, &end);
    gtk_editable_delete_text (GTK_EDITABLE (priv->entry), start, end);

    gtk_editable_insert_text (GTK_EDITABLE (priv->entry), TEXT, -1, &start);
    gtk_editable_set_position (GTK_EDITABLE (priv->entry), start + strlen(TEXT));
}



//!
//! @brief Sets the query text of the program using the informtion from the searchitem
//!
//! @param item a LwSearchItem argument.
//!
void 
gw_searchwindow_set_entry_text_by_searchitem (GwSearchWindow* window, LwSearchItem *item)
{
    //Declarations
    GwApplication *application;
    GwSearchWindowPrivate *priv;
    char hex_color_string[100];
    GdkRGBA color;
    LwPreferences *preferences;

    application = gw_window_get_application (GW_WINDOW (window));
    priv = window->priv;
    preferences = gw_application_get_preferences (application);

    //If there is no window, set the default colors
    if (item == NULL)
    {
      gtk_entry_set_text (priv->entry, "");
/*
      gtk_widget_override_background_color (GTK_WIDGET (priv->entry), GTK_STATE_NORMAL, NULL);
      gtk_widget_override_color (GTK_WIDGET (priv->entry), GTK_STATE_NORMAL, NULL);
*/
    }
    //There was previously a window, set the match colors from the prefs
    else
    {
      if (item->queryline != NULL && strlen(item->queryline->string) > 0)
      {
        if (strcmp(gtk_entry_get_text (priv->entry), item->queryline->string) != 0)
        {
          gtk_entry_set_text (priv->entry, item->queryline->string);
          gtk_editable_set_position (GTK_EDITABLE (priv->entry), -1);
        }
      }
      else
      {
        gtk_entry_set_text (priv->entry, "");
      }

      //Set the foreground color
      lw_preferences_get_string_by_schema (preferences, hex_color_string, LW_SCHEMA_HIGHLIGHT, LW_KEY_MATCH_FG, 100);
      if (gdk_rgba_parse (&color, hex_color_string) == FALSE)
      {
        lw_preferences_reset_value_by_schema (preferences, LW_SCHEMA_HIGHLIGHT, LW_KEY_MATCH_FG);
        return;
      }
      //gtk_widget_override_color (GTK_WIDGET (priv->entry), GTK_STATE_NORMAL, &color);

      //Set the background color
      lw_preferences_get_string_by_schema (preferences, hex_color_string, LW_SCHEMA_HIGHLIGHT, LW_KEY_MATCH_BG, 100);
      if (gdk_rgba_parse (&color, hex_color_string) == FALSE)
      {
        lw_preferences_reset_value_by_schema (preferences, LW_SCHEMA_HIGHLIGHT, LW_KEY_MATCH_BG);
        return;
      }
      //gtk_widget_override_background_color (GTK_WIDGET (priv->entry), GTK_STATE_NORMAL, &color);
    }
}


//!
//! @brief Sets the main window title text of the program using the informtion from the searchitem
//!
//! @param item a LwSearchItem argument.
//!
char* 
gw_searchwindow_get_title_by_searchitem (GwSearchWindow* window, LwSearchItem *item)
{
    //Declarations
    GwApplication *application;
    char *title;
    const char *program_name;
    int num_relevant, num_total;
    char *query;

    application = gw_window_get_application (GW_WINDOW (window));

    if (item == NULL || item->queryline == NULL)
    {
      //Initializations
      title = g_strdup (gw_application_get_program_name (application));
    }
    else
    {
      //Initializations
      program_name = gw_application_get_program_name(application);
      query = item->queryline->string;
      num_relevant = item->total_relevant_results;
      num_total = item->total_results;
      title = g_strdup_printf ("%s [%d/%d] - %s", query, num_relevant, num_total, program_name);
    }

    return title;
}

//!
//! @brief Sets the main window title text of the program using the informtion from the searchitem
//!
//! @param item a LwSearchItem argument.
//!
void 
gw_searchwindow_set_title_by_searchitem (GwSearchWindow* window, LwSearchItem *item)
{
    //Declarations
    char *title;

    //Initializations
    title = gw_searchwindow_get_title_by_searchitem (window, item);

    gtk_window_set_title (GTK_WINDOW (window), title);

    //Cleanup
    g_free (title);
}


//!
//! @brief Set's the progress label of the program using the inforamtion from the searchitem
//!
//! @param item A LwSearchItem pointer to gleam information from
//!
void 
gw_searchwindow_set_total_results_label_by_searchitem (GwSearchWindow *window, LwSearchItem* item)
{
    //Declarations
    GtkWidget *label;
    
    //Initializations
    label = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "progress_label"));

    if (item == NULL)
    {
      gtk_label_set_text(GTK_LABEL (label), "");
    }
    else
    {
      //Declarations
      int relevant = item->total_relevant_results;
      int total = item->total_results;

      char *idle_message_none = "";
      const char *searching_message_none = gettext("Searching...");

      const char *idle_message_total = ngettext("Found %d result", "Found %d results", total);
      const char *searching_message_total = ngettext("Searching... %d result", "Searching... %d results", total);

      // TRANSLATORS: relevant what ? It's the number of "relevant" result(s) displayed while or after searching.
      const char *message_relevant = ngettext("(%d Relevant)", "(%d Relevant)", relevant);

      char *base_message = NULL;
      char *final_message = NULL;

      //Initializations
      switch (item->status)
      {
        case LW_SEARCHSTATUS_IDLE:
            if (item->current == 0L)
              gtk_label_set_text (GTK_LABEL (label), idle_message_none);
            else if (relevant == total)
              final_message = g_strdup_printf (idle_message_total, total);
            else
            {
              base_message = g_strdup_printf ("%s %s", idle_message_total, message_relevant);
              if (base_message != NULL)
                final_message = g_strdup_printf (base_message, total, relevant);
            }
            break;
        case LW_SEARCHSTATUS_SEARCHING:
            if (item->total_results == 0)
              gtk_label_set_text(GTK_LABEL (label), searching_message_none);
            else if (relevant == total)
              final_message = g_strdup_printf (searching_message_total, total);
            else
            {
              base_message = g_strdup_printf ("%s %s", searching_message_total, message_relevant);
              if (base_message != NULL)
                final_message = g_strdup_printf (base_message, total, relevant);
            }
            break;
        case LW_SEARCHSTATUS_CANCELING:
            break;
      }

      //Finalize
      if (base_message != NULL)
        g_free (base_message);
      if (final_message != NULL)
      {
        gtk_label_set_text(GTK_LABEL (label), final_message);
        g_free (final_message);
      }
    }
}


//!
//! @brief Sets the current dictionary by using the load position
//!
//! @param request Sets the current dictionary by the number here
//!
void 
gw_searchwindow_set_dictionary (GwSearchWindow *window, int request)
{
    GwApplication *application;
    GwSearchWindowPrivate *priv;
    LwDictInfo *di;
    GtkMenuShell *shell;
    GList *list;
    GtkWidget *radioitem;
    LwDictInfoList *dictinfolist;

    application = gw_window_get_application (GW_WINDOW (window));
    priv = window->priv;
    dictinfolist = LW_DICTINFOLIST (gw_application_get_dictinfolist (application));
    di = lw_dictinfolist_get_dictinfo_by_load_position (dictinfolist, request);
    if (di == NULL) return;

    priv->dictinfo = di;

    //Make sure the correct radio menuitem is selected
    shell = GTK_MENU_SHELL (gw_window_get_object (GW_WINDOW (window), "dictionary_popup"));
    if (shell != NULL)
    {
      list = gtk_container_get_children (GTK_CONTAINER (shell));
      radioitem = GTK_WIDGET (g_list_nth_data (list, request));
      g_signal_handlers_block_by_func (radioitem, gw_searchwindow_dictionary_radio_changed_cb, window);
      if (g_list_length (list) > 3 && radioitem != NULL)
      {
        gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (radioitem), TRUE);
      }
      g_signal_handlers_unblock_by_func (radioitem, gw_searchwindow_dictionary_radio_changed_cb, window);
      g_list_free (list);
    }

    //Make sure the correct combobox item is selected
    g_signal_handlers_block_by_func (priv->combobox, gw_searchwindow_dictionary_combobox_changed_cb, NULL);
    gtk_combo_box_set_active (priv->combobox, request);
    g_signal_handlers_unblock_by_func (priv->combobox, gw_searchwindow_dictionary_combobox_changed_cb, NULL);
}


//!
//! @brief Uses a LwSearchItem to set the currently active dictionary
//!
//! This function is greatly useful for doing searches from the history.
//!
//! @param item A lnSearchItem to gleam information from
//!
void 
gw_searchwindow_set_dictionary_by_searchitem (GwSearchWindow *window, LwSearchItem *item)
{
    if (item == NULL)
      gw_searchwindow_set_dictionary (window, 0);
    else if (item->dictionary != NULL)
      gw_searchwindow_set_dictionary (window, item->dictionary->load_position);
}


LwDictInfo* 
gw_searchwindow_get_dictionary (GwSearchWindow* window)
{
    GwSearchWindowPrivate *priv;

    priv = window->priv;

    return priv->dictinfo;
}


//!
//! @brief Updates the status of the window progressbar
//!
//! @param item A LwSearchItem to gleam information from
//!
void 
gw_searchwindow_set_search_progressbar_by_searchitem (GwSearchWindow *window, LwSearchItem *item)
{
    //Declarations
    GwSearchWindowPrivate *priv;
    GtkWidget *progressbar;
    GtkWidget *statusbar;
    double fraction;

    //Initializations
    priv = window->priv;
    progressbar = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "search_progressbar"));
    statusbar = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "statusbar"));
    fraction = lw_searchitem_get_progress (item);

    if (gtk_widget_get_visible (statusbar))
    {
      gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (progressbar), fraction);
      gtk_entry_set_progress_fraction (priv->entry, 0.0);
    }
    else
    {
      gtk_entry_set_progress_fraction (priv->entry, fraction);
    }
}


//!
//! @brief Updates the history menu popup menu
//!
void 
gw_searchwindow_update_history_menu_popup (GwSearchWindow *window)
{
    //Declarations
    GwSearchWindowPrivate *priv;
    const char *ID;
    GtkMenuShell *shell;
    GList *children;
    GList *list;
    GList *iter;
    LwSearchItem *item;
    GtkWidget *menuitem;

    //Initializations
    priv = window->priv;
    ID = "history_popup";
    shell = GTK_MENU_SHELL (gw_window_get_object (GW_WINDOW (window), ID));
    children = gtk_container_get_children (GTK_CONTAINER (shell));
    iter = children;

    //Skip over the back/forward buttons
    if (iter != NULL) iter = g_list_next (iter);
    if (iter != NULL) iter = g_list_next (iter);

    //Remove all widgets after the back and forward menuitem buttons
    while (iter != NULL)
    {
      gtk_widget_destroy (GTK_WIDGET (iter->data));
      iter = iter->next;
    }
    g_list_free (children);


    list = lw_history_get_combined_list (priv->history);

    //Add a separator if there are some items in history
    if (list != NULL)
    {
      //Add a seperator to the end of the history popup
      menuitem = GTK_WIDGET (gtk_separator_menu_item_new());
      gtk_menu_shell_append (GTK_MENU_SHELL (shell), menuitem);
      gtk_widget_show (menuitem);
    }

    //Fill the history items
    for (iter = list; iter != NULL; iter = iter->next)
    {
      item = LW_SEARCHITEM (iter->data);

      GtkWidget *menu_item, *accel_label, *label;

      accel_label = gtk_label_new (item->dictionary->longname);
      gtk_widget_set_sensitive (GTK_WIDGET (accel_label), FALSE);
      label = gtk_label_new (item->queryline->string);

      GtkWidget *box;
      box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

      menu_item = gtk_menu_item_new();
      g_signal_connect (GTK_WIDGET (menu_item), 
                        "activate",
                        G_CALLBACK (gw_searchwindow_search_from_history_cb), 
                        window                                    );

      gtk_menu_shell_append(GTK_MENU_SHELL (shell), GTK_WIDGET (menu_item));
      gtk_container_add (GTK_CONTAINER (menu_item), box);
      gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);
      gtk_box_pack_end (GTK_BOX (box), accel_label, FALSE, FALSE, 0);

      gtk_widget_show(label);
      gtk_widget_show(accel_label);
      gtk_widget_show(box);
      gtk_widget_show(menu_item);
    }
    g_list_free (list);
}


//!
//! @brief PRIVATE FUNCTION. Populate the menu item lists for the back and forward buttons
//!
//! @param id Id of the popuplist
//! @param list history list to compair against
//!
static void _rebuild_history_button_popup (GwSearchWindow *window, char* id, GList* list)
{
    //Get a reference to the history_popup
    GtkWidget *popup;
    popup = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), id));

    GList     *children;
    children = gtk_container_get_children(GTK_CONTAINER (popup));

    //Remove all widgets after the back and forward menuitem buttons
    while (children != NULL )
    {
      gtk_widget_destroy(children->data);
      children = g_list_delete_link(children, children);
    }

    //Add a seperator to the end of the history popup
    GtkWidget *menuitem;
    menuitem = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL (popup), menuitem);
    gtk_widget_show (menuitem);

    //Declarations
    LwSearchItem *item;

    children = list;
    while (children != NULL)
    {
      item = children->data;

      //Ensure a minimuim width of the menu
      int leftover = 200;
      char label[leftover];
      strncpy (label, item->queryline->string, leftover);
      leftover -= strlen (item->queryline->string);
      while (leftover > 180)
      {
        strncat (label, " ", leftover);
        leftover -= 1;
      }

      menuitem = GTK_WIDGET (gtk_menu_item_new_with_label(label));

      //Create the new menuitem
      gtk_menu_shell_append(GTK_MENU_SHELL (popup), menuitem);
      gtk_widget_show  (menuitem);
      g_signal_connect (GTK_WIDGET (menuitem), 
                        "activate",
                        G_CALLBACK (gw_searchwindow_search_from_history_cb), 
                        window                                    );
   
      children = children->next;
    }
}


//!
//! @brief Convenience function to update both the back and forward histories etc
//!
void 
gw_searchwindow_update_history_popups (GwSearchWindow* window)
{
    GwSearchWindowPrivate *priv;
    GList* list;
    const char *id;
    GtkAction *action;

    priv = window->priv;

    gw_searchwindow_update_history_menu_popup (window);

    list = lw_history_get_forward_list (priv->history);
    _rebuild_history_button_popup(window, "forward_popup", list);

    list = lw_history_get_back_list (priv->history);
    _rebuild_history_button_popup(window, "back_popup", list);

    //Update back button
    id = "history_back_action";
    action = GTK_ACTION (gw_window_get_object (GW_WINDOW (window), id));
    gtk_action_set_sensitive (action, lw_history_has_back (priv->history));

    //Update forward button
    id = "history_forward_action";
    action = GTK_ACTION (gw_window_get_object (GW_WINDOW (window), id));
    gtk_action_set_sensitive (action, lw_history_has_forward (priv->history));
}


//!
//! @brief Sets the style of the toolbar [icons/text/both/both-horizontal]
//!
//! @param request The name of the style
//!
void 
gw_searchwindow_set_toolbar_style (GwSearchWindow *window, const char *request) 
{
    GwSearchWindowPrivate *priv;
    GtkToolbarStyle style;

    priv = window->priv;

    if (strcmp(request, "text") == 0)
      style = GTK_TOOLBAR_TEXT;
    else if (strcmp(request, "icons") == 0)
      style = GTK_TOOLBAR_ICONS;
    else if (strcmp(request, "both-horiz") == 0)
      style = GTK_TOOLBAR_BOTH_HORIZ;
    else
      style = GTK_TOOLBAR_BOTH;

    gtk_toolbar_set_style (priv->toolbar, style);
}


//!
//! @brief Appends some text to the text buffer
//!
//! @param item A LwSearchItem to gleam information from
//! @param text The text to append to the buffer
//! @param tag1 A tag to apply to the text or NULL
//! @param tag2 A tag to apply to the text or NULL
//! @param start_line Returns the start line of the text inserted
//! @param end_line Returns the end line of the text inserted
//!
void 
gw_searchwindow_append_to_buffer (GwSearchWindow *window, LwSearchItem *item, const char *text, char *tag1,
                                       char *tag2, int *start_line, int *end_line)
{
    if (item == NULL) return;

    //Assertain the target text buffer
    GwSearchData *sd;
    GtkTextView *view;
    GtkTextBuffer *buffer;

    sd = (GwSearchData*) lw_searchitem_get_data (item);
    view = GTK_TEXT_VIEW (sd->view);
    if (view == NULL) return;
    buffer = gtk_text_view_get_buffer (view);
    if (buffer == NULL) return;

    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(buffer, &iter);
    if (start_line != NULL)
      *start_line = gtk_text_iter_get_line(&iter);

    if (tag1 == NULL && tag2 == NULL)
      gtk_text_buffer_insert (buffer, &iter, text, -1);
    else if (tag2 == NULL)
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, text, -1, tag1, NULL);
    else if (tag1 == NULL)
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, text, -1, tag2, NULL);
    else
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, text, -1, tag1, tag2, NULL);

    gtk_text_buffer_get_end_iter (buffer, &iter);
    if (end_line != NULL)
      *end_line = gtk_text_iter_get_line(&iter);
}


//!
//! @brief Performs initializations absolutely necessary before a window can take place
//!
//! Correctly the pointer in the LwSearchItem to the correct textbuffer and moves marks
//!
//! @param item A LwSearchItem to gleam information from
//!
void 
gw_searchwindow_initialize_buffer_by_searchitem (GwSearchWindow *window, LwSearchItem *item)
{
    //Sanity check
    g_assert (lw_searchitem_has_data (item));

    //Make sure searches done from the history are pointing at a valid target
    GwSearchData *data;
    GtkTextView *view;
    GtkTextBuffer *buffer;

    data = GW_SEARCHDATA (lw_searchitem_get_data (item));
    view = GTK_TEXT_VIEW (data->view);
    buffer = gtk_text_view_get_buffer (view);

    if (view == NULL || buffer == NULL) return;

    gtk_text_buffer_set_text (buffer, "", -1);

    //Clear the target text buffer
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter (buffer, &iter);
    gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, "\n", -1, "small", NULL);

    gtk_text_buffer_get_end_iter (buffer, &iter);
    gtk_text_buffer_create_mark (buffer, "more_relevant_header_mark", &iter, TRUE);
    gtk_text_buffer_insert (buffer, &iter, "\n\n", -1);

    gtk_text_buffer_get_end_iter (buffer, &iter);
    gtk_text_buffer_insert (buffer, &iter, "\n", -1);
    gtk_text_buffer_get_end_iter (buffer, &iter); gtk_text_iter_backward_line (&iter);
    gtk_text_buffer_create_mark (buffer, "more_rel_content_insertion_mark", &iter, FALSE);
    gtk_text_buffer_create_mark (buffer, "content_insertion_mark", &iter, FALSE);

    gtk_text_buffer_get_end_iter (buffer, &iter);
    gtk_text_buffer_create_mark (buffer, "less_relevant_header_mark", &iter, TRUE);
    gtk_text_buffer_insert (buffer, &iter, "\n\n", -1);

    gtk_text_buffer_get_end_iter (buffer, &iter);
    gtk_text_buffer_insert (buffer, &iter, "\n", -1);
    gtk_text_buffer_get_end_iter (buffer, &iter); gtk_text_iter_backward_line (&iter);
    gtk_text_buffer_create_mark (buffer, "less_rel_content_insertion_mark", &iter, FALSE);

    gtk_text_buffer_get_end_iter (buffer, &iter);
    gtk_text_buffer_insert (buffer, &iter, "\n\n\n", -1);
    gtk_text_buffer_get_end_iter (buffer, &iter);
    gtk_text_buffer_create_mark (buffer, "footer_insertion_mark", &iter, FALSE);

    gw_searchwindow_set_total_results_label_by_searchitem (window, item);
}


//!
//! @brief Inserts text into the window entry
//!
//! @param text The text to insert
//!
void 
gw_searchwindow_entry_insert (GwSearchWindow *window, char* text)
{
    //Declarations
    GwSearchWindowPrivate *priv;
    glong length;
    gint start, end;

    //Initializations
    length = strlen (text);
    priv = window->priv;

    gtk_editable_get_selection_bounds (GTK_EDITABLE (priv->entry), &start, &end);
    gtk_editable_delete_text (GTK_EDITABLE (priv->entry), start, end);

    gtk_editable_insert_text(GTK_EDITABLE (priv->entry), text, length, &start);
    gtk_editable_set_position (GTK_EDITABLE (priv->entry), start);
}


//!
//! @brief Selects all text in a target
//!
//! @param TARGET The widget where to select all text
//!
void 
gw_searchwindow_select_all (GwSearchWindow* window, GtkWidget *widget)
{
    //Sanity check
    g_assert (window != NULL && widget != NULL); 

    //Declarations
    GtkTextBuffer *buffer;
    GtkTextIter start, end;

    if (GTK_IS_ENTRY (widget))
    {
      gtk_editable_select_region (GTK_EDITABLE (widget), 0,-1);
    }
    else if (GTK_IS_TEXT_VIEW (widget))
    {
      buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (widget));
      gtk_text_buffer_get_start_iter (buffer, &start);
      gtk_text_buffer_get_end_iter (buffer, &end);
      gtk_text_buffer_select_range (buffer, &start, &end);
    }
    else
    {
      g_warning ("Unsupported widget type for gw_searchwindow_select_all()\n");
    }
}


//!
//! @brief Deselects all text in a target
//!
//! @param TARGET The widget where to deselect all text
//!
void 
gw_searchwindow_select_none (GwSearchWindow *window, GtkWidget *widget)
{
    //Sanity check
    g_assert (window != NULL && widget != NULL); 

    //Declarations
    GtkTextBuffer *buffer;
    GtkTextIter start, end;

    if (GTK_IS_ENTRY (widget))
    {
      gtk_editable_select_region (GTK_EDITABLE (widget), -1,-1);
    }
    else if (GTK_IS_TEXT_VIEW (widget))
    {
      buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (widget));
      gtk_text_buffer_get_end_iter (buffer, &start);
      gtk_text_buffer_get_end_iter (buffer, &end);
      gtk_text_buffer_select_range (buffer, &start, &end);
    }
    else
    {
      g_warning ("Unsupported widget type for gw_searchwindow_select_none()\n");
    }
}


//!
//! @brief Copy Text into the target output
//!
void 
gw_searchwindow_copy_text (GwSearchWindow* window, GtkWidget *widget)
{
    //Sanity check
    g_assert (window != NULL && widget != NULL); 

    //Declarations
    GtkClipboard *clipbd;
    GtkTextBuffer *buffer;

    if (GTK_IS_ENTRY (widget))
    {
      gtk_editable_copy_clipboard (GTK_EDITABLE (widget));
    }
    else if (GTK_IS_TEXT_VIEW (widget))
    {
      buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (widget));
      clipbd = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
      gtk_text_buffer_copy_clipboard (buffer, clipbd);
    }
    else
    {
      g_warning ("Unsupported widget type for gw_searchwindow_copy_text()\n");
    }
}


//!
//! @brief Cut Text into the target output
//!
void 
gw_searchwindow_cut_text (GwSearchWindow *window, GtkWidget *widget)
{
    //Sanity check
    g_assert (window != NULL && widget != NULL); 

    if (GTK_IS_ENTRY (widget))
    {
        gtk_editable_cut_clipboard (GTK_EDITABLE (widget));
    }
    else if (GTK_IS_TEXT_VIEW (widget))
    {
      //We don't allow user editing the textview
    }
    else
    {
      g_warning ("Unsupported widget type for gw_searchwindow_cut_text()\n");
    }
}


//!
//! @brief Pastes Text into the target output
//!
void 
gw_searchwindow_paste_text (GwSearchWindow* window, GtkWidget *widget)
{
    //Sanity check
    g_assert (window != NULL && widget != NULL); 

    if (GTK_IS_ENTRY (widget))
    {
      gtk_editable_paste_clipboard (GTK_EDITABLE (widget));
    }
    else if (GTK_IS_TEXT_VIEW (widget))
    {
      //We don't allow user editing the textview
    }
    else
    {
      g_warning ("Unsupported widget type for gw_searchwindow_paste_text()\n");
    }
}



//!
//! @brief Returns the slice of characters between to line numbers in the target output buffer
//! @param sl The start line number
//! @param el The end line number
//!
char* 
gw_searchwindow_buffer_get_text_slice_from_current_view (GwSearchWindow* window, int sl, int el)
{
    //Assertain the target text buffer
    GtkTextView *view;
    GtkTextBuffer *buffer;

    view = gw_searchwindow_get_current_textview (window);
    if (view == NULL) return NULL;
    buffer = gtk_text_view_get_buffer (view);

    //Set up the text
    GtkTextIter si, ei;
    gtk_text_buffer_get_iter_at_line (buffer, &si, sl);
    gtk_text_buffer_get_iter_at_line (buffer, &ei, el);

    return gtk_text_buffer_get_slice (buffer, &si, &ei, TRUE);
}


//!
//! @brief Returns the character currently under the cursor in the main results window
//!
//! @param x Pointer to the x coordinate
//! @param y Pointer to the y coordinate
//!
//! @return Returns the character that is being moused over
//!
gunichar 
gw_searchwindow_get_hovered_character (GwSearchWindow* window, int *x, int *y, GtkTextIter *start)
{
    //Declarations;
    gint trailing;
    GtkTextView* view;

    //Initializations
    trailing = 0;
    view = gw_searchwindow_get_current_textview (window);
    if (view == NULL) return 0;

    gtk_text_view_window_to_buffer_coords (view, GTK_TEXT_WINDOW_TEXT, *x, *y, x, y);
    gtk_text_view_get_iter_at_position (view, start, &trailing, *x, *y);

    return gtk_text_iter_get_char (start);
} 


//!
//! @brief A convenience function to set the gdk cursor
//!
//! @param GdkCursorType The prefered cursor to set
//!
void 
gw_searchwindow_set_cursor (GwSearchWindow* window, GdkCursorType CURSOR)
{
    //Declarations
    GdkWindow* gdkwindow;
    GtkTextView *view;
    GdkCursor* cursor;

    //Initializations
    view = gw_searchwindow_get_current_textview (window);
    gdkwindow = gtk_text_view_get_window (view, GTK_TEXT_WINDOW_TEXT);
    cursor = gdk_cursor_new (CURSOR);

    gdk_window_set_cursor (gdkwindow, cursor);
    gdk_cursor_unref (cursor);
}


//!
//! @brief Cycles the dictionaries forward or backward, looping when it reaches the end
//!
//! @param cycle_forward A boolean to choose the cycle direction
//!
void 
gw_searchwindow_cycle_dictionaries (GwSearchWindow* window, gboolean cycle_forward)
{
    int increment;

    if (cycle_forward)
      increment = 1;
    else
      increment = -1;

    //Declarations
    GwSearchWindowPrivate *priv;
    GtkTreeIter iter;
    gint active;
    gboolean set;

    priv = window->priv;
    active = gtk_combo_box_get_active (priv->combobox);
    set = FALSE;

    if ((active = active + increment) == -1)
    {
      do {
        active++;
        gtk_combo_box_set_active (priv->combobox, active);
        set = gtk_combo_box_get_active_iter (priv->combobox, &iter);
      } while (set);
      active--;
      gtk_combo_box_set_active (priv->combobox, active);
    }
    else
    {
      gtk_combo_box_set_active (priv->combobox, active);
      set = gtk_combo_box_get_active_iter (priv->combobox, &iter);
      if (!set)
        gtk_combo_box_set_active (priv->combobox, 0);
    }
}


//!
//! @brief  Returns the unfreeable text from a gtk widget by target
//!
//! The fancy thing about this function is it will only return the
//! highlighted text if some is highlighted.
//!
char* 
gw_searchwindow_get_text (GwSearchWindow *window, GtkWidget *widget)
{
    GtkTextView *view;
    GtkTextBuffer *buffer;
    GtkTextIter s, e;

    view = gw_searchwindow_get_current_textview (window);
    buffer = gtk_text_view_get_buffer (view);

    if (gtk_text_buffer_get_has_selection (buffer))
    {
      gtk_text_buffer_get_selection_bounds (buffer, &s, &e);
    }
    //Get the region of text to be saved if no text is highlighted
    else
    {
      gtk_text_buffer_get_start_iter (buffer, &s);
      gtk_text_buffer_get_end_iter (buffer, &e);
    }
    return gtk_text_buffer_get_text (buffer, &s, &e, FALSE);
}



//!
//! @brief A simple window initiater function made to be looped by a timer
//!
//! @param data An unused gpointer.  It should always be NULL
//!
gboolean 
gw_searchwindow_keep_searching_timeout (GwSearchWindow *window)
{
    //Declarations
    GwApplication *application;
    GwSearchWindowPrivate *priv;
    const char *query;

    //Initializations
    application = gw_window_get_application (GW_WINDOW (window));
    priv = window->priv;
    query = gtk_entry_get_text (GTK_ENTRY (priv->entry));
    //Sanity check
    if (!gw_application_can_start_search (application)) return TRUE;
    if (!priv->keep_searching_enabled) return TRUE;
    if (priv->timeoutid[GW_SEARCHWINDOW_TIMEOUTID_KEEP_SEARCHING] == 0) return FALSE;

    if (priv->keep_searching_query == NULL) priv->keep_searching_query = g_strdup ("");

    if (priv->keep_searching_delay >= GW_SEARCHWINDOW_KEEP_SEARCHING_MAX_DELAY || strlen(query) == 0)
    {
      priv->keep_searching_delay = 0;
      gtk_widget_activate (GTK_WIDGET (priv->entry));
    }
    else
    {
      if (strcmp(priv->keep_searching_query, query) == 0)
      {
        priv->keep_searching_delay++;
      }
      else
      {
        if (priv->keep_searching_query != NULL)
          g_free (priv->keep_searching_query);
        priv->keep_searching_query = g_strdup (query);
        priv->keep_searching_delay = 0;
      }
    }
    
    return TRUE;
}


//!
//! @brief Abstraction function to find out if some text is selected
//!
//! It gets the requested text buffer and then returns if it has text selected
//! or not.
//!
gboolean 
gw_searchwindow_has_selection (GwSearchWindow *window, GtkWidget *widget)
{
    //Declarations
    gboolean has_selection;
    GtkTextBuffer *buffer;

    //Initializations

    if (GTK_IS_ENTRY (widget))
    {
      has_selection = gtk_editable_get_selection_bounds (GTK_EDITABLE (widget), NULL, NULL);
    }
    else if (GTK_IS_TEXT_VIEW (widget))
    {
      buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (widget));
      has_selection = (buffer != NULL && gtk_text_buffer_get_has_selection (buffer));
    }
    else
    {
      has_selection = FALSE;
    }

    return has_selection;
}


void 
gw_searchwindow_cancel_search_by_searchitem (GwSearchWindow *window, LwSearchItem *item)
{
    lw_searchitem_cancel_search (item);
}


//!
//! @brief Cancels all searches in all currently open tabs
//!
void 
gw_searchwindow_cancel_all_searches (GwSearchWindow *window)
{
    //Declarations
    GwSearchWindowPrivate *priv;
    GList *iter;
    LwSearchItem *item;

    priv = window->priv;

    for (iter = priv->tablist; iter != NULL; iter = iter->next)
    {
      item = LW_SEARCHITEM (iter->data);
      gw_searchwindow_cancel_search_by_searchitem (window, item);
    }

    gw_searchwindow_cancel_search_by_searchitem (window, priv->mouse_item);
}


//!
//! @brief Cancels the search of the tab number
//! @param page_num The page number of the tab to cancel the search of
//!
void 
gw_searchwindow_cancel_search_by_tab_number (GwSearchWindow *window, const int page_num)
{
    //Declarations
    GwSearchWindowPrivate *priv;
    LwSearchItem *item;

    //Initializations
    priv = window->priv;
    item = LW_SEARCHITEM (g_list_nth_data (priv->tablist, page_num));

    //Sanity check
    if (item == NULL) return;

    gw_searchwindow_cancel_search_by_searchitem (window, item);
}


//!
//! @brief Cancels the search of the currently visibile tab
//!
void 
gw_searchwindow_cancel_search_for_current_tab (GwSearchWindow *window)
{
    //Declarations
    GwSearchWindowPrivate *priv;
    int page_num;

    //Initializations
    priv = window->priv;
    page_num = gtk_notebook_get_current_page (priv->notebook);

    gw_searchwindow_cancel_search_by_tab_number (window, page_num);
}


//!
//! @brief Cancels a search by identifying matching gpointer
//! @param container A pointer to the top-most widget in the desired tab to cancel the search of.
//!
void 
gw_searchwindow_cancel_search_by_content (GwSearchWindow *window, gpointer container)
{
    //Declarations
    GwSearchWindowPrivate *priv;
    int position;
    LwSearchItem *item;

    //Initializations
    priv = window->priv;
    position = gtk_notebook_page_num (priv->notebook, container);

    //Sanity check
    if (position == -1) return;

    item = LW_SEARCHITEM (g_list_nth_data (priv->tablist, position));

    //Sanity check
    if (item == NULL) return;

    gw_searchwindow_cancel_search_by_searchitem (window, item);
}


GtkTextView* 
gw_searchwindow_get_current_textview (GwSearchWindow *window)
{
    //Sanity check
    g_assert (window != NULL);

    //Declarations
    GwSearchWindowPrivate *priv;
    int page_num;
    GtkScrolledWindow *scrolledwindow;
    GtkTextView *view;

    //Initializations
    priv = window->priv;
    view = NULL;
    page_num = gtk_notebook_get_current_page (priv->notebook);
    scrolledwindow = GTK_SCROLLED_WINDOW (gtk_notebook_get_nth_page (priv->notebook, page_num));
    if (scrolledwindow != NULL)
      view = GTK_TEXT_VIEW (gtk_bin_get_child (GTK_BIN (scrolledwindow)));

    return view;
}


//!
//! @brief Makes sure that at least one tab is available to output search results.
//!
void 
gw_searchwindow_guarantee_first_tab (GwSearchWindow *window)
{
    //Declarations
    GwSearchWindowPrivate *priv;
    int pages;

    //Initializations
    priv = window->priv;
    pages = gtk_notebook_get_n_pages (priv->notebook);

    if (pages == 0)
    {
      gw_searchwindow_new_tab (window);
      gw_searchwindow_sync_current_searchitem (window);
    }
}


//!
//! @brief Sets the title text of the current tab.
//! @param TEXT The text to set to the tab
//!
void 
gw_searchwindow_set_tab_text_by_searchitem (GwSearchWindow *window, LwSearchItem *item)
{
    //Declarations
    GwSearchWindowPrivate *priv;
    int page_num;
    GtkWidget *container;
    GtkWidget *box;
    GtkWidget *vbox;
    GList *hchildren;
    GList *vchildren;
    GtkWidget *label;
    const char *text;
    GList *iter;

    priv = window->priv;

    if (item == NULL)
    {
      page_num = 0;
      text = gettext("(Empty)");
      for (iter = priv->tablist; iter != NULL; iter = iter->next)
      {
        if (iter->data == NULL)
        {
          container = gtk_notebook_get_nth_page (priv->notebook, page_num);
          box = GTK_WIDGET (gtk_notebook_get_tab_label(priv->notebook, GTK_WIDGET (container)));
          hchildren = gtk_container_get_children (GTK_CONTAINER (box));
          vbox = GTK_WIDGET (hchildren->data);
          vchildren = gtk_container_get_children (GTK_CONTAINER (vbox));
          label = GTK_WIDGET (vchildren->data);
          gtk_label_set_text (GTK_LABEL (label), text);

          g_list_free (hchildren);
          g_list_free (vchildren);
        }
        page_num++;
      }
    }
    else
    {
      page_num = g_list_index (priv->tablist, item);
      g_assert (page_num != -1);
      container = gtk_notebook_get_nth_page (priv->notebook, page_num);
      box = GTK_WIDGET (gtk_notebook_get_tab_label (priv->notebook, GTK_WIDGET (container)));
      hchildren = gtk_container_get_children (GTK_CONTAINER (box));
      vbox = GTK_WIDGET (hchildren->data);
      vchildren = gtk_container_get_children (GTK_CONTAINER (vbox));
      label = GTK_WIDGET (vchildren->data);
      g_assert (item->queryline != NULL);
      text = item->queryline->string;

      gtk_label_set_text (GTK_LABEL (label), text);

      //Cleanup
      g_list_free (hchildren);
      g_list_free (vchildren);
    }
}


//!
//! @brief Creats a new tab.  The focus and other details are handled by gw_tabs_new_cb ()
//!
int 
gw_searchwindow_new_tab (GwSearchWindow *window)
{
    //Declarations
    GwApplication *application;
    GwSearchWindowPrivate *priv;
    GtkWidget *scrolledwindow;
    GtkTextView *view;
    GtkTextBuffer *buffer;
    GtkTextIter iter;
    GtkTextTagTable *tagtable;

    //Initializations
    application = gw_window_get_application (GW_WINDOW (window));
    priv = window->priv;
    tagtable = gw_application_get_tagtable (application);
    scrolledwindow = GTK_WIDGET (gtk_scrolled_window_new (NULL, NULL));
    buffer = GTK_TEXT_BUFFER (gtk_text_buffer_new (tagtable));
    view = GTK_TEXT_VIEW (gtk_text_view_new_with_buffer (buffer));

    //Set up the text buffer
    gtk_text_buffer_get_start_iter (buffer, &iter);
    gtk_text_buffer_create_mark (buffer, "more_relevant_header_mark", &iter, TRUE);
    gtk_text_buffer_create_mark (buffer, "less_relevant_header_mark", &iter, TRUE);
    gtk_text_buffer_create_mark (buffer, "less_rel_content_insertion_mark", &iter, FALSE);
    gtk_text_buffer_create_mark (buffer, "more_rel_content_insertion_mark", &iter, FALSE);
    gtk_text_buffer_create_mark (buffer, "content_insertion_mark", &iter, FALSE);
    gtk_text_buffer_create_mark (buffer, "footer_insertion_mark", &iter, FALSE);

    //Set up the text view
    gtk_text_view_set_right_margin (view, 10);
    gtk_text_view_set_left_margin (view, 10);
    gtk_text_view_set_cursor_visible (view, FALSE); 
    gtk_text_view_set_editable (view, FALSE);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
    gtk_text_view_set_wrap_mode (view, GTK_WRAP_WORD);

    g_signal_connect (G_OBJECT (view), "drag_motion", G_CALLBACK (gw_searchwindow_drag_motion_1_cb), window);
    g_signal_connect (G_OBJECT (view), "button_press_event", G_CALLBACK (gw_searchwindow_get_position_for_button_press_cb), window);
    g_signal_connect (G_OBJECT (view), "motion_notify_event", G_CALLBACK (gw_searchwindow_get_iter_for_motion_cb), window);
    g_signal_connect (G_OBJECT (view), "drag_drop", G_CALLBACK (gw_searchwindow_drag_drop_1_cb), window);
    g_signal_connect (G_OBJECT (view), "button_release_event", G_CALLBACK (gw_searchwindow_get_iter_for_button_release_cb), window);
    g_signal_connect (G_OBJECT (view), "drag_leave", G_CALLBACK (gw_searchwindow_drag_leave_1_cb), window);
    g_signal_connect (G_OBJECT (view), "drag_data_received", G_CALLBACK (gw_searchwindow_search_drag_data_recieved_cb), window);
    g_signal_connect (G_OBJECT (view), "key_press_event", G_CALLBACK (gw_searchwindow_focus_change_on_key_press_cb), window);
    g_signal_connect (G_OBJECT (view), "scroll_event", G_CALLBACK (gw_searchwindow_scroll_or_zoom_cb), window);


    gtk_container_add (GTK_CONTAINER (scrolledwindow), GTK_WIDGET (view));
    gtk_widget_show_all (GTK_WIDGET (scrolledwindow));

    //Create the tab label
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *label;
    GtkWidget *close_button;
    GtkWidget *button_image;
    GtkCssProvider *provider;
    char *style_data;
    GtkStyleContext *context;

    //Initializations
    hbox = GTK_WIDGET (gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3));
    label = GTK_WIDGET (gtk_label_new (NULL));
    close_button = GTK_WIDGET (gtk_button_new ());
    button_image = GTK_WIDGET (gtk_image_new_from_stock (GTK_STOCK_CLOSE, GTK_ICON_SIZE_MENU));
    style_data = "* {\n"
                 "-GtkButton-default-border : 0;\n"
                 "-GtkButton-default-outside-border : 0;\n"
                 "-GtkButton-inner-border: 0;\n"
                 "-GtkWidget-focus-line-width : 0;\n"
                 "-GtkWidget-focus-padding : 0;\n"
                 "padding: 0;\n"
                 "}";
    provider = gtk_css_provider_new ();
    context = gtk_widget_get_style_context (close_button);

    //Set up the button
    gtk_button_set_relief (GTK_BUTTON (close_button), GTK_RELIEF_NONE);
    gtk_button_set_focus_on_click (GTK_BUTTON (close_button), FALSE);
    gtk_container_set_border_width (GTK_CONTAINER (close_button), 0);
    gtk_misc_set_padding (GTK_MISC (button_image), 0, 0);
    gtk_widget_set_size_request (GTK_WIDGET (button_image), 14, 14);
    gtk_css_provider_load_from_data (provider,  style_data, strlen(style_data), NULL); 
    gtk_style_context_add_provider (context, GTK_STYLE_PROVIDER (provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref (provider);

    //Put all the elements together
    gtk_container_add (GTK_CONTAINER (close_button), button_image);
    g_signal_connect (G_OBJECT (close_button), "clicked", G_CALLBACK (gw_searchwindow_remove_tab_cb), scrolledwindow);
    vbox = GTK_WIDGET (gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 1);
    gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);
    vbox = GTK_WIDGET (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
    gtk_box_pack_start (GTK_BOX (vbox), close_button, FALSE, FALSE, 1);
    gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);
    gtk_widget_show_all (GTK_WIDGET (hbox));

    //Initializations
    int position;

    //Initializations
    position = gtk_notebook_append_page (priv->notebook, scrolledwindow, hbox);
    priv->tablist = g_list_append (priv->tablist, NULL);

    //Put everything together
//    gtk_notebook_set_tab_reorderable (priv->notebook, scrolledwindow, TRUE);
    gw_searchwindow_set_font (window);
    gtk_notebook_set_current_page (priv->notebook, position);
    gw_searchwindow_set_entry_text_by_searchitem (window, NULL);
    gtk_widget_grab_focus (GTK_WIDGET (priv->entry));
    gw_searchwindow_set_current_searchitem (window, NULL);

    return position;
}


void 
gw_searchwindow_remove_tab (GwSearchWindow *window, int index)
{
    //Sanity check
    g_assert (window != NULL && index > -1);

    //Declarations
    GwSearchWindowPrivate *priv;
    int pages;
    GList *iter;
    LwSearchItem *item;

    //Initializations
    priv = window->priv;
    pages = gtk_notebook_get_n_pages (priv->notebook);

    //Sanity check
    if (pages <= 1) {
      return;
    }

    gw_searchwindow_cancel_search_by_tab_number (window, index);

    iter = g_list_nth (priv->tablist, index);
    if (iter != NULL)
    {
      item = LW_SEARCHITEM (iter->data);
      if (lw_searchitem_has_history_relevance (item, priv->keep_searching_enabled))
      {
        lw_history_add_searchitem (priv->history, item);
        gw_searchwindow_update_history_popups (window);
      }
      else if (item != NULL)
      {
        lw_searchitem_free (item);
      }
    }
    priv->tablist = g_list_delete_link (priv->tablist, iter);

    gtk_notebook_remove_page (priv->notebook, index);
    gtk_widget_grab_focus (GTK_WIDGET (priv->entry));
    gw_searchwindow_sync_current_searchitem (window);
}


void 
gw_searchwindow_sync_current_searchitem (GwSearchWindow *window)
{
  //Declarations
  LwSearchItem *item;
  
  //Initializations
  item = gw_searchwindow_get_current_searchitem (window);

  gw_searchwindow_set_current_searchitem (window, item);
}


//!
//! @brief The searchwindow searchitem should be set when a new search takes place
//!        using a newly made searchitem.
//!
void 
gw_searchwindow_set_current_searchitem (GwSearchWindow *window, LwSearchItem *item)
{
    //Declarations
    GwSearchWindowPrivate *priv;
    GtkAction *action;
    GtkLabel *label;
    gboolean enable;
    const char *id;
    GList *link;
    int page_num;

    //Initializations
    priv = window->priv;
    page_num = gtk_notebook_get_current_page (priv->notebook);
    if (page_num == -1) return;
    link = g_list_nth (priv->tablist, page_num);
    if (link == NULL) return;
    link->data = item;

    //Update the window to match the searchitem data
    gw_searchwindow_set_tab_text_by_searchitem (window, item);
    gw_searchwindow_set_dictionary_by_searchitem (window, item);
    gw_searchwindow_set_entry_text_by_searchitem (window, item);
    gw_searchwindow_set_title_by_searchitem (window, item);
    gw_searchwindow_set_total_results_label_by_searchitem (window, item);
    gw_searchwindow_set_search_progressbar_by_searchitem (window, item);

    //Update Save sensitivity state
    id = "file_append_action";
    action = GTK_ACTION (gw_window_get_object (GW_WINDOW (window), id));
    enable = (item != NULL);
    gtk_action_set_sensitive (action, enable);

    //Update Save as sensitivity state
    id = "file_save_as_action";
    action = GTK_ACTION (gw_window_get_object (GW_WINDOW (window), id));
    enable = (item != NULL);
    gtk_action_set_sensitive (action, enable);

    //Update Print sensitivity state
    id = "file_print_action";
    action = GTK_ACTION (gw_window_get_object (GW_WINDOW (window), id));
    enable = (item != NULL);
    gtk_action_set_sensitive (action, enable);

    //Update Print preview sensitivity state
    id = "file_print_preview_action";
    action = GTK_ACTION (gw_window_get_object (GW_WINDOW (window), id));
    enable = (item != NULL);
    gtk_action_set_sensitive (action, enable);

    //Set the label's mnemonic widget since glade doesn't seem to want to
    id = "search_entry_label";
    label = GTK_LABEL (gw_window_get_object (GW_WINDOW (window), id));
    gtk_label_set_mnemonic_widget (label, GTK_WIDGET (priv->entry));
}


//!
//! @brief The searchwindow searchitem will be the one currently loaded in the current tab
//!
LwSearchItem* 
gw_searchwindow_get_current_searchitem (GwSearchWindow *window)
{
    GwSearchWindowPrivate *priv;
    LwSearchItem *item;
    int page_num;

    priv = window->priv;
    page_num = gtk_notebook_get_current_page (priv->notebook);
    if (page_num == -1)
      item = NULL;
    else
      item = LW_SEARCHITEM (g_list_nth_data (priv->tablist, page_num));

    return item;
}


void 
gw_searchwindow_start_search (GwSearchWindow *window, LwSearchItem* item)
{
    //Sanity check
    g_assert (window != NULL && item != NULL);

    //Declarations
    GwApplication *application;
    GwSearchData *sdata;
    GtkTextView *view;

    //Initializations
    application = gw_window_get_application (GW_WINDOW (window));
    if (!gw_application_can_start_search (application)) return;
    view = gw_searchwindow_get_current_textview (window);
    sdata = GW_SEARCHDATA (gw_searchdata_new (view, window));

    gw_searchwindow_guarantee_first_tab (window);
    lw_searchitem_set_data (item, sdata, LW_SEARCHITEM_DATA_FREE_FUNC (gw_searchdata_free));
    gw_searchwindow_set_current_searchitem (window, item);
    gw_searchwindow_initialize_buffer_by_searchitem (sdata->window, item);

    lw_searchitem_start_search (item, TRUE, FALSE);
    gw_searchwindow_update_history_popups (window);
}


//!
//! @brief Sets the requested font with magnification applied
//!
void 
gw_searchwindow_set_font (GwSearchWindow *window)
{
    //Declarations
    GwApplication *application;
    GwSearchWindowPrivate *priv;
    LwPreferences *preferences;

    GtkTextView *view;
    gboolean use_global_font_setting;
    int size;
    int magnification;
    char font[50];
    PangoFontDescription *desc;
    int i;
    GtkContainer *container;
    const char *id;
    GtkAction *action;
    gboolean enable;


    //Initializations
    application = gw_window_get_application (GW_WINDOW (window));
    priv = window->priv;
    preferences = gw_application_get_preferences (application);

    use_global_font_setting = lw_preferences_get_boolean_by_schema (preferences, LW_SCHEMA_FONT, LW_KEY_FONT_USE_GLOBAL_FONT);
    magnification = lw_preferences_get_int_by_schema (preferences, LW_SCHEMA_FONT, LW_KEY_FONT_MAGNIFICATION);

    if (use_global_font_setting)
      lw_preferences_get_string_by_schema (preferences, font, LW_SCHEMA_GNOME_INTERFACE, LW_KEY_DOCUMENT_FONT_NAME, 50);
    else
      lw_preferences_get_string_by_schema (preferences, font, LW_SCHEMA_FONT, LW_KEY_FONT_CUSTOM_FONT, 50);

    desc = pango_font_description_from_string (font);
    if (desc != NULL)
    {
      if (pango_font_description_get_size_is_absolute (desc))
        size = pango_font_description_get_size (desc) + magnification;
      else
        size = PANGO_PIXELS (pango_font_description_get_size (desc)) + magnification;

      //Make sure the font size is sane
      if (size < GW_APPLICATION_MIN_FONT_SIZE)
        size = GW_APPLICATION_MIN_FONT_SIZE;
      else if (size > GW_APPLICATION_MAX_FONT_SIZE)
        size = GW_APPLICATION_MAX_FONT_SIZE;

      priv->font_size = size;

      pango_font_description_set_size (desc, size * PANGO_SCALE);

      //Set it
      i = 0;
      while ((container = GTK_CONTAINER (gtk_notebook_get_nth_page (priv->notebook, i))) != NULL)
      {
        view = GTK_TEXT_VIEW (gtk_bin_get_child (GTK_BIN (container)));
        if (view != NULL)
          gtk_widget_override_font (GTK_WIDGET (view), desc);
        i++;
      }

      //Cleanup
      pango_font_description_free (desc);

      //Update Zoom in sensitivity state
      id = "view_zoom_in_action";
      action = GTK_ACTION (gw_window_get_object (GW_WINDOW (window), id));
      enable = (magnification < GW_APPLICATION_MAX_FONT_MAGNIFICATION);
      gtk_action_set_sensitive (action, enable);

      //Update Zoom out sensitivity state
      id = "view_zoom_out_action";
      action = GTK_ACTION (gw_window_get_object (GW_WINDOW (window), id));
      enable = (magnification > GW_APPLICATION_MIN_FONT_MAGNIFICATION);
      gtk_action_set_sensitive (action, enable);

      //Update Zoom 100 sensitivity state
      id = "view_zoom_100_action";
      action = GTK_ACTION (gw_window_get_object (GW_WINDOW (window), id));
      enable = (magnification != GW_APPLICATION_DEFAULT_FONT_MAGNIFICATION);
      gtk_action_set_sensitive (action, enable);
    }
}




static void 
gw_searchwindow_attach_signals (GwSearchWindow *window)
{
    //Declarations
    GwApplication *application;
    GwSearchWindowPrivate *priv;
    GwDictInfoList *dictinfolist;
    LwPreferences *preferences;

    application = gw_window_get_application (GW_WINDOW (window));
    priv = window->priv;
    dictinfolist = gw_application_get_dictinfolist (application);
    preferences = gw_application_get_preferences (application);


    g_signal_connect_after (G_OBJECT (window), "delete-event", 
                            G_CALLBACK (gw_searchwindow_delete_event_action_cb), window);
    g_signal_connect (G_OBJECT (window), "key-release-event", 
                      G_CALLBACK (gw_searchwindow_key_release_modify_status_update_cb), window);
    g_signal_connect (G_OBJECT (window), "key-press-event", 
                      G_CALLBACK (gw_searchwindow_key_press_modify_status_update_cb), window);
    g_signal_connect (G_OBJECT (window), "focus-in-event", 
                      G_CALLBACK (gw_searchwindow_focus_in_event_cb), window);
    g_signal_connect (G_OBJECT (window), "event-after", 
                      G_CALLBACK (gw_searchwindow_event_after_cb), window);
    g_signal_connect (G_OBJECT (window), "destroy",
                      G_CALLBACK (gw_searchwindow_remove_signals), NULL);

    priv->signalid[GW_SEARCHWINDOW_SIGNALID_TOOLBAR_SHOW] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_BASE,
        LW_KEY_TOOLBAR_SHOW,
        gw_searchwindow_sync_toolbar_show_cb,
        window
    );

    priv->signalid[GW_SEARCHWINDOW_SIGNALID_STATUSBAR_SHOW] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_BASE,
        LW_KEY_STATUSBAR_SHOW,
        gw_searchwindow_sync_statusbar_show_cb,
        window
    );

    priv->signalid[GW_SEARCHWINDOW_SIGNALID_USE_GLOBAL_FONT] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_FONT,
        LW_KEY_FONT_USE_GLOBAL_FONT,
        gw_searchwindow_sync_font_cb,
        window
    );
    priv->signalid[GW_SEARCHWINDOW_SIGNALID_CUSTOM_FONT] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_FONT,
        LW_KEY_FONT_CUSTOM_FONT,
        gw_searchwindow_sync_font_cb,
        window
    );

    priv->signalid[GW_SEARCHWINDOW_SIGNALID_FONT_MAGNIFICATION] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_FONT,
        LW_KEY_FONT_MAGNIFICATION,
        gw_searchwindow_sync_font_cb,
        window
    );

    priv->signalid[GW_SEARCHWINDOW_SIGNALID_KEEP_SEARCHING] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_BASE,
        LW_KEY_SEARCH_AS_YOU_TYPE,
        gw_searchwindow_sync_search_as_you_type_cb,
        window
    );

    priv->signalid[GW_SEARCHWINDOW_SIGNALID_SPELLCHECK] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_BASE,
        LW_KEY_SPELLCHECK,
        gw_searchwindow_sync_spellcheck_cb,
        window
    );

    priv->signalid[GW_SEARCHWINDOW_SIGNALID_DICTIONARIES_ADDED] = g_signal_connect (
        G_OBJECT (dictinfolist->model),
        "row-inserted",
        G_CALLBACK (gw_searchwindow_dictionaries_added_cb),
        window 
    );

    priv->signalid[GW_SEARCHWINDOW_SIGNALID_DICTIONARIES_DELETED] = g_signal_connect (
        G_OBJECT (dictinfolist->model),
        "row-deleted",
        G_CALLBACK (gw_searchwindow_dictionaries_deleted_cb),
        window 
    );

    priv->timeoutid[GW_SEARCHWINDOW_TIMEOUTID_KEEP_SEARCHING] = gdk_threads_add_timeout (
          500,
          (GSourceFunc) gw_searchwindow_keep_searching_timeout, 
          window
    );

    priv->timeoutid[GW_SEARCHWINDOW_TIMEOUTID_PROGRESS] = g_timeout_add_full (
          G_PRIORITY_LOW, 
          100, 
          (GSourceFunc) gw_searchwindow_update_progress_feedback_timeout, 
          window, 
          NULL
    );
    priv->timeoutid[GW_SEARCHWINDOW_TIMEOUTID_APPEND_RESULT] = g_timeout_add_full (
          G_PRIORITY_LOW, 
          100, 
          (GSourceFunc) gw_searchwindow_append_result_timeout, 
          window, 
          NULL
    );
}


static void 
gw_searchwindow_remove_signals (GwSearchWindow *window)
{
    //Declarations
    GwApplication *application;
    GwSearchWindowPrivate *priv;
    GwDictInfoList *dictinfolist;
    LwPreferences *preferences;
    GSource *source;
    gint i;

    application = gw_window_get_application (GW_WINDOW (window));
    priv = window->priv;
    dictinfolist = gw_application_get_dictinfolist (application);
    preferences = gw_application_get_preferences (application);

    for (i = 0; i < TOTAL_GW_SEARCHWINDOW_TIMEOUTIDS; i++)
    {
      if (g_main_current_source () != NULL &&
          !g_source_is_destroyed (g_main_current_source ()) &&
          priv->timeoutid[i] > 0
         )
      {
        source = g_main_context_find_source_by_id (NULL, priv->timeoutid[i]);
        if (source != NULL)
        {
          g_source_destroy (source);
        }
      }
      priv->timeoutid[i] = 0;
    }

    lw_preferences_remove_change_listener_by_schema (
        preferences,
        LW_SCHEMA_BASE,
        priv->signalid[GW_SEARCHWINDOW_SIGNALID_TOOLBAR_SHOW]
    );
    lw_preferences_remove_change_listener_by_schema (
        preferences,
        LW_SCHEMA_BASE,
        priv->signalid[GW_SEARCHWINDOW_SIGNALID_STATUSBAR_SHOW]
    );
    lw_preferences_remove_change_listener_by_schema (
        preferences,
        LW_SCHEMA_FONT,
        priv->signalid[GW_SEARCHWINDOW_SIGNALID_USE_GLOBAL_FONT]
    );
    lw_preferences_remove_change_listener_by_schema (
        preferences,
        LW_SCHEMA_FONT,
        priv->signalid[GW_SEARCHWINDOW_SIGNALID_CUSTOM_FONT]
    );
    lw_preferences_remove_change_listener_by_schema (
        preferences,
        LW_SCHEMA_FONT,
        priv->signalid[GW_SEARCHWINDOW_SIGNALID_FONT_MAGNIFICATION]
    );
    lw_preferences_remove_change_listener_by_schema (
        preferences,
        LW_SCHEMA_BASE,
        priv->signalid[GW_SEARCHWINDOW_SIGNALID_KEEP_SEARCHING]
    );
    lw_preferences_remove_change_listener_by_schema (
        preferences,
        LW_SCHEMA_BASE,
        priv->signalid[GW_SEARCHWINDOW_SIGNALID_SPELLCHECK]
    );

    g_signal_handler_disconnect (
        G_OBJECT (dictinfolist->model),
        priv->signalid[GW_SEARCHWINDOW_SIGNALID_DICTIONARIES_ADDED]
    );

    g_signal_handler_disconnect (
        G_OBJECT (dictinfolist->model),
        priv->signalid[GW_SEARCHWINDOW_SIGNALID_DICTIONARIES_DELETED]
    );
}


void 
gw_searchwindow_initialize_dictionary_combobox (GwSearchWindow *window)
{
    //Declarations
    GwApplication *application;
    GwSearchWindowPrivate *priv;
    GtkCellRenderer *renderer;
    GwDictInfoList *dictinfolist;

    //Initializations
    application = gw_window_get_application (GW_WINDOW (window));
    priv = window->priv;
    renderer = gtk_cell_renderer_text_new ();
    dictinfolist = gw_application_get_dictinfolist (application);

    gtk_combo_box_set_model (priv->combobox, NULL);
    gtk_cell_layout_clear (GTK_CELL_LAYOUT (priv->combobox));

    gtk_combo_box_set_model (priv->combobox, GTK_TREE_MODEL (dictinfolist->model));
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (priv->combobox), renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (priv->combobox), renderer, "text", GW_DICTINFOLIST_COLUMN_LONG_NAME, NULL);
    gtk_combo_box_set_active (priv->combobox, 0);
}


void 
gw_searchwindow_initialize_dictionary_menu (GwSearchWindow *window)
{
    GwApplication *application;
    GtkMenuShell *shell;
    GList *list, *iter;
    GtkWidget *widget;
    GwDictInfoList *dictinfolist;
    GtkAccelGroup *accelgroup;

    application = gw_window_get_application (GW_WINDOW (window));
    shell = GTK_MENU_SHELL (gw_window_get_object (GW_WINDOW (window), "dictionary_popup"));
    dictinfolist = gw_application_get_dictinfolist (application);
    accelgroup = gw_window_get_accel_group (GW_WINDOW (window));

    if (shell != NULL)
    {
      list = gtk_container_get_children (GTK_CONTAINER (shell));
      for (iter = list; iter != NULL; iter = iter->next)
      {
        widget = GTK_WIDGET (iter->data);
        if (widget != NULL)
        {
          gtk_widget_destroy(widget);
        }
      }
      g_list_free (list);
    }

    GSList *group;
    LwDictInfo *di;

    group = NULL;

    //Refill the menu
    for (iter = dictinfolist->list; iter != NULL; iter = iter->next)
    {
      di = LW_DICTINFO (iter->data);
      if (di != NULL)
      {
        widget = GTK_WIDGET (gtk_radio_menu_item_new_with_label (group, di->longname));
        group = gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM (widget));
        gtk_menu_shell_append (GTK_MENU_SHELL (shell),  GTK_WIDGET (widget));
        if (di->load_position == 0)
          gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (widget), TRUE);
        g_signal_connect(G_OBJECT (widget), "toggled", G_CALLBACK (gw_searchwindow_dictionary_radio_changed_cb), window);
        if (di->load_position < 9)
          gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", accelgroup, (GDK_KEY_0 + di->load_position + 1), GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);
        gtk_widget_show (widget);
      }
    }

    //Fill in the other menu items
    widget = GTK_WIDGET (gtk_separator_menu_item_new());
    gtk_menu_shell_append (GTK_MENU_SHELL (shell), GTK_WIDGET (widget));
    gtk_widget_show (GTK_WIDGET (widget));

    widget = GTK_WIDGET (gtk_menu_item_new_with_mnemonic(gettext("_Cycle Up")));
    gtk_menu_shell_append (GTK_MENU_SHELL (shell), GTK_WIDGET (widget));
    g_signal_connect (G_OBJECT (widget), "activate", G_CALLBACK (gw_searchwindow_cycle_dictionaries_backward_cb), window);
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", accelgroup, GDK_KEY_Up, GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_show (GTK_WIDGET (widget));

    widget = GTK_WIDGET (gtk_menu_item_new_with_mnemonic(gettext("Cycle _Down")));
    gtk_menu_shell_append (GTK_MENU_SHELL (shell), GTK_WIDGET (widget));
    g_signal_connect (G_OBJECT (widget), "activate", G_CALLBACK (gw_searchwindow_cycle_dictionaries_forward_cb), window);
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", accelgroup, GDK_KEY_Down, GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_show (GTK_WIDGET (widget));
}


