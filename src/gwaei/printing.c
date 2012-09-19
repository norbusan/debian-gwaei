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
//! @file printing.c
//!
//! @brief To be written
//!


#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <gtk/gtk.h>

#include <gwaei/gwaei.h>
#include <gwaei/gettext.h>



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

#define GW_PAGEINFO(object) (GwPageInfo*)object

//!
//! @brief Allocates a new GwPageInfo object
//!
GwPageInfo* gw_pageinfo_new (GtkTextIter start, GtkTextIter end)
{
    GwPageInfo *temp;

    temp = (GwPageInfo*) malloc (sizeof(GwPageInfo));

    if (temp != NULL)
    {
      temp->start = start;
      temp->end = end;
    }

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

struct _GwPrintData {
  GList *pages;
  GwSearchWindow *window;
};
typedef struct _GwPrintData GwPrintData;


GwPrintData* gw_printdata_new (GwSearchWindow *window)
{
    GwPrintData *temp;

    temp = (GwPrintData*) malloc(sizeof(GwPrintData));

    if (temp != NULL)
    {
      temp->pages = NULL;
      temp->window = window;
    }
    
    return temp;
}

void gw_printdata_free (GwPrintData *data)
{
    //Declarations
    GList *iter;
    GwPageInfo *pi;

    for (iter = data->pages; iter != NULL; iter = iter->next)
    {
       pi = GW_PAGEINFO (iter->data);
       if (pi != NULL)
         gw_pageinfo_free (pi);

    }
    g_list_free (data->pages);
    
    free (data);
}


//!
//! @brief function for signal fired upon start of printing.
//! @sa _done() _begin_print() draw() _paginate()
//!
static void _begin_print (GtkPrintOperation *operation,
                          GtkPrintContext   *context,
                          GwPrintData       *data      ) {
  printf("begin_print!\n");
}


static void _draw_page_title (GtkPrintContext *context, GwPageInfo *page, GwPrintData *data) {
    //Declarations
    PangoLayout *layout;
    char *text;
    PangoFontDescription *desc;
    int width;
    int height;
    gint index;
    cairo_t *cr;
    LwSearch *search;

    //Initializations
    index = gw_searchwindow_get_current_tab_index (data->window);
    search = gw_searchwindow_get_searchitem_by_index (data->window, index);
    text = gw_searchwindow_get_title_by_searchitem (data->window, search);
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


static void _draw_page_number (GtkPrintContext *context, gint page_nr, GwPageInfo *page, GwPrintData *data)
{
    //Declarations
    PangoLayout *layout;
    char *text;
    PangoFontDescription *desc;
    int width;
    int height;
    cairo_t *cr;
    gdouble drawable_width;

    //Initializations
    text = g_strdup_printf (gettext("Page %d/%d"), page_nr + 1, g_list_length (data->pages));
    layout = gtk_print_context_create_pango_layout (context);
    desc = pango_font_description_from_string ("sans 8");
    cr = gtk_print_context_get_cairo_context (context);
    drawable_width = gtk_print_context_get_width (context);

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

 
static void _draw_page_results (GtkPrintContext *context, GwPageInfo *page, GwPrintData *data)
{
    //Declarations
    GtkTextView *view;
    GtkTextBuffer *buffer;
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
    view = gw_searchwindow_get_current_textview (data->window);
    buffer = gtk_text_view_get_buffer (view);
    text = gtk_text_buffer_get_text (buffer, &(page->start), &(page->end), FALSE);
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
      gtk_text_buffer_get_iter_at_line (buffer, &(page->end), line_end);
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
                           GwPrintData       *data      )
{
    printf("paginate!\n");

    //Declarations
    GwPageInfo *page;
    GwPageInfo *prev_page;
    GList *iter;
    GtkTextIter end_bound;
    GtkTextIter start_bound;
    GtkTextView *view;
    GtkTextBuffer *buffer;

    //Initializations
    view = gw_searchwindow_get_current_textview (data->window);
    buffer = gtk_text_view_get_buffer (view);

    //Get the draw bounds
    if (gtk_text_buffer_get_has_selection (buffer))
    {
      gtk_text_buffer_get_selection_bounds (buffer, &start_bound, &end_bound);
    }
    else
    {
      gtk_text_buffer_get_start_iter (buffer, &start_bound);
      gtk_text_buffer_get_end_iter (buffer, &end_bound);
    }

    //Create the first page
    if (data->pages == NULL)
    {
      page = gw_pageinfo_new (start_bound, end_bound);
      _draw_page_results (context, page, data);
      data->pages = g_list_append (data->pages, page);
      return FALSE;
    }

    iter = g_list_last (data->pages);
    prev_page = GW_PAGEINFO (iter->data);

    //Finish paginating
    if (gtk_text_iter_get_line (&(prev_page->end)) == gtk_text_iter_get_line (&end_bound))
    {
      gtk_print_operation_set_n_pages (operation, g_list_length (data->pages));
      return TRUE;
    }

    //Add another page
    page = gw_pageinfo_new (prev_page->end, end_bound);
    _draw_page_results (context, page, data);
    data->pages = g_list_append (data->pages, page);

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
                        GwPrintData       *data     )
{
    printf("draw!\n");

    //Declarations
    GwPageInfo *page;

    //Initializations
    page = GW_PAGEINFO (g_list_nth_data (data->pages, page_nr));

    if (page != NULL)
    {
      _draw_page_title (context, page, data);
      _draw_page_number (context, page_nr, page, data);
      _draw_page_results (context, page, data);
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
static void _done (GtkPrintOperation       *operation,
                   GtkPrintOperationResult  result,
                   GwPrintData             *data      ) 
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
void gw_print (const GtkPrintOperationAction ACTION, GwSearchWindow *window)
{
    //Declarations
    GwPrintData *data;
    GtkPrintOperation *operation;
    GtkPrintOperationResult res;
    
    //Initializations
    data = gw_printdata_new (window);
    operation = gtk_print_operation_new ();

    //Force at least some minimal margins on the pages that print
    gtk_print_operation_set_default_page_setup (operation, NULL);
    gtk_print_operation_set_use_full_page (operation, FALSE);
    gtk_print_operation_set_unit (operation, GTK_UNIT_MM);

    if (_settings != NULL)
      gtk_print_operation_set_print_settings (operation, _settings);

    g_signal_connect (operation, "begin_print", G_CALLBACK (_begin_print), data);
    g_signal_connect (operation, "draw_page", G_CALLBACK (_draw_page), data);
    g_signal_connect (operation, "paginate", G_CALLBACK (_paginate), data);
    g_signal_connect (operation, "done", G_CALLBACK (_done), data);

    res = gtk_print_operation_run (operation, ACTION, NULL, NULL);

    if (res == GTK_PRINT_OPERATION_RESULT_APPLY)
    {
        if (_settings != NULL) g_object_unref (_settings);
        _settings = g_object_ref (gtk_print_operation_get_print_settings (operation));
    }

    //Cleanup
    gw_printdata_free (data);
    g_object_unref (operation);
}
//!
//! @brief Sets up a print operation for the current results
//!
//! The function checks the results of the results text buffer, and then attempts
//! to set up a print operation.  If a section of the search results are highlighted
//! only those results are printed.
//!
G_MODULE_EXPORT void gw_print_cb (GSimpleAction *action,
                                  GVariant      *parameter,
                                  gpointer       data)
{
    GwSearchWindow *window;
    GwApplication *application;

    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    if (window == NULL) return;
    application = gw_window_get_application (GW_WINDOW (window));

    gw_application_block_searches (application);
    gw_print (GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG, window);
    gw_application_unblock_searches (application);
}


//!
//! @brief Sets up a print preview for the results
//!
G_MODULE_EXPORT void gw_print_preview_cb (GSimpleAction *action,
                                          GVariant      *parameter,
                                          gpointer       data)
{
    GwSearchWindow *window;
    GwApplication *application;

    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    if (window == NULL) return;
    application = gw_window_get_application (GW_WINDOW (window));

    gw_application_block_searches (application);
    gw_print (GTK_PRINT_OPERATION_ACTION_PREVIEW, window);
    gw_application_unblock_searches (application);
}

