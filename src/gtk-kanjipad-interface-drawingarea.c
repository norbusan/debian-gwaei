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

#include <gwaei/gtk-kanjipad-interface.h>
#include <gwaei/gtk-kanjipad-interface-drawingarea.h>
#include <gwaei/gtk-kanjipad-callbacks.h>



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
        
        x = CLAMP (x, 0, pad->drawing_widget->allocation.width - swidth);
        y = CLAMP (y, 0, pad->drawing_widget->allocation.height - sheight);

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
static void drawingarea_init (GwKanjipad *pad)
{
    GList *tmp_list;
    int index = 1;
    
    guint16 width = pad->drawing_widget->allocation.width;
    guint16 height = pad->drawing_widget->allocation.height;

    cairo_surface_t *cst = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cairo_t *cr = cairo_create(cst);
    /* do all your drawing here */


    cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
    cairo_rectangle (cr, 0, 0, width, height);
    cairo_fill (cr);

    double half_width = ((double) width / 2.0);
    double half_height = ((double) height / 2.0);

    cairo_set_source_rgb (cr, 0.8, 0.9, 1.0);

    cairo_set_line_width (cr, 6.0 );
    cairo_rectangle (cr, 0, 0, width, height);
    cairo_stroke (cr);

    cairo_set_line_width (cr, 3.0);
    double dashes[] = {
      10.0,  /* ink */
      10.0,  /* skip */
    };
    int    ndash  = sizeof (dashes)/sizeof(dashes[0]);
    double offset = 0.0;
    cairo_set_dash (cr, dashes, ndash, offset);

    cairo_move_to (cr, half_width, 0.0);
    cairo_line_to (cr, half_width, height);
    cairo_stroke (cr);
    cairo_move_to (cr, 0.0,  half_height);
    cairo_line_to (cr, width, half_height);
    cairo_stroke (cr);

    cairo_set_dash (cr, NULL, 0, 0);
    cairo_set_line_width (cr, 2.0);
    cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);

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

    /* End drawing here */
    cairo_destroy(cr);

    cairo_t *cr_pixmap = gdk_cairo_create(pad->pixmap);
    cairo_set_source_surface (cr_pixmap, cst, 0, 0);

    cairo_paint(cr_pixmap);
    cairo_destroy(cr_pixmap);
    cairo_surface_destroy(cst);

    gtk_widget_queue_draw (pad->drawing_widget);
}


//!
//! @brief To be written
//!
static int drawingarea_configure_event (GtkWidget *w, GdkEventConfigure *event, GwKanjipad *pad)
{
    if (pad->pixmap)
      g_object_unref (pad->pixmap);

    pad->pixmap = gdk_pixmap_new (w->window, event->width, event->height, -1);

    drawingarea_init (pad);
    
    return TRUE;
}


//!
//! @brief To be written
//!
static int drawingarea_expose_event (GtkWidget *widget, GdkEventExpose *event, GwKanjipad *pad)
{
    if (!pad->pixmap)
      return 0;

    gdk_draw_drawable (widget->window,
                       widget->style->fg_gc[GTK_STATE_NORMAL], pad->pixmap,
                       event->area.x, event->area.y,
                       event->area.x, event->area.y,
                       event->area.width, event->area.height
                      );

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
      cairo_surface_t *cst = NULL;;
      cairo_t *cr = NULL;
      guint16 width = pad->drawing_widget->allocation.width;
      guint16 height = pad->drawing_widget->allocation.height;
      cst = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);
      cr = cairo_create(cst);
      drawingarea_annotate_stroke (pad, pad->curstroke, cr, g_list_length (pad->strokes) + 1);
      cairo_destroy(cr);
      cairo_t *cr_pixmap = gdk_cairo_create (pad->pixmap);
      cairo_set_source_surface (cr_pixmap, cst, 0, 0);
      cairo_paint(cr_pixmap);
      cairo_destroy(cr_pixmap);
      cairo_surface_destroy(cst);
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
    gint x,y;
    GdkModifierType state;

    if (event->is_hint)
    {
      gdk_window_get_pointer (w->window, &x, &y, &state);
    }
    else
    {
      x = event->x;
      y = event->y;
      state = event->state;
    }

    if (pad->instroke && state & GDK_BUTTON1_MASK)
    {
      GdkRectangle rect;
      GdkPoint *p;
      int xmin, ymin, xmax, ymax;
      GdkPoint *old = (GdkPoint *)g_list_last (pad->curstroke)->data;

      /*Cairo code start*/
      guint16 width = pad->drawing_widget->allocation.width;
      guint16 height = pad->drawing_widget->allocation.height;
      cairo_surface_t *cst = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
      cairo_t *cr = cairo_create(cst);
      /* do all your drawing here */

      cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
      cairo_set_line_width (cr, 2.0 );
      cairo_move_to (cr, old->x,  old->y);
      cairo_line_to (cr, x, y);
      cairo_stroke (cr);

      /*end drawing*/
      cairo_destroy(cr);

      cairo_t *cr_pixmap = gdk_cairo_create(pad->pixmap);
      cairo_set_source_surface (cr_pixmap, cst, 0, 0);

      cairo_paint(cr_pixmap);
      cairo_destroy(cr_pixmap);
      cairo_surface_destroy(cst);
      /*Cairo code end*/

      if (old->x < x) { xmin = old->x; xmax = x; }
      else            { xmin = x;      xmax = old->x; }

      if (old->y < y) { ymin = old->y; ymax = y; }
      else            { ymin = y;      ymax = old->y; }

      rect.x = xmin - 1 - 3; 
      rect.y = ymin = 1 - 3;
      rect.width  = xmax - xmin + 2 + 3;
      rect.height = ymax - ymin + 2 + 3;
      gdk_window_invalidate_rect (w->window, &rect, FALSE);

      p = g_new (GdkPoint, 1);
      p->x = x;
      p->y = y;
      pad->curstroke = g_list_append (pad->curstroke, p);
    }

    return TRUE;
}


//!
//! @brief To be written
//!
void drawingarea_clear (GwKanjipad *pad)
{
    GList *tmp_list;

    tmp_list = pad->strokes;
    while (tmp_list)
    {
      drawingarea_free_stroke (tmp_list->data);
      tmp_list = tmp_list->next;
    }
    g_list_free (pad->strokes);
    pad->strokes = NULL;

#if 0
    tmp_list = thinned;
    while (tmp_list)
    {
      drawingarea_free_stroke (tmp_list->data);
      tmp_list = tmp_list->next;
    }
    g_list_free (thinned);
    thinned = NULL;
#endif

    g_list_free (pad->curstroke);
    pad->curstroke = NULL;

    drawingarea_init (pad);
}


//!
//! @brief To be written
//!
void drawingarea_set_annotate (GwKanjipad *pad, gboolean annotate)
{
    if (pad->annotate != annotate)
    {
      pad->annotate = annotate;
      drawingarea_init (pad);
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
    g_signal_connect (widget, "expose_event", G_CALLBACK (drawingarea_expose_event), pad);
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


