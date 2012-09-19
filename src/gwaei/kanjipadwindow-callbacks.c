/******************************************************************************
    AUTHOR:
    KanjiPad - Japanese handwriting recognition front end
    Copyright (C) 1997 Owen Taylor
    File heavily modified and updated by Zachary Dovel.

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
//! @file kanjipadwindow-callbacks.c
//!
//! @brief To be written
//!


#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <gtk/gtk.h>

#include <gwaei/gwaei.h>
#include <gwaei/kanjipadwindow-private.h>


//!
//! @brief Does the needed calls to kpad to look up the kanji
//! 
G_MODULE_EXPORT gboolean gw_kanjipadwindow_look_up_cb (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    //Declarations
    GwKanjipadWindow *window;
    GwKanjipadWindowPrivate *priv;
    GList *iter;
    GList *inner_iter;
    GString *message;
    GError *error;
    gint16 x;
    gint16 y;

    //Initializations
    window = GW_KANJIPADWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_KANJIPADWINDOW));
    if (window == NULL) return FALSE;
    priv = window->priv;

    if (priv->to_engine == NULL)
      return FALSE;
    message = g_string_new (NULL);
    error = NULL;
      
    for (iter = priv->strokes; iter != NULL; iter = iter->next)
    {
      for (inner_iter = iter->data; inner_iter != NULL; inner_iter = inner_iter->next)
      {
        x = ((GdkPoint*) inner_iter->data)->x;
        y = ((GdkPoint*) inner_iter->data)->y;
        g_string_append_printf (message, "%d %d ", x, y);
      }
      g_string_append (message, "\n");
    }
    g_string_append (message, "\n");

    if (g_io_channel_write_chars (priv->to_engine, message->str, message->len, NULL, &error) != G_IO_STATUS_NORMAL)
    {
      fprintf (stderr, "Cannot write message to engine: %s\n", error->message);
      exit (EXIT_FAILURE);
    }

    if (g_io_channel_flush (priv->to_engine, &error) != G_IO_STATUS_NORMAL)
    {
      fprintf (stderr, "Error flushing message to engine: %s\n", error->message);
      exit (EXIT_FAILURE);
    }

    g_string_free (message, FALSE);

    return FALSE;
}


//!
//! @brief Clears the pad drawing area for the next kanji
//! 
G_MODULE_EXPORT void gw_kanjipadwindow_clear_drawingarea_cb (GtkWidget *widget, gpointer data)
{
    GwKanjipadWindow *window;

    window = GW_KANJIPADWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_KANJIPADWINDOW));
    if (window == NULL) return;

    gw_kanjipadwindow_clear_drawingarea (window);
}


//!
//! @brief Adds the strokecounts to every line drawn in the padarea
//! 
G_MODULE_EXPORT void do_kanjipad_annotate_toggled (GtkWidget *widget, gpointer data)
{
    GwKanjipadWindow *window;
    gboolean request;

    window = GW_KANJIPADWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_KANJIPADWINDOW));
    if (window == NULL) return;
    request = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));

    gw_kanjipadwindow_set_drawingarea_annotate (window, request);
}


G_MODULE_EXPORT void gw_kanjipadwindow_close_cb (GtkWidget *widget, gpointer data)
{
    GwKanjipadWindow *window;

    window = GW_KANJIPADWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_KANJIPADWINDOW));
    if (window == NULL) return;
   
    gtk_widget_destroy (GTK_WIDGET (window));
}


//!
//! @brief Preforms the action the window manager close event
//! @see gw_searchwindow_close_cb ()
//! @param widget GtkWidget pointer to the window to close
//! @param data Currently unused gpointer
//! @return Always returns true
//!
G_MODULE_EXPORT gboolean gw_kanjipadwindow_delete_event_action_cb (GtkWidget *widget, GdkEvent *event, gpointer data)
{ 
    gw_kanjipadwindow_close_cb (widget, data);    
    return TRUE;
}

