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
//!  @file resultline.c
//!


#include "../private.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <glib.h>

#include <libwaei/libwaei.h>

static char *FIRST_DEFINITION_PREFIX_STR = "(1)";



//!
//! @brief Creates a new LwResultLine object
//! @return An allocated LwResultLine that will be needed to be freed by lw_resultline_free.
//!
LwResultLine* 
lw_resultline_new ()
{
    LwResultLine* temp;

    temp = (LwResultLine*) malloc(sizeof(LwResultLine));

    if (temp != NULL)
    {
      lw_resultline_init (temp);
    }

    return temp;
}

//!
//! @brief Releases a LwResultLine object from memory.
//! @param rl A LwResultLine object created by lw_resultline_new.
//!
void 
lw_resultline_free (LwResultLine *rl)
{
    lw_resultline_deinit (rl);
    free (rl);
}


//!
//! @brief Used to initialize the memory inside of a new LwDictInfo
//!        object.  Usually lw_dictinfo_new calls this for you.  It is also 
//!        used in class implimentations that extends LwDictInfo.
//! @param rl The LwResultline to initialize the memory of
//!
void 
lw_resultline_init (LwResultLine *rl)
{
    //A place for a copy of the raw string
    rl->string[0] = '\0';

    rl->relevance = LW_RESULTLINE_RELEVANCE_UNSET;
    
    //General formatting
    rl->def_start[0] = NULL;
    rl->def_total = 0;
    rl->kanji_start = NULL;
    rl->furigana_start = NULL;
    rl->classification_start = NULL;
    rl->important = FALSE;

    //Kanji things
    rl->strokes = NULL;
    rl->frequency = NULL;
    rl->readings[0] = NULL;
    rl->readings[1] = NULL;
    rl->readings[2] = NULL;
    rl->meanings = NULL;
    rl->grade = NULL;
    rl->jlpt = NULL;
    rl->kanji = NULL;
    rl->radicals = NULL;
}


//!
//! @brief Used to free the memory inside of a LwResultLine object.
//!         Usually lw_dictinfo_free calls this for you.  It is also used
//!         in class implimentations that extends LwResultLine.
//! @param rl The LwResultLine object to have it's inner memory freed.
//!
void 
lw_resultline_deinit (LwResultLine *rl)
{
}


//!
//! @brief Parses a string for a Edict format string
//! @param rl The Resultline object this method works on
//!
void 
lw_resultline_parse_edict_result_string (LwResultLine *rl)
{
    //Reinitialize Variables to help prevent craziness
    rl->def_start[0] = NULL;
    rl->def_total = 0;
    rl->kanji_start = NULL;
    rl->furigana_start = NULL;
    rl->classification_start = NULL;
    rl->important = FALSE;
    rl->strokes = NULL;
    rl->frequency = NULL;
    rl->readings[0] = NULL;
    rl->readings[1] = NULL;
    rl->readings[2] = NULL;
    rl->meanings = NULL;
    rl->grade = NULL;
    rl->jlpt = NULL;
    rl->kanji = NULL;
    rl->radicals = NULL;

    char *ptr = rl->string;
    char *next = NULL;
    char *nextnext = NULL;
    char *nextnextnext = NULL;
    char *temp = NULL;

    //Remove the final line break
    if ((temp = g_utf8_strchr (rl->string, -1, '\n')) != NULL)
    {
        temp--;
        *temp = '\0';
    }

    //Set the kanji pointers
    rl->kanji_start = ptr;
    ptr = g_utf8_strchr (ptr, -1, L' ');
    *ptr = '\0';

    //Set the furigana pointer
    ptr++;
    if (g_utf8_get_char(ptr) == L'[' && g_utf8_strchr (ptr, -1, L']') != NULL)
    {
      ptr = g_utf8_next_char(ptr);
      rl->furigana_start = ptr;
      ptr = g_utf8_strchr (ptr, -1, L']');
      *ptr = '\0';
    }
    else
    {
      rl->furigana_start = NULL;
      ptr--;
    }

    //Find if there is a type description classification
    temp = ptr;
    temp++;
    temp = g_utf8_strchr (temp, -1, L'/');
    if (temp != NULL && g_utf8_get_char(temp + 1) == L'(')
    {
      rl->classification_start = temp + 2;
      temp = g_utf8_strchr (temp, -1, L')');
      *temp = '\0';
      ptr = temp;
    }

    //Set the definition pointers
    ptr++;
    ptr = g_utf8_next_char(ptr);
    rl->def_start[0] = ptr;
    rl->number[0] = FIRST_DEFINITION_PREFIX_STR;
    int i = 1;

    temp = ptr;
    while ((temp = g_utf8_strchr(temp, -1, L'(')) != NULL && i < 50)
    {
      next = g_utf8_next_char (temp);
      nextnext = g_utf8_next_char (next);
      nextnextnext = g_utf8_next_char (nextnext);
      if (*next != '\0' && *nextnext != '\0' &&
          *next == L'1' && *nextnext == L')')
      {
         rl->def_start[0] = rl->def_start[0] + 4;
      }
      else if (*next != '\0' && *nextnext != '\0' && *nextnextnext != '\0' &&
               *next >= L'1' && *next <= L'9' && (*nextnext == L')' || *nextnextnext == L')'))
      {
         *(temp - 1) = '\0';
         rl->number[i] = temp;
         temp = g_utf8_strchr (temp, -1, L')');
         *(temp + 1) = '\0';
         rl->def_start[i] = temp + 2;
         i++;
      }
      temp = temp + 2;
    }
    rl->def_total = i;
    rl->def_start[i] = NULL;
    rl->number[i] = NULL;
    i--;

    //Get the importance
    //temp = rl->def_start[i] + strlen(rl->def_start[i]) - 4;
    if ((temp = g_utf8_strrchr (rl->def_start[i], -1, L'(')) != NULL)
    {
      rl->important = (*temp == '(' && *(temp + 1) == 'P' && *(temp + 2) == ')');
      if (rl->important) 
      {
        *(temp - 1) = '\0';
      }
    }
}


//!
//! @brief Parses a string for a Kanjidic format string
//! @param rl The Resultline object this method works on
//!
void 
lw_resultline_parse_kanjidict_result_string (LwResultLine *rl)
{
    GMatchInfo* match_info;
    int start[LW_RE_TOTAL];
    int end[LW_RE_TOTAL];
    GUnicodeScript script;
    char *ptr = rl->string;

    //Reinitialize Variables to help prevent craziness
    rl->def_start[0] = NULL;
    rl->def_total = 0;
    rl->kanji_start = NULL;
    rl->furigana_start = NULL;
    rl->classification_start = NULL;
    rl->important = FALSE;
    rl->readings[0] = NULL;
    rl->readings[1] = NULL;
    rl->readings[2] = NULL;
    rl->meanings = NULL;
    rl->kanji = NULL;
    rl->radicals = NULL;

    //First generate the grade, stroke, frequency, and jlpt fields

    //Get strokes
    rl->strokes = NULL;
    g_regex_match (lw_re[LW_RE_STROKES], ptr, 0, &match_info);
    if (g_match_info_matches (match_info))
    {
      g_match_info_fetch_pos (match_info, 0, &start[LW_RE_STROKES], &end[LW_RE_STROKES]);
      rl->strokes = ptr + start[LW_RE_STROKES] + 1;
    }
    g_match_info_free (match_info);


    //Get frequency
    rl->frequency = NULL;
    g_regex_match (lw_re[LW_RE_FREQUENCY], ptr, 0, &match_info);
    if (g_match_info_matches (match_info))
    {
      g_match_info_fetch_pos (match_info, 0, &start[LW_RE_FREQUENCY], &end[LW_RE_FREQUENCY]);
      rl->frequency = ptr + start[LW_RE_FREQUENCY] + 1;
    }
    g_match_info_free (match_info);


    //Get grade level
    rl->grade = NULL;
    g_regex_match (lw_re[LW_RE_GRADE], ptr, 0, &match_info);
    if (g_match_info_matches (match_info))
    {
      g_match_info_fetch_pos (match_info, 0, &start[LW_RE_GRADE], &end[LW_RE_GRADE]);
      rl->grade = ptr + start[LW_RE_GRADE] + 1;
    }
    g_match_info_free (match_info);


    //Get JLPT level
    rl->jlpt = NULL;
    g_regex_match (lw_re[LW_RE_JLPT], ptr, 0, &match_info);
    if (g_match_info_matches (match_info))
    {
      g_match_info_fetch_pos (match_info, 0, &start[LW_RE_JLPT], &end[LW_RE_JLPT]);
      rl->jlpt = ptr + start[LW_RE_JLPT] + 1;
    }
    g_match_info_free (match_info);


    //Get the kanji character
    rl->kanji = ptr;
    ptr = g_utf8_strchr (ptr, -1, g_utf8_get_char (" "));
    if (ptr == NULL)
    {
      fprintf(stderr, "This dictionary is incorrectly formatted\n");
      exit (1);
    }
    *ptr = '\0';
    ptr++;

    //Test if the radicals information is present
    rl->radicals = NULL;
    script = g_unichar_get_script (g_utf8_get_char (ptr));
    if (script != G_UNICODE_SCRIPT_LATIN)
    {
      rl->radicals = ptr;
      ptr = g_utf8_next_char (ptr);
      script = g_unichar_get_script (g_utf8_get_char (ptr));
      while (*ptr == ' ' || (script != G_UNICODE_SCRIPT_LATIN && script != G_UNICODE_SCRIPT_COMMON))
      {
        ptr = g_utf8_next_char(ptr);
        script = g_unichar_get_script (g_utf8_get_char (ptr));
      }
      *(ptr - 1) = '\0';
    }

    //Go to the readings section
    script = g_unichar_get_script (g_utf8_get_char(ptr));
    while (script != G_UNICODE_SCRIPT_KATAKANA && script != G_UNICODE_SCRIPT_HIRAGANA && *ptr != '\0')
    {
      ptr = g_utf8_next_char (ptr);
      script = g_unichar_get_script (g_utf8_get_char(ptr));
    }
    rl->readings[0] = ptr;

    //Copy the rest of the data
    while (*ptr != '\0' && *ptr != '{')
    {
      //The strange T1 character between kana readings
      if (g_utf8_get_char (ptr) == g_utf8_get_char ("T")) {
        ptr = g_utf8_next_char (ptr);
        if (g_utf8_get_char (ptr) == g_utf8_get_char ("1"))
        {
          *(ptr - 1) = '\0';
          ptr = g_utf8_next_char (ptr);
          ptr = g_utf8_next_char (ptr);
          rl->readings[1] = ptr;
        }
        else if (g_utf8_get_char (ptr) == g_utf8_get_char ("2"))
        {
          *(ptr - 1) = '\0';
          ptr = g_utf8_next_char (ptr);
          ptr = g_utf8_next_char (ptr);
          rl->readings[2] = ptr;
        }
      }
      else
      {
        ptr = g_utf8_next_char (ptr);
      }
    }
    *(ptr - 1) = '\0';

    rl->meanings = ptr;

    if ((ptr = g_utf8_strrchr (ptr, -1, g_utf8_get_char ("\n"))) != NULL)
      *ptr = '\0';

    if (rl->strokes)   *(rl->string + end[LW_RE_STROKES]) = '\0';
    if (rl->frequency) *(rl->string + end[LW_RE_FREQUENCY]) = '\0';
    if (rl->grade)     *(rl->string + end[LW_RE_GRADE]) = '\0';
    if (rl->jlpt)      *(rl->string + end[LW_RE_JLPT]) = '\0';
}


//!
//! @brief Parses a string for an example format string
//! @param rl The Resultline object this method works on
//!
void 
lw_resultline_parse_examplesdict_result_string (LwResultLine *rl)
{
    //Reinitialize Variables to help prevent craziness
    rl->def_start[0] = NULL;
    rl->def_total = 0;
    rl->kanji_start = NULL;
    rl->furigana_start = NULL;
    rl->classification_start = NULL;
    rl->important = FALSE;
    rl->strokes = NULL;
    rl->frequency = NULL;
    rl->readings[0] = NULL;
    rl->readings[1] = NULL;
    rl->readings[2] = NULL;
    rl->meanings = NULL;
    rl->grade = NULL;
    rl->jlpt = NULL;
    rl->kanji = NULL;
    rl->radicals = NULL;

    //First generate the grade, stroke, frequency, and jplt fields
    rl->kanji = rl->string;

    char *temp = NULL;

    //Normal Japanese:    B:日本語[tab]English:B:読み解説
    temp = rl->string;
    if ((temp = strstr(temp, "A:")) != NULL)
    {
      *temp = '\0';
      temp++;
      *temp = '\0';
      temp++;
      *temp = '\0';
      temp++;
      rl->kanji_start = temp;
    }
    else
    {
      rl->kanji_start = NULL;
      temp = rl->string;
    }

    //English explanation:    A:日本語[tab]English:B:読み解説
    if ((temp = strstr(temp, "\t")) != NULL)
    {
      *temp = '\0';
      temp++;
      rl->def_start[0] = temp;
      rl->def_start[1] = NULL;
    }
    else
    {
      rl->def_start[0] = NULL;
      temp = rl->string;
    }

    //Explained Japanese:    日本語[tab]English:B:読み解説
    if ((temp = strstr(temp, ":B:")) != NULL)
    {
      *temp = '\0';
      temp++;
      *temp = '\0';
      temp++;
      *temp = '\0';
      temp++;
      *temp = '\0';
      temp++;
      rl->furigana_start = temp;
    }
    else
    {
      rl->furigana_start = NULL;
      temp = rl->string;
    }
    //Meh.  Deciding not to show this line from the dictionary for now.
    rl->furigana_start = NULL;
}


//!
//! @brief Parses a string for an unknown format string
//! @param rl The Resultline object this method works on
//!
void 
lw_resultline_parse_unknowndict_result_string (LwResultLine *rl)
{
}


gboolean 
lw_resultline_is_similar (LwResultLine *rl1, LwResultLine *rl2)
{
    //Declarations
    gboolean same_def_totals, same_first_def;

    if (rl1 == NULL || rl2 == NULL) return FALSE;

    //Initializations
    same_def_totals = (rl1->def_total == rl2->def_total);
    same_first_def = (strcmp(rl1->def_start[0], rl2->def_start[0]) == 0);

    return (same_first_def && same_def_totals);
}

