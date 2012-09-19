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
//! @file src/queryline-object.c
//!
//! @brief Currently unused preliminary query object
//!
//! The GwQueryLine object will be used for parsing
//! the query into token so comparisons can be more
//! intelligently handled.
//!

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>

#include <glib.h>

#include <gwaei/definitions.h>
#include <gwaei/regex.h>
#include <gwaei/utilities.h>
#include <gwaei/queryline-object.h>
#include <gwaei/preferences.h>

#define EFLAGS_EXIST    (REG_EXTENDED | REG_ICASE | REG_NOSUB)
#define EFLAGS_LOCATE   (REG_EXTENDED | REG_ICASE)

//Create the needed regex for searching and locating


GwQueryLine* gw_queryline_new ()
{
    GwQueryLine* temp;

    if ((temp = (GwQueryLine*) malloc(sizeof(GwQueryLine))) == NULL) return NULL;

    //A place for a copy of the raw string
    temp->string[0]      = '\0';
    temp->furi_total = 0;
    temp->kanji_total = 0;
    temp->roma_total = 0;
    temp->mix_total = 0;
    temp->strokes_total = 0;
    temp->frequency_total = 0;
    temp->grade_total = 0;
    temp->jlpt_total = 0;
    return temp;
}

void gw_queryline_free (GwQueryLine *ql)
{
    int i;
    for (i = 0; i < ql->furi_total; i++)
    {
      regfree(&(ql->furi_regex[GW_QUERYLINE_EXIST][i]));
      regfree(&(ql->furi_regex[GW_QUERYLINE_LOCATE][i]));
      regfree(&(ql->furi_regex[GW_QUERYLINE_HIGH][i]));
      regfree(&(ql->furi_regex[GW_QUERYLINE_MED][i]));
    }
    for (i = 0; i < ql->kanji_total; i++)
    {
      regfree(&(ql->kanji_regex[GW_QUERYLINE_EXIST][i]));
      regfree(&(ql->kanji_regex[GW_QUERYLINE_LOCATE][i]));
      regfree(&(ql->kanji_regex[GW_QUERYLINE_HIGH][i]));
      regfree(&(ql->kanji_regex[GW_QUERYLINE_MED][i]));
    }
    for (i = 0; i < ql->roma_total; i++)
    {
      regfree(&(ql->roma_regex[GW_QUERYLINE_EXIST][i]));
      regfree(&(ql->roma_regex[GW_QUERYLINE_LOCATE][i]));
      regfree(&(ql->roma_regex[GW_QUERYLINE_HIGH][i]));
      regfree(&(ql->roma_regex[GW_QUERYLINE_MED][i]));
    }
    for (i = 0; i < ql->mix_total; i++)
    {
      regfree(&(ql->mix_regex[GW_QUERYLINE_EXIST][i]));
      regfree(&(ql->mix_regex[GW_QUERYLINE_LOCATE][i]));
      regfree(&(ql->mix_regex[GW_QUERYLINE_HIGH][i]));
      regfree(&(ql->mix_regex[GW_QUERYLINE_MED][i]));
    }
    for (i = 0; i < ql->strokes_total; i++)
    {
      regfree(&(ql->strokes_regex[GW_QUERYLINE_EXIST][i]));
      regfree(&(ql->strokes_regex[GW_QUERYLINE_LOCATE][i]));
      regfree(&(ql->strokes_regex[GW_QUERYLINE_HIGH][i]));
      regfree(&(ql->strokes_regex[GW_QUERYLINE_MED][i]));
    }
    for (i = 0; i < ql->frequency_total; i++)
    {
      regfree(&(ql->frequency_regex[GW_QUERYLINE_EXIST][i]));
      regfree(&(ql->frequency_regex[GW_QUERYLINE_LOCATE][i]));
      regfree(&(ql->frequency_regex[GW_QUERYLINE_HIGH][i]));
      regfree(&(ql->frequency_regex[GW_QUERYLINE_MED][i]));
    }
    for (i = 0; i < ql->grade_total; i++)
    {
      regfree(&(ql->grade_regex[GW_QUERYLINE_EXIST][i]));
      regfree(&(ql->grade_regex[GW_QUERYLINE_LOCATE][i]));
      regfree(&(ql->grade_regex[GW_QUERYLINE_HIGH][i]));
      regfree(&(ql->grade_regex[GW_QUERYLINE_MED][i]));
    }
    for (i = 0; i < ql->jlpt_total; i++)
    {
      regfree(&(ql->jlpt_regex[GW_QUERYLINE_EXIST][i]));
      regfree(&(ql->jlpt_regex[GW_QUERYLINE_LOCATE][i]));
      regfree(&(ql->jlpt_regex[GW_QUERYLINE_HIGH][i]));
      regfree(&(ql->jlpt_regex[GW_QUERYLINE_MED][i]));
    }

    free (ql);
}


//!
//! @brief Parses a query using the edict style
//!
//! The program will (to be written)
//!
//! @param ql Pointer to a GwQueryLine object ot parse a query string into.
//! @param string constant string that is the raw query.
//!
int gw_queryline_parse_edict_string (GwQueryLine *ql, const char* string)
{
   //Make sure it isn't already created
   if (strcmp(ql->string, string) == 0) return TRUE;

   //Make a perminent copy of the query
   strncpy(ql->string, string, MAX_QUERY); 

   //Load the preference settings
   int rk_conv_pref = gw_pref_get_int (GCKEY_GW_ROMAN_KANA, 0);
   gboolean want_rk_conv = (rk_conv_pref == 0 || (rk_conv_pref == 2 && !gw_util_is_japanese_locale()));
   gboolean want_hk_conv;
   want_hk_conv = gw_pref_get_boolean (GCKEY_GW_HIRA_KATA, TRUE);
   gboolean want_kh_conv;
   want_kh_conv = gw_pref_get_boolean (GCKEY_GW_KATA_HIRA, TRUE);
   ql->hira_string[0] = '\0';

   //Create atoms
   int i = 0;
   char *generic_atoms[MAX_ATOMS];

   generic_atoms[i] = ql->string;
   while ((generic_atoms[i + 1] = g_utf8_strchr (generic_atoms[i], -1, L'&')) != NULL && i < MAX_ATOMS)
   {
     i++;
     *generic_atoms[i] = '\0';
     *generic_atoms[i]++;
   }
   generic_atoms[i + 1] = NULL;

   //Organize atoms
   i = 0;
   int kanji_pos = ql->kanji_total;
   int furi_pos  = ql->furi_total;
   int mix_pos   = ql->mix_total;
   int roma_pos  = ql->roma_total;
   char temp[300];
   char *temp_ptr = NULL;
   gboolean want_conv;

   while (generic_atoms[i] != NULL && i < MAX_ATOMS)
   {
     if (gw_util_is_kanji_ish_str (generic_atoms[i]) || gw_util_is_kanji_str (generic_atoms[i]))
     {
       strcpy(ql->hira_string, "(");
       strcat(ql->hira_string, generic_atoms[i]);
       strcat(ql->hira_string, ")");

       if (g_utf8_strlen (generic_atoms[i], -1) == 4 && gw_util_is_kanji_str (generic_atoms[i]))
       {
          g_utf8_strncpy (temp, generic_atoms[i], 2);
          temp_ptr = temp;
          temp_ptr = g_utf8_find_next_char (temp_ptr, NULL);
          temp_ptr = g_utf8_find_next_char (temp_ptr, NULL);
          *temp_ptr = '\0';

          strcat (ql->hira_string, "|");
          strcat(ql->hira_string, "(");
          strcat (ql->hira_string, temp);
          strcat(ql->hira_string, ")");
          strcat (ql->hira_string, "|");

          temp_ptr = generic_atoms[i];
          temp_ptr = g_utf8_find_next_char (temp_ptr, NULL);
          temp_ptr = g_utf8_find_next_char (temp_ptr, NULL);
          strcat(ql->hira_string, "(");
          strcat (ql->hira_string, temp_ptr);
          strcat(ql->hira_string, ")");
       }

       if (regcomp (&(ql->kanji_regex                         [GW_QUERYLINE_EXIST] [kanji_pos]), ql->hira_string, EFLAGS_EXIST)  != 0) return FALSE;
       if (regcomp (&(ql->kanji_regex                         [GW_QUERYLINE_LOCATE][kanji_pos]), ql->hira_string, EFLAGS_LOCATE) != 0) return FALSE;
       if (gw_regex_create_kanji_high_regex (&(ql->kanji_regex[GW_QUERYLINE_HIGH]  [kanji_pos]), ql->hira_string, EFLAGS_EXIST)  != 0) return FALSE;
       if (gw_regex_create_kanji_med_regex (&(ql->kanji_regex [GW_QUERYLINE_MED]   [kanji_pos]), ql->hira_string, EFLAGS_EXIST)  != 0) return FALSE;
       kanji_pos++;
       ql->kanji_total++;
     }

     else if (gw_util_is_furigana_str (generic_atoms[i]))
     {
       strcpy(ql->hira_string, generic_atoms[i]);
       strcpy(generic_atoms[i], "(");
       strcat(generic_atoms[i], ql->hira_string);
       strcat(generic_atoms[i], ")");

       if (want_hk_conv && gw_util_is_hiragana_str (ql->hira_string))
       {
         strcpy(temp, ql->hira_string);
         gw_util_str_shift_hira_to_kata (temp);
	       strcat(generic_atoms[i], "|");
         strcat(generic_atoms[i], "(");
         strcat(generic_atoms[i], temp);
         strcat(generic_atoms[i], ")");
       }
       else if (want_kh_conv && gw_util_is_katakana_str (ql->hira_string))
       {
         strcpy(temp, ql->hira_string);
         gw_util_str_shift_kata_to_hira (temp);
	       strcat(generic_atoms[i], "|");
         strcat(generic_atoms[i], "(");
         strcat(generic_atoms[i], temp);
         strcat(generic_atoms[i], ")");
       }

       if (regcomp (&(ql->furi_regex                        [GW_QUERYLINE_EXIST] [furi_pos]), generic_atoms[i], EFLAGS_EXIST)  != 0)  return FALSE;
       if (regcomp (&(ql->furi_regex                        [GW_QUERYLINE_LOCATE][furi_pos]), generic_atoms[i], EFLAGS_LOCATE) != 0)  return FALSE;
       if (gw_regex_create_furi_high_regex (&(ql->furi_regex[GW_QUERYLINE_HIGH]  [furi_pos]), generic_atoms[i], EFLAGS_EXIST)  != 0)  return FALSE;
       if (gw_regex_create_furi_med_regex  (&(ql->furi_regex[GW_QUERYLINE_MED]   [furi_pos]), generic_atoms[i], EFLAGS_EXIST)  != 0)  return FALSE;
       furi_pos++;
       ql->furi_total++;
       strcpy(generic_atoms[i], ql->hira_string);
     }
     else if (gw_util_is_romaji_str (generic_atoms[i]))
     {
       strcpy(ql->hira_string, generic_atoms[i]);
       if (regcomp (&(ql->roma_regex                        [GW_QUERYLINE_EXIST] [roma_pos]), ql->hira_string, EFLAGS_EXIST)  != 0)  return FALSE;
       if (regcomp (&(ql->roma_regex                        [GW_QUERYLINE_LOCATE][roma_pos]), ql->hira_string, EFLAGS_LOCATE) != 0)  return FALSE;
       if (gw_regex_create_roma_high_regex (&(ql->roma_regex[GW_QUERYLINE_HIGH]  [roma_pos]), ql->hira_string, EFLAGS_EXIST)  != 0)  return FALSE;
       if (gw_regex_create_roma_med_regex  (&(ql->roma_regex[GW_QUERYLINE_MED]   [roma_pos]), ql->hira_string, EFLAGS_EXIST)  != 0)  return FALSE;
       roma_pos++;
       ql->roma_total++;

       //Add coversions to search on success
       if (gw_util_str_roma_to_hira (generic_atoms[i], temp, 300) && want_rk_conv)
       {
         //Hiragana
         strcpy(ql->hira_string, "(");
         strcat(ql->hira_string, temp);
         strcat(ql->hira_string, ")");

         if (want_hk_conv)
         {
           //Katakana
	         strcat(ql->hira_string, "|");
           strcat(ql->hira_string, "(");
           gw_util_str_shift_hira_to_kata (temp);
           strcat(ql->hira_string, temp);
           strcat(ql->hira_string, ")");
         }
       }
       if (regcomp (&(ql->furi_regex                        [GW_QUERYLINE_EXIST] [furi_pos]), ql->hira_string, EFLAGS_EXIST)  != 0)  return FALSE;
       if (regcomp (&(ql->furi_regex                        [GW_QUERYLINE_LOCATE][furi_pos]), ql->hira_string, EFLAGS_LOCATE) != 0)  return FALSE;
       if (gw_regex_create_furi_high_regex (&(ql->furi_regex[GW_QUERYLINE_HIGH]
       [furi_pos]), ql->hira_string, EFLAGS_EXIST)  != 0)  return FALSE;
       if (gw_regex_create_furi_med_regex  (&(ql->furi_regex[GW_QUERYLINE_MED]   [furi_pos]), ql->hira_string, EFLAGS_EXIST)  != 0)  return FALSE;
       furi_pos++;
       ql->furi_total++;
     }
     else
     {
       if (regcomp (&(ql->mix_regex                       [GW_QUERYLINE_EXIST] [mix_pos]), generic_atoms[i], EFLAGS_EXIST)  != 0) return FALSE;
       if (regcomp (&(ql->mix_regex                       [GW_QUERYLINE_LOCATE][mix_pos]), generic_atoms[i], EFLAGS_LOCATE) != 0) return FALSE;
       if (gw_regex_create_mix_high_regex (&(ql->mix_regex[GW_QUERYLINE_HIGH]  [mix_pos]), generic_atoms[i], EFLAGS_EXIST)  != 0) return FALSE;
       if (gw_regex_create_mix_med_regex  (&(ql->mix_regex[GW_QUERYLINE_MED]   [mix_pos]), generic_atoms[i], EFLAGS_EXIST)  != 0) return FALSE;
       mix_pos++;
       ql->mix_total++;
     }
     i++;
   }

   return TRUE;
}


//!
//! @brief Parses a query using the kanjidict style
//!
//! The kanjidict style puts an emphasis on separate
//! atoms, linking them with AND.  Special cases the function
//! will try to parse out are hiragana words, kanji/radicals, english
//! phrases.
//!
//! @param ql Pointer to a GwQueryLine object ot parse a query string into.
//! @param string constant string that is the raw query.
//!
int gw_queryline_parse_kanjidict_string (GwQueryLine *ql, const char* string)
{
    //Make sure it isn't already created
    if (strcmp(ql->string, string) == 0) return TRUE;

    //Make a permanent copy of the query
    strncpy(ql->string, string, MAX_QUERY); 

    //Variable preparations
    char *ptr = ql->string;
    char *next = NULL;
    char temp[300];
    gunichar character;
    size_t nmatch = 1;
    char *start;
    int length;
    char atom[MAX_QUERY];
    regmatch_t pmatch[nmatch];
    int rk_conv_pref = gw_pref_get_int (GCKEY_GW_ROMAN_KANA, 0);
    gboolean want_rk_conv = (rk_conv_pref == 0 || (rk_conv_pref == 2 && !gw_util_is_japanese_locale()));

    //Get stroke
    if (regexec(gw_re[GW_RE_QUERY_STROKES], ptr, nmatch, pmatch, 0) == 0)
    {
      start = ptr + pmatch[0].rm_so + 1;
      length = pmatch[0].rm_eo - pmatch[0].rm_so - 1;
      strncpy (atom, start, length);
      atom[length] = '\0';
      if (gw_regex_create_exact_regex (&(ql->strokes_regex [GW_QUERYLINE_EXIST]  [ql->strokes_total]), atom, EFLAGS_EXIST)  != 0) return FALSE;
      if (gw_regex_create_exact_regex (&(ql->strokes_regex [GW_QUERYLINE_LOCATE]  [ql->strokes_total]), atom, EFLAGS_LOCATE)  != 0) return FALSE;
      if (gw_regex_create_exact_regex (&(ql->strokes_regex [GW_QUERYLINE_HIGH]  [ql->strokes_total]), atom, EFLAGS_EXIST)  != 0) return FALSE;
      if (gw_regex_create_exact_regex (&(ql->strokes_regex [GW_QUERYLINE_MED]  [ql->strokes_total]), atom, EFLAGS_EXIST)  != 0) return FALSE;
      ql->strokes_total++;
    }

    //Get Frequency
    if (regexec(gw_re[GW_RE_QUERY_FREQUENCY], ptr, nmatch, pmatch, 0) == 0)
    {
      start = ptr + pmatch[0].rm_so + 1;
      length = pmatch[0].rm_eo - pmatch[0].rm_so - 1;
      strncpy (atom, start, length);
      atom[length] = '\0';
      if (gw_regex_create_exact_regex (&(ql->frequency_regex [GW_QUERYLINE_EXIST]  [ql->frequency_total]), atom, EFLAGS_EXIST)  != 0) return FALSE;
      if (gw_regex_create_exact_regex (&(ql->frequency_regex [GW_QUERYLINE_LOCATE]  [ql->frequency_total]), atom, EFLAGS_LOCATE)  != 0) return FALSE;
      if (gw_regex_create_exact_regex (&(ql->frequency_regex [GW_QUERYLINE_HIGH]  [ql->frequency_total]), atom, EFLAGS_EXIST)  != 0) return FALSE;
      if (gw_regex_create_exact_regex (&(ql->frequency_regex [GW_QUERYLINE_MED]  [ql->frequency_total]), atom, EFLAGS_EXIST)  != 0) return FALSE;
      ql->frequency_total++;
    }

    //Get Grade
    if (regexec(gw_re[GW_RE_QUERY_GRADE], ptr, nmatch, pmatch, 0) == 0)
    {
      start = ptr + pmatch[0].rm_so + 1;
      length = pmatch[0].rm_eo - pmatch[0].rm_so - 1;
      strncpy (atom, start, length);
      atom[length] = '\0';
      if (gw_regex_create_exact_regex (&(ql->grade_regex [GW_QUERYLINE_EXIST]  [ql->grade_total]), atom, EFLAGS_EXIST)  != 0) return FALSE;
      if (gw_regex_create_exact_regex (&(ql->grade_regex [GW_QUERYLINE_LOCATE]  [ql->grade_total]), atom, EFLAGS_LOCATE)  != 0) return FALSE;
      if (gw_regex_create_exact_regex (&(ql->grade_regex [GW_QUERYLINE_HIGH]  [ql->grade_total]), atom, EFLAGS_EXIST)  != 0) return FALSE;
      if (gw_regex_create_exact_regex (&(ql->grade_regex [GW_QUERYLINE_MED]  [ql->grade_total]), atom, EFLAGS_EXIST)  != 0) return FALSE;
      ql->grade_total++;
    }

    //Get JLPT
    if (regexec(gw_re[GW_RE_QUERY_JLPT], ptr, nmatch, pmatch, 0) == 0)
    {
      start = ptr + pmatch[0].rm_so + 1;
      length = pmatch[0].rm_eo - pmatch[0].rm_so - 1;
      strncpy (atom, start, length);
      atom[length] = '\0';
      if (gw_regex_create_exact_regex (&(ql->jlpt_regex [GW_QUERYLINE_EXIST]  [ql->jlpt_total]), atom, EFLAGS_EXIST)  != 0) return FALSE;
      if (gw_regex_create_exact_regex (&(ql->jlpt_regex [GW_QUERYLINE_LOCATE]  [ql->jlpt_total]), atom, EFLAGS_LOCATE)  != 0) return FALSE;
      if (gw_regex_create_exact_regex (&(ql->jlpt_regex [GW_QUERYLINE_HIGH]  [ql->jlpt_total]), atom, EFLAGS_EXIST)  != 0) return FALSE;
      if (gw_regex_create_exact_regex (&(ql->jlpt_regex [GW_QUERYLINE_MED]  [ql->jlpt_total]), atom, EFLAGS_EXIST)  != 0) return FALSE;
      ql->jlpt_total++;
    }

    //Kanji
    ptr = ql->string;
    int i = 0;
    while ((character = g_utf8_get_char(ptr)) != '\0' && i < MAX_ATOMS)
    {
      next = g_utf8_next_char (ptr);
      if (character >= L'ア')
      {
        strncpy (atom, ptr, next - ptr);
        atom[next - ptr] = '\0';
        if (regcomp (&(ql->kanji_regex                         [GW_QUERYLINE_EXIST] [ql->kanji_total]), atom, EFLAGS_EXIST)  != 0) return FALSE;
        if (regcomp (&(ql->kanji_regex                         [GW_QUERYLINE_LOCATE][ql->kanji_total]), atom, EFLAGS_LOCATE) != 0) return FALSE;
        if (gw_regex_create_kanji_high_regex (&(ql->kanji_regex[GW_QUERYLINE_HIGH]  [ql->kanji_total]), atom, EFLAGS_EXIST)  != 0) return FALSE;
        if (gw_regex_create_kanji_med_regex  (&(ql->kanji_regex[GW_QUERYLINE_MED]   [ql->kanji_total]), atom, EFLAGS_EXIST)  != 0) return FALSE;
        ql->kanji_total++;
        i++;
      }
      ptr = next;
    }

    //copy the hirakana/kanakana search atom
    char exp[1000];
    strcpy(exp, "[(");
    strcat(exp, HIRAGANA);
    strcat(exp, "|");
    strcat(exp, KATAKANA);
    strcat(exp, ")]+");
    ptr = ql->string;
    if (gw_regex_locate_boundary_byte_pointers(ptr, exp, &ptr, &next) && (next - ptr) % 3 == 0)
    {
      strncpy (atom, ptr, next - ptr);
      atom[next - ptr] = '\0';
      if (regcomp (&(ql->furi_regex                         [GW_QUERYLINE_EXIST] [ql->furi_total]), atom, EFLAGS_EXIST)  != 0) return FALSE;
      if (regcomp (&(ql->furi_regex                         [GW_QUERYLINE_LOCATE][ql->furi_total]), atom, EFLAGS_LOCATE) != 0) return FALSE;
      if (gw_regex_create_furi_high_regex (&(ql->furi_regex [GW_QUERYLINE_HIGH]  [ql->furi_total]), atom, EFLAGS_EXIST)  != 0) return FALSE;
      if (gw_regex_create_furi_med_regex  (&(ql->furi_regex [GW_QUERYLINE_MED]   [ql->furi_total]), atom, EFLAGS_EXIST)  != 0) return FALSE;
      ql->furi_total++;
    }

    //English
    ptr = ql->string;
    if (gw_regex_locate_boundary_byte_pointers(ptr, "[A-Za-z][a-z ]{1,20}", &ptr, &next))
    {
      strncpy (atom, ptr, next - ptr);
      atom[next - ptr] = '\0';
      char *high_pattern = g_strdup_printf("\\{%s\\}", atom);
      char *med_pattern = g_strdup_printf("\\b%s\\b", atom);
      if (regcomp (&(ql->roma_regex [GW_QUERYLINE_EXIST] [ql->roma_total]), atom, EFLAGS_EXIST)  != 0) return FALSE;
      if (regcomp (&(ql->roma_regex [GW_QUERYLINE_LOCATE][ql->roma_total]), atom, EFLAGS_LOCATE) != 0) return FALSE;
      if (regcomp (&(ql->roma_regex [GW_QUERYLINE_HIGH]  [ql->roma_total]), high_pattern, EFLAGS_EXIST)  != 0) return FALSE;
      if (regcomp (&(ql->roma_regex [GW_QUERYLINE_MED]   [ql->roma_total]), med_pattern, EFLAGS_EXIST)  != 0) return FALSE;
      if (high_pattern != NULL) g_free(high_pattern);
      if (med_pattern != NULL) g_free(med_pattern);
      /*
      if (gw_regex_create_roma_high_regex (&(ql->roma_regex [GW_QUERYLINE_HIGH]  [ql->roma_total]), atom, EFLAGS_EXIST)  != 0) return FALSE;
      if (gw_regex_create_roma_med_regex  (&(ql->roma_regex [GW_QUERYLINE_MED]   [ql->roma_total]), atom, EFLAGS_EXIST)  != 0) return FALSE;
      */
      ql->roma_total++;

      //Add conversions to search on success
      if (gw_util_str_roma_to_hira (atom, temp, 300) && want_rk_conv)
      {
        //Hiragana
        strcpy(ql->hira_string, temp);
        if (regcomp (&(ql->furi_regex                        [GW_QUERYLINE_EXIST] [ql->furi_total]), temp            , EFLAGS_EXIST)  != 0)  return FALSE;
        if (regcomp (&(ql->furi_regex                        [GW_QUERYLINE_LOCATE][ql->furi_total]), temp            , EFLAGS_LOCATE) != 0)  return FALSE;
        if (gw_regex_create_furi_high_regex (&(ql->furi_regex[GW_QUERYLINE_HIGH]  [ql->furi_total]), temp            , EFLAGS_EXIST)  != 0)  return FALSE;
        if (gw_regex_create_furi_med_regex  (&(ql->furi_regex[GW_QUERYLINE_MED]   [ql->furi_total]), temp            , EFLAGS_EXIST)  != 0)  return FALSE;
        ql->furi_total++;
      }
    }

    return TRUE;
}


//!
//! @brief Parses a query using the example style
//!
//! The program will (to be written)
//!
//! @param ql Pointer to a GwQueryLine object ot parse a query string into.
//! @param string constant string that is the raw query.
//!
int gw_queryline_parse_exampledict_string (GwQueryLine *ql, const char* string)
{
   //Make sure it isn't already created
   if (strcmp(ql->string, string) == 0) return TRUE;

   //Make a perminent copy of the query
   strncpy(ql->string, string, MAX_QUERY); 

   //Load the preference settings
   int rk_conv_pref = gw_pref_get_int (GCKEY_GW_ROMAN_KANA, 0);
   gboolean want_rk_conv = (rk_conv_pref == 0 || (rk_conv_pref == 2 && !gw_util_is_japanese_locale()));
   gboolean want_hk_conv;
   want_hk_conv = gw_pref_get_boolean (GCKEY_GW_HIRA_KATA, TRUE);
   gboolean want_kh_conv;
   want_kh_conv = gw_pref_get_boolean (GCKEY_GW_KATA_HIRA, TRUE);
   ql->hira_string[0] = '\0';

   //Create atoms
   int i = 0;
   char *generic_atoms[MAX_ATOMS];

   generic_atoms[i] = ql->string;
   while ((generic_atoms[i + 1] = g_utf8_strchr (generic_atoms[i], -1, L'&')) != NULL && i < MAX_ATOMS)
   {
     i++;
     *generic_atoms[i] = '\0';
     *generic_atoms[i]++;
   }
   generic_atoms[i + 1] = NULL;

   //Organize atoms
   i = 0;
   int kanji_pos = ql->kanji_total;
   int furi_pos  = ql->furi_total;
   int mix_pos   = ql->mix_total;
   int roma_pos  = ql->roma_total;
   char temp[300];
   gboolean want_conv;

   while (generic_atoms[i] != NULL && i < MAX_ATOMS)
   {
     if (gw_util_is_kanji_ish_str (generic_atoms[i]) || gw_util_is_kanji_str (generic_atoms[i]))
     {
       strcpy(ql->hira_string, generic_atoms[i]);
       if (regcomp (&(ql->kanji_regex                         [GW_QUERYLINE_EXIST] [kanji_pos]), generic_atoms[i], EFLAGS_EXIST)  != 0) return FALSE;
       if (regcomp (&(ql->kanji_regex                         [GW_QUERYLINE_LOCATE][kanji_pos]), generic_atoms[i], EFLAGS_LOCATE) != 0) return FALSE;
       if (regcomp (&(ql->kanji_regex                         [GW_QUERYLINE_HIGH] [kanji_pos]), generic_atoms[i], EFLAGS_EXIST)  != 0) return FALSE;
       if (gw_regex_create_kanji_med_regex (&(ql->kanji_regex [GW_QUERYLINE_MED]   [kanji_pos]), generic_atoms[i], EFLAGS_EXIST)  != 0) return FALSE;
       kanji_pos++;
       ql->kanji_total++;
     }

     else if (gw_util_is_furigana_str (generic_atoms[i]))
     {
       strcpy(ql->hira_string, generic_atoms[i]);
       if (regcomp (&(ql->furi_regex                        [GW_QUERYLINE_EXIST] [furi_pos]), generic_atoms[i], EFLAGS_EXIST)  != 0)  return FALSE;
       if (regcomp (&(ql->furi_regex                        [GW_QUERYLINE_LOCATE][furi_pos]), generic_atoms[i], EFLAGS_LOCATE) != 0)  return FALSE;
       if (regcomp (&(ql->furi_regex                        [GW_QUERYLINE_HIGH] [furi_pos]), generic_atoms[i], EFLAGS_EXIST)  != 0)  return FALSE;
       if (gw_regex_create_furi_med_regex  (&(ql->furi_regex[GW_QUERYLINE_MED]   [furi_pos]), generic_atoms[i], EFLAGS_EXIST)  != 0)  return FALSE;
       furi_pos++;
       ql->furi_total++;

       if (want_hk_conv && gw_util_is_hiragana_str (generic_atoms[i]))
       {
         strcpy(temp, generic_atoms[i]);
         gw_util_str_shift_hira_to_kata (temp);
         if (regcomp (&(ql->furi_regex                        [GW_QUERYLINE_EXIST] [furi_pos]), temp            , EFLAGS_EXIST)  != 0)  return FALSE;
         if (regcomp (&(ql->furi_regex                        [GW_QUERYLINE_LOCATE][furi_pos]), temp            , EFLAGS_LOCATE) != 0)  return FALSE;
         if (regcomp (&(ql->furi_regex                        [GW_QUERYLINE_HIGH] [furi_pos]), temp            , EFLAGS_EXIST)  != 0)  return FALSE;
         if (gw_regex_create_furi_med_regex  (&(ql->furi_regex[GW_QUERYLINE_MED]   [furi_pos]), temp            , EFLAGS_EXIST)  != 0)  return FALSE;
         furi_pos++;
         ql->furi_total++;
       }
       else if (want_kh_conv && gw_util_is_katakana_str (generic_atoms[i]))
       {
         strcpy(temp, generic_atoms[i]);
         gw_util_str_shift_kata_to_hira (temp);
         strcpy(ql->hira_string, generic_atoms[i]);
         if (regcomp (&(ql->furi_regex                        [GW_QUERYLINE_LOCATE][furi_pos]), temp            , EFLAGS_LOCATE) != 0)  return FALSE;
         if (regcomp (&(ql->furi_regex                        [GW_QUERYLINE_EXIST] [furi_pos]), temp            , EFLAGS_EXIST)  != 0)  return FALSE;
         if (regcomp (&(ql->furi_regex                        [GW_QUERYLINE_HIGH][furi_pos]), temp            , EFLAGS_LOCATE) != 0)  return FALSE;
         if (gw_regex_create_furi_med_regex  (&(ql->furi_regex[GW_QUERYLINE_MED]   [furi_pos]), temp            , EFLAGS_EXIST)  != 0)  return FALSE;
         furi_pos++;
         ql->furi_total++;
       }
     }
     else if (gw_util_is_romaji_str (generic_atoms[i]))
     {
       if (regcomp (&(ql->roma_regex                        [GW_QUERYLINE_EXIST] [roma_pos]), generic_atoms[i], EFLAGS_EXIST)  != 0)  return FALSE;
       if (regcomp (&(ql->roma_regex                        [GW_QUERYLINE_LOCATE][roma_pos]), generic_atoms[i], EFLAGS_LOCATE) != 0)  return FALSE;
       if (gw_regex_create_roma_med_regex  (&(ql->roma_regex[GW_QUERYLINE_HIGH]   [roma_pos]), generic_atoms[i], EFLAGS_EXIST)  != 0)  return FALSE;
       if (gw_regex_create_roma_med_regex  (&(ql->roma_regex[GW_QUERYLINE_MED]   [roma_pos]), generic_atoms[i], EFLAGS_EXIST)  != 0)  return FALSE;
       roma_pos++;
       ql->roma_total++;

       //Add coversions to search on success
       if (gw_util_str_roma_to_hira (generic_atoms[i], temp, 300)  && want_rk_conv)
       {
         //Hiragana
         strcpy(ql->hira_string, temp);
         if (regcomp (&(ql->furi_regex                        [GW_QUERYLINE_EXIST] [furi_pos]), temp            , EFLAGS_EXIST)  != 0)  return FALSE;
         if (regcomp (&(ql->furi_regex                        [GW_QUERYLINE_LOCATE][furi_pos]), temp            , EFLAGS_LOCATE) != 0)  return FALSE;
         if (regcomp (&(ql->furi_regex                        [GW_QUERYLINE_HIGH] [furi_pos]), temp            , EFLAGS_EXIST)  != 0)  return FALSE;
         if (gw_regex_create_furi_med_regex  (&(ql->furi_regex[GW_QUERYLINE_MED]   [furi_pos]), temp            , EFLAGS_EXIST)  != 0)  return FALSE;
         furi_pos++;
         ql->furi_total++;

         if (want_hk_conv)
         {
           //Katakana
           gw_util_str_shift_hira_to_kata (temp);
           if (regcomp (&(ql->furi_regex                        [GW_QUERYLINE_EXIST] [furi_pos]), temp            , EFLAGS_EXIST)  != 0)  return FALSE;
           if (regcomp (&(ql->furi_regex                        [GW_QUERYLINE_LOCATE][furi_pos]), temp            , EFLAGS_LOCATE) != 0)  return FALSE;
           if (regcomp (&(ql->furi_regex                        [GW_QUERYLINE_HIGH] [furi_pos]), temp            , EFLAGS_EXIST)  != 0)  return FALSE;
           if (gw_regex_create_furi_med_regex  (&(ql->furi_regex[GW_QUERYLINE_MED]   [furi_pos]), temp            , EFLAGS_EXIST)  != 0)  return FALSE;
           furi_pos++;
           ql->furi_total++;
         }
       }
     }
     else
     {
       if (regcomp (&(ql->mix_regex                       [GW_QUERYLINE_EXIST] [mix_pos]), generic_atoms[i], EFLAGS_EXIST)  != 0) return FALSE;
       if (regcomp (&(ql->mix_regex                       [GW_QUERYLINE_LOCATE][mix_pos]), generic_atoms[i], EFLAGS_LOCATE) != 0) return FALSE;
       if (gw_regex_create_mix_high_regex (&(ql->mix_regex[GW_QUERYLINE_HIGH]  [mix_pos]), generic_atoms[i], EFLAGS_EXIST)  != 0) return FALSE;
       if (gw_regex_create_mix_med_regex  (&(ql->mix_regex[GW_QUERYLINE_MED]   [mix_pos]), generic_atoms[i], EFLAGS_EXIST)  != 0) return FALSE;
       mix_pos++;
       ql->mix_total++;
     }
     i++;
   }

   return TRUE;
}


