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
//! @file src/gtk-main-callbacks.c
//!
//! @brief Abstraction layer for the drawing area
//!
//! Callbacks for activities initiated by the user. Most of the gtk code here
//! should still be abstracted to the interface C file when possible.
//!


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <gtk/gtk.h>

#include <gwaei/gtk.h>

#include <gwaei/gtk-kanjipad-interface.h>
#include <gwaei/gtk-kanjipad-interface-drawingarea.h>
#include <gwaei/gtk-kanjipad-interface-candidatearea.h>
#include <gwaei/gtk-kanjipad-callbacks.h>


//!
//! @brief To be written
//!
void gw_kanjipad_candidatearea_initialize (GwKanjipad *pad)
{
    GtkWidget *widget = pad->candidate_widget;

    g_signal_connect (widget, "configure_event", G_CALLBACK (candidatearea_configure_event), (gpointer) pad);
    g_signal_connect (widget, "expose_event", G_CALLBACK (candidatearea_expose_event), (gpointer) pad);
    g_signal_connect (widget, "button_press_event", G_CALLBACK (candidatearea_button_press_event), (gpointer) pad);

    gtk_widget_add_events (widget, GDK_EXPOSURE_MASK  | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);

    PangoFontDescription *font_desc;
    font_desc = pango_font_description_from_string ("Sans 18");
    gtk_widget_modify_font (widget, font_desc);
    pango_font_description_free (font_desc);
}


//!
//! @brief To be written
//!
static void candidatearea_get_char_size (GtkWidget *widget,
                                 int       *width,
                                 int       *height)
{
    PangoLayout *layout = gtk_widget_create_pango_layout (widget, "\xe6\xb6\x88");
    pango_layout_get_pixel_size (layout, width, height);
    g_object_unref (layout);
}


//!
//! @brief To be written
//!
static gchar *utf8_for_char (char wide_character[])
{
    gchar *string_utf;
    GError *err = NULL;
    gchar str[3];

    str[0] = wide_character[0] + 0x80;
    str[1] = wide_character[1] + 0x80;
    str[2] = '\0';

    string_utf = g_convert (str, -1, "UTF-8", "EUC-JP", NULL, NULL, &err);
    if (!string_utf)
    {
      g_printerr ("Cannot convert string from EUC-JP to UTF-8: %s\n",
      err->message);
      exit (EXIT_FAILURE);
    }

    return string_utf;
}


//!
//! @brief To be written
//!
static void candidatearea_draw_character (GtkWidget *w, int index, int selected)
{
    PangoLayout *layout;
    gchar *string_utf;
    gint char_width, char_height;
    gint x;

    candidatearea_get_char_size (w, &char_width, &char_height);

    if (selected >= 0)
    {
      gdk_draw_rectangle (pa->kpixmap,
        selected ? w->style->bg_gc[GTK_STATE_SELECTED] :
        w->style->white_gc, TRUE, 0, (char_height + 6) *index, w->allocation.width - 1, char_height + 5);
    }

    string_utf = utf8_for_char (pa->kanji_candidates[index]);
    layout = gtk_widget_create_pango_layout (w, string_utf);
    g_free (string_utf);

    x = (w->allocation.width - char_width) / 2;
    
    gdk_draw_layout (pa->kpixmap, 
       (selected > 0) ? w->style->white_gc : w->style->black_gc, x, (char_height + 6) * index + 3, layout);

    g_object_unref (layout);
}


//!
//! @brief To be written
//!
void candidatearea_draw (GtkWidget *w)
{
    gint width = w->allocation.width;
    gint height = w->allocation.height;
    int i;

    gdk_draw_rectangle (pa->kpixmap, w->style->white_gc, TRUE, 0, 0, width, height);

    for (i=0; i<pa->total_candidates; i++)
    {
      if (strcmp (pa->kselected, pa->kanji_candidates[i]) == 0)
        candidatearea_draw_character (w, i, 1);
      else
        candidatearea_draw_character (w, i, -1);
    }

    gtk_widget_queue_draw (w);
}


//!
//! @brief To be written
//!
gboolean candidatearea_configure_event (GtkWidget *w, GdkEventConfigure *event)
{
    if (pa->kpixmap)
      g_object_unref (pa->kpixmap);

    pa->kpixmap = gdk_pixmap_new (w->window, event->width, event->height, -1);

    candidatearea_draw (w);
    
    return TRUE;
}


//!
//! @brief To be written
//!
gboolean candidatearea_expose_event (GtkWidget *w, GdkEventExpose *event)
{
    if (!pa->kpixmap)
      return FALSE;

    gdk_draw_drawable (w->window,
                       w->style->fg_gc[GTK_STATE_NORMAL], pa->kpixmap,
                       event->area.x, event->area.y,
                       event->area.x, event->area.y,
                       event->area.width, event->area.height);

    return FALSE;
}


//!
//! @brief To be written
//!
static int candidatearea_erase_selection (GtkWidget *w)
{
    int i;
    if (pa->kselected[0] || pa->kselected[1])
    {
      for (i=0; i<pa->total_candidates; i++)
      {
        if (strncmp (pa->kselected, pa->kanji_candidates[i], 2))
        {
          candidatearea_draw_character (w, i, 0);
        }
      }
    }
    return TRUE;
}


//!
//! @brief To be written
//!
static void candidatearea_primary_clear (GtkClipboard *clipboard,
                                 gpointer      owner     )
{
    GtkWidget *w = owner;
    
    candidatearea_erase_selection (w);
    pa->kselected[0] = pa->kselected[1] = 0;

    gtk_widget_queue_draw (w);
}


//!
//! @brief To be written
//!
static void candidatearea_primary_get (GtkClipboard     *clipboard,
                                       GtkSelectionData *selection_data,
                                       guint             info,
                                       gpointer          owner           )
{
    if (pa->kselected[0] || pa->kselected[1])
    {
      gchar *string_utf = utf8_for_char (pa->kselected);
      gtk_selection_data_set_text (selection_data, string_utf, -1);
      g_free (string_utf);
    }
}


//!
//! @brief To be written
//!
gboolean candidatearea_button_press_event (GtkWidget *w, GdkEventButton *event)
{
    //Clear the highlighted area of the entry and store the start position
    gint start, end;
    GtkWidget *output_widget =  gw_kanjipad_get_target_text_widget ();
    gtk_editable_get_selection_bounds (GTK_EDITABLE (output_widget), &start, &end);
    gtk_editable_delete_text (GTK_EDITABLE (output_widget), start, end);

    int j;
    gint char_height;
    GtkClipboard *clipboard = gtk_clipboard_get (GDK_SELECTION_PRIMARY);

    static const GtkTargetEntry targets[] = {
      { "STRING", 0, 0 },
      { "TEXT",   0, 0 }, 
      { "COMPOUND_TEXT", 0, 0 },
      { "UTF8_STRING", 0, 0 }
    };

    candidatearea_erase_selection (w);

    candidatearea_get_char_size (w, NULL, &char_height);

    j = event->y / (char_height + 6);
    if (j < pa->total_candidates)
    {
      candidatearea_draw (w);
      strncpy(pa->kselected, pa->kanji_candidates[j], 2);
      candidatearea_draw_character (w, j, 1);
      
      if (!gtk_clipboard_set_with_owner (clipboard, targets, G_N_ELEMENTS (targets),
        candidatearea_primary_get, candidatearea_primary_clear, G_OBJECT (w)))
      candidatearea_primary_clear (clipboard, w);
    }
    else
    {
      pa->kselected[0] = 0;
      pa->kselected[1] = 0;
      if (gtk_clipboard_get_owner (clipboard) == G_OBJECT (w))
        gtk_clipboard_clear (clipboard);
    }

    gtk_widget_queue_draw (w);

    //Copy to clipboard if output_widget is NULL
    if ((pa->kselected[0] || pa->kselected[1]) && output_widget == NULL)
    {
      char *string_utf = utf8_for_char (pa->kselected);
        gtk_clipboard_set_text (gtk_clipboard_get (GDK_SELECTION_CLIPBOARD), string_utf, -1);
      g_free (string_utf);
    }
    //Insert the text into the editable widget
    else if (pa->kselected[0] || pa->kselected[1])
    {
      //Append the text at the cursor position
      char *string_utf;
      string_utf = utf8_for_char (pa->kselected);
      gtk_editable_insert_text (GTK_EDITABLE(output_widget), string_utf, -1, &start);
      gtk_editable_set_position (GTK_EDITABLE(output_widget), start);
      g_free (string_utf);
    }

    //Cleanup so the user can draw the next character
    drawingarea_clear (pa);

    return TRUE;
}


