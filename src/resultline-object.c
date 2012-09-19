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
//!  @file src/resultline-object.c
//!
//!  @brief Management of result lines
//!

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>

#include <glib.h>

#include <gwaei/definitions.h>
#include <gwaei/regex.h>
#include <gwaei/resultline-object.h>



GwResultLine* gw_resultline_new ()
{
    GwResultLine* temp;

    if ((temp = (GwResultLine*) malloc(sizeof(GwResultLine))) == NULL) return NULL;

    //A place for a copy of the raw string
    temp->string[0] = '\0';
    
    //General formatting
    temp->def_start[0] = NULL;
    temp->def_total = 0;
    temp->kanji_start = NULL;
    temp->furigana_start = NULL;
    temp->classification_start = NULL;
    temp->important = FALSE;

    //Kanji things
    temp->strokes = NULL;
    temp->frequency = NULL;
    temp->readings[0] = NULL;
    temp->readings[1] = NULL;
    temp->readings[2] = NULL;
    temp->meanings = NULL;
    temp->grade = NULL;
    temp->jlpt = NULL;
    temp->kanji = NULL;
    temp->radicals = NULL;

    return temp;
}

void gw_resultline_clear_variables (GwResultLine *temp)
{
    //A place for a copy of the raw string
    temp->string[0] = '\0';
    
    //General formatting
    temp->def_start[0] = NULL;
    temp->def_total = 0;
    temp->kanji_start = NULL;
    temp->furigana_start = NULL;
    temp->classification_start = NULL;
    temp->important = FALSE;

    //Kanji things
    temp->strokes = NULL;
    temp->frequency = NULL;
    temp->readings[0] = NULL;
    temp->readings[1] = NULL;
    temp->readings[2] = NULL;
    temp->meanings = NULL;
    temp->grade = NULL;
    temp->jlpt = NULL;
    temp->kanji = NULL;
    temp->radicals = NULL;
}

void gw_resultline_free (GwResultLine *item)
{
    free (item);
}


//!
//! @brief Parses a string for a Edict format string
//!
//! String parsing for the Jim Breen Edict dictionary.
//!
//! @param line line
//! @param string string
//!
void gw_resultline_parse_edict_result_string (GwResultLine *rl)
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
//!
//! String parsing for the Jim Breen Kanji dictionary.  It also supports the
//! gWaei custom Mix dictionary.
//!
//! @param line line
//! @param string string
//!
void gw_resultline_parse_kanjidict_result_string (GwResultLine *rl)
{
    size_t nmatch = 1;
    regmatch_t pmatch[nmatch];
    enum temp_enum { STROKES, FREQUENCY, GRADE, JLPT, LENGTH };
    char *end[LENGTH];
    gboolean found[LENGTH];
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
    if (found[STROKES] = (regexec(gw_re[GW_RE_QUERY_STROKES], ptr, nmatch, pmatch, 0) == 0))
    {
      rl->strokes = ptr + pmatch[0].rm_so + 1;
      end[STROKES] = ptr + pmatch[0].rm_eo;
    }

    //Get frequency
    rl->frequency = NULL;
    if (found[FREQUENCY] = (regexec(gw_re[GW_RE_QUERY_FREQUENCY], ptr, nmatch, pmatch, 0) == 0))
    {
      rl->frequency = ptr + pmatch[0].rm_so + 1;
      end[FREQUENCY] = ptr + pmatch[0].rm_eo;
    }

    //Get grade
    rl->grade = NULL;
    if (found[GRADE] = (regexec(gw_re[GW_RE_QUERY_GRADE], ptr, nmatch, pmatch, 0) == 0))
    {
      rl->grade = ptr + pmatch[0].rm_so + 1;
      end[GRADE] = ptr + pmatch[0].rm_eo;
    }

    //Get JLPT
    rl->jlpt = NULL;
    if (found[JLPT] = (regexec(gw_re[GW_RE_QUERY_JLPT], ptr, nmatch, pmatch, 0) == 0))
    {
      rl->jlpt = ptr + pmatch[0].rm_so + 1;
      end[JLPT] = ptr + pmatch[0].rm_eo;
    }


    //Get the kanji character
    rl->kanji = ptr;
    while (g_utf8_get_char(ptr) != L' ') {
      ptr = g_utf8_next_char(ptr);
    }
    *ptr = '\0';
    ptr++;

    //Test if the radicals information is present
    if(g_utf8_get_char(ptr) > 3040)
    {
      rl->radicals = ptr;
      while((g_utf8_get_char(ptr) > 3040 || g_utf8_get_char(ptr) == L' '))
      {
        ptr = g_utf8_next_char(ptr);
      }
      *(ptr - 1) = '\0';
    }
    else
      rl->radicals = NULL;

    //Go to the readings section
    while (g_utf8_get_char(ptr) < 3041 && *ptr != '\0')
      ptr = g_utf8_next_char (ptr);
    rl->readings[0] = ptr;

    //Copy the rest of the data
    char *next = ptr;
    while (*ptr != '\0' && (next = g_utf8_next_char(ptr)) != NULL && g_utf8_get_char(next) != '{')
    {
      //The strange T1 character between kana readings
      if (g_utf8_get_char (ptr) == L'T' && g_utf8_get_char(next) == L'1') {
        *(ptr - 1) = '\0';
        rl->readings[1] = next + 2;
      }
      //The strange T2 character between kana readings
      if (g_utf8_get_char (ptr) == L'T' && g_utf8_get_char(next) == L'2') {
        *(ptr - 1) = '\0';
        rl->readings[2] = next + 2;
      }
      ptr = next;
    }
    *ptr = '\0';
    rl->meanings = next;

    ptr++;
    if ((ptr = g_utf8_strrchr (ptr, -1, '\n')))
      *ptr = '\0';

    if (found[STROKES])
      *end[STROKES] = '\0';
    if (found[FREQUENCY])
      *end[FREQUENCY] = '\0';
    if (found[GRADE])
      *end[GRADE] = '\0';
    if (found[JLPT])
      *end[JLPT] = '\0';
}


//!
//! @brief Parses a string for an example format string
//!
//! String parsing for the Jim Breen Example dictionaries.
//!
//! @param line line
//! @param string string
//!
void gw_resultline_parse_examplesdict_result_string (GwResultLine *rl)
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
    char *eraser = NULL;
    int i = 0;

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
//!
//! This is the fallback format for user installed unknown dictionaries. Should be generally
//! compatible with anything.
//!
//! @param line line
//! @param string string
//!
void gw_resultline_parse_unknowndict_result_string (GwResultLine *rl)
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

    char *temp = NULL;
    if (temp = g_utf8_strchr (rl->string, -1, L'\n'))
    {
      *temp = '\0';
    }

    rl->def_start[0] = rl->string;
    rl->def_start[1] = NULL;
    rl->def_total = 1;
    rl->kanji_start = rl->string;
    rl->furigana_start = rl->string;
}

