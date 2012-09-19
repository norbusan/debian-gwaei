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
#include <regex.h>
#include <locale.h>
#include <libintl.h>

#include <glib.h>

#include <gwaei/definitions.h>
#include <gwaei/regex.h>
#include <gwaei/dictionary-objects.h>
#include <gwaei/search-objects.h>
#include <gwaei/engine.h>
#include <gwaei/utilities.h>

static gboolean less_relevant_title_inserted = FALSE;


//!
//! @brief Gets a stored result in a search item and posts it to the output.
//!
//! THIS IS A PRIVATE FUNCTION. The memory is allocated and tthis function makes
//! sure to cleanly free it and then post it to the approprate output, be it the
//! terminal or a text buffer widget.
//!
//! @param item a GwSearchItem
//! @param results the result stored in a GList to free
//!
static void append_stored_result_to_output (GwSearchItem *item, GList **results)
{
    //Swap the lines
    item->swap_resultline = item->backup_resultline;
    item->backup_resultline = item->resultline;
    item->resultline = item->swap_resultline;
    item->swap_resultline = NULL;

    //Replace the current result line with the stored one
    if (item->resultline != NULL)
      gw_resultline_free (item->resultline);
    item->resultline = (GwResultLine*)(*results)->data;
    *results = g_list_delete_link(*results, *results);
      
    //Append to the buffer 
    if (item->show_less_relevant_results || item->total_relevant_results == 0)
    {
      (*item->gw_searchitem_ui_append_results_to_output)(item);
    }
}


//!
//! @brief Find the relevance of a returned result
//!
//! THIS IS A PRIVATE FUNCTION. Function uses the stored relevance regrex
//! expressions in the GwSearchItem to get the relevance of a returned result.  It
//! then returns the answer to the caller in the form of an int.
//!
//! @param text a string to check the relevance of
//! @param item a search item to grab the regrexes from
//! @return Returns one of the integers: LOW_RELEVANCE, MEDIUM_RELEVANCE, or HIGH_RELEVANCE.
//!
static int get_relevance (GwSearchItem *item) {
    if (gw_searchitem_existance_generic_comparison (item, GW_QUERYLINE_HIGH))
      return GW_RESULT_HIGH_RELEVANCE;
    else if (gw_searchitem_existance_generic_comparison (item, GW_QUERYLINE_MED))
      return GW_RESULT_MEDIUM_RELEVANCE;
    else
      return GW_RESULT_LOW_RELEVANCE;
}


//!
//! @brief Preforms the brute work of the search
//!
//! THIS IS A PRIVATE FUNCTION. This function returns true until it finishes
//! searching the whole file.  It works in specified chunks before going back to
//! the thread to help improve speed.  
//!
//! @param data A GwSearchItem to search with
//! @return Returns true when the search isn't finished yet.
//!
static gboolean stream_results_thread (GwSearchItem *item)
{
    int chunk = 0;
    if (item->fd == NULL) return FALSE;

    //We loop, processing lines of the file until the max chunk size has been
    //reached or we reach the end of the file or a cancel request is recieved.
    while (chunk < MAX_CHUNK               &&
           item->status != GW_SEARCH_GW_DICT_STATUS_CANCELING &&
           fgets(item->resultline->string, MAX_LINE, item->fd) != NULL)
    {
      chunk++;
      item->current_line++;

      //Commented input in the dictionary...we should skip over it
      if(item->resultline->string[0] == '#' || g_utf8_get_char(item->resultline->string) == L'？') 
        continue;

      else if (item->resultline->string[0] == 'A' && item->resultline->string[1] == ':' &&
               fgets(item->scratch_buffer, MAX_LINE, item->fd) != NULL             )
      {
        char *eraser = NULL;
        if ((eraser = g_utf8_strchr (item->resultline->string, -1, L'\n')) != NULL) { *eraser = '\0'; }
        if ((eraser = g_utf8_strchr (item->scratch_buffer, -1, L'\n')) != NULL) { *eraser = '\0'; }
        if ((eraser = g_utf8_strrchr (item->resultline->string, -1, L'#')) != NULL) { *eraser = '\0'; }
        strcat(item->resultline->string, ":");
        strcat(item->resultline->string, item->scratch_buffer);
      }
      (*item->gw_searchitem_parse_result_string)(item->resultline);

      //Update the progress feeback
      (*item->gw_searchitem_ui_update_progress_feedback)(item);
/*
      if (item->target_tb == (gpointer*) get_gobject_from_target(item->target))
        gw_ui_verb_check_with_suggestion (item);
*/
      //Results match, add to the text buffer
      if (gw_searchitem_existance_generic_comparison (item, GW_QUERYLINE_EXIST))
      {
        int relevance = get_relevance(item);
        switch(relevance)
        {
          case GW_RESULT_HIGH_RELEVANCE:
              item->total_results++;
              item->total_relevant_results++;
              if (item->target != GW_TARGET_KANJI)
                (*item->gw_searchitem_ui_append_more_relevant_header_to_output)(item);
              (*item->gw_searchitem_ui_append_results_to_output)(item);

              //Swap the result lines
              item->swap_resultline = item->backup_resultline;
              item->backup_resultline = item->resultline;
              item->resultline = item->swap_resultline;
              item->swap_resultline = NULL;
              break;
          case GW_RESULT_MEDIUM_RELEVANCE:
              if ((item->dictionary->type == GW_DICT_TYPE_KANJI || item->total_irrelevant_results < MAX_MEDIUM_IRRELIVENT_RESULTS) &&
                   (item->swap_resultline = gw_resultline_new ()) != NULL && item->target != GW_TARGET_KANJI)
              {
                //Store the result line and create an empty one in its place
                item->total_irrelevant_results++;
                item->results_medium =  g_list_append(item->results_medium, item->resultline);
                item->resultline = item->swap_resultline;
                item->swap_resultline = NULL;
              }
              break;
          default:
              if ((item->dictionary->type == GW_DICT_TYPE_KANJI || item->total_irrelevant_results < MAX_LOW_IRRELIVENT_RESULTS) &&
                   (item->swap_resultline = gw_resultline_new ()) != NULL && item->target != GW_TARGET_KANJI)
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
      continue;
    }


    //If the chunk reached the max chunk size, there is still file left to load
    if ( chunk == MAX_CHUNK ) {
      return TRUE;
    }

    //Make sure the more relevant header banner is visible
    if (item->target != GW_TARGET_KANJI)
      (*item->gw_searchitem_ui_append_more_relevant_header_to_output)(item);

    //Insert the less relevant title header if needed
    if ( item->show_less_relevant_results    &&
         !less_relevant_title_inserted &&
         (item->results_medium != NULL || item->results_low != NULL) )
    {
      (*item->gw_searchitem_ui_append_less_relevant_header_to_output)(item);
      less_relevant_title_inserted = TRUE;
    }

    //Append the medium relevent results
    if (item->results_medium != NULL)
    {
      for (chunk = 0; item->results_medium != NULL && chunk < MAX_CHUNK; chunk++)
      {
        item->total_results++;
        append_stored_result_to_output(item, &(item->results_medium));
      }
      //Update the progress feeback
      (*item->gw_searchitem_ui_update_progress_feedback)(item);
      return TRUE;
    }

    //Append the least relevent results
    if (item->results_low != NULL)
    {
      for (chunk = 0; item->results_low != NULL && chunk < MAX_CHUNK; chunk++)
      {
        item->total_results++;
        append_stored_result_to_output(item, &(item->results_low));
      }
      //Update the progress feeback
      (*item->gw_searchitem_ui_update_progress_feedback)(item);
      return TRUE;
    }

    return FALSE;
}


//!
//! @brief Preforms necessary cleanups after the search thread finishes
//!
//! THIS IS A PRIVATE FUNCTION. The calls to this function are made by
//! gw_search_get_results.  Do not call this function directly.
//!
//! @see gw_search_get_results()
//! @param data A GwSearchItem to clean up the data of
//! @return currently unused
//!
static gboolean stream_results_cleanup (GwSearchItem *item)
{
    if (item->fd == NULL) return FALSE;

    less_relevant_title_inserted = FALSE;
    item->status = GW_SEARCH_FINISHING;
    (*item->gw_searchitem_ui_after_search_cleanup)(item);
    gw_searchitem_do_post_search_clean (item);
    item->status = GW_SEARCH_IDLE;
}


//!
//! @brief Start a dictionary search
//!
//! This is the entry point for starting a search.  It handles setting up the
//! query, checking things that need to be checked before the final go, and
//! initializing the search loop or thread.
//!
//! @param item a GwSearchItem argument.
//!
void gw_search_get_results (GwSearchItem *item)
{
    gw_util_force_japanese_locale();

    //Misc preparations
    if (item->target != GW_TARGET_CONSOLE &&
        (item->dictionary->type == GW_DICT_TYPE_KANJI || item->dictionary->type == GW_DICT_TYPE_RADICALS))
      item->show_less_relevant_results = TRUE;

    if (gw_searchitem_do_pre_search_prep (item) == FALSE)
    {
      gw_searchitem_free(item);
      return;
    }

    (*item->gw_searchitem_ui_pre_search_prep) (item);

    //Start the search
    if (gw_util_get_runmode () == GW_CONSOLE_RUNMODE || gw_util_get_runmode () == GW_NCURSES_RUNMODE)
    {
      while (stream_results_thread(item))
        ;
      stream_results_cleanup(item);
    }
    else
    {
      g_timeout_add_full (G_PRIORITY_DEFAULT_IDLE, 1,
                          (GSourceFunc)stream_results_thread, item,
                          (GDestroyNotify)stream_results_cleanup     );
    }
}

