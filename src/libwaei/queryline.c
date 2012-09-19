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
//! The LwQueryLine object will be used for parsing
//! the query into token so comparisons can be more
//! intelligently handled.
//!

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <glib.h>

#include <libwaei/libwaei.h>

//Private function declarations
static GRegex*** _allocate_regex_pointers (int);
static void _free_regex_pointers (GRegex***);
static void _free_queryline_pointers (LwQueryLine*);
static char** _initialize_queryline_pointers (LwQueryLine*, const char*);


LwQueryLine* lw_queryline_new ()
{
    LwQueryLine* temp;

    if ((temp = (LwQueryLine*) malloc(sizeof(LwQueryLine))) == NULL) return NULL;

    //A place for a copy of the raw string
    temp->string = NULL;
    temp->re_kanji = NULL;
    temp->re_furi = NULL;
    temp->re_roma = NULL;
    temp->re_mix = NULL;
    temp->re_strokes = NULL;
    temp->re_frequency = NULL;
    temp->re_grade = NULL;
    temp->re_jlpt = NULL;

    return temp;
}


void lw_queryline_free (LwQueryLine *ql)
{
    _free_queryline_pointers (ql);
    free (ql);
}


static void _free_regex_pointer (GRegex ***re)
{
    //Sanity check
    if (re == NULL) return;

    //Declarations
    int i;
    int j;

    //Free pointers intelligently
    for (i = 0; re[i] != NULL; i++)
      for (j = 0; j < GW_RELEVANCE_TOTAL; j++)
        if (re[i][j] != NULL) g_regex_unref(re[i][j]);

    for (i = 0; re[i] != NULL; i++)
      free (re[i]);

    free (re);
}


static void _free_queryline_pointers (LwQueryLine *ql)
{
   g_free (ql->string);

   _free_regex_pointer (ql->re_kanji);
   _free_regex_pointer (ql->re_furi);
   _free_regex_pointer (ql->re_roma);
   _free_regex_pointer (ql->re_mix);
   _free_regex_pointer (ql->re_strokes);
   _free_regex_pointer (ql->re_frequency);
   _free_regex_pointer (ql->re_grade);
   _free_regex_pointer (ql->re_jlpt);

   ql->string = NULL;
   ql->re_kanji = NULL;
   ql->re_furi = NULL;
   ql->re_roma = NULL;
   ql->re_mix = NULL;
   ql->re_strokes = NULL;
   ql->re_frequency = NULL;
   ql->re_grade = NULL;
   ql->re_jlpt = NULL;
}
   

static char** _initialize_queryline_pointers (LwQueryLine *ql, const char *string)
{
    //Declarations
    char **atoms;
    int length;

    //Initializations
    ql->string = lw_util_prepare_query (string, FALSE);
    atoms = g_strsplit (ql->string, "&", GW_QUERYLINE_MAX_ATOMS);

    length = g_strv_length (atoms);

    //Allocations
    ql->re_kanji     = _allocate_regex_pointers (length);
    ql->re_furi      = _allocate_regex_pointers (length);
    ql->re_roma      = _allocate_regex_pointers (length);
    ql->re_mix       = _allocate_regex_pointers (length);
    ql->re_strokes   = _allocate_regex_pointers (length);
    ql->re_frequency = _allocate_regex_pointers (length);
    ql->re_grade     = _allocate_regex_pointers (length);
    ql->re_jlpt      = _allocate_regex_pointers (length);

    return atoms;
}


static GRegex*** _allocate_regex_pointers (int length)
{
    //Declarations
    int i;
    int j;
    GRegex ***re;

    //Allocate the memory
    re = (GRegex***) malloc((length + 1) * sizeof(GRegex**));

    //Initialize it to NULL
    for (i = 0; i < length; i++)
    {
      re[i] = (GRegex**) malloc(GW_RELEVANCE_TOTAL * sizeof (GRegex*));
      for (j = 0; j < GW_RELEVANCE_TOTAL; j++)
        re[i][j] = NULL;
    }
    re[i] = NULL;

    //Return the address
    return re;
}


//!
//! @brief Parses a query using the edict style
//!
//! The program will (to be written)
//!
//! @param ql Pointer to a LwQueryLine object ot parse a query string into.
//! @param string constant string that is the raw query.
//!
gboolean lw_queryline_parse_edict_string (LwQueryLine *ql, const char* string, GError **error)
{
   //Sanity check
   if (error != NULL && *error != NULL) return FALSE;

   //Sanity check
   if (ql->string != NULL && strcmp(ql->string, string) == 0) return TRUE;

   //Free previously used memory
    _free_queryline_pointers (ql);

   //Declarations
   char **atoms;
   char **iter;
   char *atom;
   char *temp;
   char *expression;
   char *half;
   char buffer[300];
   int rk_conv_pref;
   gboolean want_rk_conv;
   gboolean want_hk_conv;
   gboolean want_kh_conv;
   gboolean want_conv;
   gboolean all_regex_built;
   int length;
   GRegex ***re;
   int i;

   //Memory initializations
   all_regex_built = TRUE;

   rk_conv_pref = lw_pref_get_int_by_schema (GW_SCHEMA_BASE, GW_KEY_ROMAN_KANA);
   want_rk_conv = (rk_conv_pref == 0 || (rk_conv_pref == 2 && !lw_util_is_japanese_locale()));
   want_hk_conv = lw_pref_get_boolean_by_schema (GW_SCHEMA_BASE, GW_KEY_HIRA_KATA);
   want_kh_conv = lw_pref_get_boolean_by_schema (GW_SCHEMA_BASE, GW_KEY_KATA_HIRA);

   atoms = _initialize_queryline_pointers (ql, string);
   length = g_strv_length (atoms);

   //Setup the expression to be used in the base of the regex for kanji-ish strings
   re = ql->re_kanji;
   for (iter = atoms; *iter != NULL && re < (ql->re_kanji + length); iter++)
   {
     atom = *iter;

     if (lw_util_is_kanji_ish_str (atom) || lw_util_is_kanji_str (atom)) //Figures out if the string may contain hiragana
     {
       expression = g_strdup_printf ("(%s)", atom);

       if (lw_util_is_yojijukugo_str (atom))  //Check for yojijukugo
       {
          //First half of the yojijukugo
          half = g_strndup (atom, g_utf8_next_char(g_utf8_next_char(atom))- atom);
          temp = g_strdup_printf ("%s|(%s)", expression, half);
          g_free (expression);
          expression = temp;
          g_free (half);

          //Second half of the yojijukugo
          half = g_strdup (g_utf8_next_char(g_utf8_next_char(atom)));
          temp = g_strdup_printf ("%s|(%s)", expression, half);
          g_free (expression);
          expression = temp;
          g_free (half);
       }

       //Compile the regexes
       for (i = 0; i < GW_RELEVANCE_TOTAL; i++)
         if (((*re)[i] = lw_regex_kanji_new (expression, GW_ENGINE_EDICT, i, error)) == NULL) all_regex_built = FALSE;

       g_free (expression);
       re++;
     }
   }


   //Setup the expression to be used in the base of the regex for furigana strings
   re = ql->re_furi;
   for (iter = atoms; *iter != NULL && re < (ql->re_furi + length); iter++)
   {
     atom = *iter;
     if (lw_util_is_furigana_str (atom))
     {
       expression = g_strdup_printf ("(%s)", atom);

       if (want_hk_conv && lw_util_is_hiragana_str (atom))
       {
         temp = g_strdup (atom);
         lw_util_str_shift_hira_to_kata (temp);
         g_free (expression);
         expression = g_strdup_printf("(%s)|(%s)", atom, temp);
         g_free (temp);
       }
       else if (want_kh_conv && lw_util_is_katakana_str (atom))
       {
         temp = g_strdup (atom);
         lw_util_str_shift_kata_to_hira (temp);
         g_free (expression);
         expression = g_strdup_printf("(%s)|(%s)", atom, temp);
         g_free (temp);
       }

       //Compile the regexes
       for (i = 0; i < GW_RELEVANCE_TOTAL; i++)
         if (((*re)[i] = lw_regex_furi_new (expression, GW_ENGINE_EDICT, i, error)) == NULL) all_regex_built = FALSE;

       g_free (expression);
       re++;
     }
     else if (lw_util_is_romaji_str (atom) && lw_util_str_roma_to_hira (atom, buffer, 300) && want_rk_conv)
     {
       expression = g_strdup_printf("(%s)", buffer);

       if (want_hk_conv)
       {
         temp = g_strdup (buffer);
         lw_util_str_shift_hira_to_kata (temp);
         g_free (expression);
         expression = g_strdup_printf("(%s)|(%s)", buffer, temp);
         g_free (temp);
       }

       //Compile the regexes
       for (i = 0; i < GW_RELEVANCE_TOTAL; i++)
         if (((*re)[i] = lw_regex_furi_new (expression, GW_ENGINE_EDICT, i, error)) == NULL) all_regex_built = FALSE;

       g_free (expression);
       re++;
     }
   }


   //Setup the expression to be used in the base of the regex
   re = ql->re_roma;
   for (iter = atoms; *iter != NULL && re < (ql->re_roma + length); iter++)
   {
     atom = *iter;
     g_strstrip(atom);
     if (strlen(atom) > 0 && lw_util_is_romaji_str (atom) && g_regex_match (lw_re[GW_RE_NUMBER], atom, 0, NULL) == FALSE)
     {
       expression = g_strdup (atom);

       //Compile the regexes
       for (i = 0; i < GW_RELEVANCE_TOTAL; i++)
         if (((*re)[i] = lw_regex_romaji_new (expression, GW_ENGINE_EDICT, i, error)) == NULL) all_regex_built = FALSE;

       g_free (expression);
       re++;
     }
   }  


   //Setup the expression to be used in the base of the regex
   re = ql->re_mix;
   for (iter = atoms; *iter != NULL && re < (ql->re_roma + length); iter++)
   {
     atom = *iter;
     if (!lw_util_is_kanji_ish_str (atom) &&
         !lw_util_is_kanji_str (atom)     && 
         !lw_util_is_furigana_str (atom)  &&
         !lw_util_is_romaji_str (atom)      )
     {
       expression = g_strdup (atom);

       //Compile the regexes
       for (i = 0; i < GW_RELEVANCE_TOTAL; i++)
         if (((*re)[i] = lw_regex_mix_new (expression, GW_ENGINE_EDICT, i, error)) == NULL) all_regex_built = FALSE;

       g_free (expression);
       re++;
     }
   }

   //Cleanup
   g_strfreev (atoms);
   atoms = NULL;

   return all_regex_built;
}


static GRegex*** _compile_and_allocate_number_search_regex (const char* subject, const LwRegexDataIndex INDEX, GError **error)
{
    //Sanity check
    if (error != NULL && *error != NULL) return NULL;

    //Declarations
    GRegex ***re;
    GRegex ***iter;
    GMatchInfo *match_info;
    gboolean all_regex_built;
    char *match_text;
    char *expression;
    char *ptr;
    int i;

    //Initializations
    all_regex_built = TRUE;

    //Search the query text for the specfic search terms
    g_regex_match (lw_re[INDEX], subject, 0, &match_info);

    //Allocate the memory depending on how many were found
    re = _allocate_regex_pointers (g_match_info_get_match_count (match_info));

    //Investigate the matches and compile the new regexes
    for (iter = re; all_regex_built && iter - re < GW_QUERYLINE_MAX_ATOMS && g_match_info_matches (match_info); iter++)
    {
      match_text = g_match_info_fetch (match_info, 0);
      expression = g_strdup_printf("\\b%s\\b", match_text + 1);

      for (i = 0; all_regex_built && i < GW_RELEVANCE_TOTAL; i++)
        if (((*iter)[i] = lw_regex_new (expression, GW_ENGINE_KANJI, i, error)) == NULL) all_regex_built = FALSE;

      g_free (expression);
      g_free (match_text);

      g_match_info_next (match_info, NULL);
    }
    g_match_info_free (match_info);
    (*iter) = NULL;

    //Cleanup if there was an error
    for (iter = re; !all_regex_built && iter - re < GW_QUERYLINE_MAX_ATOMS; iter++)
    {
      printf("error\n");
      exit(0);
      for (i = 0; i < GW_RELEVANCE_TOTAL; i++)
      {
        if ((*iter)[i] != NULL) g_regex_unref ((*re)[i]);
        (*iter)[i] = NULL;
      }
    }

    //Finish
    return re;
}



//!
//! @brief Parses a query using the kanjidict style
//!
//! The kanjidict style puts an emphasis on separate
//! atoms, linking them with AND.  Special cases the function
//! will try to parse out are hiragana words, kanji/radicals, english
//! phrases.
//!
//! @param ql Pointer to a LwQueryLine object ot parse a query string into.
//! @param string constant string that is the raw query.
//!
gboolean lw_queryline_parse_kanjidict_string (LwQueryLine *ql, const char* string, GError **error)
{
    //Sanity check
    if (error != NULL && *error != NULL) return FALSE;

    //Make sure it isn't already created
    if (ql->string != NULL && strcmp(ql->string, string) == 0) return TRUE;

    //Free previously used memory
    _free_queryline_pointers (ql);

    //Declarations
    GMatchInfo *match_info;
    int rk_conv_pref;
    int i;
    int length;
    char **atoms;
    char **iter;
    char *atom;
    char *ptr;
    char *start;
    char *end;
    GRegex ***re;
    gboolean want_rk_conv;
    gboolean all_regex_built;
    gunichar character;
    GUnicodeScript script;

    //Initializations
    rk_conv_pref = lw_pref_get_int_by_schema (GW_SCHEMA_BASE, GW_KEY_ROMAN_KANA);
    want_rk_conv = (rk_conv_pref == 0 || (rk_conv_pref == 2 && !lw_util_is_japanese_locale()));
    all_regex_built = TRUE;
    if (ql->string != NULL) g_free (ql->string);
    ql->string = lw_util_prepare_query (string, FALSE);

    //Get stroke
    if ((ql->re_strokes = _compile_and_allocate_number_search_regex (string, GW_RE_STROKES, error)) == NULL)
      all_regex_built = FALSE;

    //Get frequency
    if ((ql->re_frequency = _compile_and_allocate_number_search_regex (string, GW_RE_FREQUENCY, error)) == NULL)
      all_regex_built = FALSE;

    //Get grade level
    if ((ql->re_grade = _compile_and_allocate_number_search_regex (string, GW_RE_GRADE, error)) == NULL)
      all_regex_built = FALSE;

    //Get JLPT level
    if ((ql->re_jlpt = _compile_and_allocate_number_search_regex (string, GW_RE_JLPT, error)) == NULL)
      all_regex_built = FALSE;

    //Get Kanji
    length = 0;
    for (ptr = ql->string; *ptr != '\0'; ptr = g_utf8_next_char (ptr))
    {
      character = g_utf8_get_char (ptr);
      script = g_unichar_get_script (character);
      if (script == G_UNICODE_SCRIPT_HAN) length++;
    }
    ql->re_kanji = _allocate_regex_pointers (length);

    re = ql->re_kanji;
    for (ptr = ql->string; *ptr != '\0' && re - ql->re_kanji < length; ptr = g_utf8_next_char (ptr))
    {
      character = g_utf8_get_char (ptr);
      script = g_unichar_get_script (character);
      if (script == G_UNICODE_SCRIPT_HAN)
      {
        start = ptr;
        end = g_utf8_next_char (ptr);
        atom = g_strndup (start, end - start);
        for (i = 0; all_regex_built && i < GW_RELEVANCE_TOTAL; i++)
          (*re)[i] = lw_regex_kanji_new (atom, GW_ENGINE_KANJI, i, error);
        g_free (atom);
        re++;
      }
    }

    //Get Hiragana and Katakana (aka Furigana)
    atoms = lw_util_get_furigana_atoms_from_string (ql->string);
    length = g_strv_length (atoms);
    ql->re_furi = _allocate_regex_pointers (length);
    re = ql->re_furi;
    ptr = ql->string;

    for (iter = atoms; *iter != NULL; iter++)
    {
      atom = *iter;

      for (i = 0; all_regex_built && i < GW_RELEVANCE_TOTAL; i++)
        if (((*re)[i] = lw_regex_furi_new (atom, GW_ENGINE_KANJI, i, error)) == NULL) all_regex_built = FALSE;

      re++;
    }

    g_strfreev (atoms);

    //Romaji
    atoms = lw_util_get_romaji_atoms_from_string (ql->string);
    length = g_strv_length (atoms);
    ql->re_roma = _allocate_regex_pointers (length);
    re = ql->re_roma;
    ptr = ql->string;

    for (iter = atoms; *iter != NULL; iter++)
    {
      atom = *iter;
      GMatchInfo *match_info;
      int match_start_byte_offset;
      int match_end_byte_offset;

      //This was a special regex that shouldn't be placed in the romaji area
      if (g_regex_match (lw_re[GW_RE_NUMBER], atom, 0, &match_info) == TRUE)
      {
        //Attepm to remove it
        g_match_info_fetch_pos (match_info, 0, &match_start_byte_offset, &match_end_byte_offset);
        for (i = match_start_byte_offset; i < match_end_byte_offset; i++)
          atom[i] = ' ';
        g_strstrip (atom);

        for (i = 0; strlen(atom) > 0 && all_regex_built && i < GW_RELEVANCE_TOTAL; i++)
          (*re)[i] = NULL;;

      }
      //Do the normal regex building
      else
      {
        for (i = 0; all_regex_built && i < GW_RELEVANCE_TOTAL; i++)
          if (((*re)[i] = lw_regex_romaji_new (atom, GW_ENGINE_KANJI, i, error)) == NULL) all_regex_built = FALSE;
      }

      g_match_info_free (match_info);
      re++;
    }

    g_strfreev (atoms);

    return all_regex_built;
}


//!
//! @brief Parses a query using the example style
//!
//! The program will (to be written)
//!
//! @param ql Pointer to a LwQueryLine object ot parse a query string into.
//! @param string constant string that is the raw query.
//!
gboolean lw_queryline_parse_exampledict_string (LwQueryLine *ql, const char* string, GError **error)
{
    //Sanity check
    if (error != NULL && *error != NULL) return FALSE;

    //Make sure it isn't already created
    if (ql->string != NULL && strcmp(ql->string, string) == 0) return TRUE;

    //Free previously used memory
    _free_queryline_pointers (ql);

    //Declarations
    char *atom;
    char **atoms;
    char **iter;
    int rk_conv_pref;
    gboolean want_rk_conv;
    gboolean want_hk_conv;
    gboolean want_kh_conv;
    gboolean all_regex_built;
    int length;
    int i;
    GRegex ***re;
    char buffer[300];
    char *temp;
    char *expression;

    //Initializations
    rk_conv_pref = lw_pref_get_int_by_schema (GW_SCHEMA_BASE, GW_KEY_ROMAN_KANA);
    want_rk_conv = (rk_conv_pref == 0 || (rk_conv_pref == 2 && !lw_util_is_japanese_locale()));
    want_hk_conv = lw_pref_get_boolean_by_schema (GW_SCHEMA_BASE, GW_KEY_HIRA_KATA);
    want_kh_conv = lw_pref_get_boolean_by_schema (GW_SCHEMA_BASE, GW_KEY_KATA_HIRA);
    all_regex_built = TRUE;
 
    atoms = _initialize_queryline_pointers (ql, string);
    length = g_strv_length (atoms);

    re = ql->re_kanji;
    for (iter = atoms; *iter != NULL && re < (ql->re_furi + length); iter++)
    {
      atom = *iter;
      if (lw_util_is_kanji_ish_str (atom) || lw_util_is_kanji_str (atom))
      {
        for (i = 0; all_regex_built && i < GW_RELEVANCE_TOTAL; i++)
          (*re)[i] = lw_regex_kanji_new (atom, GW_ENGINE_EXAMPLES, i, error);
        re++;
      }
    }

    //Setup the expression to be used in the base of the regex for furigana strings
    re = ql->re_furi;
    for (iter = atoms; *iter != NULL && re < (ql->re_furi + length); iter++)
    {
      atom = *iter;
      if (lw_util_is_furigana_str (atom))
      {
        expression = g_strdup_printf ("(%s)", atom);

        if (want_hk_conv && lw_util_is_hiragana_str (atom))
        {
          temp = g_strdup (atom);
          lw_util_str_shift_hira_to_kata (temp);
          g_free (expression);
          expression = g_strdup_printf("(%s)|(%s)", atom, temp);
          g_free (temp);
        }
        else if (want_kh_conv && lw_util_is_katakana_str (atom))
        {
          temp = g_strdup (atom);
          lw_util_str_shift_kata_to_hira (temp);
          g_free (expression);
          expression = g_strdup_printf("(%s)|(%s)", atom, temp);
          g_free (temp);
        }

        //Compile the regexes
        for (i = 0; i < GW_RELEVANCE_TOTAL; i++)
          if (((*re)[i] = lw_regex_furi_new (expression, GW_ENGINE_EDICT, i, error)) == NULL) all_regex_built = FALSE;

        g_free (expression);
        re++;
      }
      else if (lw_util_is_romaji_str (atom) && lw_util_str_roma_to_hira (atom, buffer, 300) && want_rk_conv)
      {
        expression = g_strdup_printf("(%s)", buffer);

        if (want_hk_conv)
        {
          temp = g_strdup (buffer);
          lw_util_str_shift_hira_to_kata (temp);
          g_free (expression);
          expression = g_strdup_printf("(%s)|(%s)", buffer, temp);
          g_free (temp);
        }

        //Compile the regexes
        for (i = 0; i < GW_RELEVANCE_TOTAL; i++)
          if (((*re)[i] = lw_regex_furi_new (expression, GW_ENGINE_EDICT, i, error)) == NULL) all_regex_built = FALSE;

        g_free (expression);
        re++;
      }
    }

    //Setup the expression to be used in the base of the regex
    re = ql->re_roma;
    for (iter = atoms; *iter != NULL && re < (ql->re_roma + length); iter++)
    {
      atom = *iter;
      g_strstrip(atom);
      if (strlen(atom) > 0 && lw_util_is_romaji_str (atom) && g_regex_match (lw_re[GW_RE_NUMBER], atom, 0, NULL) == FALSE)
      {
        expression = g_strdup (atom);
 
       //Compile the regexes
        for (i = 0; i < GW_RELEVANCE_TOTAL; i++)
          if (((*re)[i] = lw_regex_romaji_new (expression, GW_ENGINE_EDICT, i, error)) == NULL) all_regex_built = FALSE;
 
        g_free (expression);
        re++;
      }
    }  

    //Cleanup
    g_strfreev (atoms);
    atoms = NULL;

    return all_regex_built;
}


