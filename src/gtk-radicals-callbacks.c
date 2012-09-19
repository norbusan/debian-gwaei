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
//! @file src/gtk-radicals-callbacks.c
//!
//! @brief Abstraction layer for gtk callbacks 
//!
//! This file in specifically written for the radical search tool popup.
//!


#include <regex.h>
#include <string.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include <gwaei/definitions.h>
#include <gwaei/utilities.h>
#include <gwaei/dictionary-objects.h>
#include <gwaei/search-objects.h>
#include <gwaei/engine.h>

#include <gwaei/gtk.h>
#include <gwaei/gtk-main-interface.h>
#include <gwaei/gtk-main-interface-tabs.h>
#include <gwaei/gtk-radicals-interface.h>


//!
//! @brief Resets the states of all the buttons as if the dialog was just freshly opened
//!
//! @param widget Currently unused GtkWidget pointer
//! @param data Currently unused gpointer
//!
G_MODULE_EXPORT void do_radical_clear (GtkWidget *widget, gpointer data)
{
  gw_ui_deselect_all_radicals ();
  gw_ui_set_strokes_checkbox_state (FALSE);

  //Checks to make sure everything is sane
  if (gw_ui_cancel_search_for_current_tab () == FALSE)
    return;
}


//!
//! @brief The function that does the grunt work of setting up a search using the window
//!
//! The function will get the data from the buttons to set up the query and the dictionary
//! with that to set up the searchitem. 
//!
//! @param widget Currently unused GtkWidget pointer
//! @param data Currently unused gpointer
//!
G_MODULE_EXPORT void do_radical_search (GtkWidget *widget, gpointer data)
{
    //Ininitializations
    GwHistoryList* hl = gw_historylist_get_list(GW_HISTORYLIST_RESULTS);   
    char *query_text = NULL;
    char *radicals_text = gw_ui_strdup_all_selected_radicals ();
    char *strokes_text = gw_ui_strdup_prefered_stroke_count ();
    GwDictInfo *di = gw_dictlist_get_dictinfo_by_alias ("Radicals");

    //Create the query string
    if (radicals_text != NULL && strokes_text != NULL)
    {
      query_text = g_strdup_printf ("%s%s", radicals_text, strokes_text);
    }
    else if (radicals_text != NULL)
    {
      query_text = radicals_text;
      radicals_text = NULL;
    }
    else if (strokes_text != NULL)
    {
      query_text = strokes_text;
      strokes_text = NULL;
    }

    //Free unneeded variables
    if (strokes_text != NULL)
    {
      g_free (strokes_text);
      strokes_text = NULL;
    }
    if (radicals_text != NULL)
    {
      g_free (radicals_text);
      strokes_text = NULL;
    }

    //Sanity checks
    if (query_text == NULL || strlen(query_text) == 0) return;
    if (di == NULL || di->status != GW_DICT_STATUS_INSTALLED) return;
    if (gw_ui_cancel_search_for_current_tab () == FALSE) return;

    //Prep the search
    gw_ui_clear_search_entry ();
    gw_ui_search_entry_insert (query_text);
    gw_ui_text_select_all_by_target (GW_TARGET_ENTRY);

    if (hl->current != NULL && (hl->current)->total_results > 0) 
    {
      gw_historylist_add_searchitem_to_history(GW_HISTORYLIST_RESULTS, hl->current);
      hl->current = NULL;
      gw_ui_update_history_popups();
    }
    else if (hl->current != NULL)
    {
      gw_searchitem_free (hl->current);
      hl->current = NULL;
    }
 
    hl->current = gw_searchitem_new (query_text, di, GW_TARGET_RESULTS);

    //Set the search item reference in the tabs
    GtkWidget *notebook = GTK_WIDGET (gtk_builder_get_object (builder, "notebook"));
    int page_num = gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));
    gw_tab_set_searchitem_by_page_num (hl->current, page_num);

    //Start the search
    gw_search_get_results (hl->current);

    //Cleanup
    if (query_text != NULL)
    {
      g_free (query_text);
      query_text = NULL;
    }
}


//!
//! @brief Forces a search when the checkbox sensitivity is changed
//!
//! @param widget Currently unused GtkWidget pointer
//! @param data Currently unused gpointer
//!
G_MODULE_EXPORT void do_radical_kanji_stroke_checkbox_update (GtkWidget *widget, gpointer data)
{
    gw_ui_update_strokes_checkbox_state ();

    //Start the search
    do_radical_search (NULL, NULL);
}


