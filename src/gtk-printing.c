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

#include <gwaei/definitions.h>
#include <gwaei/utilities.h>
#include <gwaei/dictionary-objects.h>
#include <gwaei/search-objects.h>

#include <gwaei/engine.h>

#include <gwaei/gtk.h>
#include <gwaei/gtk-main-interface.h>
#include <gwaei/gtk-printing.h>



//!
//! @brief Storage of the current print settings
//!
static GtkPrintSettings *settings = NULL;

//!
//! @brief Primitive for storing information on printing
//!
typedef struct GwPageInfo {
    GList *pages;              //!< pages GList of all the created pages
    GtkTextIter page_end_line; //!< Mark in the buffer where we have paginated to
    gint total_pages;          //!< The total pages that need pagination
} GwPageInfo;


//!
//! @brief Allocates a new GwPageInfo object
//!
GwPageInfo* gw_pageinfo_new()
{
    GwPageInfo *temp;
    if ((temp = (GwPageInfo*)malloc(sizeof(GwPageInfo))) == NULL)
      return NULL;

    GwHistoryList *hl = gw_historylist_get_list (GW_HISTORYLIST_RESULTS);
    GObject *tb = get_gobject_by_target (GW_TARGET_RESULTS);

    //Start from the start of the highlighted text
    if (gtk_text_buffer_get_has_selection (GTK_TEXT_BUFFER (tb)))
    {
      GtkTextIter unused;
      gtk_text_buffer_get_selection_bounds( GTK_TEXT_BUFFER (tb),
                                            &(temp->page_end_line),
                                            &unused                                 );
    }
    //If no text is highlight, start the print job from the beginnig
    else
    {
      gtk_text_buffer_get_start_iter( GTK_TEXT_BUFFER (tb),
                                      &(temp->page_end_line)                  );
    }

    temp->pages = NULL;
    temp->total_pages = 0;

    //Finish
    return temp;
}

//!
//! @brief Frees a GwPageInfo object
//!
//! @param pi a GwPageInfo object to free
//!
void gw_pageinfo_free(GwPageInfo *pi) {
    if (pi == NULL)
      return;

    if (pi->pages != NULL) {
      GList *item = pi->pages;
      while (item != NULL) {
        free(item->data);
        item = item->next;
      }
      g_list_free(pi->pages);
      item = NULL;
    }

    //Finalize
    free(pi);
}


//!
//! @brief function for signal fired upon start of printing.
//!
//! THIS IS A PRIVATE FUNCTION.  This function currently does nothing.
//!
//! @sa done() begin_print() draw() paginate()
//!
static void begin_print() {
  printf("begin_print!\n");
}


//!
//! @brief Pagination algorithm to calculate how many pages are needed
//!
//! THIS IS A PRIVATE FUNCTION.  The function loops through and draws all the
//! pages to figure out exactly what will need to be drawn.
//!
//! @param operation Unused GtkPrintOperation
//! @param context Pointer co a GtkPrintContext to draw on
//! @param page_nr Integer of the current page number being draw
//! @param data Painter to a PageInfo struct to use for information
//! @return Return true when pagination finishes
//! @sa done() begin_print() draw() begin_print()
//!
static gboolean paginate(GtkPrintOperation *operation,
                         GtkPrintContext   *context,
                         gpointer           user_data )
{
    printf("paginate!\n");

    GwHistoryList *hl = gw_historylist_get_list (GW_HISTORYLIST_RESULTS);
    GObject *tb = get_gobject_by_target (GW_TARGET_RESULTS);

    GwPageInfo *pi  = user_data;

    //Set the page_start_iter to the page_end_iter's positon
    GtkTextIter *page_start_line = malloc(sizeof(GtkTextIter));
    gint line = gtk_text_iter_get_line( &pi->page_end_line );
    gtk_text_buffer_get_iter_at_line ( GTK_TEXT_BUFFER (tb),
                                       page_start_line,
                                       line                                   );

    //Set the page_end_line to the end of the document/selection (this gets corrected later)
    if (gtk_text_buffer_get_has_selection (GTK_TEXT_BUFFER (tb)))
    {
      GtkTextIter unused;
      gtk_text_buffer_get_selection_bounds( GTK_TEXT_BUFFER (tb),
                                            &unused,
                                            &(pi->page_end_line)                   );
    }
    else
    {
      gtk_text_buffer_get_end_iter ( GTK_TEXT_BUFFER (tb),
                                     &pi->page_end_line                         );
    }

    //We finished paginating if the page_start_line is at the end of the document
    if (line >= gtk_text_iter_get_line( &pi->page_end_line ) && pi-> total_pages != 0) {
      //Reset the iters to the beginning of the document
      gtk_text_buffer_get_start_iter ( GTK_TEXT_BUFFER (tb),
                                       &pi->page_end_line                       );
      gtk_print_operation_set_n_pages (operation, pi->total_pages);
      return TRUE;
    }

    //Prepare the cairo/pango context
    cairo_t *cr;
    PangoLayout *layout;
    gdouble width, height, text_height;
    gint layout_height;
    PangoFontDescription *desc;
    
    cr = gtk_print_context_get_cairo_context (context);
    width = gtk_print_context_get_width      (context);
    height = gtk_print_context_get_height    (context);
    
    layout = gtk_print_context_create_pango_layout (context);
    
    desc = pango_font_description_from_string ("sans 10");
    pango_layout_set_font_description (layout, desc);
    pango_font_description_free (desc);

    //Get the text from the text_buffer
    char* text;
    text = gtk_text_buffer_get_text( GTK_TEXT_BUFFER (tb),
                                     page_start_line, &pi->page_end_line,
                                     FALSE                                );
    
    //Set the text to the pango context from the string
    pango_layout_set_text (layout, text, -1);
    pango_layout_set_width (layout, width * PANGO_SCALE);
    pango_layout_set_alignment (layout, PANGO_ALIGN_LEFT);
    pango_layout_set_height(layout, height * PANGO_SCALE);
                
    pango_layout_get_size (layout, NULL, &layout_height);
    text_height = (gdouble)layout_height / PANGO_SCALE;
    
    //Set the page_end_line to to where the text actually got cutoff in the pango layout.
    //We remove two lines for the header.
    line =  line + pango_layout_get_line_count (layout) - 1; 
    gtk_text_buffer_get_iter_at_line( GTK_TEXT_BUFFER (tb),
                                      &pi->page_end_line,
                                      line                                   );

    //Finish
    pango_cairo_show_layout (cr, layout);
    
    //Cleanup
    g_free(text);
    g_object_unref (layout);

    //Increment pages
    pi->pages = g_list_append(pi->pages, page_start_line); 
    pi->total_pages++;
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
//! @sa done() begin_print() paginate() begin_print()
//!
static void draw_page (GtkPrintOperation *operation,
                       GtkPrintContext   *context,
                       gint               page_nr,
                       gpointer           data      )
{
    GwPageInfo *pi  = data;
    GwHistoryList *hl = gw_historylist_get_list (GW_HISTORYLIST_RESULTS);
    GObject *tb = get_gobject_by_target (GW_TARGET_RESULTS);

    GList *page = pi->pages;
    int i = 0;
    while (page != NULL && i != page_nr) {
      i++;
      page = page->next;
    }

    //Set the page_end_line to the end of the document/selection (this gets corrected later)
    if (gtk_text_buffer_get_has_selection (GTK_TEXT_BUFFER (tb)))
    {
      GtkTextIter unused;
      gtk_text_buffer_get_selection_bounds(GTK_TEXT_BUFFER (tb),
                                           &unused,
                                           &(pi->page_end_line) );
    }
    else
    {
      gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (tb),
                                    &pi->page_end_line   );
    }

    //Prepare the cairo/pango context
    cairo_t *cr;
    PangoLayout *layout;
    gdouble width, height, text_height;
    gint layout_height;
    PangoFontDescription *desc;
    
    cr = gtk_print_context_get_cairo_context (context);
    width = gtk_print_context_get_width      (context);
    height = gtk_print_context_get_height    (context);
    
    layout = gtk_print_context_create_pango_layout (context);
    
    desc = pango_font_description_from_string ("sans 10");
    pango_layout_set_font_description (layout, desc);
    pango_font_description_free (desc);
 
    //Get the text from the text_buffer
    char *input_text = NULL;
    input_text = gtk_text_buffer_get_text (GTK_TEXT_BUFFER (tb),
                                           page->data, &pi->page_end_line,
                                           FALSE                          );
    char *page_number = NULL;
    page_number = g_strdup_printf (gettext("Page %d"), page_nr + 1);
    char *output_text = NULL;
    output_text = malloc (strlen(input_text) + strlen(page_number) + 100);

    if (input_text == NULL || page_number == NULL || output_text == NULL)
    {
      if (input_text != NULL) g_free (input_text);
      if (page_number != NULL) g_free (page_number);
      if (output_text != NULL) free (output_text);
      return;
    }

    strcpy (output_text, "");
    strcat (output_text, page_number);
    strcat (output_text, "\n\n");
    strcat (output_text, input_text);
    
    //Add the text to the  pango context
    pango_layout_set_text (layout, output_text, -1);
    pango_layout_set_width (layout, width * PANGO_SCALE);
    pango_layout_set_alignment (layout, PANGO_ALIGN_LEFT);
    pango_layout_set_height (layout, height * PANGO_SCALE);
                
    pango_layout_get_size (layout, NULL, &layout_height);
    text_height = (gdouble)layout_height / PANGO_SCALE;
    

    //Finish
    pango_cairo_show_layout (cr, layout);
    
    //Cleanup
    if (input_text != NULL)
    {
      g_free(input_text);
      input_text = NULL;
    }
    if (output_text != NULL)
    {
      free(output_text);
      output_text = NULL;
    }
    if (page_number != NULL)
    {
      g_free (page_number);
      page_number = NULL;
    }
    g_object_unref (layout);
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
//! @sa draw_page() begin_print() pageinate() begin_print()
//!
static void done (GtkPrintOperation      *operation,
                  GtkPrintOperationResult result,
                  gpointer                data      ) 
{
    //Cleanup
    GwPageInfo *pi  = data;
    gw_pageinfo_free(pi);
    pi = NULL;
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
    GwPageInfo *pi = gw_pageinfo_new();


    //Start setting up the print operation
    GtkPrintOperation *operation;
    GtkPrintOperationResult res;
    GtkPageSetup *setup;
    
    operation = gtk_print_operation_new ();

    //Force at least some minimal margins on the pages that print
    setup = gtk_page_setup_new();
    gtk_page_setup_set_left_margin   ( setup, 20.0, GTK_UNIT_MM);
    gtk_page_setup_set_right_margin  ( setup, 20.0, GTK_UNIT_MM);
    gtk_page_setup_set_top_margin    ( setup, 20.0, GTK_UNIT_MM);
    gtk_page_setup_set_bottom_margin ( setup, 20.0, GTK_UNIT_MM);
    gtk_print_operation_set_default_page_setup ( operation, setup );

    if (settings != NULL)
      gtk_print_operation_set_print_settings(operation, settings);


    g_signal_connect (operation, "begin_print", G_CALLBACK (begin_print), pi);
    g_signal_connect (operation, "draw_page", G_CALLBACK (draw_page), pi);
    g_signal_connect (operation, "paginate", G_CALLBACK (paginate), pi);
    g_signal_connect (operation, "done", G_CALLBACK (done), pi);

    res = gtk_print_operation_run (operation, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,
                                   NULL, NULL);

    if (res == GTK_PRINT_OPERATION_RESULT_APPLY)
      {
        if (settings != NULL)
        g_object_unref (settings);
        settings = g_object_ref(gtk_print_operation_get_print_settings (operation));
      }
    g_object_unref (operation);
}

