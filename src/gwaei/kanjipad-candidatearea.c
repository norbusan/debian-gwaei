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
#include <pango/pangocairo.h>

#include <gwaei/gwaei.h>


//!
//! @brief To be written
//!
void gw_kanjipad_candidatearea_initialize (GwKanjipad *pad)
{
    GtkWidget *widget = pad->candidate_widget;

    g_signal_connect (widget, "configure_event", G_CALLBACK (candidatearea_configure_event), pad);
    g_signal_connect (widget, "draw", G_CALLBACK (candidatearea_draw_cb), pad);
    g_signal_connect (widget, "button_press_event", G_CALLBACK (candidatearea_button_press_event), (gpointer) pad);

    gtk_widget_add_events (widget, GDK_EXPOSURE_MASK  | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);

    PangoFontDescription *font_desc;
    font_desc = pango_font_description_from_string ("Sans 18");
    gtk_widget_override_font (widget, font_desc);
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
static void candidatearea_draw_character (GtkWidget *widget, int index, int selected, GwKanjipad *pad)
{
    //Declarations
    PangoLayout *layout;
    gchar *string_utf;
    gint char_width, char_height;
    gint x, y;
    int allocated_width, allocated_height;
    int width, height;
    cairo_t *cr;
    GtkStyleContext *context;
    GdkRGBA fgcolorn;
    GdkRGBA bgcolorn;
    GdkRGBA fgcolors;
    GdkRGBA bgcolors;

    //Initializations
    cr = cairo_create (pad->ksurface);
    allocated_width = gtk_widget_get_allocated_width (widget);
    allocated_height = gtk_widget_get_allocated_height (widget);
    context = gtk_widget_get_style_context (widget);

    gtk_style_context_get_color (context, GTK_STATE_FLAG_NORMAL, &fgcolorn);
    gtk_style_context_get_background_color (context, GTK_STATE_FLAG_NORMAL, &bgcolorn);
    gtk_style_context_get_color (context, GTK_STATE_FLAG_SELECTED, &fgcolors);
    gtk_style_context_get_background_color (context, GTK_STATE_FLAG_SELECTED, &bgcolors);

    candidatearea_get_char_size (widget, &char_width, &char_height);

    if (selected >= 0)
    {
      if (selected)
        cairo_set_source_rgba (cr, bgcolors.red, bgcolors.green, bgcolors.blue, 1.0);
      else
        cairo_set_source_rgba (cr, bgcolorn.red, bgcolorn.green, bgcolorn.blue, 1.0);

      x = 0;
      y = (char_height + 6) * index;
      width = allocated_width - 1;
      height = char_height + 5;

      cairo_rectangle (cr, x, y, width, height);
      cairo_fill (cr);
    }

    x = (allocated_width - char_width) / 2;
    y = (char_height + 6) * index + 3;;
    cairo_translate(cr, x, y);
    string_utf = utf8_for_char (pa->kanji_candidates[index]);
    layout = gtk_widget_create_pango_layout (widget, string_utf);
    g_free (string_utf);
    
    if (selected >= 0 && selected)
      cairo_set_source_rgba (cr, fgcolors.red, fgcolors.green, fgcolors.blue, 1.0);
    else
      cairo_set_source_rgba (cr, fgcolorn.red, fgcolorn.green, fgcolorn.blue, 1.0);

    pango_cairo_update_layout (cr, layout);
    pango_cairo_show_layout (cr, layout);

    g_object_unref (layout);
    cairo_destroy (cr);
}


//!
//! @brief To be written
//!
void _candidatearea_draw (GtkWidget *widget, GwKanjipad *pad)
{
    //Declarations
    gint width;
    gint height;
    int i;
    cairo_t *cr;

    //Initializations
    height = gtk_widget_get_allocated_height (widget);
    width = gtk_widget_get_allocated_width (widget);
    cr = cairo_create (pad->ksurface);

    cairo_save (cr);
    cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 0.0);
    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint (cr);
    cairo_fill (cr);
    cairo_restore (cr);

    for (i = 0; i < pa->total_candidates; i++)
    {
      if (strcmp (pa->kselected, pa->kanji_candidates[i]) == 0)
        candidatearea_draw_character (widget, i, 1, pad);
      else
        candidatearea_draw_character (widget, i, -1, pad);
    }

    gtk_widget_queue_draw (widget);
}


//!
//! @brief To be written
//!
gboolean candidatearea_configure_event (GtkWidget *widget, GdkEventConfigure *event, GwKanjipad *pad)
{
    if (pad->ksurface)
      cairo_surface_destroy (pad->ksurface);

    pad->ksurface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, event->width, event->height);

    _candidatearea_draw (widget, pad);

    return TRUE;
}


//!
//! @brief To be written
//!
gboolean candidatearea_draw_cb (GtkWidget *widget, cairo_t *cr, GwKanjipad *pad)
{
    if (pad->ksurface == NULL)
      return FALSE;

    cairo_set_source_surface (cr, pad->ksurface, 0, 0);
    cairo_paint (cr);

    return TRUE;
}


//!
//! @brief To be written
//!
static int candidatearea_erase_selection (GtkWidget *widget, GwKanjipad *pad)
{
    int i;
    if (pa->kselected[0] || pa->kselected[1])
    {
      for (i=0; i<pa->total_candidates; i++)
      {
        if (strncmp (pa->kselected, pa->kanji_candidates[i], 2))
        {
          candidatearea_draw_character (widget, i, 0, pad);
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
    GtkWidget *widget = owner;
    
    candidatearea_erase_selection (widget, pa);
    pa->kselected[0] = pa->kselected[1] = 0;

    gtk_widget_queue_draw (widget);
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
gboolean candidatearea_button_press_event (GtkWidget *widget, GdkEventButton *event, GwKanjipad *pad)
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

    candidatearea_erase_selection (widget, pad);

    candidatearea_get_char_size (widget, NULL, &char_height);

    j = event->y / (char_height + 6);
    if (j < pa->total_candidates)
    {
      _candidatearea_draw (widget, pad);
      strncpy(pa->kselected, pa->kanji_candidates[j], 2);
      candidatearea_draw_character (widget, j, 1, pad);
      
      if (!gtk_clipboard_set_with_owner (clipboard, targets, G_N_ELEMENTS (targets),
        candidatearea_primary_get, candidatearea_primary_clear, G_OBJECT (widget)))
      candidatearea_primary_clear (clipboard, widget);
    }
    else
    {
      pa->kselected[0] = 0;
      pa->kselected[1] = 0;
      if (gtk_clipboard_get_owner (clipboard) == G_OBJECT (widget))
        gtk_clipboard_clear (clipboard);
    }

    gtk_widget_queue_draw (widget);

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


