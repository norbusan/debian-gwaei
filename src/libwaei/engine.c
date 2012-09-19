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
//! @file engine.c
//!


#include "../private.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <glib.h>

#include <libwaei/libwaei.h>
#include <libwaei/engine-data.h>


//!
//! @brief Find the relevance of a returned result
//!
//! THIS IS A PRIVATE FUNCTION. Function uses the stored relevance regrex
//! expressions in the LwSearchItem to get the relevance of a returned result.  It
//! then returns the answer to the caller in the form of an int.
//!
//! @param text a string to check the relevance of
//! @param item a search item to grab the regrexes from
//! @return Returns one of the integers: LOW_RELEVANCE, MEDIUM_RELEVANCE, or HIGH_RELEVANCE.
//!
static int _get_relevance (LwSearchItem *item) {
    if (lw_searchitem_run_comparison (item, LW_RELEVANCE_HIGH))
      return LW_RELEVANCE_HIGH;
    else if (lw_searchitem_run_comparison (item, LW_RELEVANCE_MEDIUM))
      return LW_RELEVANCE_MEDIUM;
    else
      return LW_RELEVANCE_LOW;
}


//!
//! @brief Preforms the brute work of the search
//!
//! THIS IS A PRIVATE FUNCTION. This function returns true until it finishes
//! searching the whole file.  It works in specified chunks before going back to
//! the thread to help improve speed.  
//!
//! @param data A LwSearchItem to search with
//! @return Returns true when the search isn't finished yet.
//!
static gpointer _stream_results_thread (gpointer data)
{
    //Declarations
    LwEngineData *enginedata;
    LwSearchItem *item;
    gboolean show_only_exact_matches;

    //Initializations
    enginedata = LW_ENGINEDATA (data);
    item = LW_SEARCHITEM (enginedata->item);
    show_only_exact_matches = enginedata->exact;

    if (item == NULL || item->fd == NULL) return NULL;
    char *line_pointer = NULL;

    lw_searchitem_lock (item);
    item->status = LW_SEARCHSTATUS_SEARCHING;

    //We loop, processing lines of the file until the max chunk size has been
    //reached or we reach the end of the file or a cancel request is recieved.
    while ((line_pointer = fgets(item->resultline->string, LW_IO_MAX_FGETS_LINE, item->fd)) != NULL &&
           item->status != LW_SEARCHSTATUS_FINISHING)
    {

      //Give a chance for something else to run
      lw_searchitem_unlock (item);
/*
      //THIS CODE CAUSES A DEADLOCK ON GTK+3.3.X
      if (item->status != LW_SEARCHSTATUS_FINISHING && g_main_context_pending (NULL))
      {
        g_main_context_iteration (NULL, FALSE);
      }
*/
      lw_searchitem_lock (item);

      item->current += strlen(item->resultline->string);

      //Commented input in the dictionary...we should skip over it
      if(item->resultline->string[0] == '#' || g_utf8_get_char(item->resultline->string) == L'？') 
      {
        continue;
      }
      else if (item->resultline->string[0] == 'A' && item->resultline->string[1] == ':' &&
               fgets(item->scratch_buffer, LW_IO_MAX_FGETS_LINE, item->fd) != NULL             )
      {
        item->current += strlen(item->scratch_buffer);
        char *eraser = NULL;
        if ((eraser = g_utf8_strchr (item->resultline->string, -1, L'\n')) != NULL) { *eraser = '\0'; }
        if ((eraser = g_utf8_strchr (item->scratch_buffer, -1, L'\n')) != NULL) { *eraser = '\0'; }
        if ((eraser = g_utf8_strrchr (item->resultline->string, -1, L'#')) != NULL) { *eraser = '\0'; }
        strcat(item->resultline->string, ":");
        strcat(item->resultline->string, item->scratch_buffer);
      }
      lw_searchitem_parse_result_string (item);


      //Results match, add to the text buffer
      if (lw_searchitem_run_comparison (item, LW_RELEVANCE_LOW))
      {
        int relevance = _get_relevance (item);
        switch(relevance)
        {
          case LW_RELEVANCE_HIGH:
              if (item->total_relevant_results < LW_MAX_HIGH_RELEVENT_RESULTS)
              {
                item->total_results++;
                item->total_relevant_results++;
                item->resultline->relevance = LW_RESULTLINE_RELEVANCE_HIGH;
                item->results_high =  g_list_append (item->results_high, item->resultline);
                item->resultline = lw_resultline_new ();
              }
              break;
          if (!show_only_exact_matches)
          {
          case LW_RELEVANCE_MEDIUM:
              if (item->total_irrelevant_results < LW_MAX_MEDIUM_IRRELEVENT_RESULTS)
              {
                item->total_results++;
                item->total_irrelevant_results++;
                item->resultline->relevance = LW_RESULTLINE_RELEVANCE_MEDIUM;
                item->results_medium =  g_list_append (item->results_medium, item->resultline);
                item->resultline = lw_resultline_new ();
              }
              break;
          default:
              if (item->total_irrelevant_results < LW_MAX_LOW_IRRELEVENT_RESULTS)
              {
                item->total_results++;
                item->total_irrelevant_results++;
                item->resultline->relevance = LW_RESULTLINE_RELEVANCE_LOW;
                item->results_low = g_list_append (item->results_low, item->resultline);
                item->resultline = lw_resultline_new ();
              }
              break;
          }
        }
      }
    }

    lw_searchitem_cleanup_search (item);
    lw_enginedata_free (enginedata);

    lw_searchitem_unlock (item);

    return NULL;
}


//!
//! @brief Start a dictionary search
//! @param item a LwSearchItem argument to calculate results
//! @param create_thread Whether the search should run in a new thread.
//! @param exact Whether to show only exact matches for this search
//!
void lw_searchitem_start_search (LwSearchItem *item, gboolean create_thread, gboolean exact)
{
    GError *error;
    gpointer data;

    error = NULL;
    data = lw_enginedata_new (item, exact);

    if (data != NULL)
    {
      lw_searchitem_prepare_search (item);
      if (create_thread)
      {
        item->thread = g_thread_try_new (
          "libwaei-search",
          (GThreadFunc) _stream_results_thread, 
          (gpointer) data, 
          &error
        );
        if (item->thread == NULL)
        {
          g_warning ("Thread Creation Error: %s\n", error->message);
          g_error_free (error);
          error = NULL;
        }
      }
      else
      {
        item->thread = NULL;
        _stream_results_thread ((gpointer) data);
      }
    }
}


//!
//! @brief Uses a searchitem to cancel a window
//!
//! @param item A LwSearchItem to gleam information from
//!
void lw_searchitem_cancel_search (LwSearchItem *item)
{
    if (item == NULL) return;

    lw_searchitem_set_status (item, LW_SEARCHSTATUS_FINISHING);

    if (item->thread != NULL)
    {
      g_thread_join (item->thread);
      item->thread = NULL;
    }

    lw_searchitem_set_status (item, LW_SEARCHSTATUS_IDLE);
}


//!
//! @brief Gets a result and removes a LwResultLine from the beginnig of a list of results
//! @returns a LwResultLine that should be freed with lw_resultline_free
//!
LwResultLine* lw_searchitem_get_result (LwSearchItem *item)
{
    g_assert (item != NULL);

    LwResultLine *line;

    lw_searchitem_lock (item);

    if (item->results_high != NULL)
    {
      line = LW_RESULTLINE (item->results_high->data);
      item->results_high = g_list_delete_link (item->results_high, item->results_high);
    }
    else if (item->results_medium != NULL && item->status == LW_SEARCHSTATUS_FINISHING)
    {
      line = LW_RESULTLINE (item->results_medium->data);
      item->results_medium = g_list_delete_link (item->results_medium, item->results_medium);
    }
    else if (item->results_low != NULL && item->status == LW_SEARCHSTATUS_FINISHING)
    {
      line = LW_RESULTLINE (item->results_low->data);
      item->results_low = g_list_delete_link (item->results_low, item->results_low);
    }
    else
    {
      line = NULL;
    }

    lw_searchitem_unlock (item);

    return line;
}



//!
//! @brief Tells if you should keep checking for results
//!
gboolean 
lw_searchitem_should_check_results (LwSearchItem *item)
{
    if (item == NULL) return FALSE;

    gboolean should_check_results;
    gboolean results_in_queue;
    LwSearchStatus status;

    status = lw_searchitem_get_status (item);

    if (status == LW_SEARCHSTATUS_FINISHING)
    {
      if (item->thread != NULL)
      {
        g_thread_join (item->thread);
        item->thread = NULL;
      }
    }

    lw_searchitem_lock (item);
    results_in_queue = (item->results_high != NULL ||
                        item->results_medium != NULL ||
                        item->results_low != NULL);
    lw_searchitem_unlock (item);

    if (status == LW_SEARCHSTATUS_FINISHING && 
                       (item->results_high == NULL &&
                        item->results_medium == NULL &&
                        item->results_low == NULL))
      lw_searchitem_set_status (item, LW_SEARCHSTATUS_IDLE);

    should_check_results = (results_in_queue || status != LW_SEARCHSTATUS_IDLE);

    return should_check_results;
}


