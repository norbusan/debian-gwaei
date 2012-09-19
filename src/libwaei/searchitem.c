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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include <libwaei/libwaei.h>

static gboolean _query_is_sane (const char* query)
{
    //Declarations
    char *q;
    gboolean is_sane;

    //Initializations
    q = lw_util_prepare_query (query, TRUE); 
    is_sane = TRUE;

    //Tests
    if (strlen (q) == 0)
      is_sane = FALSE;

    if (g_str_has_prefix (q, "|") || g_str_has_prefix (q, "&")) 
      is_sane = FALSE;
    if (g_str_has_suffix (q, "\\") || g_str_has_suffix (q, "|") || g_str_has_suffix (q, "&")) 
      is_sane = FALSE;

    g_free (q);

    return is_sane;
}


//!
//! @brief Creates a new LwSearchItem object. 
//! @param query The text to be search for
//! @param dictionary The LwDictInfo object to use
//! @param TARGET The widget to output the results to
//! @param pm The Application preference manager to get information from
//! @param error A GError to place errors into or NULL
//! @return Returns an allocated LwSearchItem object that should be freed with lw_searchitem_free or NULL on error
//!
LwSearchItem* 
lw_searchitem_new (const char* query, LwDictInfo* dictionary, LwPreferences *pm, GError **error)
{
    if (!_query_is_sane (query)) return NULL;

    LwSearchItem *temp;

    temp = (LwSearchItem*) malloc(sizeof(LwSearchItem));

    if (temp != NULL)
    {
      lw_searchitem_init (temp, query, dictionary, pm, error);

      if (error != NULL && *error != NULL)
      {
        lw_searchitem_free (temp);
        temp = NULL;
      }
    }

    return temp;
}


//!
//! @brief Releases a LwSearchItem object from memory. 
//!
//! All of the various interally allocated memory in the LwSearchItem is freed.
//! The file descriptiors and such are made sure to also be closed.
//!
//! @param item The LwSearchItem to have it's memory freed.
//!
void 
lw_searchitem_free (LwSearchItem* item)
{
    //Sanity check
    g_assert (item != NULL && item->status == LW_SEARCHSTATUS_IDLE);

    lw_searchitem_deinit (item);

    free (item);
}


//!
//! @brief Used to initialize the memory inside of a new LwSearchItem
//!        object.  Usually lw_searchitem_new calls this for you.  It is also 
//!        used in class implimentations that extends LwSearchItem.
//! @param item A LwSearchItem to initialize the inner variables of
//! @param query The text to be search for
//! @param dictionary The LwDictInfo object to use
//! @param TARGET The widget to output the results to
//! @param pm The Application preference manager to get information from
//! @param error A GError to place errors into or NULL
//!
void 
lw_searchitem_init (LwSearchItem *item, const char* query, LwDictInfo* dictionary, LwPreferences *pm, GError **error)
{
    item->results_high = NULL;
    item->results_medium = NULL;
    item->results_low = NULL;
    item->thread = NULL;
    item->mutex = g_mutex_new ();

    //Set the internal pointers to the correct global variables
    item->fd = NULL;
    item->status = LW_SEARCHSTATUS_IDLE;
    item->scratch_buffer = NULL;
    item->dictionary = dictionary;
    item->data = NULL;
    item->free_data_func = NULL;
    item->total_relevant_results = 0;
    item->total_irrelevant_results = 0;
    item->total_results = 0;
    item->current = 0L;
    item->resultline = NULL;
    item->queryline = lw_queryline_new ();
    item->history_relevance_idle_timer = 0;

    //Set function pointers
    switch (item->dictionary->type)
    {
        case LW_DICTTYPE_EDICT:
          lw_queryline_parse_edict_string (item->queryline, pm, query, error);
          break;
        case LW_DICTTYPE_KANJI:
          lw_queryline_parse_kanjidict_string (item->queryline, pm, query, error);
          break;
        case LW_DICTTYPE_EXAMPLES:
          lw_queryline_parse_exampledict_string (item->queryline, pm, query, error);
          break;
        default:
          lw_queryline_parse_edict_string (item->queryline, pm, query, error);
          break;
    }
}


//!
//! @brief Used to free the memory inside of a LwSearchItem object.
//!         Usually lw_searchitem_free calls this for you.  It is also used
//!         in class implimentations that extends LwSearchItem.
//! @param item The LwSearchItem object to have it's inner memory freed.
//!
void 
lw_searchitem_deinit (LwSearchItem *item)
{
    if (item->thread != NULL) 
    {
      item->status = LW_SEARCHSTATUS_CANCELING;
      g_thread_join (item->thread);
      item->thread = NULL;
    }
    lw_searchitem_clear_results (item);
    lw_searchitem_cleanup_search (item);
    lw_queryline_free (item->queryline);
    if (lw_searchitem_has_data (item))
      lw_searchitem_free_data (item);

    g_mutex_free (item->mutex);
    item->mutex = NULL;
}


void 
lw_searchitem_clear_results (LwSearchItem *item)
{
    item->total_relevant_results = 0;
    item->total_irrelevant_results = 0;
    item->total_results = 0;

    while (item->results_low != NULL)
    {
      lw_resultline_free (LW_RESULTLINE (item->results_low->data));
      item->results_low = g_list_delete_link (item->results_low, item->results_low);
    }
    while (item->results_medium != NULL)
    {
      lw_resultline_free (LW_RESULTLINE (item->results_medium->data));
      item->results_medium = g_list_delete_link (item->results_medium, item->results_medium);
    }
    while (item->results_high != NULL)
    {
      lw_resultline_free (LW_RESULTLINE (item->results_high->data));
      item->results_high = g_list_delete_link (item->results_high, item->results_high);
    }
}


//!
//! @brief Does variable preparation required before a search
//!
//! The input and output scratch buffers have their memory allocated
//! the current_line integer is reset to 0, the comparison buffer
//! reset to it's initial state, the search status set to
//! SEARCHING, and the file descriptior is opened.
//!
//! @param item The LwSearchItem to its variables prepared
//! @return Returns false on seachitem prep failure.
//!
void  
lw_searchitem_prepare_search (LwSearchItem* item)
{
    lw_searchitem_clear_results (item);
    lw_searchitem_cleanup_search (item);

    //Declarations
    char *path;

    //Initializations
    item->scratch_buffer = (char*) malloc (sizeof(char*) * LW_IO_MAX_FGETS_LINE);
    item->resultline = lw_resultline_new ();
    item->current = 0L;
    item->total_relevant_results = 0;
    item->total_irrelevant_results = 0;
    item->total_results = 0;
    item->thread = NULL;

    path = lw_dictinfo_get_uri (item->dictionary);
    if (path != NULL)
    {
      item->fd = fopen (path, "r");
      g_free (path);
    }

    item->status = LW_SEARCHSTATUS_SEARCHING;
}


//!
//! @brief Cleanups after a search completes
//!
//! The file descriptior is closed, various variables are
//! reset, and the search status is set to IDLE.
//!
//! @param item The LwSearchItem to its state reset.
//!
void 
lw_searchitem_cleanup_search (LwSearchItem* item)
{
    if (item->fd != NULL)
    {
      fclose(item->fd);
      item->fd = NULL;
    }

    if (item->scratch_buffer != NULL)
    {
      free(item->scratch_buffer);
      item->scratch_buffer = NULL;
    }

    if (item->resultline != NULL)
    {
      lw_resultline_free (item->resultline);
      item->resultline = NULL;
    }

    item->thread = NULL;
    item->status = LW_SEARCHSTATUS_IDLE;
}


static gboolean _edict_existance_comparison (LwQueryLine *ql, LwResultLine *rl, const LwRelevance RELEVANCE)
{
    //Declarations
    int j;
    GRegex *re;
    GRegex ***iter;

    //Compare kanji atoms
    if (rl->kanji_start != NULL)
    {
      for (iter = ql->re_kanji; *iter != NULL && **iter != NULL; iter++)
      {
        re = (*iter)[RELEVANCE];
        if (g_regex_match (re, rl->kanji_start, 0, NULL) == FALSE) break;
      }
      if (ql->re_kanji[0][RELEVANCE] != NULL && *iter == NULL) return TRUE;
    }

    //Compare furigana atoms
    if (rl->furigana_start != NULL)
    {
      for (iter = ql->re_furi; *iter != NULL && **iter != NULL; iter++)
      {
        re = (*iter)[RELEVANCE];
        if (g_regex_match (re, rl->furigana_start, 0, NULL) == FALSE) break;
      }
      if (ql->re_furi[0][RELEVANCE] != NULL && *iter == NULL) return TRUE;
    }

    if (rl->kanji_start != NULL)
    {
      for (iter = ql->re_furi; *iter != NULL && **iter != NULL; iter++)
      {
        re = (*iter)[RELEVANCE];
        if (g_regex_match (re, rl->kanji_start, 0, NULL) == FALSE) break;
      }
      if (ql->re_furi[0][RELEVANCE] != NULL && *iter == NULL) return TRUE;
    }


    //Compare romaji atoms
    for (j = 0; rl->def_start[j] != NULL; j++)
    {
      for (iter = ql->re_roma; *iter != NULL && **iter != NULL; iter++)
      {
        re = (*iter)[RELEVANCE];
        if (g_regex_match (re, rl->def_start[j], 0, NULL) == FALSE) break;
      }
      if (ql->re_roma[0][RELEVANCE] != NULL && *iter == NULL) return TRUE;
    }

    //Compare mix atoms
    if (rl->string != NULL)
    {
      for (iter = ql->re_mix; *iter != NULL && **iter != NULL; iter++)
      {
        re = (*iter)[RELEVANCE];
        if (g_regex_match (re, rl->string, 0, NULL) == FALSE) break;
      }
      if (ql->re_roma[0][RELEVANCE] != NULL && *iter == NULL) return TRUE;
    }

    return FALSE;
}


static gboolean _kanji_existance_comparison (LwQueryLine *ql, LwResultLine *rl, const LwRelevance RELEVANCE)
{
    //Declarations
    gboolean strokes_check_passed;
    gboolean frequency_check_passed;
    gboolean grade_check_passed;
    gboolean jlpt_check_passed;
    gboolean romaji_check_passed;
    gboolean furigana_check_passed;
    gboolean kanji_check_passed;
    gboolean radical_check_passed;
    int kanji_index;
    int radical_index;

    GRegex ***iter;
    GRegex *re;

    int i;

    //Initializations
    strokes_check_passed = TRUE;
    frequency_check_passed = TRUE;
    grade_check_passed = TRUE;
    jlpt_check_passed = TRUE;
    romaji_check_passed = TRUE;
    furigana_check_passed = TRUE;
    kanji_check_passed = TRUE;
    radical_check_passed = TRUE;
    kanji_index = -1;
    radical_index = -1;

    //Calculate the strokes check
    if (rl->strokes != NULL)
    {
      for (iter = ql->re_strokes; iter != NULL && *iter != NULL; iter++)
      {
        re = (*iter)[RELEVANCE];
        if (re != NULL && g_regex_match (re, rl->strokes, 0, NULL) == FALSE) 
          strokes_check_passed = FALSE;
      }
    }
    else
      if (ql->re_strokes != NULL && *(ql->re_strokes) != NULL)
        strokes_check_passed = FALSE;

    //Calculate the frequency check
    if (rl->frequency != NULL)
    {
      for (iter = ql->re_frequency; iter != NULL && *iter != NULL; iter++)
      {
        re = (*iter)[RELEVANCE];
        if (re != NULL && g_regex_match (re, rl->frequency, 0, NULL) == FALSE) 
          frequency_check_passed = FALSE;
      }
    }
    else
      if (ql->re_frequency != NULL && *(ql->re_frequency) != NULL)
        frequency_check_passed = FALSE;


    //Calculate the grade check
    if (rl->grade != NULL)
    {
      for (iter = ql->re_grade; iter != NULL && *iter != NULL; iter++)
      {
        re = (*iter)[RELEVANCE];
        if (re != NULL && g_regex_match (re, rl->grade, 0, NULL) == FALSE) 
          grade_check_passed = FALSE;
      }
    }
    else
      if (ql->re_grade != NULL && *(ql->re_grade) != NULL)
        grade_check_passed = FALSE;


    //Calculate the jlpt check
    if (rl->jlpt != NULL)
    {
      for (iter = ql->re_jlpt; iter != NULL && *iter != NULL; iter++)
      {
        re = (*iter)[RELEVANCE];
        if (re != NULL && g_regex_match (re, rl->jlpt, 0, NULL) == FALSE) 
          jlpt_check_passed = FALSE;
      }
    }
    else
      if (ql->re_jlpt != NULL && *(ql->re_jlpt) != NULL)
        jlpt_check_passed = FALSE;



    //Calculate the romaji check
    if (rl->meanings != NULL)
    {
      for (iter = ql->re_roma; ql->re_roma[0] != NULL && iter != NULL && *iter != NULL; iter++)
      {
        re = (*iter)[RELEVANCE];
        if (re != NULL) romaji_check_passed = FALSE;
      }
      //if (ql->re_roma[0] != NULL && ql->re_roma[0][0] != NULL) romaji_check_passed = FALSE;

      for (iter = ql->re_roma; iter != NULL && *iter != NULL; iter++)
      {
        re = (*iter)[RELEVANCE];

        if (re != NULL && g_regex_match (re, rl->meanings, 0, NULL) == TRUE) 
        {
          romaji_check_passed = TRUE;
        }
      }
    }


    //Calculate the furigana check
    if (*(ql->re_furi) != NULL && (rl->readings[0] != NULL || rl->readings[1] != NULL || rl->readings[2] != NULL))
    {
       furigana_check_passed = FALSE;
    }
    for (i = 0; i < 3; i++)
    {
      if (rl->readings[i] == NULL) continue;

      for (iter = ql->re_furi; iter != NULL && *iter != NULL; iter++)
      {
        re = (*iter)[RELEVANCE];
        if (re != NULL && g_regex_match (re, rl->readings[i], 0, NULL) == TRUE) 
        {
          furigana_check_passed = TRUE;
        }
      }
    }

    //Calculate the kanji check
    if (*(ql->re_kanji) != NULL && rl->kanji != NULL)
    {
      kanji_index = 0;
      for (iter = ql->re_kanji; iter != NULL && *iter != NULL; iter++)
      {
        re = (*iter)[RELEVANCE];
        if (re != NULL && g_regex_match (re, rl->kanji, 0, NULL) == FALSE) 
        {
          kanji_check_passed = FALSE;
          kanji_index = -1;
        }
        if (kanji_check_passed == TRUE) kanji_index++;
      }
    }

    //Calculate the radical check
    if (rl->radicals != NULL)
    {
      radical_index = 0;
      for (iter = ql->re_kanji; iter != NULL && *iter != NULL; iter++)
      {
        re = (*iter)[RELEVANCE];
        if (re != NULL && g_regex_match (re, rl->radicals, 0, NULL) == FALSE) 
        {
          if (radical_index != kanji_index) //Make sure the radical wasn't found as a kanji before setting false
             radical_check_passed = FALSE;
        }
        radical_index++;
      }
    }

    //Return our results
    return (strokes_check_passed &&
            frequency_check_passed &&
            grade_check_passed &&
            jlpt_check_passed &&
            romaji_check_passed &&
            furigana_check_passed &&
            (radical_check_passed || kanji_check_passed));
}


//!
//! @brief Comparison function that should be moved to the LwSearchItem file when it matures
//! @param item A LwSearchItem to get search information from
//! @param RELEVANCE A LwRelevance
//! @returns Returns true according to the relevance level
//!
gboolean 
lw_searchitem_run_comparison (LwSearchItem *item, const LwRelevance RELEVANCE)
{
    //Declarations
    LwResultLine *rl;
    LwQueryLine *ql;

    //Initializations
    rl = item->resultline;
    ql = item->queryline;

    //Kanji radical dictionary search
    switch (item->dictionary->type)
    {
      case LW_DICTTYPE_EDICT:
        return _edict_existance_comparison (ql, rl, RELEVANCE);
      case LW_DICTTYPE_KANJI:
        return _kanji_existance_comparison (ql, rl, RELEVANCE);
      case LW_DICTTYPE_EXAMPLES:
        return _edict_existance_comparison (ql, rl, RELEVANCE);
      default:
        return _edict_existance_comparison (ql, rl, RELEVANCE);
    }
}


//!
//! @brief comparison function for determining if two LwSearchItems are equal
//! @param item1 The first item
//! @param item2 The second item
//! @returns Returns true when both items are either the same item or have similar innards
//!
gboolean 
lw_searchitem_is_equal (LwSearchItem *item1, LwSearchItem *item2)
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
  queries_are_equal = (strcmp(item1->queryline->string, item2->queryline->string) == 0);
  dictionaries_are_equal = (item1->dictionary == item2->dictionary);

  return (queries_are_equal && dictionaries_are_equal);
}


//!
//! @brief a method for incrementing an internal integer for determining if a result set has worth
//! @param item The LwSearchItem to increment the timer on
//!
void 
lw_searchitem_increment_history_relevance_timer (LwSearchItem *item)
{
  if (item != NULL && item->history_relevance_idle_timer < LW_HISTORY_TIME_TO_RELEVANCE)
    item->history_relevance_idle_timer++;
}


//!
//! @brief Checks if the relevant timer has passed a threshold
//! @param item The LwSearchItem to check for history relevance
//! @param use_idle_timer This variable shoud be set to true if the program does automatic searches so it checks the timer
//!
gboolean 
lw_searchitem_has_history_relevance (LwSearchItem *item, gboolean use_idle_timer)
{
  return (item != NULL && 
          item->total_results > 0 && 
          (!use_idle_timer || item->history_relevance_idle_timer >= LW_HISTORY_TIME_TO_RELEVANCE));
}


//!
//! @brief Used to set custom search data (Such as Window or TextView pointers)
//! @param item The LwSearchItem to set the data on.  It will free any previous data if it is already set.
//! @param data The data to set.
//! @param free_data_func A callback to use to free the data automatically as needed
//!
void 
lw_searchitem_set_data (LwSearchItem *item, gpointer data, LwSearchItemDataFreeFunc free_data_func)
{
    //Sanity check
    g_assert (item != NULL);

    if (lw_searchitem_has_data (item))
      lw_searchitem_free_data (item);

    item->data = data;
    item->free_data_func = free_data_func;
}


//!
//! @brief to retieve custom search data (Such as Window or TextView pointers)
//! @param item The LwSearchItem object to retrieve the data on.
//! @returns A generic pointer to the data that should be cast.
//!
gpointer 
lw_searchitem_get_data (LwSearchItem *item)
{
    //Sanity check
    g_assert (item != NULL);

    return item->data;
}


//!
//! @brief Frees the data on an LwSearchItem object if it exists
//! @param item The LwSearchItem to free the data on
//!
void 
lw_searchitem_free_data (LwSearchItem *item)
{
    //Sanity check
    g_assert (item != NULL);

    if (item->free_data_func != NULL && item->data != NULL)
    {
      (item->free_data_func) (item->data);
    }

    item->data = NULL;
    item->free_data_func = NULL;
}


//!
//! @brief Returns true if the LwSearchItem had its data set
//! @param item An LwSearchItem to check for data
//! @returns Returns true if the data is not NULL
//!
gboolean 
lw_searchitem_has_data (LwSearchItem *item)
{
    g_assert (item != NULL);

    return (item->data != NULL && item->free_data_func != NULL);
}


//!
//! @brief Parses a line from the dictionary using edict rules
//! @param item A LwSearchItem to parse and store the result string into
//!
void 
lw_searchitem_parse_result_string (LwSearchItem *item)
{
    switch (item->dictionary->type)
    {
        case LW_DICTTYPE_EDICT:
          lw_resultline_parse_edict_result_string (item->resultline);
          break;
        case LW_DICTTYPE_KANJI:
          lw_resultline_parse_kanjidict_result_string (item->resultline);
          break;
        case LW_DICTTYPE_EXAMPLES:
          lw_resultline_parse_examplesdict_result_string (item->resultline);
          break;
        case LW_DICTTYPE_UNKNOWN:
          lw_resultline_parse_unknowndict_result_string (item->resultline);
          break;
        default:
          g_assert_not_reached ();
          break;
    }
}

static int _locks = 0;

//!
//! @brief A wrapper around gmutex made for LwSearchItem objects
//! @param item An LwSearchItem to lock the mutex on
//!
void 
lw_searchitem_lock_mutex (LwSearchItem *item)
{
  _locks++;
//  printf("LOCKS %d\n", _locks);
  g_mutex_lock (item->mutex);
}

//!
//! @brief A wrapper around gmutex made for LwSearchItem objects
//! @param item An LwSearchItem to unlock the mutex on
//!
void 
lw_searchitem_unlock_mutex (LwSearchItem *item)
{
  _locks--;
//  printf("LOCKS %d\n", _locks);
  g_mutex_unlock(item->mutex);
}


double 
lw_searchitem_get_progress (LwSearchItem *item)
{
    //Declarations
    long current;
    long length;
    double fraction;

    //Initializations
    current = 0L;
    length = 0L;
    fraction = 0.0;

    if (item != NULL && item->dictionary != NULL && item->status == LW_SEARCHSTATUS_SEARCHING)
    {
      current = item->current;
      length = item->dictionary->length;

      if (current > 0L && length > 0L && current != length) 
        fraction = (double) current / (double) length;
    }

    return fraction;
}
