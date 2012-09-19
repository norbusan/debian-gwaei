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

#include <gwaei/gwaei.h>



//!
//! @brief To be written
//!
static void drawingarea_free_stroke (GList *stroke)
{
    GList *tmp_list = stroke;
    while (tmp_list)
    {
      g_free (tmp_list->data);
      tmp_list = tmp_list->next;
    }
    g_list_free (stroke);
}


//!
//! @brief To be written
//!
static void drawingarea_annotate_stroke (GwKanjipad *pad, GList *stroke, cairo_t *cr, gint index)
{
    GdkPoint *cur, *old;

    // Annotate the stroke with the stroke number - the algorithm
    // for placing the digit is pretty simple. The text is inscribed
    // in a circle tangent to the stroke. The circle will be above
    // and/or to the left of the line

    if (stroke)
    {
      old = (GdkPoint *)stroke->data;
      do
      {
        cur = (GdkPoint *)stroke->data;
        stroke = stroke->next;
      }
      while (stroke && abs(cur->x - old->x) < 5 && abs (cur->y - old->y) < 5);
      
      if (stroke)
      {
        char buffer[16];
        PangoLayout *layout;
        int swidth, sheight;
        gint16 x, y;
        double r;
        double dx = cur->x - old->x;
        double dy = cur->y - old->y;
        double dl = sqrt(dx*dx+dy*dy);
        int sign = (dy <= dx) ? 1 : -1;
        GdkRectangle update_area;

        sprintf (buffer, "%d", index);

        r = sqrt(swidth*swidth + sheight*sheight);
        
        x = 0.5 + old->x + 0.5*r*dx/dl + sign * 0.5*r*dy/dl;
        y = 0.5 + old->y + 0.5*r*dy/dl - sign * 0.5*r*dx/dl;
        
        x -= swidth/2;
        y -= sheight/2;

        update_area.x = x;
        update_area.y = y;
        update_area.width = swidth;
        update_area.height = sheight;
        
        x = CLAMP (x, 0, gtk_widget_get_allocated_width (pad->drawing_widget) - swidth);
        y = CLAMP (y, 0, gtk_widget_get_allocated_height (pad->drawing_widget) - sheight);

        cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_source_rgb (cr, 1.0, 0.0, 0.0);
        cairo_set_font_size (cr, 10.0);
        cairo_move_to (cr, x, y);
        cairo_show_text (cr, buffer);
        cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
      }
    }
}

//!
//! @brief To be written
//!
static void _drawingarea_init (GwKanjipad *pad)
{
    //Declarations
    GList *tmp_list;
    int index = 1;
    guint16 width;
    guint16 height;
    cairo_surface_t *cst;
    cairo_t *cr;
    double half_width;
    double half_height;
    double dashes[] = { 10.0, 10.0,  };
    int ndash;
    double offset;

    GtkStyleContext *context;
    GdkRGBA fgcolorn;
    GdkRGBA bgcolorn;
    GdkRGBA fgcolors;
    GdkRGBA bgcolors;

    //Initializations
    index = 1;
    cr = cairo_create (pad->surface);
    width = gtk_widget_get_allocated_width (pad->drawing_widget);
    height = gtk_widget_get_allocated_height (pad->drawing_widget);
    half_width = ((double) width / 2.0);
    half_height = ((double) height / 2.0);
    ndash  = sizeof (dashes)/sizeof(dashes[0]);
    offset = 0.0;

    context = gtk_widget_get_style_context (pad->drawing_widget);
    gtk_style_context_get_color (context, GTK_STATE_FLAG_NORMAL, &fgcolorn);
    gtk_style_context_get_background_color (context, GTK_STATE_FLAG_NORMAL, &bgcolorn);
    gtk_style_context_get_color (context, GTK_STATE_FLAG_SELECTED, &fgcolors);
    gtk_style_context_get_background_color (context, GTK_STATE_FLAG_SELECTED, &bgcolors);


    //Drawing
    cairo_set_source_rgba (cr, fgcolors.red, fgcolors.green, fgcolors.blue, 1.0);
    cairo_rectangle (cr, 0, 0, width, height);
    cairo_fill (cr);


    //Draw bounding box
    cairo_set_source_rgba (cr, bgcolors.red, bgcolors.green, bgcolors.blue, 0.5);
    cairo_set_line_width (cr, 6.0 );
    cairo_rectangle (cr, 0, 0, width, height);
    cairo_stroke (cr);

    //Draw cross hairs
    cairo_set_source_rgba (cr, bgcolors.red, bgcolors.green, bgcolors.blue, 0.5);
    cairo_set_line_width (cr, 3.0);
    cairo_set_dash (cr, dashes, ndash, offset);

    cairo_move_to (cr, half_width, 0.0);
    cairo_line_to (cr, half_width, height);
    cairo_stroke (cr);
    cairo_move_to (cr, 0.0,  half_height);
    cairo_line_to (cr, width, half_height);
    cairo_stroke (cr);

    //Draw strokes
    cairo_set_dash (cr, NULL, 0, 0);
    cairo_set_line_width (cr, 2.0);
    cairo_set_source_rgba (cr, fgcolorn.red, fgcolorn.green, fgcolorn.blue, 1.0);

    tmp_list = pad->strokes;
    while (tmp_list)
    {
      GdkPoint *cur, *old;
      GList *stroke_list = tmp_list->data;

      old = NULL;

      if (pad->annotate)
        drawingarea_annotate_stroke (pad, stroke_list, cr, index);

      while (stroke_list)
      {
        cur = (GdkPoint *)stroke_list->data;
        if (old)
        {
          cairo_move_to (cr, old->x, old->y);
          cairo_line_to (cr, cur->x, cur->y);
          cairo_stroke (cr);
        }

        old = cur;
        stroke_list = stroke_list->next;
      }
      
      tmp_list = tmp_list->next;
      index++;
    }

    // End drawing here 
    cairo_destroy(cr);

    gtk_widget_queue_draw (pad->drawing_widget);
}


//!
//! @brief To be written
//!
static int drawingarea_configure_event (GtkWidget *w, GdkEventConfigure *event, GwKanjipad *pad)
{
    if (pad->surface != NULL)
      cairo_surface_destroy (pad->surface);

    pad->surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, event->width, event->height);

    _drawingarea_init (pad);

    return TRUE;
}


//!
//! @brief To be written
//!
static int drawingarea_draw_cb (GtkWidget *widget, cairo_t *cr, GwKanjipad *pad)
{
    if (pad->surface == NULL)
      return FALSE;

    cairo_set_source_surface (cr, pad->surface, 0, 0);
    cairo_paint (cr);

    return TRUE;
}


//!
//! @brief To be written
//!
static int drawingarea_button_press_event (GtkWidget *w, GdkEventButton *event, GwKanjipad *pad)
{
    if (event->button == 1)
    {
      GdkPoint *p = g_new (GdkPoint, 1);
      p->x = event->x;
      p->y = event->y;
      pad->curstroke = g_list_append (pad->curstroke, p);
      pad->instroke = TRUE;
    }

    return TRUE;
}


//!
//! @brief To be written
//!
static int drawingarea_button_release_event (GtkWidget *w, GdkEventButton *event, GwKanjipad *pad)
{
    if (pad->annotate)
    {
      //Declarations
      cairo_t *cr;

      //Initializations
      cr = cairo_create(pad->surface);

      drawingarea_annotate_stroke (pad, pad->curstroke, cr, g_list_length (pad->strokes) + 1);
      //cairo_paint(cr);
      cairo_destroy(cr);
      gtk_widget_queue_draw (pad->drawing_widget);
    }

    pad->strokes = g_list_append (pad->strokes, pad->curstroke);
    pad->curstroke = NULL;
    pad->instroke = FALSE;

    return FALSE;
}


//!
//! @brief To be written
//!
static int drawingarea_motion_event (GtkWidget *w, GdkEventMotion *event, GwKanjipad *pad)
{
    //Declarations
    gint x,y;
    GdkModifierType state;
    GdkRectangle rect;
    GdkPoint *p;
    int xmin, ymin, xmax, ymax;
    GdkPoint *old;
    guint16 width;
    guint16 height;

    if (event->is_hint)
    {
      gdk_window_get_pointer (gtk_widget_get_window (w), &x, &y, &state);
    }
    else
    {
      x = event->x;
      y = event->y;
      state = event->state;
    }

    if (pad->instroke && state & GDK_BUTTON1_MASK)
    {
      old = (GdkPoint*) g_list_last (pad->curstroke)->data;
      width = gtk_widget_get_allocated_width (pad->drawing_widget);
      height = gtk_widget_get_allocated_height (pad->drawing_widget);

      //extend line
      cairo_t *cr = cairo_create(pad->surface);
      cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
      cairo_set_line_width (cr, 2.0 );
      cairo_move_to (cr, old->x,  old->y);
      cairo_line_to (cr, x, y);
      cairo_stroke (cr);
      cairo_destroy(cr);

      if (old->x < x) { xmin = old->x; xmax = x; }
      else            { xmin = x;      xmax = old->x; }

      if (old->y < y) { ymin = old->y; ymax = y; }
      else            { ymin = y;      ymax = old->y; }

      rect.x = xmin - 1 - 3; 
      rect.y = ymin = 1 - 3;
      rect.width  = xmax - xmin + 2 + 3;
      rect.height = ymax - ymin + 2 + 3;
      gdk_window_invalidate_rect (gtk_widget_get_window (w), &rect, FALSE);

      p = g_new (GdkPoint, 1);
      p->x = x;
      p->y = y;
      pad->curstroke = g_list_append (pad->curstroke, p);

      gtk_widget_queue_draw (pad->drawing_widget);
    }

    return TRUE;
}


//!
//! @brief To be written
//!
void drawingarea_clear (GwKanjipad *pad)
{
    //Declarations
    GList *tmp_list;

    //Initializations
    tmp_list = pad->strokes;

    while (tmp_list)
    {
      drawingarea_free_stroke (tmp_list->data);
      tmp_list = tmp_list->next;
    }
    g_list_free (pad->strokes);
    pad->strokes = NULL;

    g_list_free (pad->curstroke);
    pad->curstroke = NULL;

    _drawingarea_init (pad);
}


//!
//! @brief To be written
//!
void drawingarea_set_annotate (GwKanjipad *pad, gboolean annotate)
{
    if (pad->annotate != annotate)
    {
      pad->annotate = annotate;
      _drawingarea_init (pad);
    }
}


//!
//! @brief To be written
//!
void gw_kanjipad_drawingarea_initialize (GwKanjipad *pad)
{
    GtkWidget *widget = pad->drawing_widget;

    gtk_widget_set_size_request (widget, 100, 100);

    g_signal_connect (widget, "configure_event", G_CALLBACK (drawingarea_configure_event), pad);
    g_signal_connect (widget, "draw", G_CALLBACK (drawingarea_draw_cb), pad);
    g_signal_connect (widget, "button_press_event", G_CALLBACK (drawingarea_button_press_event), pad);
    g_signal_connect (widget, "button_release_event", G_CALLBACK (drawingarea_button_release_event), pad);
    g_signal_connect (widget, "motion_notify_event", G_CALLBACK (drawingarea_motion_event), pad);
    gint mask = (
                  GDK_EXPOSURE_MASK |
                  GDK_BUTTON_PRESS_MASK |
                  GDK_BUTTON_RELEASE_MASK |
                  GDK_POINTER_MOTION_MASK |
                  GDK_POINTER_MOTION_HINT_MASK
                );
    gtk_widget_add_events (widget, mask);
    g_signal_connect (widget, "button_release_event", G_CALLBACK (do_kanjipad_look_up), (gpointer) pad);
}


