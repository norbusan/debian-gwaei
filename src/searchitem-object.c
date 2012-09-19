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
#include <regex.h>

#include <glib.h>

#include <gwaei/definitions.h>
#include <gwaei/utilities.h>
#include <gwaei/dictionary-objects.h>
#include <gwaei/search-objects.h>
#include <gwaei/preferences.h>

#include <gwaei/main.h>


//!
//! @brief Creates a new GwSearchItem object. 
//!
//! Takes the query and parses it according to the dictionary and TARGET give
//! to it.  Searchitem also stores various variables such as the file
//! it uses and the tallied results.
//!
//! @param query The text to be search for
//! @param dictionary The GwDictInfo object to use
//! @param TARGET The widget to output the results to
//! @return Returns an allocated GwSearchItem object
//!
GwSearchItem* gw_searchitem_new (char* query, GwDictInfo* dictionary, const int TARGET)
{
    GwSearchItem *temp;

    //Allocate some memory
    if ((temp = malloc(sizeof(GwSearchItem))) == NULL) return NULL;

    temp->results_medium = NULL;
    temp->results_low = NULL;
    
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
    temp->queryline = gw_queryline_new ();

    char *key = GCKEY_GW_LESS_RELEVANT_SHOW; 
    temp->show_less_relevant_results = gw_pref_get_boolean (key, TRUE);

    if (gw_main_verify_output_generic_functions () == FALSE)
    {
      printf("ERROR: The interface hasn't set all the output functions. Please try a different interface.\n");
      exit (EXIT_FAILURE);
    }


    //Set function pointers
    switch (temp->dictionary->type)
    {
        case GW_DICT_TYPE_EDICT:
          if (!gw_queryline_parse_edict_string (temp->queryline, query)) return NULL;
          temp->gw_searchitem_parse_result_string = &gw_resultline_parse_edict_result_string;
          temp->gw_searchitem_ui_append_results_to_output = gw_output_generic_append_edict_results;
          break;
        case GW_DICT_TYPE_KANJI:
          if (!gw_queryline_parse_kanjidict_string (temp->queryline, query)) return NULL;
          temp->gw_searchitem_parse_result_string = &gw_resultline_parse_kanjidict_result_string;
          temp->gw_searchitem_ui_append_results_to_output = gw_output_generic_append_kanjidict_results;
          break;
        case GW_DICT_TYPE_EXAMPLES:
          if (!gw_queryline_parse_exampledict_string (temp->queryline, query)) return NULL;
          temp->gw_searchitem_parse_result_string = &gw_resultline_parse_examplesdict_result_string;
          temp->gw_searchitem_ui_append_results_to_output = gw_output_generic_append_examplesdict_results;
        break;
        default:
          if (!gw_queryline_parse_edict_string (temp->queryline, query)) return NULL;
          temp->gw_searchitem_parse_result_string = &gw_resultline_parse_unknowndict_result_string;
          temp->gw_searchitem_ui_append_results_to_output = gw_output_generic_append_unknowndict_results;
          break;
    }
    temp->gw_searchitem_ui_update_progress_feedback = gw_output_generic_update_progress_feedback;
    temp->gw_searchitem_ui_append_less_relevant_header_to_output = gw_output_generic_append_less_relevant_header_to_output;
    temp->gw_searchitem_ui_append_more_relevant_header_to_output = gw_output_generic_append_more_relevant_header_to_output;
    temp->gw_searchitem_ui_pre_search_prep = gw_output_generic_pre_search_prep;
    temp->gw_searchitem_ui_after_search_cleanup = gw_output_generic_after_search_cleanup;

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
//! @param item The GwSearchItem to its variables prepared
//! @return Returns false on seachitem prep failure.
//!
gboolean gw_searchitem_do_pre_search_prep (GwSearchItem* item)
{
    if (item->scratch_buffer != NULL || (item->scratch_buffer = malloc (MAX_LINE)) == NULL)
    {
      return FALSE;
    }
    if (item->resultline != NULL || (item->resultline = gw_resultline_new ()) == NULL)
    {
      free (item->scratch_buffer);
      item->scratch_buffer = NULL;
      return FALSE;
    }
    if (item->backup_resultline != NULL || (item->backup_resultline = gw_resultline_new ()) == NULL)
    {
      gw_resultline_free (item->resultline);
      item->resultline = NULL;
      free (item->scratch_buffer);
      item->scratch_buffer = NULL;
      return FALSE;
    }

    //Reset internal variables
    item->current_line = 0;
    item->previous_line = 0;
    item->total_relevant_results = 0;
    item->total_irrelevant_results = 0;
    item->total_results = 0;
    item->previous_total_results = 0;

    if (item->fd == NULL)
      item->fd = fopen ((item->dictionary)->path, "r");
    item->status = GW_SEARCH_SEARCHING;
    return TRUE;
}


//!
//! @brief Cleanups after a search completes
//!
//! The file descriptior is closed, various variables are
//! reset, and the search status is set to IDLE.
//!
//! @param item The GwSearchItem to its state reset.
//!
void gw_searchitem_do_post_search_clean (GwSearchItem* item)
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
      gw_resultline_free (item->resultline);
      item->resultline = NULL;
    }
    if (item->backup_resultline != NULL)
    {
      gw_resultline_free (item->backup_resultline);
      item->backup_resultline = NULL;
    }

    item->status = GW_SEARCH_IDLE;
}


//!
//! @brief Releases a GwSearchItem object from memory. 
//!
//! All of the various interally allocated memory in the GwSearchItem is freed.
//! The file descriptiors and such are made sure to also be closed.
//!
//! @param item The GwSearchItem to have it's memory freed.
//!
void gw_searchitem_free (GwSearchItem* item)
{
  gw_searchitem_do_post_search_clean (item);
  free (item->queryline);
  free (item);
  item = NULL;
}


//!
//! @brief Comparison function that should be moved to the GwSearchItem file when it matures
//!
//! @param item A GwSearchItem to get search information from
//! @param REGEX_TYPE A constant int representing the regex type to test
//!
gboolean gw_searchitem_existance_generic_comparison (GwSearchItem *item, const int REGEX_TYPE)
{
    GwResultLine *rl;
    GwQueryLine *ql;
    rl = item->resultline;
    ql = item->queryline;

    //Kanji radical dictionary search
    int i = 0;
    if (item->dictionary->type == GW_DICT_TYPE_KANJI || item->dictionary->type == GW_DICT_TYPE_RADICALS)
    {
      if (item->dictionary->type == GW_DICT_TYPE_RADICALS && ql->roma_total > 0) return FALSE;

      gboolean strokes_check_passed = TRUE;
      gboolean frequency_check_passed = TRUE;
      gboolean grade_check_passed = TRUE;
      gboolean jlpt_check_passed = TRUE;
      gboolean romaji_high_check_passed = TRUE;
      gboolean romaji_check_passed = TRUE;
      gboolean furigana_check_passed = TRUE;
      gboolean kanji_check_passed = TRUE;
      gboolean radical_check_passed = TRUE;

      //Calculate the strokes check
      if (ql->strokes_total > 0)
      {
        if (rl->strokes == NULL || regexec(&(ql->strokes_regex[REGEX_TYPE][i]), rl->strokes, 1, NULL, 0) != 0)
          strokes_check_passed = FALSE;
      }

      //Calculate the frequency check
      if (ql->frequency_total > 0)
      {
        if (rl->frequency == NULL || regexec(&(ql->frequency_regex[REGEX_TYPE][i]), rl->frequency, 1, NULL, 0) != 0)
          frequency_check_passed = FALSE;
      }

      //Calculate the grade check
      if (ql->grade_total > 0)
      {
        if (rl->grade == NULL || regexec(&(ql->grade_regex[REGEX_TYPE][i]), rl->grade, 1, NULL, 0) != 0)
          grade_check_passed = FALSE;
      }

      //Calculate the jlpt check
      if (ql->jlpt_total > 0)
      {
        if (rl->jlpt == NULL || regexec(&(ql->jlpt_regex[REGEX_TYPE][i]), rl->jlpt, 1, NULL, 0) != 0)
          jlpt_check_passed = FALSE;
      }

      //Calculate the romaji check
      if (ql->roma_total > 0 && rl->meanings != NULL)
      {
        if (regexec(&(ql->roma_regex[GW_QUERYLINE_HIGH][i]), rl->meanings, 1, NULL, 0) != 0)
          romaji_high_check_passed = FALSE;
      }

      //Calculate the romaji check
      if (ql->roma_total > 0 && rl->meanings != NULL)
      {
        if (regexec(&(ql->roma_regex[REGEX_TYPE][i]), rl->meanings, 1, NULL, 0) != 0)
          romaji_check_passed = FALSE;
      }

      //Calculate the furigana check
      if (ql->furi_total > 0 && rl->readings[0] != NULL)
      {
        if (regexec(&(ql->furi_regex[REGEX_TYPE][i]), rl->readings[0], 1, NULL, 0) != 0)
          furigana_check_passed = FALSE;
      }

      //Calculate the kanji check
      for (i = 0; i < ql->kanji_total && rl->kanji != NULL; i++)
      {
        if (regexec(&(ql->kanji_regex[REGEX_TYPE][i]), rl->kanji, 1, NULL, 0) != 0)
          kanji_check_passed = FALSE;
      }

      //Calculate the radical check
      if (rl->radicals == NULL)
      {
        radical_check_passed = FALSE;
      }
      for (i = 0; i < ql->kanji_total && radical_check_passed; i++)
      {
        if (regexec(&(ql->kanji_regex[REGEX_TYPE][i]), rl->radicals, 1, NULL, 0) != 0)
          radical_check_passed = FALSE;
      }

      //Return our results
      if (REGEX_TYPE == GW_QUERYLINE_HIGH)
      {
        return (kanji_check_passed && romaji_high_check_passed);
      }
      else
      {
        return (strokes_check_passed &&
                frequency_check_passed &&
                grade_check_passed &&
                jlpt_check_passed &&
                romaji_check_passed &&
                furigana_check_passed &&
                (radical_check_passed | kanji_check_passed));
      }
    }
    //Standard dictionary search
    else
    {
      int i;
      int j;
      //Compare kanji atoms
      i = 0;
      while (i < ql->kanji_total && rl->kanji_start != NULL)
      {
        if (regexec(&(ql->kanji_regex[REGEX_TYPE][i]), rl->kanji_start, 1, NULL, 0) != 0)
	  break;
        i++;  
      }
      if (i > 0 && i == ql->kanji_total) return TRUE;

      //Compare furigana atoms
      i = 0;
      while (i < ql->furi_total && rl->furigana_start != NULL)
      {
        if (regexec(&(ql->furi_regex[REGEX_TYPE][i]), rl->furigana_start, 1, NULL, 0) != 0)
	  break;
        i++;  
      }
      if (i > 0 && i == ql->furi_total) return TRUE;

      //Compare furigana atoms
      i = 0;
      while (i < ql->furi_total && rl->kanji_start != NULL && rl->furigana_start == NULL)
      {
        if (regexec(&(ql->furi_regex[REGEX_TYPE][i]), rl->kanji_start, 1, NULL, 0) != 0)
	  break;
        i++;  
      }
      if (i > 0 && i == ql->furi_total) return TRUE;

      //Compare romaji atoms
      j = 0;
      while (rl->def_start[j] != NULL)
      {
        i = 0;
        while (i < ql->roma_total)
        {
          if (regexec(&(ql->roma_regex[REGEX_TYPE][i]), rl->def_start[j], 1, NULL, 0) != 0)
	    break;
          i++;
        }
      	if (i > 0 && i == ql->roma_total) return TRUE;
        j++;  
      }

      //Compare word classification atoms
      i = 0;
      while (i < ql->roma_total && rl->classification_start != NULL)
      {
        if (regexec(&(ql->roma_regex[REGEX_TYPE][i]), rl->classification_start, 1, NULL, 0) != 0)
          break;
        i++;  
      }
      if (i > 0 && i == ql->roma_total) return TRUE;

      return FALSE;

      //Compare mix atoms
      i = 0;
      while (i < ql->mix_total && rl->string != NULL)
      {
        if (regexec(&(ql->mix_regex[REGEX_TYPE][i]), rl->string, 1, NULL, 0) == 0)
          return TRUE;
        i++;  
      }

      return FALSE;
    }
}

