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
//! @file radicalswindow-callbacks.c
//!
//! @brief To be written
//!


#include <string.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include <gwaei/gwaei.h>
#include <gwaei/radicalswindow-private.h>


//!
//! @brief Resets the states of all the buttons as if the dialog was just freshly opened
//!
//! @param widget Currently unused GtkWidget pointer
//! @param data Currently unused gpointer
//!
G_MODULE_EXPORT void 
gw_radicalswindow_clear_cb (GtkWidget *widget, gpointer data)
{
    //Declaratins
    GwApplication *application;
    GwRadicalsWindow *window;
    GwRadicalsWindowPrivate *priv;

    //Initializations
    window = GW_RADICALSWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_RADICALSWINDOW));
    if (window == NULL) return;
    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));

    gw_application_block_searches (application);

    gw_radicalswindow_deselect_all_radicals (window);
    gtk_toggle_button_set_active (priv->strokes_checkbutton, FALSE);

    gw_application_unblock_searches (application);
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
G_MODULE_EXPORT void 
gw_radicalswindow_search_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwApplication *application;
    GwRadicalsWindow *window;
    GwSearchWindow *searchwindow;
    LwDictInfoList *dictinfolist;
    LwDictInfo *di;
    char *text_query;
    char *text_radicals;
    char *text_strokes;

    //Initializations
    window = GW_RADICALSWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_RADICALSWINDOW));
    if (window == NULL) return;
    searchwindow = GW_SEARCHWINDOW (gtk_window_get_transient_for (GTK_WINDOW (window)));
    g_assert (searchwindow != NULL);
    application = gw_window_get_application (GW_WINDOW (window));
    dictinfolist = LW_DICTINFOLIST (gw_application_get_dictinfolist (application));
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


//!
//! @brief Forces a search when the checkbox sensitivity is changed
//!
G_MODULE_EXPORT void 
gw_radicalswindow_strokes_checkbox_toggled_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwRadicalsWindow *window;
    GwRadicalsWindowPrivate *priv;
    gboolean request;

    //Initializations
    window = GW_RADICALSWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_RADICALSWINDOW));
    if (window == NULL) return;
    priv = window->priv;
    request = gtk_toggle_button_get_active (priv->strokes_checkbutton);

    gtk_widget_set_sensitive (GTK_WIDGET (priv->strokes_spinbutton), request);

    gw_radicalswindow_search_cb (widget, data);
}


G_MODULE_EXPORT void 
gw_radicalswindow_close_cb (GtkWidget* widget, gpointer data)
{
    //Declarations
    GwRadicalsWindow *window;

    //Initializations
    window = GW_RADICALSWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_RADICALSWINDOW));
    if (window == NULL) return;

    gtk_widget_hide (GTK_WIDGET (window));
}


G_MODULE_EXPORT void 
gw_radicalswindow_show_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GtkWindow *window;
    GtkScrolledWindow *scrolledwindow;
    GtkPolicyType policy;
    int x, y, width, height, max_height;

    //Initializations
    window = GTK_WINDOW (widget);
    if (window == NULL) return;
    scrolledwindow = GTK_SCROLLED_WINDOW (data);
    gtk_scrolled_window_get_policy (scrolledwindow, NULL, &policy);

    gtk_scrolled_window_set_policy (scrolledwindow, GTK_POLICY_NEVER, GTK_POLICY_NEVER);
    gtk_widget_queue_resize_no_redraw (GTK_WIDGET (window));
    gtk_window_get_position (window, &x, &y);
    gtk_window_get_size (window, &width, &height);
    max_height = gdk_screen_height () - y;
    if (max_height > 50) max_height -= 50;

    if (height > max_height)
    {
      gtk_scrolled_window_set_policy (scrolledwindow, GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
      gtk_window_resize (GTK_WINDOW (window), width, max_height);
    }
    else
    {
      gtk_scrolled_window_set_policy (scrolledwindow, GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    }

    return;
}

