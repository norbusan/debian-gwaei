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
//! @file kanjipad-candidatearea.c
//!
//! @brief To be written
//!


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <gtk/gtk.h>
#include <pango/pangocairo.h>

#include <gwaei/gwaei.h>
#include <gwaei/kanjipadwindow-private.h>


//!
//! @brief To be written
//!
void gw_kanjipadwindow_initialize_candidates (GwKanjipadWindow *window)
{
    //Declarations
    GwKanjipadWindowPrivate *priv;
    gint mask;
    PangoFontDescription *desc;

    //Initializations
    priv = window->priv;
    mask = (GDK_EXPOSURE_MASK  | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
    desc = pango_font_description_from_string ("Sans 18");

    g_signal_connect (priv->candidates, "configure_event", G_CALLBACK (gw_kanjipadwindow_candidatearea_configure_event_cb), window);
    g_signal_connect (priv->candidates, "draw", G_CALLBACK (gw_kanjipadwindow_candidatearea_draw_cb), window);
    g_signal_connect (priv->candidates, "button_press_event", G_CALLBACK (gw_kanjipadwindow_candidatearea_button_press_event_cb), window);
    gtk_widget_add_events (GTK_WIDGET (priv->candidates), mask);

    if (desc != NULL)
    {
      gtk_widget_override_font (GTK_WIDGET (priv->candidates), desc);
      pango_font_description_free (desc);
    }
}


//!
//! @brief To be written
//!
static void _kanjipadwindow_get_candidate_character_size (GwKanjipadWindow *window, int *width, int *height)
{
    GwKanjipadWindowPrivate *priv;
    PangoLayout *layout;

    priv = window->priv;
    layout = gtk_widget_create_pango_layout (GTK_WIDGET (priv->candidates), "\xe6\xb6\x88");

    if (layout != NULL)
    {
      pango_layout_get_pixel_size (layout, width, height);
      g_object_unref (layout);
    }
    else
    {
      *width = 0;
      *height = 0;
    }
}


//!
//! @brief To be written
//!
static gchar *_kanjipadwindow_utf8_for_char (char wide_character[])
{
    //Declarations
    gchar *string_utf;
    GError *error;
    gchar str[3];

    //Initializaitons
    error = NULL;
    str[0] = wide_character[0] + 0x80;
    str[1] = wide_character[1] + 0x80;
    str[2] = '\0';
    string_utf = g_convert (str, -1, "UTF-8", "EUC-JP", NULL, NULL, &error);

    if (error != NULL)
    {
      g_printerr ("Cannot convert string from EUC-JP to UTF-8: %s\n",
      error->message);
      exit (EXIT_FAILURE);
    }

    return string_utf;
}


//!
//! @brief To be written
//!
static void _kanjipadwindow_draw_candidate_character (GwKanjipadWindow *window, int index, int selected)
{
    //Declarations
    GwKanjipadWindowPrivate *priv;
    PangoLayout *layout;
    gchar *string_utf;
    gint char_width, char_height;
    gint x, y;
    int allocated_width;
    int width, height;
    cairo_t *cr;
    GtkStyleContext *context;
    GdkRGBA fgcolorn;
    GdkRGBA bgcolorn;
    GdkRGBA fgcolors;
    GdkRGBA bgcolors;

    //Initializations
    priv = window->priv;
    cr = cairo_create (priv->ksurface);
    allocated_width = gtk_widget_get_allocated_width (GTK_WIDGET (priv->candidates));
    context = gtk_widget_get_style_context (GTK_WIDGET (priv->candidates));

    gtk_style_context_get_color (context, GTK_STATE_FLAG_NORMAL, &fgcolorn);
    gtk_style_context_get_background_color (context, GTK_STATE_FLAG_NORMAL, &bgcolorn);
    gtk_style_context_get_color (context, GTK_STATE_FLAG_SELECTED, &fgcolors);
    gtk_style_context_get_background_color (context, GTK_STATE_FLAG_SELECTED, &bgcolors);

    _kanjipadwindow_get_candidate_character_size (window, &char_width, &char_height);

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
    y = (char_height + 6) * index + 3;
    cairo_translate(cr, x, y);
    string_utf = _kanjipadwindow_utf8_for_char (priv->kanji_candidates[index]);
    

    layout = gtk_widget_create_pango_layout (GTK_WIDGET (priv->candidates), string_utf);
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
void gw_kanjipadwindow_draw_candidates (GwKanjipadWindow *window)
{
    //Declarations
    GwKanjipadWindowPrivate *priv;
    //gint width;
    //gint height;
    int i;
    cairo_t *cr;

    //Initializations
    priv = window->priv;
    //height = gtk_widget_get_allocated_height (GTK_WIDGET (priv->candidates));
    //width = gtk_widget_get_allocated_width (GTK_WIDGET (priv->candidates));
    cr = cairo_create (priv->ksurface);

    cairo_save (cr);
    cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 0.0);
    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint (cr);
    cairo_fill (cr);
    cairo_restore (cr);

    for (i = 0; i < priv->total_candidates; i++)
    {
      if (strcmp (priv->kselected, priv->kanji_candidates[i]) == 0)
        _kanjipadwindow_draw_candidate_character (window, i, 1);
      else
        _kanjipadwindow_draw_candidate_character (window, i, -1);
    }

    gtk_widget_queue_draw (GTK_WIDGET (priv->candidates));
}


//!
//! @brief To be written
//!
G_MODULE_EXPORT gboolean gw_kanjipadwindow_candidatearea_configure_event_cb (GtkWidget *widget, GdkEventConfigure *event, gpointer data)
{
    GwKanjipadWindow *window;
    GwKanjipadWindowPrivate *priv;

    window = GW_KANJIPADWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_KANJIPADWINDOW));
    if (window == NULL) return FALSE;
    priv = window->priv;

    if (priv->ksurface)
      cairo_surface_destroy (priv->ksurface);

    priv->ksurface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, event->width, event->height);

    gw_kanjipadwindow_draw_candidates (window);

    return TRUE;
}


//!
//! @brief To be written
//!
G_MODULE_EXPORT gboolean gw_kanjipadwindow_candidatearea_draw_cb (GtkWidget *widget, cairo_t *cr, gpointer data)
{
    GwKanjipadWindow *window;
    GwKanjipadWindowPrivate *priv;

    window = GW_KANJIPADWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_KANJIPADWINDOW));
    if (window == NULL) return FALSE;
    priv = window->priv;

    //Sanity check
    if (priv->ksurface == NULL) return FALSE;

    cairo_set_source_surface (cr, priv->ksurface, 0, 0);
    cairo_paint (cr);

    return TRUE;
}


//!
//! @brief To be written
//!
static int _kanjipadwindow_erase_candidate_selection (GwKanjipadWindow *window)
{
    //Declarations
    GwKanjipadWindowPrivate *priv;
    int i;

    //Initializations
    priv = window->priv;

    if (priv->kselected[0] || priv->kselected[1])
    {
      for (i = 0; i < priv->total_candidates; i++)
      {
        if (strncmp (priv->kselected, priv->kanji_candidates[i], 2))
        {
          _kanjipadwindow_draw_candidate_character (window, i, 0);
        }
      }
    }
    return TRUE;
}


//!
//! @brief To be written
//!
static void _kanjipadwindow_primary_candidates_clear (GtkClipboard *clipboard, gpointer data)
{
    GwKanjipadWindow *window;
    GwKanjipadWindowPrivate *priv;

    window = GW_KANJIPADWINDOW (data);
    if (window == NULL) return;
    priv = window->priv;

    _kanjipadwindow_erase_candidate_selection (window);

    priv->kselected[0] = 0;
    priv->kselected[1] = 0;

    gtk_widget_queue_draw (GTK_WIDGET (priv->candidates));
}


//!
//! @brief To be written
//!
static void _kanjipadwindow_primary_candidates_get (GtkClipboard *clipboard, GtkSelectionData *selection_data, guint info, gpointer data)
{
    GwKanjipadWindow *window;
    GwKanjipadWindowPrivate *priv;
    gchar *string_utf;

    window = GW_KANJIPADWINDOW (data);
    priv = window->priv;

    if (priv->kselected[0] || priv->kselected[1])
    {
      string_utf = _kanjipadwindow_utf8_for_char (priv->kselected);
      gtk_selection_data_set_text (selection_data, string_utf, -1);
      g_free (string_utf);
    }
}


//!
//! @brief To be written
//!
G_MODULE_EXPORT gboolean gw_kanjipadwindow_candidatearea_button_press_event_cb (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    //Declarations
    GwKanjipadWindow *window;
    GwKanjipadWindowPrivate *priv;
    GwSearchWindow *searchwindow;
    int j;
    gint char_height;
    GtkClipboard *clipboard;
    char *string_utf;

    //Initializations
    window = GW_KANJIPADWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_KANJIPADWINDOW));
    if (window == NULL) return FALSE;
    priv = window->priv;
    searchwindow = GW_SEARCHWINDOW (gtk_window_get_transient_for (GTK_WINDOW (window)));
    g_assert (searchwindow != NULL);
    clipboard = gtk_clipboard_get (GDK_SELECTION_PRIMARY);

    static const GtkTargetEntry targets[] = {
      { "STRING", 0, 0 },
      { "TEXT",   0, 0 }, 
      { "COMPOUND_TEXT", 0, 0 },
      { "UTF8_STRING", 0, 0 }
    };

    _kanjipadwindow_erase_candidate_selection (window);
    _kanjipadwindow_get_candidate_character_size (window, NULL, &char_height);

    j = event->y / (char_height + 6);
    if (j < priv->total_candidates)
    {
      gw_kanjipadwindow_draw_candidates (window); 
      strncpy(priv->kselected, priv->kanji_candidates[j], 2);
      _kanjipadwindow_draw_candidate_character (window, j, 1);
      
      if (!gtk_clipboard_set_with_owner (clipboard, targets, G_N_ELEMENTS (targets),
        _kanjipadwindow_primary_candidates_get, _kanjipadwindow_primary_candidates_clear, G_OBJECT (widget)))
      _kanjipadwindow_primary_candidates_clear (clipboard, widget);
    }
    else
    {
      priv->kselected[0] = 0;
      priv->kselected[1] = 0;
      if (gtk_clipboard_get_owner (clipboard) == G_OBJECT (widget))
        gtk_clipboard_clear (clipboard);
    }

    gtk_widget_queue_draw (widget);


    //Copy to clipboard if output_widget is NULL
    if ((priv->kselected[0] || priv->kselected[1]) && searchwindow == NULL)
    {
      string_utf = _kanjipadwindow_utf8_for_char (priv->kselected);
      gtk_clipboard_set_text (gtk_clipboard_get (GDK_SELECTION_CLIPBOARD), string_utf, -1);
      g_free (string_utf);
    }
    //Insert the text into the editable widget
    else if (priv->kselected[0] || priv->kselected[1])
    {
      string_utf = _kanjipadwindow_utf8_for_char (priv->kselected);
      gw_searchwindow_entry_insert_text (searchwindow, string_utf);
      g_free (string_utf);
    }

    //Cleanup so the user can draw the next character
    gw_kanjipadwindow_clear_drawingarea (window);

    return TRUE;
}


