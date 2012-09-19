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
//! @file src/gtk/main.c
//!
//! @brief Abstraction layer for gtk objects
//!
//! Used as a go between for functions interacting with GUI interface objects.
//! This is the gtk version.
//!

#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <libintl.h>

#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include <gwaei/gwaei.h>


//Private variables
static LwSearchItem *_progress_feedback_item = NULL;
static gboolean _prev_selection_icon_state = FALSE;
static int _previous_tip = 0;


//!
//! @brief Sets up the variables in main-interface.c and main-callbacks.c for use
//!
void gw_main_initialize ()
{
    gw_common_load_ui_xml ("main.ui");

    //Declarations
    GtkWidget *toolbar;
    GtkBuilder *builder;

    //Initializations
    builder = gw_common_get_builder ();
    toolbar = GTK_WIDGET (gtk_builder_get_object (builder, "toolbar"));
    gtk_style_context_add_class (gtk_widget_get_style_context (toolbar), GTK_STYLE_CLASS_PRIMARY_TOOLBAR);
#ifdef G_OS_WIN32
    gtk_toolbar_set_style (GTK_TOOLBAR (toolbar), GTK_TOOLBAR_ICONS);
#else
    gtk_toolbar_unset_style (GTK_TOOLBAR (toolbar));
#endif
    gw_spellcheck_attach_to_entry (GTK_ENTRY (gw_common_get_widget_by_target (GW_TARGET_ENTRY)));

    gw_tabs_initialize ();
}


void gw_main_free ()
{
  gw_tabs_free ();
}


//!
//! @brief Updates the progress information based on the LwSearchItem info
//! @param item A LwSearchItem pointer to gleam information from.
//! @returns Currently always returns TRUE
//!
gboolean gw_main_update_progress_feedback_timeout (gpointer data)
{
    //Declarations
    GtkBuilder *builder;
    GtkWidget *notebook;
    int page_num;
    LwSearchItem *item;

    //Initializations
    builder = gw_common_get_builder ();
    notebook = GTK_WIDGET (gtk_builder_get_object (builder, "notebook"));
    page_num = gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));
    item = g_list_nth_data (gw_tabs_get_searchitem_list (), page_num);

    if (item != NULL) 
    {
      g_mutex_lock (item->mutex);
      gdk_threads_enter ();
        if (item != _progress_feedback_item || item->current_line != item->progress_feedback_line && item->target != GW_TARGET_KANJI)
        {
          _progress_feedback_item = item;
          item->progress_feedback_line = item->current_line;
          gw_main_set_search_progressbar_by_searchitem (item);
          gw_main_set_total_results_label_by_searchitem (item);
          gw_main_set_main_window_title_by_searchitem (item);
        }
      gdk_threads_leave ();
      g_mutex_unlock (item->mutex);
    }
   return TRUE;
}


//!
//! @brief Sets the query text of the program using the informtion from the searchitem
//!
//! @param item a LwSearchItem argument.
//!
void gw_main_set_entry_text_by_searchitem (LwSearchItem *item)
{
    //Declarations
    char hex_color_string[100];
    GdkRGBA color;
    GtkWidget *entry;

    //Initializations
    entry = gw_common_get_widget_by_target (GW_TARGET_ENTRY);

    //If there is no search, set the default colors
    if (item == NULL)
    {
      gtk_entry_set_text (GTK_ENTRY (entry), "");
      gtk_widget_override_background_color (GTK_WIDGET (entry), GTK_STATE_NORMAL, NULL);
      gtk_widget_override_color (GTK_WIDGET (entry), GTK_STATE_NORMAL, NULL);
    }
    //There was previously a search, set the match colors from the prefs
    else
    {
      if (item->queryline != NULL && strlen(item->queryline->string) > 0)
      {
        if (strcmp(gtk_entry_get_text (GTK_ENTRY (entry)), item->queryline->string) != 0)
        {
          gtk_entry_set_text (GTK_ENTRY (entry), item->queryline->string);
          gtk_editable_set_position (GTK_EDITABLE (entry), -1);
        }
      }
      else
      {
        gtk_entry_set_text (GTK_ENTRY (entry), "");
      }

      //Set the foreground color
      lw_pref_get_string_by_schema (hex_color_string, GW_SCHEMA_HIGHLIGHT, GW_KEY_MATCH_FG, 100);
      if (gdk_rgba_parse (&color, hex_color_string) == FALSE)
      {
        lw_pref_reset_value_by_schema (GW_SCHEMA_HIGHLIGHT, GW_KEY_MATCH_FG);
        return;
      }
      //gtk_widget_override_color (GTK_WIDGET (entry), GTK_STATE_NORMAL, &color);

      //Set the background color
      lw_pref_get_string_by_schema (hex_color_string, GW_SCHEMA_HIGHLIGHT, GW_KEY_MATCH_BG, 100);
      if (gdk_rgba_parse (&color, hex_color_string) == FALSE)
      {
        lw_pref_reset_value_by_schema (GW_SCHEMA_HIGHLIGHT, GW_KEY_MATCH_BG);
        return;
      }
      //gtk_widget_override_background_color (GTK_WIDGET (entry), GTK_STATE_NORMAL, &color);
    }
}


//!
//! @brief Returns the program name.  It should not be freed or modified
//! @returns A constanst string representing the program name
//!
const char* gw_main_get_program_name () 
{
  return gettext("gWaei Japanese-English Dictionary");
}

//!
//! @brief Allocates a string containing the text for the main window title
//! @param item A search item to create a title with
//! @returns Returns an allocated string that must be freed with g_free
//!
char* gw_main_get_window_title_by_searchitem (LwSearchItem *item) 
{
    //Sanity check
    if (item == NULL || item->queryline == NULL) return NULL;

    //Declarations
    char *title;
    const char *program_name;
    int num_relevant, num_total;
    char *query;

    //Initializations
    program_name = gw_main_get_program_name();
    query = item->queryline->string;
    num_relevant = item->total_relevant_results;
    num_total = item->total_results;
    title = g_strdup_printf ("%s [%d/%d] - %s", query, num_relevant, num_total, program_name);

    return title;
}


//!
//! @brief Sets the main window title text of the program using the informtion from the searchitem
//!
//! @param item a LwSearchItem argument.
//!
void gw_main_set_main_window_title_by_searchitem (LwSearchItem *item)
{
    //Declarations
    GtkBuilder *builder;
    GtkWidget *window;
    char *title;

    //Initializations
    builder = gw_common_get_builder ();
    window = GTK_WIDGET (gtk_builder_get_object (builder, "main_window"));
    title = gw_main_get_window_title_by_searchitem (item);

    if (title != NULL)
    {
      gtk_window_set_title (GTK_WINDOW (window), title);
      g_free (title);
    }
    else
    {
      gtk_window_set_title (GTK_WINDOW (window), gw_main_get_program_name ());
    }
}


//!
//! @brief Closes the suggestion box.  (Currently unused feature of gWaei)
//!
void gw_main_close_suggestion_box ()
{
    GtkBuilder *builder = gw_common_get_builder ();

    GtkWidget *suggestion_hbox;
    suggestion_hbox = GTK_WIDGET (gtk_builder_get_object (builder, "suggestion_hbox"));
    gtk_widget_hide (suggestion_hbox);
}


//!
//! @brief Currently unused function
//!
//! @param string Unknown
//! @param query Unknown
//! @param query_length Unknown
//! @param extension Unknown
//!
void gw_main_set_information_box_label (const char* string, const char* query, int query_length, const char* extension)
{
    GtkBuilder *builder = gw_common_get_builder ();

    if (query_length > 1)
    {
      char query_short[300];
      strncpy(query_short, query, query_length);
      query_short[query_length] = '\0';

      GtkWidget *suggestion_label;
      suggestion_label = GTK_WIDGET (gtk_builder_get_object (builder, "suggestion_label"));
      char *label_text = g_strdup_printf (string, query_short, extension);
      if (label_text != NULL)
      {
        gtk_label_set_text (GTK_LABEL (suggestion_label), label_text);
        g_free (label_text);

        GtkWidget *suggestion_hbox;
        suggestion_hbox = GTK_WIDGET (gtk_builder_get_object (builder, "suggestion_hbox"));
        gtk_widget_show (suggestion_hbox);
      }
    }
}


//!
//! @brief Currently unused function
//!
//! @param item A LwSearchItem pointer to gleam information from
//!
void gw_main_verb_check_with_suggestion (LwSearchItem *item)
{
/*
    if (item == NULL || item->queryline == NULL || item->resultline == NULL || item->target != GW_TARGET_RESULTS) return;

    GtkBuilder *builder = gw_common_get_builder ();

    //It's already shown.  No need to do anything.
    GtkWidget *suggestion_hbox;
    suggestion_hbox = GTK_WIDGET (gtk_builder_get_object (builder, "suggestion_hbox"));
    if (gtk_widget_get_visible (suggestion_hbox) == TRUE) return;

    char *query = item->queryline->hira_string;
    LwResultLine *rl = item->resultline;

    if (query[0] == '\0' || rl->kanji_start == NULL)
      return;

    gunichar query_first_letter;
    query_first_letter = g_utf8_get_char (query);
    gunichar result_kanji_first_letter;
    result_kanji_first_letter = g_utf8_get_char (rl->kanji_start);
    gunichar result_furigana_first_letter; 
    if (rl->furigana_start != NULL)
      result_furigana_first_letter = g_utf8_get_char (rl->furigana_start);
    else
      result_furigana_first_letter = result_kanji_first_letter;

    //Make sure the query and the search result start similarly
    if (rl->classification_start == NULL || (query_first_letter != result_kanji_first_letter && query_first_letter != result_furigana_first_letter))
      return;

    GtkWidget *suggestion_label;
    suggestion_label = GTK_WIDGET (gtk_builder_get_object (builder, "suggestion_label"));
    GtkWidget *suggestion_eventbox;
    suggestion_eventbox = GTK_WIDGET (gtk_builder_get_object (builder, "suggestion_eventbox"));
    GtkWidget *window;
    window = GTK_WIDGET (gtk_builder_get_object (builder, "main_window"));
    GdkRGBA fgcolor;
    fgcolor = window->style->fg[GTK_STATE_SELECTED];
    GdkRGBA bgcolor;
    bgcolor = window->style->bg[GTK_STATE_SELECTED];

    gtk_event_box_set_visible_window (GTK_EVENT_BOX (suggestion_eventbox), TRUE);
    gtk_widget_modify_fg (suggestion_eventbox, GTK_STATE_NORMAL, &fgcolor);
    gtk_widget_modify_bg (suggestion_eventbox, GTK_STATE_NORMAL, &bgcolor);

    //i-adjective stuffs
    if (strstr(rl->classification_start, "adj-i"))
    {
      int nmatch = 1;
      regmatch_t pmatch[nmatch];
      if (regexec (&re_i_adj_past, query, nmatch, pmatch, 0) == 0)
      {
        char *message = gettext("Your query is possibly a past-form i-adjective.  (Try searching for %s%s?)");
        gw_main_set_inforation_box_label (message, query, pmatch[0].rm_so, "い");
      }
      else if (regexec (&re_i_adj_negative, query, nmatch, pmatch, 0) == 0)
      {
        char *message = gettext("Your query is possibly a negative i-adjective.  (Try searching for %s%s?)");
        gw_main_set_inforation_box_label (message, query, pmatch[0].rm_so, "い");
      }
      else if (regexec (&re_i_adj_te, query, nmatch, pmatch, 0) == 0)
      {
        char *message = gettext("Your query is possibly a te-form i-adjective.  (Try searching for %s%s?)");
        gw_main_set_inforation_box_label (message, query, pmatch[0].rm_so, "い");
      }
      else if (regexec (&re_i_adj_causative, query, nmatch, pmatch, 0) == 0)
      {
        char *message = gettext("Your query is possibly a causitive i-adjective.  (Try searching for %s%s?)");
        gw_main_set_inforation_box_label (message, query, pmatch[0].rm_so, "い");
      }
      else if (regexec (&re_i_adj_conditional, query, nmatch, pmatch, 0) == 0)
      {
        char *message = gettext("Your query is possibly a conditional-form i-adjective.  (Try searching for %s%s?)");
        gw_main_set_inforation_box_label (message, query, pmatch[0].rm_so, "い");
      }
    }

    //na-adjective stuffs
    else if (strstr(rl->classification_start, "adj-na"))
    {
      int nmatch = 1;
      regmatch_t pmatch[nmatch];
      if (regexec(&re_na_adj_past, query, nmatch, pmatch, 0) == 0)
      {
        char *message = gettext("Your query is possibly a past-form na-adjective.  (Try searching for %s%s?)");
        gw_main_set_inforation_box_label (message, query, pmatch[0].rm_so, "");
      }
      else if (regexec(&re_na_adj_negative, query, nmatch, pmatch, 0) == 0)
      {
        char *message = gettext("Your query is possibly a negative na-adjective.  (Try searching for %s%s?)");
        gw_main_set_inforation_box_label (message, query, pmatch[0].rm_so, "");
      }
      else if (regexec(&re_na_adj_te, query, nmatch, pmatch, 0) == 0)
      {
        char *message = gettext("Your query is possibly a te-form na-adjective.  (Try searching for %s%s?)");
        gw_main_set_inforation_box_label (message, query, pmatch[0].rm_so, "");
      }
      else if (regexec(&re_na_adj_causative, query, nmatch, pmatch, 0) == 0)
      {
        char *message = gettext("Your query is possibly a causitive na-adjective.  (Try searching for %s%s?)");
        gw_main_set_inforation_box_label (message, query, pmatch[0].rm_so, "");
      }
      else if (regexec(&re_na_adj_conditional, query, nmatch, pmatch, 0) == 0)
      {
        char *message = gettext("Your query is possibly a conditional-form na-adjective.  (Try searching for %s%s?)");
        gw_main_set_inforation_box_label (message, query, pmatch[0].rm_so, "");
      }
    }
*/
}


//!
//! @brief Updates the states of the toolbar buttons etc in the main interface
//!
void gw_main_update_toolbar_buttons ()
{
    GtkBuilder *builder = gw_common_get_builder ();
    GtkWidget *search_entry = gw_common_get_widget_by_target (GW_TARGET_ENTRY);

    //Delarations
    GtkAction *action;
    GtkWidget *menuitem;
    gboolean enable;
    char *id;

    LwSearchItem* history_search_item = lw_historylist_get_current (GW_TARGET_RESULTS);
    GtkWidget *notebook = GTK_WIDGET (gtk_builder_get_object (builder, "notebook"));
    int page_num = gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));
    int pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (notebook));
    LwSearchItem *tab_search_item = g_list_nth_data (gw_tabs_get_searchitem_list (), page_num);

    int current_font_magnification;
    current_font_magnification = lw_pref_get_int_by_schema (GW_SCHEMA_FONT, GW_KEY_FONT_MAGNIFICATION);

    GtkWidget *results_tv = gw_common_get_widget_by_target(GW_TARGET_RESULTS);

    //Update Zoom in sensitivity state
    id = "view_zoom_in_action";
    action = GTK_ACTION (gtk_builder_get_object (builder, id));
    enable = (tab_search_item != NULL && current_font_magnification < GW_MAX_FONT_MAGNIFICATION);
    gtk_action_set_sensitive(action, enable);

    //Update Zoom out sensitivity state
    id = "view_zoom_out_action";
    action = GTK_ACTION (gtk_builder_get_object (builder, id));
    enable = (tab_search_item != NULL && current_font_magnification > GW_MIN_FONT_MAGNIFICATION);
    gtk_action_set_sensitive(action, enable);

    //Update Zoom 100 sensitivity state
    id = "view_zoom_100_action";
    action = GTK_ACTION (gtk_builder_get_object(builder, id));
    enable = (tab_search_item != NULL && current_font_magnification != GW_DEFAULT_FONT_MAGNIFICATION);
    gtk_action_set_sensitive(action, enable);

    //Update Save sensitivity state
    id = "file_append_action";
    action = GTK_ACTION (gtk_builder_get_object(builder, id));
    enable = (tab_search_item != NULL);
    gtk_action_set_sensitive(action, enable);

    //Update Save as sensitivity state
    id = "file_save_as_action";
    action = GTK_ACTION (gtk_builder_get_object(builder, id));
    enable = (tab_search_item != NULL);
    gtk_action_set_sensitive(action, enable);

    //Update Print sensitivity state
    id = "file_print_action";
    action = GTK_ACTION (gtk_builder_get_object(builder, id));
    enable = (tab_search_item != NULL);
    gtk_action_set_sensitive(action, enable);

    //Update radicals search tool menuitem
    id = "insert_radicals_action";
    action = GTK_ACTION (gtk_builder_get_object (builder, id));
    enable = (lw_dictinfolist_get_dictinfo (GW_ENGINE_KANJI, "Kanji") != NULL);
    gtk_action_set_sensitive(action, enable);

    //Update back button
    id = "history_back_action";
    action = GTK_ACTION (gtk_builder_get_object (builder, id));
      gtk_action_set_sensitive (action, (lw_historylist_get_back_history (GW_TARGET_RESULTS) != NULL));

    //Update forward button
    id = "history_forward_action";
    action = GTK_ACTION (gtk_builder_get_object (builder, id));
      gtk_action_set_sensitive (action, (lw_historylist_get_forward_history (GW_TARGET_RESULTS) != NULL));

    //Update cut/copy buttons
    gboolean sensitive;
    if (gtk_widget_has_focus (search_entry))
    {
      sensitive = (gtk_editable_get_selection_bounds (GTK_EDITABLE (search_entry), NULL, NULL));
      id = "edit_copy_action";
      action = GTK_ACTION (gtk_builder_get_object (builder, id));
      gtk_action_set_sensitive (action, sensitive);
      id = "edit_cut_action";
      action = GTK_ACTION (gtk_builder_get_object (builder, id));
      gtk_action_set_sensitive (action, sensitive);
    }
    else if (results_tv != NULL && gtk_widget_has_focus (results_tv))
    {
      sensitive = (gw_main_has_selection_by_target (GW_TARGET_RESULTS));
      id = "edit_copy_action";
      action = GTK_ACTION (gtk_builder_get_object (builder, id));
      gtk_action_set_sensitive (action, sensitive);
      id = "edit_cut_action";
      action = GTK_ACTION (gtk_builder_get_object (builder, id));
      gtk_action_set_sensitive (action, FALSE);
    }

    id = "previous_tab_menuitem";
    menuitem = GTK_WIDGET (gtk_builder_get_object (builder, id));
    gtk_widget_set_sensitive (menuitem, (pages > 1));

    id = "next_tab_menuitem";
    menuitem = GTK_WIDGET (gtk_builder_get_object (builder, id));
    gtk_widget_set_sensitive (menuitem, (pages > 1));
}


//!
//! @brief Set's the progress label of the program using the inforamtion from the searchitem
//!
//! @param item A LwSearchItem pointer to gleam information from
//!
void gw_main_set_total_results_label_by_searchitem (LwSearchItem* item)
{
    GtkBuilder *builder = gw_common_get_builder ();
    GtkWidget *label = GTK_WIDGET (gtk_builder_get_object(builder, "progress_label"));

    if (item == NULL)
    {
      gtk_label_set_text(GTK_LABEL (label), "");
    }
    else if (item->target != GW_TARGET_RESULTS)
    {
      return;
    }
    else
    {
      //Declarations
      int relevant = item->total_relevant_results;
      int irrelevant = item->total_irrelevant_results;
      int total = item->total_results;

      char *idle_message_none = "";
      char *searching_message_none = gettext("Searching...");

      char *idle_message_total = ngettext("Found %d result", "Found %d results", total);
      char *searching_message_total = ngettext("Searching... %d result", "Searching... %d results", total);

      // TRANSLATORS: relevant what ? It's the number of "relevant" result(s) displayed while or after searching.
      char *message_relevant = ngettext("(%d Relevant)", "(%d Relevant)", relevant);

      char *base_message = NULL;
      char *final_message = NULL;

      //Initializations
      switch (item->status)
      {
        case GW_SEARCH_IDLE:
        case GW_SEARCH_FINISHING:
            if (item->current_line == 0)
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
        case GW_SEARCH_SEARCHING:
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
void gw_main_set_dictionary (int request)
{
    GtkBuilder *builder = gw_common_get_builder ();

    //Set the correct dictionary in the dictionary list
    if (lw_dictinfolist_set_selected_by_load_position (request) == NULL)
      return;

    //Time to make sure everything matches up in the gui
    GtkWidget *combobox;
    combobox = GTK_WIDGET (gtk_builder_get_object (builder, "dictionary_combobox"));

    GtkMenuShell *shell = NULL;
    shell = GTK_MENU_SHELL (gtk_builder_get_object (builder, "dictionary_popup"));
    if (shell != NULL)
    {
      GList     *children = NULL;
      children = gtk_container_get_children (GTK_CONTAINER (shell));
      GtkWidget *radioitem = g_list_nth_data (children, request);
      if (g_list_length (children) > 3 && radioitem != NULL)
      {
        gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (radioitem), TRUE);
        gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), request);
      }
      g_list_free (children);
      children = NULL;
    }
}


//!
//! @brief Uses a LwSearchItem to set the currently active dictionary
//!
//! This function is greatly useful for doing searches from the history.
//!
//! @param item A lnSearchItem to gleam information from
//!
void gw_main_set_dictionary_by_searchitem (LwSearchItem *item)
{
    if (item != NULL && item->dictionary != NULL)
      gw_main_set_dictionary (item->dictionary->load_position);
}


//!
//! @brief Updates the status of the search progressbar
//!
//! @param item A LwSearchItem to gleam information from
//!
void gw_main_set_search_progressbar_by_searchitem (LwSearchItem *item)
{
    //Declarations
    GtkWidget *entry;
    long current;
    long total;
    double fraction;

    //Initializations
    entry = gw_common_get_widget_by_target (GW_TARGET_ENTRY);
    current = 0;
    total = 0;

    if (item != NULL && item->dictionary != NULL)
    {
      current = item->current_line;
      total = item->dictionary->total_lines;
    }

    if (current == 0) fraction = 0.0;
    else fraction = (double)current / (double)total;

    if (item == NULL || item->dictionary == NULL || total == 0 || fraction > 1.0 || item->status == GW_SEARCH_IDLE || item->status == GW_SEARCH_FINISHING || item->status == GW_SEARCH_CANCELING)
    {
      gtk_entry_set_progress_fraction (GTK_ENTRY (entry), 0.0);
    }
    else
    {
      gtk_entry_set_progress_fraction (GTK_ENTRY (entry), fraction);
    }
}


//!
//! @brief Updates the history menu popup menu
//!
void gw_main_update_history_menu_popup ()
{
    GtkBuilder *builder = gw_common_get_builder ();

    char *id;

    GtkMenuShell *shell;
    id = "history_popup";
    shell = GTK_MENU_SHELL (gtk_builder_get_object(builder, id));

    GList     *children = NULL;
    children = gtk_container_get_children (GTK_CONTAINER (shell));
    GList *iter = children;

    //Skip over the back/forward buttons
    if (iter != NULL) iter = g_list_next(iter);
    if (iter != NULL) iter = g_list_next(iter);

    //Remove all widgets after the back and forward menuitem buttons
    while (iter != NULL )
    {
      gtk_widget_destroy(iter->data);
      iter = g_list_delete_link(iter, iter);
    }
    g_list_free (children);
    children = NULL;

    //Declarations
    GtkWidget *label;
    const char *text;
    LwSearchItem *item;
    GtkWidget *menuitem;

    children = lw_historylist_get_combined_history_list (GW_HISTORYLIST_RESULTS);

    //Add a separator if there are some items in history
    if (children != NULL)
    {
      //Add a seperator to the end of the history popup
      menuitem = GTK_WIDGET (gtk_separator_menu_item_new());
      gtk_menu_shell_append (GTK_MENU_SHELL (shell), menuitem);
      gtk_widget_show (menuitem);
    }

    //Fill the history items
    while (children != NULL)
    {
      item = children->data;

      GtkWidget *menu_item, *accel_label, *label;

      accel_label = gtk_label_new (item->dictionary->longname);
      gtk_widget_set_sensitive (GTK_WIDGET (accel_label), FALSE);
      label = gtk_label_new (item->queryline->string);

      GtkWidget *hbox;
      hbox = gtk_hbox_new (FALSE, 0);

      menu_item = gtk_menu_item_new();
      g_signal_connect (GTK_WIDGET (menu_item), 
                        "activate",
                        G_CALLBACK (gw_main_search_from_history_cb), 
                        item                               );

      gtk_menu_shell_append(GTK_MENU_SHELL (shell), GTK_WIDGET (menu_item));
      gtk_container_add (GTK_CONTAINER (menu_item), hbox);
      gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
      gtk_box_pack_end (GTK_BOX (hbox), accel_label, FALSE, FALSE, 0);

      gtk_widget_show(label);
      gtk_widget_show(accel_label);
      gtk_widget_show(hbox);
      gtk_widget_show(menu_item);
      children = children->next;
    }
    g_list_free (children);
}


//!
//! @brief PRIVATE FUNCTION. Populate the menu item lists for the back and forward buttons
//!
//! @param id Id of the popuplist
//! @param list history list to compair against
//!
static void _rebuild_history_button_popup (char* id, GList* list)
{
    GtkBuilder *builder = gw_common_get_builder ();

    //Get a reference to the history_popup
    GtkWidget *popup;
    popup = GTK_WIDGET (gtk_builder_get_object(builder, id));

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
    gtk_widget_show(menuitem);

    //Declarations
    GtkWidget *label;
    const char *text;
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
                        G_CALLBACK (gw_main_search_from_history_cb), 
                        item                               );
   
      children = children->next;
    }
}


//!
//! @brief Convenience function to update both the back and forward histories etc
//!
void gw_main_update_history_popups ()
{
    GList* list = NULL;

    gw_main_update_history_menu_popup();
    list = lw_historylist_get_forward_history (GW_HISTORYLIST_RESULTS);
    _rebuild_history_button_popup("forward_popup", list);
    list = lw_historylist_get_back_history (GW_HISTORYLIST_RESULTS);
    _rebuild_history_button_popup("back_popup", list);
}


//!
//! @brief Sets the requested font with magnification applied
//!
//! @param font_description_string The font with the font size
//! @param font_magnification And describing how to enlarge or shrink the font
//!
void gw_main_set_font (char *font_description_string, int *font_magnification)
{
    //Declarations
    GtkBuilder *builder;
    GtkWidget *notebook;
    GtkWidget *textview;
    GtkWidget *scrolledwindow;
    gboolean use_global_font_setting;
    char *new_font_description_string;
    char font_family[100];
    int font_size;
    int i;
    char *pos;
    PangoFontDescription *desc;

    //Initializations
    builder = gw_common_get_builder ();
    notebook = GTK_WIDGET (gtk_builder_get_object (builder, "notebook"));
    textview = NULL;
    scrolledwindow = NULL;
    use_global_font_setting = lw_pref_get_boolean_by_schema (GW_SCHEMA_FONT, GW_KEY_FONT_USE_GLOBAL_FONT);

    //Get the font family
    if (font_description_string == NULL)
    {
      if (use_global_font_setting)
        strcpy(font_family, "Sans 10");
//        lw_pref_get_string_by_schema (font_family, GW_SCHEMA_GNOME_INTERFACE, GW_KEY_DOCUMENT_FONT_NAME, GW_DEFAULT_FONT, 100);
      else
        lw_pref_get_string_by_schema (font_family, GW_SCHEMA_FONT, GW_KEY_FONT_CUSTOM_FONT, 100);
    }
    else
    {
      strcpy (font_family, font_description_string);
    }

    //Get the font size
    pos = strrchr (font_family, ' ');
    if (pos != NULL)
    {
      *pos = '\0';
      pos++;
      font_size = (int) g_ascii_strtoll (pos, NULL, 10);
    }
    else
    {
      font_size = GW_DEFAULT_FONT_SIZE;
    }

    //Add the magnification in to the font size
    if (font_magnification == NULL)
      font_size += lw_pref_get_int_by_schema (GW_SCHEMA_FONT, GW_KEY_FONT_MAGNIFICATION);
    else
      font_size += *font_magnification;

    //Make sure the font size is sane
    if (font_size < GW_MIN_FONT_SIZE)
      font_size = GW_MIN_FONT_SIZE;
    else if (font_size > GW_MAX_FONT_SIZE)
      font_size = GW_MAX_FONT_SIZE;

    //Assemble the font description
    new_font_description_string = g_strdup_printf("%s %d", font_family, font_size);

    //Set it
    desc = pango_font_description_from_string (new_font_description_string);
    if (desc != NULL && new_font_description_string != NULL)
    {
      i = 0;
      while ((scrolledwindow = GTK_WIDGET (gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), i))) != NULL)
      {
        textview = GTK_WIDGET (gtk_bin_get_child (GTK_BIN (scrolledwindow)));
        gtk_widget_override_font (GTK_WIDGET (textview), desc);
        i++;
      }
    }

    //Cleanup
    if (desc != NULL)
    {
      pango_font_description_free (desc);
      desc = NULL;
    }
    if (new_font_description_string != NULL)
    {
      g_free (new_font_description_string);
      new_font_description_string = NULL;
    }
}


//!
//! @brief Sets the style of the toolbar [icons/text/both/both-horizontal]
//!
//! @param request The name of the style
//!
void gw_main_set_toolbar_style (const char *request) 
{
    GtkBuilder *builder = gw_common_get_builder ();

    GtkWidget *toolbar;
    toolbar = GTK_WIDGET (gtk_builder_get_object(builder, "toolbar"));

    GtkToolbarStyle style;
    if (strcmp(request, "text") == 0)
      style = GTK_TOOLBAR_TEXT;
    else if (strcmp(request, "icons") == 0)
      style = GTK_TOOLBAR_ICONS;
    else if (strcmp(request, "both-horiz") == 0)
      style = GTK_TOOLBAR_BOTH_HORIZ;
    else
      style = GTK_TOOLBAR_BOTH;

    gtk_toolbar_set_style(GTK_TOOLBAR (toolbar), style);
}


//!
//! @brief Sets the checkbox to show or hide the toolbar
//!
//! @param request How to set the toolbar
//!
void gw_main_set_toolbar_show (gboolean request)
{
    GtkBuilder *builder = gw_common_get_builder ();

    GtkWidget *toolbar;
    toolbar = GTK_WIDGET (gtk_builder_get_object(builder, "toolbar"));

    if (request == TRUE)
      gtk_widget_show(toolbar);
    else
      gtk_widget_hide(toolbar);

    GtkAction *action;
    action = GTK_ACTION (gtk_builder_get_object (builder, "view_toggle_toolbar_action"));

    g_signal_handlers_block_by_func (action, gw_main_toolbar_toggle_cb, NULL);
    gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action), request);
    g_signal_handlers_unblock_by_func (action, gw_main_toolbar_toggle_cb, NULL);
}


//!
//! @brief Sets the checkbox state of the roma-kana conversion pref
//!
//! @param request How to set the preference
//!
void gw_main_set_romaji_kana_conv (int request)
{
  GtkBuilder *builder = gw_common_get_builder ();

  GtkWidget *widget;
  widget = GTK_WIDGET (gtk_builder_get_object(builder, "query_romaji_to_kana"));

  gtk_combo_box_set_active(GTK_COMBO_BOX (widget), request);
}


//!
//! @brief Sets the checkbox state of the hira-kata conversion pref
//!
//! @param request How to set the preference
//!
void gw_main_set_hiragana_katakana_conv (gboolean request)
{
    GtkBuilder *builder = gw_common_get_builder ();

    GtkWidget *widget;
    if (widget = GTK_WIDGET (gtk_builder_get_object(builder, "query_hiragana_to_katakana")))
    {
      g_signal_handlers_block_by_func (widget, gw_settings_hira_kata_conv_toggled_cb, NULL);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (widget), request);
      g_signal_handlers_unblock_by_func (widget, gw_settings_hira_kata_conv_toggled_cb, NULL);
    }
}


//!
//! @brief Sets the katakana-hiragana conversion checkbox being mindful to disable the event handlers
//!
//! @param request A boolean to use to set the checkbox state.
//!
void gw_main_set_katakana_hiragana_conv (gboolean request)
{
    GtkBuilder *builder = gw_common_get_builder ();

    GtkWidget *widget;
    if (widget = GTK_WIDGET (gtk_builder_get_object(builder, "query_katakana_to_hiragana")))
    {
      g_signal_handlers_block_by_func (widget, gw_settings_kata_hira_conv_toggled_cb, NULL);
      
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (widget), request);
      g_signal_handlers_unblock_by_func (widget, gw_settings_kata_hira_conv_toggled_cb, NULL);
    }
}


//!
//! @brief Sets the color to a switch minding to disable the event handlers on it
//!
//! @param widget_id The id of the widget to get.
//! @param hex_color_string The color to attempt to set.
//!
void gw_main_set_color_to_swatch (const char *widget_id, const char *hex_color_string)
{
    GtkBuilder *builder = gw_common_get_builder ();

    GtkWidget *widget;
    widget = GTK_WIDGET (gtk_builder_get_object (builder, widget_id));

    GdkColor color;
    if (gdk_color_parse (hex_color_string, &color) == TRUE)
    {
      gtk_color_button_set_color (GTK_COLOR_BUTTON (widget), &color);
    }
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
void gw_main_append_to_buffer (LwSearchItem *item, char *text, char *tag1,
                               char *tag2, int *start_line, int *end_line)
{
    //Assertain the target text buffer
    GObject *tb;
    if (item == NULL || item->target_tb == NULL) return;
    tb = G_OBJECT (item->target_tb);

    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER (tb), &iter);
    if (start_line != NULL)
      *start_line = gtk_text_iter_get_line(&iter);

    if (tag1 == NULL && tag2 == NULL)
      gtk_text_buffer_insert (GTK_TEXT_BUFFER (tb),
                              &iter, text, -1          );
    else if (tag2 == NULL)
      gtk_text_buffer_insert_with_tags_by_name (GTK_TEXT_BUFFER (tb),
                                                &iter, text, -1,
                                                tag1, NULL            );
    else if (tag1 == NULL)
      gtk_text_buffer_insert_with_tags_by_name (GTK_TEXT_BUFFER (tb),
                                                &iter, text, -1,
                                                tag2, NULL            );
    else
      gtk_text_buffer_insert_with_tags_by_name (GTK_TEXT_BUFFER (tb),
                                                &iter, text, -1,
                                                tag1, tag2, NULL   );

    gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER (tb), &iter);
    if (end_line != NULL)
      *end_line = gtk_text_iter_get_line(&iter);
}


//!
//! @brief Performs initializations absolutely necessary before a search can take place
//!
//! Correctly the pointer in the LwSearchItem to the correct textbuffer and moves marks
//!
//! @param item A LwSearchItem to gleam information from
//!
void gw_main_initialize_buffer_by_searchitem (LwSearchItem *item)
{
    //Make sure searches done from the history are pointing at a valid target
    item->target_tb = (gpointer) gw_common_get_gobject_by_target (item->target);
    item->target_tv = (gpointer) gw_common_get_widget_by_target (item->target);

    if (item->target_tb == NULL || item->target_tv == NULL) return;

    gtk_text_buffer_set_text (GTK_TEXT_BUFFER (item->target_tb), "", -1);

    if (item->target == GW_TARGET_RESULTS)
    {

      //Assertain the target text buffer
      GObject *tb = G_OBJECT (item->target_tb);

      //Clear the target text buffer
      GtkTextIter iter;
      gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (tb), &iter);
      gtk_text_buffer_insert_with_tags_by_name (GTK_TEXT_BUFFER (tb), &iter, "\n", -1, "small", NULL);

      gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (tb), &iter);
      gtk_text_buffer_create_mark (GTK_TEXT_BUFFER (tb), "more_relevant_header_mark", &iter, TRUE);
      gtk_text_buffer_insert (GTK_TEXT_BUFFER (tb), &iter, "\n\n", -1);

      gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (tb), &iter);
      gtk_text_buffer_insert (GTK_TEXT_BUFFER (tb), &iter, "\n", -1);
      gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (tb), &iter); gtk_text_iter_backward_line (&iter);
      gtk_text_buffer_create_mark (GTK_TEXT_BUFFER (tb), "more_rel_content_insertion_mark", &iter, FALSE);
      gtk_text_buffer_create_mark (GTK_TEXT_BUFFER (tb), "content_insertion_mark", &iter, FALSE);

      gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (tb), &iter);
      gtk_text_buffer_create_mark (GTK_TEXT_BUFFER (tb), "less_relevant_header_mark", &iter, TRUE);
      gtk_text_buffer_insert (GTK_TEXT_BUFFER (tb), &iter, "\n\n", -1);

      gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (tb), &iter);
      gtk_text_buffer_insert (GTK_TEXT_BUFFER (tb), &iter, "\n", -1);
      gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (tb), &iter); gtk_text_iter_backward_line (&iter);
      gtk_text_buffer_create_mark (GTK_TEXT_BUFFER (tb), "less_rel_content_insertion_mark", &iter, FALSE);

      gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (tb), &iter);
      gtk_text_buffer_insert (GTK_TEXT_BUFFER (tb), &iter, "\n\n\n", -1);
      gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (tb), &iter);
      gtk_text_buffer_create_mark (GTK_TEXT_BUFFER (tb), "footer_insertion_mark", &iter, FALSE);

      gw_main_set_total_results_label_by_searchitem (item);
    }
}


//!
//! @brief Inserts text into the search entry
//!
//! @param text The text to insert
//!
void gw_main_search_entry_insert (char* text)
{
    GtkWidget *search_entry = gw_common_get_widget_by_target (GW_TARGET_ENTRY);

    glong length;
    length = strlen (text);

    gint start, end;
    gtk_editable_get_selection_bounds (GTK_EDITABLE (search_entry), &start, &end);
    gtk_editable_delete_text (GTK_EDITABLE (search_entry), start, end);

    gtk_editable_insert_text(GTK_EDITABLE (search_entry), text, length, &start);
    gtk_editable_set_position (GTK_EDITABLE (search_entry), start);
}


//!
//! @brief Sets the focus to a specific target widget
//! 
//! @param TARGET A LwTargetOutput specifying a specific target
//!
void gw_main_grab_focus_by_target (LwTargetOutput TARGET)
{
    GtkWidget* widget;
    widget = gw_common_get_widget_by_target(TARGET);
    gtk_widget_grab_focus(widget);
}


//!
//! @brief Clears the search entry
//!
void gw_main_clear_search_entry ()
{
    GtkWidget *search_entry = gw_common_get_widget_by_target (GW_TARGET_ENTRY);

    gint start, end;
    gtk_editable_select_region (GTK_EDITABLE (search_entry), 0, -1);
    gtk_editable_get_selection_bounds (GTK_EDITABLE (search_entry), &start, &end);
    gtk_editable_delete_text (GTK_EDITABLE (search_entry), start, end);
}


//!
//! @brief Copies the text held by a text type widget using a strncpy like structure
//!
//! @param output The char pointer to copy the string to
//! @param TARGET The widget to copy the text from identified by a target
//! @param MAX The Max characters to copy
//!
void gw_main_strncpy_text_from_widget_by_target (char* output, LwTargetOutput TARGET, int MAX)
{
    GtkWidget *search_entry = gw_common_get_widget_by_target (GW_TARGET_ENTRY);

    //GtkEntry
    if (TARGET == GW_TARGET_ENTRY)
    {
      GtkWidget *search_entry = gw_common_get_widget_by_target (GW_TARGET_ENTRY);
      strncpy(output, gtk_entry_get_text (GTK_ENTRY (search_entry)), MAX);
    }
  /*
    //GtkTextView
    else if (TARGET = GW_TARGET_RESULTS | TARGET = GW_TARGET_KANJI)
    {
      GObject *tb;
      switch (TARGET)
      {
        case GW_TARGET_RESULTS:
          tb = results_tb;
          break;
      }
      GtkTextIter start, end;

      gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER (tb), &start);
      gtk_text_buffer_get_end_iter( GTK_TEXT_BUFFER (tb), &end);
      gtk_text_buffer_select_range (GTK_TEXT_BUFFER (tb), &start, &end);
    }
  */
}


//!
//! @brief Selects all text in a target
//!
//! @param TARGET The widget where to select all text
//!
void gw_main_text_select_all_by_target (LwTargetOutput TARGET)
{
    GtkWidget *search_entry = gw_common_get_widget_by_target (GW_TARGET_ENTRY);

    //GtkEntry
    if (TARGET == GW_TARGET_ENTRY)
    {
      gtk_editable_select_region (GTK_EDITABLE (search_entry), 0,-1);
    }

    //GtkTextView
    else if (TARGET == GW_TARGET_RESULTS ||
             TARGET == GW_TARGET_KANJI     )
    {
      //Assertain the target text buffer
      GObject *tb;
      tb = gw_common_get_gobject_by_target (TARGET);

      GtkTextIter start, end;
      gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER (tb), &start);
      gtk_text_buffer_get_end_iter( GTK_TEXT_BUFFER (tb), &end);

      gtk_text_buffer_select_range (GTK_TEXT_BUFFER (tb), &start, &end);
    }
}


//!
//! @brief Deselects all text in a target
//!
//! @param TARGET The widget where to deselect all text
//!
void gw_main_text_select_none_by_target (LwTargetOutput TARGET)
{
    GtkWidget *search_entry = gw_common_get_widget_by_target (GW_TARGET_ENTRY);

    //GtkEntry
    if (TARGET == GW_TARGET_ENTRY)
    {
      gtk_editable_select_region (GTK_EDITABLE (search_entry), -1,-1);
    }

    //GtkTextView
    else if (TARGET == GW_TARGET_RESULTS ||
             TARGET == GW_TARGET_KANJI     )
    {
      //Assertain the target text buffer
      GObject *tb;
      tb = gw_common_get_gobject_by_target (TARGET);

      GtkTextIter start, end;
      gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER (tb), &start);
      gtk_text_buffer_get_end_iter( GTK_TEXT_BUFFER (tb), &end);

      gtk_text_buffer_select_range (GTK_TEXT_BUFFER (tb), &start, &end);
    }
}


//!
//! @brief Returns the target id corresponding to what widget has focus
//!
//! @param window_id The window to check the widgets against 
//!
guint gw_main_get_current_target_focus (char *window_id)
{
    GtkBuilder *builder = gw_common_get_builder ();

    GtkWidget *window = GTK_WIDGET (gtk_builder_get_object (builder, window_id));
    GtkWidget *widget = GTK_WIDGET (gtk_window_get_focus (GTK_WINDOW (window))); 
    GtkWidget* results_tv = gw_common_get_widget_by_target(GW_TARGET_RESULTS);
    GtkWidget *search_entry = gw_common_get_widget_by_target (GW_TARGET_ENTRY);

    if (widget == results_tv)
      return GW_TARGET_RESULTS;
    if (widget == search_entry)
      return GW_TARGET_ENTRY;
    else
      return -1;
}


//!
//! @brief Copy Text into the target output
//!
//! TARGET LwTargetOutput to specify where the text should come from
//!
void gw_main_copy_text (LwTargetOutput TARGET)
{
    GtkClipboard *clipbd;
    GObject *results_tb = gw_common_get_gobject_by_target (GW_TARGET_RESULTS);
    GtkWidget *search_entry = gw_common_get_widget_by_target (GW_TARGET_ENTRY);

    switch (TARGET)
    {
      case GW_TARGET_ENTRY:
        gtk_editable_copy_clipboard (GTK_EDITABLE (search_entry));
        break;
      case GW_TARGET_RESULTS:
        clipbd = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
        gtk_text_buffer_copy_clipboard (GTK_TEXT_BUFFER (results_tb), clipbd);
        break;
    }
}


//!
//! @brief Cut Text into the target output
//!
//! TARGET LwTargetOutput to specify where the text should come from
//!
void gw_main_cut_text (LwTargetOutput TARGET)
{
    GtkWidget *search_entry = gw_common_get_widget_by_target (GW_TARGET_ENTRY);

    switch (TARGET)
    {
      case GW_TARGET_ENTRY:
        gtk_editable_cut_clipboard (GTK_EDITABLE (search_entry));
        break;
    }
}


//!
//! @brief Pastes Text into the target output
//!
//! TARGET LwTargetOutput to specify where the text should go
//!
void gw_main_paste_text (LwTargetOutput TARGET)
{
    GtkWidget *search_entry = gw_common_get_widget_by_target (GW_TARGET_ENTRY);

    switch (TARGET)
    {
      case GW_TARGET_ENTRY:
        gtk_editable_paste_clipboard (GTK_EDITABLE (search_entry));
        break;
    }
}


//!
//! @brief Sets or updates an existing tag to the buffer
//!
//! @param id String representing the name of the tag
//! @param A constant int representing the target buffer
//! @param set_fg Boolean whether to set the foreground color or not
//! @param set_bg Boolean whether to set the background color or not
//!
gboolean _set_color_to_tagtable (char    *id,     LwTargetOutput TARGET,
                                        gboolean set_fg, gboolean set_bg  )
{
    GtkBuilder *builder = gw_common_get_builder ();

    GtkTextTag *tag;

    //Yes, we're going to update the colors for all tabs
    GObject *tb;
    GtkWidget *notebook = GTK_WIDGET (gtk_builder_get_object (builder, "notebook"));
    GtkWidget *textview = NULL;
    GtkWidget *scrolledwindow = NULL;

    int i = 0;
    while ((scrolledwindow = GTK_WIDGET (gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), i))) != NULL)
    {
      textview = GTK_WIDGET (gtk_bin_get_child (GTK_BIN (scrolledwindow)));
      tb = G_OBJECT (gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview)));

      //Load the tag table
      GtkTextTagTable *table;
      table = gtk_text_buffer_get_tag_table(GTK_TEXT_BUFFER (tb));

      //Load the set colors in the preferences
      char *key = NULL;
      char fg_color[100];
      char bg_color[100];
      char fallback_color[100];

      GdkRGBA color;

      //Set the foreground color and reset if the value is odd
      if (set_fg)
      {
        key = g_strdup_printf ("%s-foreground", id);
        if (key != NULL)
        {
          lw_pref_get_string_by_schema (fg_color, GW_SCHEMA_HIGHLIGHT, key, 100);
          if (gdk_rgba_parse (&color, fg_color) == FALSE)
          {
            printf("color failed %s\n", fg_color);
            lw_pref_reset_value_by_schema (GW_SCHEMA_HIGHLIGHT, key);
            g_free (key);
            key = NULL;
            return FALSE;
          }
          g_free (key);
          key = NULL;
        }
      }

      //Set the background color and reset if the value is odd
      if (set_bg)
      {
        key = g_strdup_printf ("%s-background", id);
        if (key != NULL)
        {
          lw_pref_get_string_by_schema (bg_color, GW_SCHEMA_HIGHLIGHT, key, 100);
          if (gdk_rgba_parse (&color, bg_color) == FALSE)
          {
            printf("color failed %s\n", bg_color);
            lw_pref_reset_value_by_schema (GW_SCHEMA_HIGHLIGHT, key);
            g_free (key);
            key = NULL;
            return FALSE;
          }
          g_free (key);
          key = NULL;
        }
      }


      //Insert the new tag into the table
      if ((tag = gtk_text_tag_table_lookup (GTK_TEXT_TAG_TABLE (table), id)) == NULL)
      {
        if (set_fg && set_bg)
          tag = gtk_text_buffer_create_tag (GTK_TEXT_BUFFER (tb), id, "foreground", fg_color, "background", bg_color, NULL );
        else if (set_fg)
          tag = gtk_text_buffer_create_tag (GTK_TEXT_BUFFER (tb), id, "foreground", fg_color, NULL);
        else if (set_bg)
          tag = gtk_text_buffer_create_tag (GTK_TEXT_BUFFER (tb), id, "background", bg_color, NULL);
      }
      //Update the tags
      else
      {
        GValue fg_value = {0}, bg_value = {0};
        tag = gtk_text_tag_table_lookup (GTK_TEXT_TAG_TABLE (table), id);
        if (set_fg)
        {
          g_value_init (&fg_value, G_TYPE_STRING);
          g_value_set_string (&fg_value, fg_color);
          g_object_set_property (G_OBJECT (tag), "foreground", &fg_value);
          g_value_unset (&fg_value);
        }
        if (set_bg)
        {
          g_value_init (&bg_value, G_TYPE_STRING);
          g_value_set_string (&bg_value, bg_color);
          g_object_set_property (G_OBJECT (tag), "background", &bg_value);
          g_value_unset (&bg_value);
        }
      }
      i++;
    }
    return TRUE;
}


//!
//! PRIVATE FUNCTION. Sets a single tag to the tagtable of the output buffer
//!
//! @param id The id of the tag
//! @param TARGET The GWTargetOutput of the buffer
//! @param atr The attribute to set
//! @param val the value to set to the attribute
//!
static void gw_main_set_tag_to_tagtable (char *id,  LwTargetOutput TARGET,
                                         char *atr, gpointer val          )
{
    //Assertain the target text buffer
    GObject *tb;
    tb = gw_common_get_gobject_by_target (TARGET);

    GtkTextTagTable* table = gtk_text_buffer_get_tag_table (GTK_TEXT_BUFFER (tb)); 
    GtkTextTag* tag = gtk_text_tag_table_lookup (table, id);

    if (tag == NULL)
    {
      tag = gtk_text_buffer_create_tag (GTK_TEXT_BUFFER (tb), id, atr, val, NULL);
    }
}


//!
//! @brief Returns the slice of characters between to line numbers in the target output buffer
//!
//! @param TARGET The LwTargetOutput to get the text slice from
//! @param sl The start line number
//! @param el The end line number
//!
char* gw_main_buffer_get_text_slice_by_target (LwTargetOutput TARGET, int sl, int el)
{
    //Assertain the target text buffer
    GObject *tb;
    tb = gw_common_get_gobject_by_target (TARGET);

    //Set up the text
    GtkTextIter si, ei;
    gtk_text_buffer_get_iter_at_line (GTK_TEXT_BUFFER (tb), &si, sl);
    gtk_text_buffer_get_iter_at_line (GTK_TEXT_BUFFER (tb), &ei, el);

    return gtk_text_buffer_get_slice (GTK_TEXT_BUFFER (tb), &si, &ei, TRUE);
}


//!
//! @brief Returns the character currently under the cursor in the main results window
//!
//! @param x Pointer to the x coordinate
//! @param y Pointer to the y coordinate
//!
//! @return Returns the character that is being moused over
//!
gunichar gw_main_get_hovered_character (int *x, int *y, GtkTextIter *start)
{
    //Declarations;
    gint trailing;
    GtkWidget* results_tv;

    //Initializations
    trailing = 0;
    results_tv = gw_common_get_widget_by_target (GW_TARGET_RESULTS);

    gtk_text_view_window_to_buffer_coords (GTK_TEXT_VIEW(results_tv), GTK_TEXT_WINDOW_TEXT, *x, *y, x, y);
    gtk_text_view_get_iter_at_position (GTK_TEXT_VIEW (results_tv), start, &trailing, *x, *y);

    return gtk_text_iter_get_char (start);
} 


//!
//! @brief A convenience function to set the gdk cursor
//!
//! @param GdkCursorType The prefered cursor to set
//!
void gw_main_set_cursor (GdkCursorType CURSOR)
{
    //Declarations
    GtkWidget* results_tv;
    GdkWindow* gdk_window;
    GdkCursor* cursor;

    //Initializations
    results_tv = gw_common_get_widget_by_target (GW_TARGET_RESULTS);
    gdk_window = gtk_text_view_get_window (GTK_TEXT_VIEW (results_tv), GTK_TEXT_WINDOW_TEXT);
    cursor = gdk_cursor_new (CURSOR);

    gdk_window_set_cursor (gdk_window, cursor);
    gdk_cursor_unref (cursor);
}


//!
//! @brief Sets the no results page to the output buffer
//!
//! @param item A LwSearchItem pointer to gleam information from
//!
void gw_main_display_no_results_found_page (LwSearchItem *item)
{
    if (item->status == GW_SEARCH_CANCELING) return; 

  gdk_threads_enter ();
    gtk_text_buffer_set_text (GTK_TEXT_BUFFER (item->target_tb), "", -1);
  gdk_threads_leave ();

    gint32 temp = g_random_int_range (0,9);
    while (temp == _previous_tip)
      temp = g_random_int_range (0,9);
    const gint32 TIP_NUMBER = temp;
    _previous_tip = temp;
    GtkTextView *tv = GTK_TEXT_VIEW (item->target_tv);
    GtkTextBuffer *tb = GTK_TEXT_BUFFER (item->target_tb);
    GtkWidget *image = NULL;
    GtkTextIter iter;
    GtkTextChildAnchor *anchor = NULL;
    GtkWidget *label = NULL;
    GtkWidget *hbox = NULL;
    char *body = NULL;
    GtkWidget *search_entry = gw_common_get_widget_by_target (GW_TARGET_ENTRY);
    const char *query_text = gtk_entry_get_text (GTK_ENTRY (search_entry));
    GList *list = NULL;
    int i = 0;
    GtkWidget *button = NULL;
    char *markup = NULL;
    list = lw_dictinfolist_get_selected();
    LwDictInfo *di_selected = list->data;

gdk_threads_enter ();
    //Add the title
    gw_main_append_to_buffer (item, "\n", "small", NULL, NULL, NULL);


    //Set the header message
    hbox = gtk_hbox_new (FALSE, 10);
    gtk_text_buffer_get_end_iter (tb, &iter);
    anchor = gtk_text_buffer_create_child_anchor (tb, &iter);
    gtk_text_view_add_child_at_anchor (GTK_TEXT_VIEW (tv), hbox, anchor);
    gtk_widget_show (hbox);

    image = gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_DIALOG);
    gtk_container_add (GTK_CONTAINER (hbox), GTK_WIDGET (image));
    gtk_widget_show (image);

    label = gtk_label_new (NULL);
    char *message = NULL;
    // TRANSLATORS: The argument is the dictionary long name
    message = g_strdup_printf(gettext("Nothing found in the %s!"), di_selected->longname);
    if (message != NULL)
    {
      markup = g_markup_printf_escaped ("<big><big><b>%s</b></big></big>", message);
      if (markup != NULL)
      {
        gtk_label_set_markup (GTK_LABEL (label), markup);
        gtk_container_add (GTK_CONTAINER (hbox), GTK_WIDGET (label));
        gtk_widget_show (label);
        g_free (markup);
        markup = NULL;
      }
      g_free (message);
      message = NULL;
    }


    //Linebreak after the image
    gw_main_append_to_buffer (item, "\n\n\n", NULL, NULL, NULL, NULL);


    if (lw_dictinfolist_get_total () > 1)
    {
      //Add label for links
      hbox = gtk_hbox_new (FALSE, 0);
      gtk_text_buffer_get_end_iter (tb, &iter);
      anchor = gtk_text_buffer_create_child_anchor (tb, &iter);
      gtk_text_view_add_child_at_anchor (GTK_TEXT_VIEW (tv), hbox, anchor);
      gtk_widget_show (hbox);

      label = gtk_label_new (NULL);
      markup = g_markup_printf_escaped ("<b>%s</b>", gettext("Search Other Dictionary: "));
      if (markup != NULL)
      {
        gtk_label_set_markup (GTK_LABEL (label), markup);
        gtk_container_add (GTK_CONTAINER (hbox), GTK_WIDGET (label));
        gtk_widget_show (label);
        g_free (markup);
        markup = NULL;
      }

      //Add internal dictionary links
      i = 0;
      list = lw_dictinfolist_get_dict_by_load_position (0);
      LwDictInfo *di = list->data;

      while ((list = lw_dictinfolist_get_dict_by_load_position(i)) != NULL)
      {
        di = list->data;
        if (di != NULL && di != di_selected)
        {
          button = gtk_button_new_with_label (di->shortname);
          g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (gw_tabs_no_results_search_for_dictionary_cb), di);
          gtk_container_add (GTK_CONTAINER (hbox), GTK_WIDGET (button));
          gtk_widget_show (GTK_WIDGET (button));
        }
        i++;
      }

      gw_main_append_to_buffer (item, "\n", NULL, NULL, NULL, NULL);
    }

    //Add label for links
    hbox = gtk_hbox_new (FALSE, 0);
    gtk_text_buffer_get_end_iter (tb, &iter);
    anchor = gtk_text_buffer_create_child_anchor (tb, &iter);
    gtk_text_view_add_child_at_anchor (GTK_TEXT_VIEW (tv), hbox, anchor);
    gtk_widget_show (hbox);

    label = gtk_label_new (NULL);
    markup = g_markup_printf_escaped ("<b>%s</b>", gettext("Search Online: "));
    if (markup != NULL)
    {
      gtk_label_set_markup (GTK_LABEL (label), markup);
      gtk_container_add (GTK_CONTAINER (hbox), GTK_WIDGET (label));
      gtk_widget_show (label);
      g_free (markup);
      markup = NULL;
    }


    //Add links
    char *website_url_menuitems[] = {
      "Google", "http://www.google.com/search?q=%s", "google.png",
      "Goo", "http://dictionary.goo.ne.jp/srch/all/%s/m0u/", "goo.png",
      "Wikipedia", "http://www.wikipedia.org/wiki/%s", "wikipedia.png",
      NULL, NULL, NULL
    };
    i = 0;
    while (website_url_menuitems[i] != NULL)
    {
      //Create handy variables
      char *name = website_url_menuitems[i];
      char *url = g_strdup_printf(website_url_menuitems[i + 1], query_text);
      char *icon_path = website_url_menuitems[i + 2];
      char *path = g_build_filename (DATADIR2, PACKAGE, icon_path, NULL);
      image = NULL;

      //Start creating
      button = gtk_link_button_new_with_label (url, name);
      if (path != NULL)
      {
        image = gtk_image_new_from_file (path);
        //Gtk doesn't use the image anymore by default so we are removing
        //if (image != NULL) gtk_button_set_image (GTK_BUTTON (button), image);
        g_free (path);
        path = NULL;
      }
      gtk_container_add (GTK_CONTAINER (hbox), GTK_WIDGET (button));
      gtk_widget_show (button);
      i += 3;
    }




    gw_main_append_to_buffer (item, "\n\n\n", NULL, NULL, NULL, NULL);




    //Insert the instruction text
    char *tip_header_str = NULL;
    tip_header_str = g_strdup_printf (gettext("gWaei Usage Tip #%d: "), (TIP_NUMBER + 1));
    if (tip_header_str != NULL)
    {
      gw_main_append_to_buffer (item, tip_header_str,
                              "important", NULL, NULL, NULL         );
      g_free (tip_header_str);
      tip_header_str = NULL;
    }
                            
    switch (TIP_NUMBER)
    {
      case 0:
        //Tip 1
        body = g_strdup_printf (gettext("Use the Unknown Character from the Insert menu or toolbar in "
                                "place of unknown Kanji. %s will return results like %s.\n\nKanjipad "
                                "is another option for inputting Kanji characters.  Because of how the "
                                "innards of Kanjipad works, drawing with the correct number of strokes "
                                "and drawing the strokes in the correct direction is very important."),
                                "日.語", "日本語");
        gw_main_append_to_buffer (item,
                                gettext("Inputting Unknown Kanji"),
                                "header", "important", NULL, NULL         );
        gw_main_append_to_buffer (item,
                                "\n\n",
                                NULL, NULL, NULL, NULL         );
        if (body != NULL)
        {
          gw_main_append_to_buffer (item, body,
                                  NULL, NULL, NULL, NULL);
          g_free (body);
          body = NULL;
        }
        break;

     case 1:
        //Tip 2
        gw_main_append_to_buffer (item,
                                gettext("Getting More Exact Matches"),
                                "important", "header", NULL, NULL         );
        gw_main_append_to_buffer (item,
                                "\n\n",
                                NULL, NULL, NULL, NULL);
        gw_main_append_to_buffer (item,
                                gettext("Use the Word-edge Mark and the Not-word-edge Mark from the "
                                "insert menu to get more relevant results.  fish\\b will return results "
                                "like fish and selfish , but not fisherman"),
                                NULL, NULL, NULL, NULL);
        break;

     case 2:
        //Tip 3
        body = g_strdup_printf (gettext("Use the And Character or Or Character to search for "
                                "results that contain a combination of words that might not be "
                                "right next to each other.  cats&dogs will return only results "
                                "that contain both the words cats and dogs like %s does."),
                                "犬猫");
        gw_main_append_to_buffer (item,
                                gettext("Searching for Multiple Words"),
                                "important", "header", NULL, NULL);
        gw_main_append_to_buffer (item,
                                "\n\n",
                                NULL, NULL, NULL, NULL);
        if (body != NULL)
        {
          gw_main_append_to_buffer (item, body,
                                  NULL, NULL, NULL, NULL);
          g_free (body);
          body = NULL;
        }
        break;

     case 3:
        //Tip 4
        gw_main_append_to_buffer (item,
                                gettext("Make a Vocabulary List"),
                                "important", "header", NULL, NULL);
        gw_main_append_to_buffer (item,
                                "\n\n",
                                NULL, NULL, NULL, NULL);
        gw_main_append_to_buffer (item,
                                gettext("Specific sections of results can be printed or saved by "
                                "dragging the mouse to highlight them.  Using this in combination "
                                "with the Append command from the File menu or toolbar, quick and "
                                "easy creation of a vocabulary lists is possible."),
                                NULL, NULL, NULL, NULL);
        break;

     case 4:
        //Tip 5
        gw_main_append_to_buffer (item,
                                gettext("Why Use the Mouse?"),
                                "important", "header", NULL, NULL);
        gw_main_append_to_buffer (item,
                                "\n\n",
                                NULL, NULL, NULL, NULL         );
        gw_main_append_to_buffer (item,
                                gettext("Typing something will move the focus to the search input "
                                "box.  Hitting the Up or Down arrow key will move the focus to the "
                                "results pane so you can scroll the results.  Hitting Alt-Up or "
                                "Alt-Down will cycle the currently installed dictionaries."),
                                NULL, NULL, NULL, NULL         );
        break;

     case 5:
        //Tip 6
        gw_main_append_to_buffer (item,
                                gettext("Get Ready for the JLPT"),
                                "important", "header", NULL, NULL);
        gw_main_append_to_buffer (item,
                                "\n\n",
                                NULL, NULL, NULL, NULL         );
        gw_main_append_to_buffer (item,
                                gettext("The Kanji dictionary has some hidden features.  One such "
                                "one is the ability to filter out Kanji that don't meet a certain "
                                "criteria.  If you are planning on taking the Japanese Language "
                                "Proficiency Test, using the phrase J# will filter out Kanji not of "
                                "that level for easy study.  For example, J4 will only show Kanji "
                                "that appears on the forth level test.\n\nAlso of interest, the "
                                "phrase G# will filter out Kanji for the grade level a Japanese "
                                "person would study it at in school."),
                                NULL, NULL, NULL, NULL         );
        break;

     case 6:
        //Tip 7
        gw_main_append_to_buffer (item,
                                gettext("Just drag words in!"),
                                "important", "header", NULL, NULL);
        gw_main_append_to_buffer (item,
                                "\n\n",
                                NULL, NULL, NULL, NULL         );
        gw_main_append_to_buffer (item,
                                gettext("If you drag and drop a highlighted word into gWaei's "
                                "search result box, gWaei will automatically start a search "
                                "using that text.  This can be a nice way to quickly look up words "
                                "while browsing webpages. "),
                                NULL, NULL, NULL, NULL         );

        break;

     case 7:
        //Tip 8
        gw_main_append_to_buffer (item,
                                gettext("What does (adj-i) mean?"),
                                "important", "header", NULL, NULL);
        gw_main_append_to_buffer (item,
                                "\n\n",
                                NULL, NULL, NULL, NULL         );
        gw_main_append_to_buffer (item,
                                gettext("It is part of the terminalogy used by the EDICT group of "
                                "dictionaries to categorize words.  Some are obvious, but there are "
                                "a number that there is no way to know the meaning other than by looking "
                                "it up.\n\ngWaei includes some of the EDICT documentation in its help "
                                "manual.  Click the Dictionary Terminology Glossary menuitem in the "
                                "Help menu to get to it."),
                                NULL, NULL, NULL, NULL         );
        break;

     case 8:
        //Tip 9
        gw_main_append_to_buffer (item,
                                gettext("Books are Heavy"),
                                "important", "header", NULL, NULL);
        gw_main_append_to_buffer (item,
                                "\n\n",
                                NULL, NULL, NULL, NULL         );
        gw_main_append_to_buffer (item,
                                gettext("Aways wear a construction helmet when working with books.  "
                                "They are dangerous heavy objects that can at any point fall on and "
                                "injure you.  Please all urge all of your friends to, too.  They will "
                                "thank you later.  Really."),
                                NULL, NULL, NULL, NULL         );
       break;
    }

    gw_main_append_to_buffer (item,
                               "\n\n",
                               NULL, NULL, NULL, NULL         );
gdk_threads_leave ();
}


//!
//! @brief Cycles the dictionaries forward or backward, looping when it reaches the end
//!
//! @param cycle_forward A boolean to choose the cycle direction
//!
void gw_main_cycle_dictionaries (gboolean cycle_forward)
{
    GtkBuilder *builder = gw_common_get_builder ();

    int increment;

    if (cycle_forward)
      increment = 1;
    else
      increment = -1;

    //Declarations
    GtkWidget *combobox;
    combobox = GTK_WIDGET (gtk_builder_get_object(builder, "dictionary_combobox"));

    gint active;
    active = gtk_combo_box_get_active (GTK_COMBO_BOX (combobox));
    GtkTreeIter iter;
    gboolean set = FALSE;

    if ((active = active + increment) == -1)
    {
      do {
        active++;
        gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), active);
        set = gtk_combo_box_get_active_iter (GTK_COMBO_BOX (combobox), &iter);
      } while (set);
      active--;
      gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), active);
    }
    else
    {
      gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), active);
      set = gtk_combo_box_get_active_iter (GTK_COMBO_BOX (combobox), &iter);
      if (!set)
        gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), 0);
    }
}


//!
//! @brief  Returns the unfreeable text from a gtk widget by target
//!
//! The fancy thing about this function is it will only return the
//! highlighted text if some is highlighted.
//!
//! @param TARGET a LwTargetOutput to get the data from
//!
char* gw_main_buffer_get_text_by_target (LwTargetOutput TARGET)
{
    GObject* tb;
    tb = gw_common_get_gobject_by_target (TARGET);

    GtkTextIter s, e;
    if (gtk_text_buffer_get_has_selection (GTK_TEXT_BUFFER (tb)))
    {
      gtk_text_buffer_get_selection_bounds(GTK_TEXT_BUFFER (tb), &s, &e);
    }
    //Get the region of text to be saved if no text is highlighted
    else
    {
      gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER (tb), &s);
      gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER (tb), &e);
    }
    return gtk_text_buffer_get_text(GTK_TEXT_BUFFER (tb), &s, &e, FALSE);
}


//!
//! @brief Resets the color tags according to the preferences
//!
void gw_main_buffer_reload_tagtable_tags ()
{
    //Declarations
    GtkWidget *entry;

    //Initializations
    entry = gw_common_get_widget_by_target (GW_TARGET_ENTRY);

    _set_color_to_tagtable ("comment", GW_TARGET_RESULTS, TRUE, FALSE);
    _set_color_to_tagtable ("comment", GW_TARGET_KANJI,   TRUE, FALSE);

    _set_color_to_tagtable ("match",   GW_TARGET_RESULTS, TRUE, TRUE );
    _set_color_to_tagtable ("match",   GW_TARGET_KANJI,   FALSE,FALSE);

    _set_color_to_tagtable ("header",  GW_TARGET_RESULTS, TRUE, FALSE);
    _set_color_to_tagtable ("header",  GW_TARGET_KANJI,   TRUE, FALSE);

    gtk_widget_override_background_color (GTK_WIDGET (entry), GTK_STATE_NORMAL, NULL);
    gtk_widget_override_color (GTK_WIDGET (entry), GTK_STATE_NORMAL, NULL);
}


//!
//! @brief Adds the tags to stylize the buffer text
//!
void gw_main_buffer_initialize_tags ()
{
    gw_main_set_tag_to_tagtable ("italic", GW_TARGET_RESULTS,
                                  "style", GINT_TO_POINTER(PANGO_STYLE_ITALIC));
    gw_main_set_tag_to_tagtable ("gray", GW_TARGET_RESULTS,
                                  "foreground",    "#888888");
    gw_main_set_tag_to_tagtable ("smaller", GW_TARGET_RESULTS,
                                  "size",    "smaller");
    //Important tag (usually bold)
    gw_main_set_tag_to_tagtable ("important", GW_TARGET_RESULTS,
                                  "weight",    GINT_TO_POINTER(PANGO_WEIGHT_BOLD));

    //Larger tag
    gw_main_set_tag_to_tagtable ("larger", GW_TARGET_RESULTS, "font", "sans 20");

    //Large tag
    gw_main_set_tag_to_tagtable ("large", GW_TARGET_RESULTS, "font", "serif 40");

    gw_main_set_tag_to_tagtable ("center", GW_TARGET_RESULTS, "justification", GINT_TO_POINTER(GTK_JUSTIFY_LEFT));

    //Small tag
    gw_main_set_tag_to_tagtable ("small", GW_TARGET_RESULTS,  "font", "serif 6");

    gw_main_buffer_reload_tagtable_tags();
}


//!
//! @brief Creates the initial marks needed for the text buffer
//!
//! tb gpointer to the textbuffer to add the marks to
//!
void gw_main_buffer_initialize_marks (gpointer tb)
{
    GtkTextIter iter;

    gtk_text_buffer_get_start_iter (GTK_TEXT_BUFFER (tb), &iter);
    gtk_text_buffer_create_mark (GTK_TEXT_BUFFER (tb), "more_relevant_header_mark", &iter, TRUE);
    gtk_text_buffer_create_mark (GTK_TEXT_BUFFER (tb), "less_relevant_header_mark", &iter, TRUE);
    gtk_text_buffer_create_mark (GTK_TEXT_BUFFER (tb), "less_rel_content_insertion_mark", &iter, FALSE);
    gtk_text_buffer_create_mark (GTK_TEXT_BUFFER (tb), "more_rel_content_insertion_mark", &iter, FALSE);
    gtk_text_buffer_create_mark (GTK_TEXT_BUFFER (tb), "content_insertion_mark", &iter, FALSE);
    gtk_text_buffer_create_mark (GTK_TEXT_BUFFER (tb), "footer_insertion_mark", &iter, FALSE);
}


//!
//! @brief A simple search initiater function made to be looped by a timer
//!
//! @param data An unused gpointer.  It should always be NULL
//!
gboolean gw_main_keep_searching_timeout (gpointer data)
{
    //Declarations
    GtkBuilder *builder;
    GtkWidget *window;

    //Initializations
    builder = gw_common_get_builder ();
    window = GTK_WIDGET (gtk_builder_get_object (builder, "settings_window"));
    
    if (gtk_widget_get_visible (window) == FALSE)
      gw_main_search_cb (NULL, NULL);

    return TRUE;
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
gboolean gw_update_icons_for_selection (gpointer data) 
{
    //Declarations
    GtkBuilder *builder;
    GtkAction *action;
    gboolean has_selection;

    //Initializations;
    builder = gw_common_get_builder ();
    action = NULL;
    has_selection = gw_main_has_selection_by_target (GW_TARGET_RESULTS);

    //Set the special buttons
    if (!_prev_selection_icon_state && has_selection)
    {
      _prev_selection_icon_state = TRUE;
      action = GTK_ACTION (gtk_builder_get_object (builder, "file_append_action"));
      gtk_action_set_label (action, gettext("A_ppend Selected"));
      action = GTK_ACTION (gtk_builder_get_object (builder, "file_save_as_action"));
      gtk_action_set_label (action, gettext("Save Selected _As"));
      action = GTK_ACTION (gtk_builder_get_object (builder, "file_print_action"));
      gtk_action_set_label (action, gettext("_Print Selected"));
    }
    //Reset the buttons to their normal states
    else if (_prev_selection_icon_state == TRUE && !has_selection)
    {
      _prev_selection_icon_state = FALSE;
      action = GTK_ACTION (gtk_builder_get_object (builder, "file_append_action"));
      gtk_action_set_label (action, gettext("A_ppend"));
      action = GTK_ACTION (gtk_builder_get_object (builder, "file_save_as_action"));
      gtk_action_set_label (action, NULL);
      action = GTK_ACTION (gtk_builder_get_object (builder, "file_print_action"));
      gtk_action_set_label (action, NULL);
    }

    gw_main_update_toolbar_buttons();

    return TRUE; 
}




//!
//! @brief Uses a searchitem to cancel a search
//!
//! @param item A LwSearchItem to gleam information from
//!
gboolean gw_main_cancel_search_by_searchitem (LwSearchItem *item)
{
    if (item == NULL) return TRUE;
    
    g_mutex_lock (item->mutex);

      //Sanity check 1
      if (item->status == GW_SEARCH_IDLE) 
      {
        g_mutex_unlock (item->mutex);
        return TRUE;
      }

      //Sanity check 2
      if(item != NULL && item->status == GW_SEARCH_CANCELING) 
      {
        g_mutex_unlock (item->mutex);
        return FALSE;
      }

      //Sanity check 3
      if (item->thread == NULL)
      {
        item->status = GW_SEARCH_IDLE;
        g_mutex_unlock (item->mutex);
        return FALSE;
      }

      //Do the cancel operation
      item->status = GW_SEARCH_CANCELING;


    g_mutex_unlock (item->mutex);

    g_thread_join(item->thread);
    item->thread = NULL;

    return FALSE;
}


//!
//! @brief Cancels a search by identifying matching gpointer
//!
//! @param container A pointer to the top-most widget in the desired tab to cancel the search of.
//!
gboolean gw_main_cancel_search_by_tab_content (gpointer container)
{
    //Declarations
    GtkBuilder *builder;
    GtkWidget *notebook;
    int position;
    GList *list;
    LwSearchItem *item;
    gboolean result;

    //Initializations
    builder = gw_common_get_builder ();
    notebook = GTK_WIDGET (gtk_builder_get_object (builder, "notebook"));
    position = gtk_notebook_page_num (GTK_NOTEBOOK (notebook), container);

    //Sanity check
    if (position == -1) return FALSE;

    list = gw_tabs_get_searchitem_list ();
    item = g_list_nth_data (list, position);

    //Sanity check
    if (item == NULL) return TRUE;

    result = gw_main_cancel_search_by_searchitem (item);

    return result;
}


//!
//! @brief Cancels all searches in all currently open tabs
//!
void gw_main_tab_cancel_all_searches ()
{
    //Declarations
    GList *iter;
    LwSearchItem *item;

    for (iter = gw_tabs_get_searchitem_list (); iter != NULL; iter = iter->next)
    {
      item = (LwSearchItem*) iter->data;
      gw_main_cancel_search_by_searchitem (item);
    }
}


//!
//! @brief Cancels the search of the tab number
//!
//! @param page_num The page number of the tab to cancel the search of
//!
gboolean gw_main_cancel_search_by_tab_number (const int page_num)
{
    //Declarations
    GtkBuilder *builder;
    GtkWidget *notebook;
    GtkWidget *content;
    gboolean result;

    //Initializations
    builder = gw_common_get_builder ();
    notebook = GTK_WIDGET (gtk_builder_get_object (builder, "notebook"));
    content = gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), page_num);

    //Sanity check
    if (content == NULL) return TRUE;

    result = gw_main_cancel_search_by_tab_content (content);

    return result;
}


//!
//! @brief Cancels the search of the currently visibile tab
//!
gboolean gw_main_cancel_search_for_current_tab ()
{
    //Declarations
    GtkBuilder *builder;
    GtkWidget *notebook;
    int page_num;
    gboolean result;

    //Initializations
    builder = gw_common_get_builder ();
    notebook = GTK_WIDGET (gtk_builder_get_object (builder, "notebook"));
    page_num = gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));
    result = gw_main_cancel_search_by_tab_number (page_num);

    return result;
}


//!
//! @brief Cancels a search using a LwTargetOutput as identification
//!
gboolean gw_main_cancel_search_by_target (LwTargetOutput TARGET)
{
    //Declarations
    LwHistoryList* hl;
    LwSearchItem *item;

    if (TARGET == GW_TARGET_KANJI)
    {
      hl = lw_historylist_get_list(GW_HISTORYLIST_RESULTS);
      item = hl->current;
      return  gw_main_cancel_search_by_searchitem (item);
    }

    return FALSE;
}


//!
//! @brief Abstraction function to find out if some text is selected
//!
//! It gets the requested text buffer and then returns if it has text selected
//! or not.
//!
//! @param TARGET A LwTargetOutput
//!
gboolean gw_main_has_selection_by_target (LwTargetOutput TARGET)
{
    //Declarations
    GObject* tb;

    //Initializations
    tb = gw_common_get_gobject_by_target (TARGET);

    return (tb != NULL && gtk_text_buffer_get_has_selection (GTK_TEXT_BUFFER (tb)));
}

