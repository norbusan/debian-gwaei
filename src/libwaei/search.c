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
//! @file searchitem.c
//!

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include <libwaei/libwaei.h>
#include <libwaei/gettext.h>

static void lw_search_init (LwSearch*, LwDictionary*, const gchar*, LwSearchFlags, GError**);
static void lw_search_deinit (LwSearch*);

//!
//! @brief Creates a new LwSearch object. 
//! @param query The text to be search for
//! @param dictionary The LwDictionary object to use
//! @param TARGET The widget to output the results to
//! @param error A GError to place errors into or NULL
//! @return Returns an allocated LwSearch object that should be freed with lw_search_free or NULL on error
//!
LwSearch* 
lw_search_new (LwDictionary* dictionary, const gchar* QUERY, LwSearchFlags flags, GError **error)
{
    g_return_val_if_fail (dictionary != NULL, NULL);
    g_return_val_if_fail (QUERY != NULL, NULL);
    if (error != NULL && *error != NULL) return NULL;

    LwSearch *temp;

    temp = g_new0 (LwSearch, 1);

    if (temp != NULL)
    {
      lw_search_init (temp, dictionary, QUERY, flags, error);

      if (error != NULL && *error != NULL)
      {
        lw_search_free (temp);
        temp = NULL;
      }
    }

    return temp;
}


//!
//! @brief Releases a LwSearch object from memory. 
//!
//! All of the various interally allocated memory in the LwSearch is freed.
//! The file descriptiors and such are made sure to also be closed.
//!
//! @param search The LwSearch to have it's memory freed.
//!
void 
lw_search_free (LwSearch* search)
{
    //Sanity check
    g_return_if_fail (search != NULL);

    lw_search_deinit (search);

    free (search);
}


//!
//! @brief Used to initialize the memory inside of a new LwSearch
//!        object.  Usually lw_search_new calls this for you.  It is also 
//!        used in class implimentations that extends LwSearch.
//! @param search A LwSearch to initialize the inner variables of
//! @param QUERY The text to be search for
//! @param dictionary The LwDictionary object to use
//! @param TARGET The widget to output the results to
//! @param error A GError to place errors into or NULL
//!
static void
lw_search_init (LwSearch *search, LwDictionary* dictionary, const gchar* TEXT, LwSearchFlags flags, GError **error)
{
    g_mutex_init (&search->mutex);
    search->status = LW_SEARCHSTATUS_IDLE;
    search->dictionary = dictionary;
    search->query = lw_query_new ();
    search->flags = flags;
    search->max = 500;

    lw_search_set_flags (search, flags);

    lw_dictionary_parse_query (search->dictionary, search->query, TEXT, error);
}


//!
//! @brief Used to free the memory inside of a LwSearch object.
//!         Usually lw_search_free calls this for you.  It is also used
//!         in class implimentations that extends LwSearch.
//! @param search The LwSearch object to have it's inner memory freed.
//!
static void 
lw_search_deinit (LwSearch *search)
{
    lw_search_cancel (search);
    lw_search_clear_results (search);
    lw_search_cleanup_search (search);
    lw_query_free (search->query);
    if (lw_search_has_data (search))
      lw_search_free_data (search);

    g_mutex_clear (&search->mutex);
}


void 
lw_search_clear_results (LwSearch *search)
{
    //Sanity checks
    g_return_if_fail (search != NULL);

    //Declarations
    gint i;

    memset(search->total_results, 0, sizeof(gint) * TOTAL_LW_RELEVANCE);

    for (i = 0; i < TOTAL_LW_RELEVANCE; i++)
    {
      g_list_foreach (search->results[i], (GFunc) lw_result_free, NULL);
      g_list_free (search->results[i]); search->results[i] = NULL;
    }
}


void
lw_search_set_max_results (LwSearch *search, gint max)
{
    search->max = max;
}


//!
//! @brief Does variable preparation required before a search
//!
//! The input and output scratch buffers have their memory allocated
//! the current_line integer is reset to 0, the comparison buffer
//! reset to it's initial state, the search status set to
//! SEARCHING, and the file descriptior is opened.
//!
//! @param search The LwSearch to its variables prepared
//! @return Returns false on seachitem prep failure.
//!
void  
lw_search_prepare_search (LwSearch* search)
{
    lw_search_clear_results (search);
    lw_search_cleanup_search (search);

    //Declarations

    //Initializations
    search->scratch_buffer = (char*) malloc (sizeof(char*) * LW_IO_MAX_FGETS_LINE);
    search->result = lw_result_new ();
    search->current = 0L;
    memset(search->total_results, 0, sizeof(gint) * TOTAL_LW_RELEVANCE);
    search->thread = NULL;
    search->fd = lw_dictionary_open (LW_DICTIONARY (search->dictionary));
    search->status = LW_SEARCHSTATUS_SEARCHING;
    search->timestamp = g_get_monotonic_time ();
}


//!
//! @brief Cleanups after a search completes
//!
//! The file descriptior is closed, various variables are
//! reset, and the search status is set to IDLE.
//!
//! @param search The LwSearch to its state reset.
//!
void 
lw_search_cleanup_search (LwSearch* search)
{
    if (search->fd != NULL)
    {
      fclose(search->fd);
      search->fd = NULL;
    }

    if (search->scratch_buffer != NULL)
    {
      free(search->scratch_buffer);
      search->scratch_buffer = NULL;
    }

    if (search->result != NULL)
    {
      lw_result_free (search->result);
      search->result = NULL;
    }

    search->status = LW_SEARCHSTATUS_FINISHING;
}


//!
//! @brief comparison function for determining if two LwSearchs are equal
//! @param item1 The first search
//! @param item2 The second search
//! @returns Returns true when both items are either the same search or have similar innards
//!
gboolean 
lw_search_is_equal (LwSearch *item1, LwSearch *item2)
{
  //Declarations
  gboolean queries_are_equal;
  gboolean dictionaries_are_equal;
  //Sanity checks
  if (item1 == NULL)
  {
    return FALSE;
  }
  if (item2 == NULL)
  {
    return FALSE;
  }

  if (item1 == item2)
  {
    return TRUE;
  }

  //Initializations
  queries_are_equal = (strcmp(item1->query->text, item2->query->text) == 0);
  dictionaries_are_equal = (item1->dictionary == item2->dictionary);

  return (queries_are_equal && dictionaries_are_equal);
}


//!
//! @brief Used to set custom search data (Such as Window or TextView pointers)
//! @param search The LwSearch to set the data on.  It will free any previous data if it is already set.
//! @param data The data to set.
//! @param free_data_func A callback to use to free the data automatically as needed
//!
void 
lw_search_set_data (LwSearch *search, gpointer data, LwSearchDataFreeFunc free_data_func)
{
    //Sanity check
    g_assert (search != NULL);

    if (lw_search_has_data (search))
      lw_search_free_data (search);

    search->data = data;
    search->free_data_func = free_data_func;
}


//!
//! @brief to retieve custom search data (Such as Window or TextView pointers)
//! @param search The LwSearch object to retrieve the data on.
//! @returns A generic pointer to the data that should be cast.
//!
gpointer 
lw_search_get_data (LwSearch *search)
{
    //Sanity check
    g_assert (search != NULL);

    return search->data;
}


//!
//! @brief Frees the data on an LwSearch object if it exists
//! @param search The LwSearch to free the data on
//!
void 
lw_search_free_data (LwSearch *search)
{
    //Sanity check
    g_assert (search != NULL);

    if (search->free_data_func != NULL && search->data != NULL)
    {
      (search->free_data_func) (search->data);
    }

    search->data = NULL;
    search->free_data_func = NULL;
}


//!
//! @brief Returns true if the LwSearch had its data set
//! @param search An LwSearch to check for data
//! @returns Returns true if the data is not NULL
//!
gboolean 
lw_search_has_data (LwSearch *search)
{
    g_assert (search != NULL);

    return (search->data != NULL && search->free_data_func != NULL);
}


//!
//! @brief A wrapper around gmutex made for LwSearch objects
//! @param search An LwSearch to lock the mutex on
//!
void 
lw_search_lock (LwSearch *search)
{
  g_mutex_lock (&search->mutex);
}

//!
//! @brief A wrapper around gmutex made for LwSearch objects
//! @param search An LwSearch to unlock the mutex on
//!
void 
lw_search_unlock (LwSearch *search)
{
  g_mutex_unlock (&search->mutex);
}


gdouble 
lw_search_get_progress (LwSearch *search)
{
    //Sanity check
    if (search == NULL) return 0.0;

    //Declarations
    glong current;
    glong length;
    gdouble fraction;

    //Initializations
    current = 0;
    length = 0;
    fraction = 0.0;

    if (search != NULL && search->dictionary != NULL && search->status == LW_SEARCHSTATUS_SEARCHING)
    {
      current = search->current;
      length = lw_dictionary_get_length (LW_DICTIONARY (search->dictionary));

      if (current > 0 && length > 0 && current != length) 
        fraction = (gdouble) current / (gdouble) length;
    }

    return fraction;
}


void
lw_search_set_status (LwSearch *search, LwSearchStatus status)
{
    lw_search_lock (search);
    search->status = status;
    lw_search_unlock (search);
}


LwSearchStatus
lw_search_get_status (LwSearch *search)
{
    LwSearchStatus status;

    lw_search_lock (search);
      status = search->status;
    lw_search_unlock (search);

    return status;
}


gboolean
lw_search_parse_result (LwSearch *search)
{
    gint bytes_read;

    bytes_read = lw_dictionary_parse_result (search->dictionary, search->result, search->fd);
    search->current += bytes_read;

    return (bytes_read > 0 && search->status == LW_SEARCHSTATUS_SEARCHING);
}


gboolean 
lw_search_compare (LwSearch *search, const LwRelevance RELEVANCE)
{
    return lw_dictionary_compare (search->dictionary, search->query, search->result, RELEVANCE);
}


//!
//! @brief Find the relevance of a returned result
//!
//! THIS IS A PRIVATE FUNCTION. Function uses the stored relevance regrex
//! expressions in the LwSearch to get the relevance of a returned result.  It
//! then returns the answer to the caller in the form of an int.
//!
//! @param text a string to check the relevance of
//! @param search a search search to grab the regrexes from
//! @return Returns one of the integers: LOW_RELEVANCE, MEDIUM_RELEVANCE, or HIGH_RELEVANCE.
//!
static int 
lw_search_get_relevance (LwSearch *search) {
    if (lw_search_compare (search, LW_RELEVANCE_HIGH))
      return LW_RELEVANCE_HIGH;
    else if (lw_search_compare (search, LW_RELEVANCE_MEDIUM))
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
//! @param data A LwSearch to search with
//! @return Returns true when the search isn't finished yet.
//!
static gpointer 
lw_search_stream_results_thread (gpointer data)
{
    //Declarations
    LwSearch *search;
    gboolean exact;
    gint relevance;

    //Initializations
    search = LW_SEARCH (data);
    g_return_val_if_fail (search != NULL && search->fd != NULL, NULL);
    exact = search->flags & LW_SEARCH_FLAG_EXACT;

    lw_search_lock (search);
    search->status = LW_SEARCHSTATUS_SEARCHING;

    //We loop, processing lines of the file until the max chunk size has been
    //reached or we reach the end of the file or a cancel request is recieved.
    while (lw_search_parse_result (search))
    {
      //Give a chance for something else to run
      lw_search_unlock (search);
      if (search->status == LW_SEARCHSTATUS_SEARCHING)
      {
        g_thread_yield ();
      }
      lw_search_lock (search);
      //Results match, add to the text buffer
      if (lw_search_compare (search, LW_RELEVANCE_LOW))
      {
        relevance = lw_search_get_relevance (search);
        if (search->total_results[relevance] < search->max)
        {
          if (!exact || (relevance == LW_RELEVANCE_HIGH && exact))
          {
            search->total_results[relevance]++;
            search->result->relevance = relevance;
            search->results[relevance] = g_list_append (search->results[relevance], search->result);
            search->result = lw_result_new ();
          }
        }
      }
    }

    lw_search_cleanup_search (search);

    lw_search_unlock (search);

    return NULL;
}


//!
//! @brief Start a dictionary search
//! @param search a LwSearch argument to calculate results
//! @param create_thread Whether the search should run in a new thread.
//! @param exact Whether to show only exact matches for this search
//!
void 
lw_search_start (LwSearch *search, gboolean create_thread)
{
    GError *error;

    error = NULL;

    lw_search_prepare_search (search);
    if (create_thread)
    {
      search->thread = g_thread_try_new (
        "libwaei-search",
        (GThreadFunc) lw_search_stream_results_thread, 
        (gpointer) search, 
        &error
      );
      if (search->thread == NULL)
      {
        g_warning ("Thread Creation Error: %s\n", error->message);
        g_error_free (error);
        error = NULL;
      }
    }
    else
    {
      search->thread = NULL;
      lw_search_stream_results_thread ((gpointer) search);
    }
}


//!
//! @brief Uses a searchitem to cancel a window
//!
//! @param search A LwSearch to gleam information from
//!
void 
lw_search_cancel (LwSearch *search)
{
    if (search == NULL) return;

    search->cancel = TRUE;
    lw_search_set_status (search, LW_SEARCHSTATUS_CANCELING);

    if (search->thread != NULL)
    {
      g_thread_join (search->thread);
      search->thread = NULL;
    }

    search->cancel = FALSE;
    lw_search_set_status (search, LW_SEARCHSTATUS_IDLE);
}


//!
//! @brief Gets a result and removes a LwResult from the beginnig of a list of results
//! @returns a LwResult that should be freed with lw_result_free
//!
LwResult* 
lw_search_get_result (LwSearch *search)
{
    //Sanity checks
    g_return_val_if_fail (search != NULL, NULL);

    //Declarations
    LwResult *result;
    gint relevance;
    gint stop;

    //Initializations
    result = NULL; 

    lw_search_lock (search);

    if (search->status == LW_SEARCHSTATUS_SEARCHING) stop = LW_RELEVANCE_HIGH;
    else stop = LW_RELEVANCE_LOW;

    for (relevance = LW_RELEVANCE_HIGH; relevance >= stop && result == NULL; relevance--)
    {
      if (search->results[relevance] != NULL)
      {
        result = LW_RESULT (search->results[relevance]->data);
        search->results[relevance] = g_list_delete_link (search->results[relevance], search->results[relevance]);
      }
    }

    if (result == NULL && search->status == LW_SEARCHSTATUS_FINISHING) search->status = LW_SEARCHSTATUS_IDLE;

    lw_search_unlock (search);

    return result;
}



//!
//! @brief Tells if you should keep checking for results
//!
gboolean 
lw_search_has_results (LwSearch *search)
{
    //Sanity checks
    if (search == NULL) return FALSE;

    //Declarations
    LwSearchStatus status;
    gboolean has_results;

    //Initializations
    lw_search_lock (search);

    status = search->status;
    has_results = FALSE;

    if (status == LW_SEARCHSTATUS_SEARCHING && search->results[LW_RELEVANCE_HIGH] != NULL) 
      has_results = TRUE;
    else if (status != LW_SEARCHSTATUS_SEARCHING && (search->results[LW_RELEVANCE_HIGH] != NULL ||
                                                     search->results[LW_RELEVANCE_MEDIUM] != NULL ||
                                                     search->results[LW_RELEVANCE_LOW] != NULL))
      has_results = TRUE;
  
    if (status == LW_SEARCHSTATUS_FINISHING && !has_results) search->status = LW_SEARCHSTATUS_IDLE;

    lw_search_unlock (search);

    return has_results;
}


gint
lw_search_get_total_results (LwSearch *search)
{
    //Sanity checks
    g_return_val_if_fail (search != NULL, 0);

    //Declarations
    gint total;
    gint relevance;

    //Initializations
    total = 0;

    for (relevance = 0; relevance < TOTAL_LW_RELEVANCE; relevance++)
    {
      total += search->total_results[relevance];
    }

    return total;
}


gint
lw_search_get_total_relevant_results (LwSearch *search)
{
    //Sanity checks
    g_return_val_if_fail (search != NULL, 0);

    return search->total_results[LW_RELEVANCE_HIGH];
}


gint
lw_search_get_total_irrelevant_results (LwSearch *search)
{
    //Sanity checks
    g_return_val_if_fail (search != NULL, 0);

    //Declarations
    gint total;

    //Initializations
    total = 0;
    total += search->total_results[LW_RELEVANCE_LOW];
    total += search->total_results[LW_RELEVANCE_MEDIUM];

    return total;
}

void
lw_search_set_flags (LwSearch *search, LwSearchFlags flags)
{
    //Sanity checks
    g_return_if_fail (search != NULL);
    g_return_if_fail (search->query != NULL);

    LwQuery *query;

    search->flags = flags;
    query = search->query;

    query->flags = flags & 0xFFFF;
}


LwSearchFlags
lw_search_get_flags (LwSearch *search)
{
    //Sanity checks
    g_return_val_if_fail (search != NULL, 0);

    return search->flags;
}


LwSearchFlags
lw_search_get_flags_from_preferences (LwPreferences *preferences)
{
    //Sanity checks
    g_return_val_if_fail (preferences != NULL, 0);

    //Declarations
    gboolean hiragana_to_katakana;
    gboolean katakana_to_hiragana;
    gint romaji_to_furigana;
    gboolean want_romaji_to_furigana_conv;
    gboolean delimit_whitespace;
    gboolean delimit_morphology;
    gboolean root_word;
    gint32 flags;

    //Initializations
    hiragana_to_katakana = lw_preferences_get_boolean_by_schema (preferences, LW_SCHEMA_BASE, LW_KEY_HIRA_KATA);
    katakana_to_hiragana = lw_preferences_get_boolean_by_schema (preferences, LW_SCHEMA_BASE, LW_KEY_KATA_HIRA);
    romaji_to_furigana = lw_preferences_get_int_by_schema (preferences, LW_SCHEMA_BASE, LW_KEY_ROMAN_KANA);
    want_romaji_to_furigana_conv = (romaji_to_furigana == 0 || (romaji_to_furigana == 2 && !lw_util_is_japanese_locale()));
    delimit_whitespace = LW_QUERY_FLAG_DELIMIT_WHITESPACE;
    delimit_morphology = LW_QUERY_FLAG_DELIMIT_MORPHOLOGY;
    root_word = LW_QUERY_FLAG_ROOT_WORD;
    flags = 0;

    if (hiragana_to_katakana) flags |= LW_SEARCH_FLAG_HIRAGANA_TO_KATAKANA;
    if (katakana_to_hiragana) flags |= LW_SEARCH_FLAG_KATAKANA_TO_HIRAGANA;
    if (want_romaji_to_furigana_conv) flags |= LW_SEARCH_FLAG_ROMAJI_TO_FURIGANA;
    if (delimit_whitespace) flags |= LW_SEARCH_FLAG_DELIMIT_WHITESPACE;
    if (delimit_morphology) flags |= LW_SEARCH_FLAG_DELIMIT_MORPHOLGY;
    if (root_word) flags |= LW_SEARCH_FLAG_ROOT_WORD;

    return flags;
}

