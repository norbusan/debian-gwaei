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
//! @file src/engine.c
//!
//! @brief search logic
//!
//! This file controls the backend of searches (usually initiated by the
//! do_search command.) get_results is the gatekeeper to stream_results.
//! get_results sets everything up that needs to be correct and double checked.
//! stream_results is called on a timer by gmainloop until it finished.  It then
//! cleans up after after the things set up in get_results.  If another search is
//! started before the previous is finished, get_results puts out a stop request
//! to stream_results and then waits for it to finish.
//!

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <libintl.h>

#include <glib.h>

#include <libwaei/libwaei.h>


//Function pointers that should be set on program startup
static void (*_output_append_edict_results)(LwSearchItem*) = NULL;
static void (*_output_append_kanjidict_results)(LwSearchItem*) = NULL;
static void (*_output_append_examplesdict_results)(LwSearchItem*) = NULL;
static void (*_output_append_unknowndict_results)(LwSearchItem*) = NULL;

static void (*_output_append_less_relevant_header)(LwSearchItem*) = NULL;
static void (*_output_append_more_relevant_header)(LwSearchItem*) = NULL;
static void (*_output_pre_search_prep)(LwSearchItem*) = NULL;
static void (*_output_after_search_cleanup)(LwSearchItem*) = NULL;


LwOutputFunc lw_engine_get_append_edict_results_func ()
{
  return _output_append_edict_results;
}
LwOutputFunc lw_engine_get_append_kanjidict_results_func ()
{
  return _output_append_kanjidict_results;
}
LwOutputFunc lw_engine_get_append_examplesdict_results_func ()
{
  return _output_append_examplesdict_results;
}
LwOutputFunc lw_engine_get_append_unknowndict_results_func ()
{
  return _output_append_unknowndict_results;
}
LwOutputFunc lw_engine_get_append_less_relevant_header_func ()
{
  return _output_append_less_relevant_header;
}
LwOutputFunc lw_engine_get_append_more_relevant_header_func ()
{
  return _output_append_more_relevant_header;
}
LwOutputFunc lw_engine_get_pre_search_prep_func ()
{
  return _output_pre_search_prep;
}
LwOutputFunc lw_engine_get_after_search_cleanup_func ()
{
  return _output_after_search_cleanup;
}


//!
//! @brief Gets a stored result in a search item and posts it to the output.
//!
//! THIS IS A PRIVATE FUNCTION. The memory is allocated and tthis function makes
//! sure to cleanly free it and then post it to the approprate output, be it the
//! terminal or a text buffer widget.
//!
//! @param item a LwSearchItem
//! @param results the result stored in a GList to free
//!
static void _append_stored_result_to_output (LwSearchItem *item, GList **results)
{
    //Swap the lines
    item->swap_resultline = item->backup_resultline;
    item->backup_resultline = item->resultline;
    item->resultline = item->swap_resultline;
    item->swap_resultline = NULL;

    //Replace the current result line with the stored one
    if (item->resultline != NULL)
      lw_resultline_free (item->resultline);
    item->resultline = (LwResultLine*)(*results)->data;
    *results = g_list_delete_link(*results, *results);
      
    //Append to the buffer 
    if (item->status != GW_SEARCH_CANCELING)
    {
      (*item->lw_searchitem_ui_append_results_to_output)(item);
    }
}


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
    if (lw_searchitem_run_comparison (item, GW_RELEVANCE_HIGH))
      return GW_RELEVANCE_HIGH;
    else if (lw_searchitem_run_comparison (item, GW_RELEVANCE_MEDIUM))
      return GW_RELEVANCE_MEDIUM;
    else
      return GW_RELEVANCE_LOW;
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
static void _stream_results_thread (gpointer data)
{
    LwSearchItem *item = (LwSearchItem*) data;
    if (item == NULL || item->fd == NULL) return;
    char *line_pointer = NULL;

    g_mutex_lock (item->mutex);

    //We loop, processing lines of the file until the max chunk size has been
    //reached or we reach the end of the file or a cancel request is recieved.
    while ((line_pointer = fgets(item->resultline->string, GW_IO_MAX_FGETS_LINE, item->fd)) != NULL &&
           item->status != GW_SEARCH_CANCELING)
    {
      //Give a chance for something else to run
      g_mutex_unlock (item->mutex);
      g_mutex_lock (item->mutex);

      item->current_line++;

      //Commented input in the dictionary...we should skip over it
      if(item->resultline->string[0] == '#' || g_utf8_get_char(item->resultline->string) == L'？') 
      {
        continue;
      }
      else if (item->resultline->string[0] == 'A' && item->resultline->string[1] == ':' &&
               fgets(item->scratch_buffer, GW_IO_MAX_FGETS_LINE, item->fd) != NULL             )
      {
        char *eraser = NULL;
        if ((eraser = g_utf8_strchr (item->resultline->string, -1, L'\n')) != NULL) { *eraser = '\0'; }
        if ((eraser = g_utf8_strchr (item->scratch_buffer, -1, L'\n')) != NULL) { *eraser = '\0'; }
        if ((eraser = g_utf8_strrchr (item->resultline->string, -1, L'#')) != NULL) { *eraser = '\0'; }
        strcat(item->resultline->string, ":");
        strcat(item->resultline->string, item->scratch_buffer);
      }
      (*item->lw_searchitem_parse_result_string)(item->resultline);

      //Update the progress feeback

//      if (item->target_tb == (gpointer*) get_gobject_from_target(item->target))
//        lw_ui_verb_check_with_suggestion (item);

      //Results match, add to the text buffer
      if (lw_searchitem_run_comparison (item, GW_RELEVANCE_LOW))
      {
        int relevance = _get_relevance (item);
        switch(relevance)
        {
          case GW_RELEVANCE_HIGH:
              if (item->total_relevant_results < MAX_HIGH_RELIVENT_RESULTS)
              {
                item->total_results++;
                item->total_relevant_results++;
                if (item->target != GW_TARGET_KANJI)
                  (*item->lw_searchitem_ui_append_more_relevant_header_to_output)(item);
                (*item->lw_searchitem_ui_append_results_to_output)(item);

                //Swap the result lines
                item->swap_resultline = item->backup_resultline;
                item->backup_resultline = item->resultline;
                item->resultline = item->swap_resultline;
                item->swap_resultline = NULL;
              }
              break;
          case GW_RELEVANCE_MEDIUM:
              if (item->total_irrelevant_results < MAX_MEDIUM_IRRELIVENT_RESULTS &&
                  !item->show_only_exact_matches && 
                   (item->swap_resultline = lw_resultline_new ()) != NULL && item->target != GW_TARGET_KANJI)
              {
                //Store the result line and create an empty one in its place
                item->total_irrelevant_results++;
                item->results_medium =  g_list_append(item->results_medium, item->resultline);
                item->resultline = item->swap_resultline;
                item->swap_resultline = NULL;
              }
              break;
          default:
              if (item->total_irrelevant_results < MAX_LOW_IRRELIVENT_RESULTS &&
                    !item->show_only_exact_matches && 
                   (item->swap_resultline = lw_resultline_new ()) != NULL && item->target != GW_TARGET_KANJI)
              {
                //Store the result line and create an empty one in its place
                item->total_irrelevant_results++;
                item->results_low = g_list_append(item->results_low, item->resultline);
                item->resultline = item->swap_resultline;
                item->swap_resultline = NULL;
              }
              break;
        }
      }
    }

    //Make sure the more relevant header banner is visible
    if (item->status != GW_SEARCH_CANCELING)
    {
      if (item->target != GW_TARGET_KANJI && item->total_results > 0)
        (*item->lw_searchitem_ui_append_more_relevant_header_to_output)(item);

      if (item->results_medium != NULL || item->results_low != NULL)
        (*item->lw_searchitem_ui_append_less_relevant_header_to_output)(item);
    }

    //Append the medium relevent results
    while (item->results_medium != NULL)
    {
      item->total_results++;
      _append_stored_result_to_output(item, &(item->results_medium));
      //Update the progress feeback

      //Give a chance for something else to run
      g_mutex_unlock (item->mutex);
      g_mutex_lock (item->mutex);
    }

    //Append the least relevent results
    while (item->results_low != NULL)
    {
      item->total_results++;
      _append_stored_result_to_output(item, &(item->results_low));
      //Update the progress feeback

      //Give a chance for something else to run
      g_mutex_unlock (item->mutex);
      g_mutex_lock (item->mutex);
    }

    //Cleanup
    (*item->lw_searchitem_ui_after_search_cleanup)(item);
    lw_searchitem_do_post_search_clean (item);

    g_mutex_unlock (item->mutex);
}


//!
//! @brief Start a dictionary search
//!
//! This is the entry point for starting a search.  It handles setting up the
//! query, checking things that need to be checked before the final go, and
//! initializing the search loop or thread.
//!
//! @param item a LwSearchItem argument.
//!
void lw_engine_get_results (LwSearchItem *item, gboolean create_thread, gboolean only_exact_matches)
{
    if (lw_searchitem_do_pre_search_prep (item) == FALSE)
    {
      lw_searchitem_free(item);
      return;
    }

    (*item->lw_searchitem_ui_pre_search_prep) (item);

    if (only_exact_matches) item->show_only_exact_matches = TRUE;

    //Start the search
    if (create_thread)
    {
      item->thread = g_thread_create ((GThreadFunc)_stream_results_thread, (gpointer) item, TRUE, NULL);
      if (item->thread == NULL)
      {
        g_warning("Couldn't create the thread");
        _stream_results_thread ((gpointer) item);
      }
    }
    else
    {
      _stream_results_thread ((gpointer) item);
    }
}


void lw_engine_initialize (
                                     void (*append_edict_results)(LwSearchItem*),
                                     void (*append_kanjidict_results)(LwSearchItem*),
                                     void (*append_examplesdict_results)(LwSearchItem*),
                                     void (*append_unknowndict_results)(LwSearchItem*),
                                     void (*append_less_relevant_header)(LwSearchItem*),
                                     void (*append_more_relevant_header)(LwSearchItem*),
                                     void (*pre_search_prep)(LwSearchItem*),
                                     void (*after_search_cleanup)(LwSearchItem*)
                                    )
{
    _output_append_edict_results = append_edict_results;
    _output_append_kanjidict_results = append_kanjidict_results;
    _output_append_examplesdict_results =  append_examplesdict_results;
    _output_append_unknowndict_results = append_unknowndict_results;

    _output_append_less_relevant_header = append_less_relevant_header;
    _output_append_more_relevant_header = append_more_relevant_header;
    _output_pre_search_prep = pre_search_prep;
    _output_after_search_cleanup = after_search_cleanup;
}

void lw_engine_free ()
{
    _output_append_edict_results = NULL;
    _output_append_kanjidict_results = NULL;
    _output_append_examplesdict_results = NULL;
    _output_append_unknowndict_results = NULL;

    _output_append_less_relevant_header = NULL;
    _output_append_more_relevant_header = NULL;
    _output_pre_search_prep = NULL;
    _output_after_search_cleanup = NULL;
}


