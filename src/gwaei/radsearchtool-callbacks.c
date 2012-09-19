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

#include <gwaei/gwaei.h>


//!
//! @brief Brings up the search tool dialog
//!
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_radsearchtool_show_cb (GtkWidget *widget, gpointer data)
{
    GtkBuilder *builder = gw_common_get_builder ();

    GtkWidget *hbox;
    hbox = GTK_WIDGET (gtk_builder_get_object (builder, "strokes_hbox"));

    GtkWidget *spinbutton;
    spinbutton = GTK_WIDGET (gtk_builder_get_object (builder, "strokes_spinbutton"));
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (spinbutton), 1.0);

    gw_radsearchtool_clear_cb (NULL, NULL);

    //Show the window
    gw_common_show_window ("radicals_window");
}


//!
//! @brief Resets the states of all the buttons as if the dialog was just freshly opened
//!
//! @param widget Currently unused GtkWidget pointer
//! @param data Currently unused gpointer
//!
G_MODULE_EXPORT void gw_radsearchtool_clear_cb (GtkWidget *widget, gpointer data)
{
  gw_radsearchtool_deselect_all_radicals ();
  gw_radsearchtool_set_strokes_checkbox_state (FALSE);

  //Checks to make sure everything is sane
  if (gw_main_cancel_search_for_current_tab () == FALSE)
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
G_MODULE_EXPORT void gw_radsearchtool_search_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GtkBuilder *builder;
    LwDictInfo *di;
    LwHistoryList* hl;
    char *query_text;
    char *radicals_text;
    char *strokes_text;
    GError *error;

    //Initializations
    builder = gw_common_get_builder ();
    di = lw_dictinfolist_get_dictinfo (GW_ENGINE_KANJI, "Kanji");
    hl = lw_historylist_get_list(GW_HISTORYLIST_RESULTS);   
    query_text = NULL;
    radicals_text = gw_radsearchtool_strdup_all_selected_radicals ();
    strokes_text = gw_radsearchtool_strdup_prefered_stroke_count ();
    error = NULL;

    //Sanity check
    if (di == NULL) return;

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
    if (gw_main_cancel_search_for_current_tab () == FALSE) return;

    //Prep the search
    gw_main_clear_search_entry ();
    gw_main_search_entry_insert (query_text);
    gw_main_text_select_all_by_target (GW_TARGET_ENTRY);

    LwSearchItem* item;
    item = gw_tabs_get_searchitem ();

    //Move the previous searchitem to the history or destroy it
    if (lw_searchitem_has_history_relevance (item))
      lw_historylist_add_searchitem_to_history (GW_HISTORYLIST_RESULTS, item);
    else
      lw_searchitem_free (item);
    
    item = lw_searchitem_new (query_text, di, GW_TARGET_RESULTS, &error);
    
    if (item != NULL) 
    {
      gw_main_set_dictionary_by_searchitem (item);
      gw_tabs_set_searchitem (item);

      //Start the search
      lw_engine_get_results (hl->current, TRUE, FALSE);
      gw_main_update_history_popups ();
    }

    if (error != NULL)
    {
      printf("%s\n", error->message);
      g_error_free (error);
    }

    //Cleanup
    g_free (query_text);
}


//!
//! @brief Forces a search when the checkbox sensitivity is changed
//!
//! @param widget Currently unused GtkWidget pointer
//! @param data Currently unused gpointer
//!
G_MODULE_EXPORT void gw_radsearchtool_radical_kanji_stroke_checkbox_update_cb (GtkWidget *widget, gpointer data)
{
    gw_radsearchtool_update_strokes_checkbox_state ();

    //Start the search
    gw_radsearchtool_search_cb (NULL, NULL);
}


