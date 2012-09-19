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
//! @file src/gtk-kanjipad-callbacks.c
//!
//! @brief Abstraction layer for gtk callbacks
//!
//! Callbacks for activities initiated by the user. Most of the gtk code here
//! should still be abstracted to the interface C file when possible.
//!


#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <gtk/gtk.h>

#include <gwaei/definitions.h>
#include <gwaei/preferences.h>
#include <gwaei/gtk-kanjipad-interface.h>
#include <gwaei/gtk-kanjipad-interface-drawingarea.h>
#include <gwaei/gtk-kanjipad-callbacks.h>


//!
//! @brief Does the needed calls to kpad to look up the kanji
//! 
G_MODULE_EXPORT gboolean do_kanjipad_look_up (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    //Declarations
    GwKanjipad *pad = (GwKanjipad*) data;

    //Sanity check
    if (pad->to_engine == NULL) return FALSE;

    /*	     kill 'HUP',$engine_pid; */
    GList *tmp_list;
    GString *message = g_string_new (NULL);
    GError *err = NULL;
      
    tmp_list = pa->strokes;
    while (tmp_list)
    {
      GList *stroke_list = tmp_list->data;
      while (stroke_list)
      {
        gint16 x = ((GdkPoint *)stroke_list->data)->x;
        gint16 y = ((GdkPoint *)stroke_list->data)->y;
        g_string_append_printf (message, "%d %d ", x, y);
        stroke_list = stroke_list->next;
      }
      g_string_append (message, "\n");
      tmp_list = tmp_list->next;
    }
    g_string_append (message, "\n");
    if (g_io_channel_write_chars (pad->to_engine, message->str, message->len, NULL, &err) != G_IO_STATUS_NORMAL)
    {
      g_printerr ("Cannot write message to engine: %s\n",
      err->message);
      exit (EXIT_FAILURE);
    }
    if (g_io_channel_flush (pad->to_engine, &err) != G_IO_STATUS_NORMAL)
    {
      g_printerr ("Error flushing message to engine: %s\n",
      err->message);
      exit (EXIT_FAILURE);
    }
    g_string_free (message, FALSE);

    return FALSE;
}


//!
//! @brief Clears the pad drawing area for the next kanji
//! 
G_MODULE_EXPORT void do_kanjipad_clear (GtkWidget *widget, gpointer data)
{
    drawingarea_clear (pa);
}


//!
//! @brief Adds the strokecounts to every line drawn in the padarea
//! 
G_MODULE_EXPORT void do_kanjipad_annotate_toggle (GtkWidget *widget, gpointer data)
{
    drawingarea_set_annotate (pa, !pa->annotate);
}


