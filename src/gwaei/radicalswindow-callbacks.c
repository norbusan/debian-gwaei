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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include <libwaei/libwaei.h>
#include <gwaei/radicalswindow.h>
#include <gwaei/radicalswindow-private.h>
#include <gwaei/gettext.h>


//!
//! @brief Resets the states of all the buttons as if the dialog was just freshly opened
//!
//! @param widget Currently unused GtkWidget pointer
//! @param data Currently unused gpointer
//!
G_MODULE_EXPORT void 
gw_radicalswindow_clear_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwRadicalsWindow *window;
    GwRadicalsWindowPrivate *priv;

    //Initializations
    window = GW_RADICALSWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_RADICALSWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;

    gw_radicalswindow_deselect (window);
    gtk_toggle_button_set_active (priv->strokes_checkbutton, FALSE);
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
gw_radicalswindow_toggled_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwRadicalsWindow *window;
    GwRadicalsWindowClass *klass;

    //Initializations
    window = GW_RADICALSWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_RADICALSWINDOW));
    g_return_if_fail (window != NULL);
    klass = GW_RADICALSWINDOW_CLASS (G_OBJECT_GET_CLASS (window));

    gw_radicalswindow_update_sensitivities (window, NULL);

    g_signal_emit (
      G_OBJECT (window), 
      klass->signalid[GW_RADICALSWINDOW_CLASS_SIGNALID_QUERY_CHANGED], 
      0
    );
}


//!
//! @brief Forces a search when the checkbox sensitivity is changed
//!
G_MODULE_EXPORT void 
gw_radicalswindow_strokes_checkbox_toggled_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwRadicalsWindow *window;
    GwRadicalsWindowClass *klass;
    GwRadicalsWindowPrivate *priv;
    gboolean request;

    //Initializations
    window = GW_RADICALSWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_RADICALSWINDOW));
    g_return_if_fail (window != NULL);
    klass = GW_RADICALSWINDOW_CLASS (G_OBJECT_GET_CLASS (window));
    priv = window->priv;
    request = gtk_toggle_button_get_active (priv->strokes_checkbutton);

    gw_radicalswindow_update_sensitivities (window, NULL);
    gtk_widget_set_sensitive (GTK_WIDGET (priv->strokes_spinbutton), request);

    g_signal_emit (
      G_OBJECT (window), 
      klass->signalid[GW_RADICALSWINDOW_CLASS_SIGNALID_QUERY_CHANGED], 
      0
    );
}


G_MODULE_EXPORT void 
gw_radicalswindow_close_cb (GSimpleAction *action, 
                            GVariant      *variant, 
                            gpointer       data)
{
    //Declarations
    GwRadicalsWindow *window;

    //Initializations
    window = GW_RADICALSWINDOW (data);

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
    g_return_if_fail (window != NULL);
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

