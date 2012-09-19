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
//! @file src/gtk-printing.c
//!
//! @brief Abstraction layer for gtk printing
//!
//! This is where the functions needed for printing are kept. This is the gtk
//! version.
//!


#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include <libintl.h>
#include <regex.h>

#include <gtk/gtk.h>

#include <gwaei/gwaei.h>



//!
//! @brief Storage of the current print settings
//!
static GtkPrintSettings *_settings = NULL;

//!
//! @brief Primitive for storing information on printing
//!
typedef struct GwPageInfo {
    GtkTextIter start; //!< pages GList of all the created pages
    GtkTextIter end;   //!< Mark in the buffer where we have paginated to
} GwPageInfo;


//!
//! @brief Allocates a new GwPageInfo object
//!
GwPageInfo* gw_pageinfo_new (GtkTextIter start, GtkTextIter end)
{
    GwPageInfo *temp;

    if ((temp = (GwPageInfo*) malloc (sizeof(GwPageInfo))) == NULL)
      return NULL;

    temp->start = start;
    temp->end = end;

    //Finish
    return temp;
}


//!
//! @brief Frees a GwPageInfo object
//! @param pi a GwPageInfo object to free
//!
void gw_pageinfo_free (GwPageInfo *pi) {
    if (pi == NULL) return;
    free(pi);
}


//!
//! @brief function for signal fired upon start of printing.
//! @sa _done() _begin_print() draw() _paginate()
//!
static void _begin_print (GtkPrintOperation *operation,
                          GtkPrintContext   *context,
                          GList             **pages     ) {
  printf("begin_print!\n");
}


static void _draw_page_title (GtkPrintContext *context, GwPageInfo *pi) {
    //Declarations
    PangoLayout *layout;
    char *text;
    PangoFontDescription *desc;
    int width;
    int height;
    cairo_t *cr;
    LwSearchItem *item;

    //Initializations
    item = lw_historylist_get_current (GW_HISTORYLIST_RESULTS);
    text = gw_main_get_window_title_by_searchitem (item);
    layout = gtk_print_context_create_pango_layout (context);
    desc = pango_font_description_from_string ("sans 8");
    cr = gtk_print_context_get_cairo_context (context);

    //Draw
    if (text != NULL && layout != NULL && desc != NULL)
    {
      cairo_move_to (cr, 0, 0);
      pango_font_description_set_weight (desc, PANGO_WEIGHT_BOLD);
      pango_layout_set_font_description (layout, desc);
      pango_layout_set_markup (layout, text, -1);
      pango_layout_get_size (layout, &width, &height);
      pango_cairo_show_layout (cr, layout);
    }

    //Cleanup
    if (text != NULL) g_free (text);
    if (layout != NULL) g_object_unref (layout);
    if (desc != NULL) pango_font_description_free (desc);
}


static void _draw_page_number (GtkPrintContext *context, gint page_nr, gint page_tl, GwPageInfo *pi)
{
    //Declarations
    PangoLayout *layout;
    char *text;
    PangoFontDescription *desc;
    int width;
    int height;
    cairo_t *cr;
    gdouble drawable_width;
    gdouble drawable_height;

    //Initializations
    text = g_strdup_printf (gettext("Page %d/%d"), page_nr + 1, page_tl);
    layout = gtk_print_context_create_pango_layout (context);
    desc = pango_font_description_from_string ("sans 8");
    cr = gtk_print_context_get_cairo_context (context);
    drawable_width = gtk_print_context_get_width (context);
    drawable_height = gtk_print_context_get_height (context);

    //Draw
    if (text != NULL && layout != NULL && desc != NULL)
    {
      pango_font_description_set_weight (desc, PANGO_WEIGHT_BOLD);
      pango_layout_set_alignment (layout, PANGO_ALIGN_RIGHT);
      pango_layout_set_font_description (layout, desc);
      pango_layout_set_markup (layout, text, -1);
      pango_layout_get_size (layout, &width, &height);
      cairo_move_to (cr, (int) drawable_width - PANGO_PIXELS (width), 0);
      pango_cairo_show_layout (cr, layout);
    }

    //Cleanup
    if (text != NULL) g_free (text);
    if (layout != NULL) g_object_unref (layout);
    if (desc != NULL) pango_font_description_free (desc);
}

 
static void _draw_page_results (GtkPrintContext *context, GwPageInfo *page)
{
    //Declarations
    GObject *tb;
    PangoLayout *layout;
    char *text;
    PangoFontDescription *desc;
    int width;
    int height;
    gdouble drawable_width, drawable_height;
    cairo_t *cr;
    gint line_start;
    gint line_end;

    //Initializations
    tb = gw_common_get_gobject_by_target (GW_TARGET_RESULTS);
    text = gtk_text_buffer_get_text (GTK_TEXT_BUFFER (tb), &(page->start), &(page->end), FALSE);
    layout = gtk_print_context_create_pango_layout (context);
    desc = pango_font_description_from_string ("sans 10");
    drawable_width = gtk_print_context_get_width (context);
    drawable_height = gtk_print_context_get_height (context);
    cr = gtk_print_context_get_cairo_context (context);
    line_start = 0;
    line_end = 0;

    //Draw
    if (text != NULL)
    {
      cairo_move_to (cr, 5, 10);
      pango_layout_set_font_description (layout, desc);
      pango_layout_set_markup (layout, text, -1);
      pango_layout_set_width (layout, drawable_width * PANGO_SCALE);
      pango_layout_set_alignment (layout, PANGO_ALIGN_LEFT);
      pango_layout_set_height (layout, drawable_height * PANGO_SCALE);
      pango_layout_get_size (layout, &width, &height);
      pango_cairo_show_layout (cr, layout);
      
      //Adjust the end GtkTextIter to the cutoff in the visible cairo context
      line_start = gtk_text_iter_get_line (&page->start);
      line_end = line_start + pango_layout_get_line_count (layout) - 1;
      gtk_text_buffer_get_iter_at_line (GTK_TEXT_BUFFER (tb), &(page->end), line_end);
    }

    //Cleanup
    if (text != NULL) g_free (text);
    if (layout != NULL) g_object_unref (layout);
    if (desc != NULL) pango_font_description_free (desc);
}


//!
//! @brief Pagination algorithm to calculate how many pages are needed
//! @param operation Unused GtkPrintOperation
//! @param context Pointer co a GtkPrintContext to draw on
//! @param page_nr Integer of the current page number being draw
//! @param data Painter to a PageInfo struct to use for information
//! @return Return true when pagination finishes
//! @sa _done() _begin_print() draw() _begin_print()
//!
static gboolean _paginate (GtkPrintOperation *operation,
                           GtkPrintContext   *context,
                           GList             **pages     )
{
    printf("paginate!\n");

    //Declarations
    GwPageInfo *page;
    GwPageInfo *prev_page;
    GList *iter;
    GtkTextIter end_bound;
    GtkTextIter start_bound;
    GObject *tb;

    //Initializations
    tb = gw_common_get_gobject_by_target (GW_TARGET_RESULTS);

    //Get the draw bounds
    if (gtk_text_buffer_get_has_selection (GTK_TEXT_BUFFER (tb)))
    {
      gtk_text_buffer_get_selection_bounds (GTK_TEXT_BUFFER (tb), &start_bound, &end_bound);
    }
    else
    {
      gtk_text_buffer_get_start_iter (GTK_TEXT_BUFFER (tb), &start_bound);
      gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (tb), &end_bound);
    }

    //Create the first page
    if (*pages == NULL)
    {
      page = gw_pageinfo_new (start_bound, end_bound);
      _draw_page_results (context, page);
      *pages = g_list_append (*pages, page);
      return FALSE;
    }

    iter = g_list_last (*pages);
    prev_page = iter->data;

    //Finish paginating
    if (gtk_text_iter_get_line (&(prev_page->end)) == gtk_text_iter_get_line (&end_bound))
    {
      gtk_print_operation_set_n_pages (operation, g_list_length (*pages));
      return TRUE;
    }

    //Add another page
    page = gw_pageinfo_new (prev_page->end, end_bound);
    _draw_page_results (context, page);
    *pages = g_list_append (*pages, page);

    return FALSE;
}


//!
//! @brief Draws a page for pagination
//!
//! THIS IS A PRIVATE FUNCTION.  This draws a page for pagination to be printed
//! using the current position.
//!
//! @param operation Unused GtkPrintOperation
//! @param context Pointer co a GtkPrintContext to draw on
//! @param page_nr Integer of the current page number being draw
//! @param data Painter to a PageInfo struct to use for information
//! @sa _done() _begin_print() _paginate() _begin_print()
//!
static void _draw_page (GtkPrintOperation *operation,
                        GtkPrintContext   *context,
                        gint               page_nr,
                        GList             **pages     )
{
    printf("draw!\n");

    //Declarations
    GwPageInfo *page;
    GList *iter;

    //Initializations
    iter = g_list_nth (*pages, page_nr);
    if (iter != NULL)
    {
      page = iter->data;

      _draw_page_title (context, page);
      _draw_page_number (context, page_nr, g_list_length (*pages), page);
      _draw_page_results (context, page);
    }
}


//!
//! @brief Called when pagination finishes.
//!
//! THIS IS A PRIVATE FUNCTION.  The function checks the results of the results
//! text buffer, and then attempts to set up a print operation.  If a section
//! of the search results are highlighted only those results are printed.
//!
//! @param operation Unused
//! @param result Unused
//! @param data Pointer to a PageInfo struct to free when pagination finishes
//! @sa _draw_page() _begin_print() pageinate() _begin_print()
//!
static void _done (GtkPrintOperation      *operation,
                   GtkPrintOperationResult result,
                   GList                  **pages    ) 
{
    printf("done\n");
}


//!
//! @brief Sets up a print operation for the current results
//!
//! The function checks the results of the results text buffer, and then attempts
//! to set up a print operation.  If a section of the search results are highlighted
//! only those results are printed.
//!
void gw_print()
{
    //Declarations
    GList *pages;
    GwPageInfo *page;
    GList *iter;
    GtkPrintOperation *operation;
    GtkPrintOperationResult res;
    
    //Initializations
    pages = NULL;
    operation = gtk_print_operation_new ();

    //Force at least some minimal margins on the pages that print
    gtk_print_operation_set_default_page_setup (operation, NULL);
    gtk_print_operation_set_use_full_page (operation, FALSE);
    gtk_print_operation_set_unit (operation, GTK_UNIT_MM);

    if (_settings != NULL)
      gtk_print_operation_set_print_settings (operation, _settings);

    g_signal_connect (operation, "begin_print", G_CALLBACK (_begin_print), &pages);
    g_signal_connect (operation, "draw_page", G_CALLBACK (_draw_page), &pages);
    g_signal_connect (operation, "paginate", G_CALLBACK (_paginate), &pages);
    g_signal_connect (operation, "done", G_CALLBACK (_done), &pages);

    res = gtk_print_operation_run (operation, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG, NULL, NULL);

    if (res == GTK_PRINT_OPERATION_RESULT_APPLY)
    {
        if (_settings != NULL) g_object_unref (_settings);
        _settings = g_object_ref (gtk_print_operation_get_print_settings (operation));
    }

    //Cleanup
    for (iter = pages; iter != NULL; iter = iter->next)
    {
      page = iter->data;
      gw_pageinfo_free (page);
    }
    g_list_free (pages);
    g_object_unref (operation);
}

