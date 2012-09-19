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
//! @file src/searchitem-object.c
//!
//! @brief Search item and history management
//!
//! Functions and objects to create search items and manage them.
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
    const char *ptr;
    int count;

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
//!
//! Takes the query and parses it according to the dictionary and TARGET give
//! to it.  Searchitem also stores various variables such as the file
//! it uses and the tallied results.
//!
//! @param query The text to be search for
//! @param dictionary The LwDictInfo object to use
//! @param TARGET The widget to output the results to
//! @return Returns an allocated LwSearchItem object
//!
LwSearchItem* lw_searchitem_new (char* query, LwDictInfo* dictionary, const int TARGET, GError **error)
{
    if (!_query_is_sane (query)) return NULL;

    LwSearchItem *temp;

    //Allocate some memory
    if ((temp = malloc(sizeof(LwSearchItem))) == NULL) return NULL;

    temp->results_medium = NULL;
    temp->results_low = NULL;
    temp->thread = NULL;
    temp->mutex = g_mutex_new ();
    
    if (TARGET != GW_TARGET_RESULTS &&
        TARGET != GW_TARGET_KANJI   &&
        TARGET != GW_TARGET_CONSOLE       )
      return NULL;

    //Set the internal pointers to the correct global variables
    temp->fd     = NULL;
    temp->status = GW_SEARCH_IDLE;
    temp->scratch_buffer = NULL;
    temp->dictionary = dictionary;
    temp->target = TARGET;
    temp->target_tb = NULL;
    temp->target_tv = NULL;
    temp->total_relevant_results = 0;
    temp->total_irrelevant_results = 0;
    temp->total_results = 0;
    temp->current_line = 0;
    temp->resultline = NULL;
    temp->backup_resultline = NULL;
    temp->swap_resultline = NULL;
    temp->queryline = lw_queryline_new ();
    temp->history_relevance_idle_timer = 0;
    temp->show_only_exact_matches = FALSE;

    //Set function pointers
    switch (temp->dictionary->engine)
    {
        case GW_ENGINE_EDICT:
          if (!lw_queryline_parse_edict_string (temp->queryline, query, error)) return NULL;
          temp->lw_searchitem_parse_result_string = &lw_resultline_parse_edict_result_string;
          temp->lw_searchitem_ui_append_results_to_output = lw_engine_get_append_edict_results_func ();
          break;
        case GW_ENGINE_KANJI:
          if (!lw_queryline_parse_kanjidict_string (temp->queryline, query, error)) return NULL;
          temp->lw_searchitem_parse_result_string = &lw_resultline_parse_kanjidict_result_string;
          temp->lw_searchitem_ui_append_results_to_output = lw_engine_get_append_kanjidict_results_func ();
          break;
        case GW_ENGINE_EXAMPLES:
          if (!lw_queryline_parse_exampledict_string (temp->queryline, query, error)) return NULL;
          temp->lw_searchitem_parse_result_string = &lw_resultline_parse_examplesdict_result_string;
          temp->lw_searchitem_ui_append_results_to_output = lw_engine_get_append_examplesdict_results_func ();
        break;
        default:
          if (!lw_queryline_parse_edict_string (temp->queryline, query, error)) return NULL;
          temp->lw_searchitem_parse_result_string = &lw_resultline_parse_unknowndict_result_string;
          temp->lw_searchitem_ui_append_results_to_output = lw_engine_get_append_unknowndict_results_func ();
          break;
    }
    temp->lw_searchitem_ui_append_less_relevant_header_to_output = lw_engine_get_append_less_relevant_header_func ();
    temp->lw_searchitem_ui_append_more_relevant_header_to_output = lw_engine_get_append_more_relevant_header_func ();
    temp->lw_searchitem_ui_pre_search_prep = lw_engine_get_pre_search_prep_func ();
    temp->lw_searchitem_ui_after_search_cleanup = lw_engine_get_after_search_cleanup_func ();

    return temp;
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
gboolean lw_searchitem_do_pre_search_prep (LwSearchItem* item)
{
    if (item->scratch_buffer != NULL || (item->scratch_buffer = malloc (GW_IO_MAX_FGETS_LINE)) == NULL)
    {
      return FALSE;
    }
    if (item->resultline != NULL || (item->resultline = lw_resultline_new ()) == NULL)
    {
      free (item->scratch_buffer);
      item->scratch_buffer = NULL;
      return FALSE;
    }
    if (item->backup_resultline != NULL || (item->backup_resultline = lw_resultline_new ()) == NULL)
    {
      lw_resultline_free (item->resultline);
      item->resultline = NULL;
      free (item->scratch_buffer);
      item->scratch_buffer = NULL;
      return FALSE;
    }

    //Reset internal variables
    item->current_line = 0;
    item->progress_feedback_line = 0;
    item->total_relevant_results = 0;
    item->total_irrelevant_results = 0;
    item->total_results = 0;
    item->thread = NULL;

    if (item->fd == NULL)
    {
      char *path = lw_dictinfo_get_uri (item->dictionary);
      item->fd = fopen (path, "r");
      g_free (path);
      path = NULL;
    }
    item->status = GW_SEARCH_SEARCHING;
    return TRUE;
}


//!
//! @brief Cleanups after a search completes
//!
//! The file descriptior is closed, various variables are
//! reset, and the search status is set to IDLE.
//!
//! @param item The LwSearchItem to its state reset.
//!
void lw_searchitem_do_post_search_clean (LwSearchItem* item)
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
    if (item->backup_resultline != NULL)
    {
      lw_resultline_free (item->backup_resultline);
      item->backup_resultline = NULL;
    }

    //item->thread = NULL;  This code creates multithreading problems
    item->status = GW_SEARCH_IDLE;
}


//!
//! @brief Releases a LwSearchItem object from memory. 
//!
//! All of the various interally allocated memory in the LwSearchItem is freed.
//! The file descriptiors and such are made sure to also be closed.
//!
//! @param item The LwSearchItem to have it's memory freed.
//!
void lw_searchitem_free (LwSearchItem* item)
{
  if (item == NULL) return;

  if (item->thread != NULL) 
  {
    item->status = GW_SEARCH_CANCELING;
    g_thread_join(item->thread);
    item->thread = NULL;
    g_mutex_free (item->mutex);
    item->mutex = NULL;
  }
  lw_searchitem_do_post_search_clean (item);
  lw_queryline_free (item->queryline);
  free (item);
  item = NULL;
}


static gboolean _edict_existance_comparison (LwQueryLine *ql, LwResultLine *rl, const LwRelevance RELEVANCE)
{
    //Declarations
    int i;
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
    }
    if (ql->re_kanji[0][RELEVANCE] != NULL && *iter == NULL) return TRUE;

    //Compare furigana atoms
    if (rl->furigana_start != NULL)
    {
      for (iter = ql->re_furi; *iter != NULL && **iter != NULL; iter++)
      {
        re = (*iter)[RELEVANCE];
        if (g_regex_match (re, rl->furigana_start, 0, NULL) == FALSE) break;
      }
    }
    if (ql->re_furi[0][RELEVANCE] != NULL && *iter == NULL) return TRUE;

    if (rl->kanji_start != NULL)
    {
      for (iter = ql->re_furi; *iter != NULL && **iter != NULL; iter++)
      {
        re = (*iter)[RELEVANCE];
        if (g_regex_match (re, rl->kanji_start, 0, NULL) == FALSE) break;
      }
    }
    if (ql->re_furi[0][RELEVANCE] != NULL && *iter == NULL) return TRUE;


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
//!
//! @param item A LwSearchItem to get search information from
//! @param RELEVANCE A LwRelevance
//!
gboolean lw_searchitem_run_comparison (LwSearchItem *item, const LwRelevance RELEVANCE)
{
    //Declarations
    LwResultLine *rl;
    LwQueryLine *ql;

    //Initializations
    rl = item->resultline;
    ql = item->queryline;

    //Kanji radical dictionary search
    int i = 0;
    switch (item->dictionary->engine)
    {
      case GW_ENGINE_EDICT:
        return _edict_existance_comparison (ql, rl, RELEVANCE);
      case GW_ENGINE_KANJI:
        return _kanji_existance_comparison (ql, rl, RELEVANCE);
      case GW_ENGINE_EXAMPLES:
        return _edict_existance_comparison (ql, rl, RELEVANCE);
      default:
        return _edict_existance_comparison (ql, rl, RELEVANCE);
    }
}


//!
//! @brief comparison function for determining if two LwSearchItems are equal
//!
gboolean lw_searchitem_is_equal (LwSearchItem *item1, LwSearchItem *item2)
{
  //Declarations
  gboolean queries_are_equal;
  gboolean dictionaries_are_equal;
  //Sanity checks
  if (item1 == item2) return TRUE;
  if (item1 == NULL) return FALSE;
  if (item2 == NULL) return FALSE;

  g_mutex_lock (item1->mutex);
  g_mutex_lock (item2->mutex);

  //Initializations
  queries_are_equal = (strcmp(item1->queryline->string, item2->queryline->string) == 0);
  dictionaries_are_equal = (item1->dictionary == item2->dictionary);

  g_mutex_unlock (item1->mutex);
  g_mutex_unlock (item2->mutex);

  return (queries_are_equal && dictionaries_are_equal);
}


//!
//! @brief a method for incrementing an internal integer for determining if a result set has worth
//!
void lw_searchitem_increment_history_relevance_timer (LwSearchItem *item)
{
  if (item != NULL && item->history_relevance_idle_timer < GW_HISTORY_TIME_TO_RELEVANCE)
    item->history_relevance_idle_timer++;
}


//!
//! @brief Checks if the relevant timer has passed a threshold
//!
gboolean lw_searchitem_has_history_relevance (LwSearchItem *item)
{
  return (item != NULL && item->total_results && item->history_relevance_idle_timer >= GW_HISTORY_TIME_TO_RELEVANCE);
}
