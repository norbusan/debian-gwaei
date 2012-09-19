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
//! @file queryline.c
//!


#include "../private.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <glib.h>

#include <libwaei/libwaei.h>


//Private methods
static GRegex*** _queryline_allocate_pointers (int);
static void _queryline_free_pointers (LwQueryLine*);
static char** _queryline_initialize_pointers (LwQueryLine*, const char*);


//!
//! @brief Creates a new LwQueryLine object
//! @return An allocated LwQueryLine that will be needed to be freed by lw_queryline_free.
//!
LwQueryLine* 
lw_queryline_new ()
{
    LwQueryLine* temp;

    if ((temp = (LwQueryLine*) malloc(sizeof(LwQueryLine))) == NULL) return NULL;

    if (temp != NULL)
    {
      lw_queryline_init (temp);
    }

    return temp;
}


//!
//! @brief Releases a LwQueryLine object from memory.
//! @param ql A LwQueryLine object created by lw_queryline_new.
//!
void 
lw_queryline_free (LwQueryLine *ql)
{
    lw_queryline_deinit (ql);
    free (ql);
}


//!
//! @brief Used to initialize the memory inside of a new LwQueryLine
//!        object.  Usually lw_queryline_new calls this for you.  It is also 
//!        used in class implimentations that extends LwQueryLine.
//! @param ql The LwQueryLine object to initialize the variables of
//!
void 
lw_queryline_init (LwQueryLine *ql)
{
    ql->string = NULL;
#ifdef WITH_MECAB
    ql->morphology = NULL;
#endif
    ql->re_kanji = NULL;
    ql->re_furi = NULL;
    ql->re_roma = NULL;
    ql->re_mix = NULL;
    ql->re_strokes = NULL;
    ql->re_frequency = NULL;
    ql->re_grade = NULL;
    ql->re_jlpt = NULL;
}


//!
//! @brief Used to free the memory inside of a new LwQueryLine
//!        object.  Usually lw_queryline_free calls this for you.  It is also 
//!        used in class implimentations that extends LwQueryLine.
//! @param ql The LwQueryLine object to free the variables of
//!
void 
lw_queryline_deinit (LwQueryLine *ql)
{
    _queryline_free_pointers (ql);
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
      for (j = 0; j < LW_RELEVANCE_TOTAL; j++)
        if (re[i][j] != NULL) g_regex_unref(re[i][j]);

    for (i = 0; re[i] != NULL; i++)
      free (re[i]);

    free (re);
}


static void _queryline_free_pointers (LwQueryLine *ql)
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
   

static char** _queryline_initialize_pointers (LwQueryLine *ql, const char *string)
{
    //Declarations
    char **atoms;
    int length;

    //Initializations
    ql->string = lw_util_prepare_query (string, FALSE);
    atoms = g_strsplit (ql->string, "&", LW_QUERYLINE_MAX_ATOMS);

    length = g_strv_length (atoms);

    //Allocations
    ql->re_kanji     = _queryline_allocate_pointers (length);
    ql->re_furi      = _queryline_allocate_pointers (length);
    ql->re_roma      = _queryline_allocate_pointers (length);
    ql->re_mix       = _queryline_allocate_pointers (length);
    ql->re_strokes   = _queryline_allocate_pointers (length);
    ql->re_frequency = _queryline_allocate_pointers (length);
    ql->re_grade     = _queryline_allocate_pointers (length);
    ql->re_jlpt      = _queryline_allocate_pointers (length);

    return atoms;
}


static GRegex*** _queryline_allocate_pointers (int length)
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
      re[i] = (GRegex**) malloc(LW_RELEVANCE_TOTAL * sizeof (GRegex*));
      for (j = 0; j < LW_RELEVANCE_TOTAL; j++)
        re[i][j] = NULL;
    }
    re[i] = NULL;

    //Return the address
    return re;
}


#ifdef WITH_MECAB
//!
//! @brief Construct a regexp for getting the morphologically deduced base forms of words
//!
//! @param ql Queryline whose ql->string has been initialized
//! @returns Newly allocated string, or NULL if nothing to add
//!
//! The regexp is of form ^WORD1$|^WORD2$|...
//!
static char *_queryline_get_morphology_regexp (LwQueryLine *ql)
{
   LwMorphology *morph;
   GList *it;
   gchar *result;
   const char *ptr;

   if (ql->morphology)
   {
     g_free (ql->morphology);
     ql->morphology = NULL;
   }

   result = NULL;

   // Do analysis only on alpha-kana-kanji strings
   for (ptr = ql->string; *ptr != '\0'; ptr = g_utf8_next_char (ptr))
   {
       gunichar character;
       GUnicodeScript script;
       character = g_utf8_get_char (ptr);
       script = g_unichar_get_script (character);
       if (script != G_UNICODE_SCRIPT_HAN &&
           script != G_UNICODE_SCRIPT_HIRAGANA &&
           script != G_UNICODE_SCRIPT_KATAKANA &&
           !g_unichar_isalnum(character) &&
           !g_unichar_isspace(character))
           return result;
   }

   morph = lw_morphology_new ();
   lw_morphology_analize (lw_morphologyengine_get_default (), morph, ql->string);

   for (it = morph->items; it; it = it->next) {
       LwMorphologyItem *item = (LwMorphologyItem *)it->data;
       char *temp;

       if (it == morph->items && it->next == NULL
               && strcmp(item->base_form, ql->string) == 0) {
           // Don't add any results for a single word in base form
           break;
       }

       if (item->base_form) {
           if (result == NULL) {
               result = g_strdup_printf("^%s$", item->base_form);
           }
           else {
               temp = g_strdup_printf ("%s|^%s$", result, item->base_form);
               g_free (result);
               result = temp;
           }
       }
       if (item->explanation) {
           if (ql->morphology == NULL) {
               ql->morphology = g_strdup (item->explanation);
           }
           else {
               temp = g_strdup_printf ("%s + %s", ql->morphology, item->explanation);
               g_free (ql->morphology);
               ql->morphology = temp;
           }
       }
   }

   lw_morphology_free (morph);

   return result;
}
#endif


//!
//! @brief Parses a query using the edict style
//! @param ql Pointer to a LwQueryLine object ot parse a query string into.
//! @param pm A LwPreferences object to load preferences from
//! @param STRING constant string that is the raw query.
//! @param error A Pointer to a GError to load errors into or NULL
//! @returns Returns true if the string was successfully parsed.
//!
gboolean 
lw_queryline_parse_edict_string (LwQueryLine *ql, LwPreferences *pm, const char* STRING, GError **error)
{
   //Sanity check
   if (error != NULL && *error != NULL) return FALSE;

   //Sanity check
   if (ql->string != NULL && strcmp(ql->string, STRING) == 0) return TRUE;

   //Free previously used memory
    _queryline_free_pointers (ql);

   //Declarations
   char **atoms;
   char **iter;
   char *atom;
   char *temp;
   char *expression;
   char *half;
#ifdef WITH_MECAB
   char *morpho_expression;
#endif
   char *expression_low;
   char buffer[300];
   int rk_conv_pref;
   gboolean want_rk_conv;
   gboolean want_hk_conv;
   gboolean want_kh_conv;
   gboolean all_regex_built;
   int length;
   GRegex ***re;
   int i;

   //Memory initializations
   all_regex_built = TRUE;

   if (pm != NULL)
   {
     rk_conv_pref = lw_preferences_get_int_by_schema (pm, LW_SCHEMA_BASE, LW_KEY_ROMAN_KANA);
     want_rk_conv = (rk_conv_pref == 0 || (rk_conv_pref == 2 && !lw_util_is_japanese_locale()));
     want_hk_conv = lw_preferences_get_boolean_by_schema (pm, LW_SCHEMA_BASE, LW_KEY_HIRA_KATA);
     want_kh_conv = lw_preferences_get_boolean_by_schema (pm, LW_SCHEMA_BASE, LW_KEY_KATA_HIRA);
   }
   else
   {
     rk_conv_pref = 1;
     want_rk_conv = TRUE;
     want_hk_conv = TRUE;
     want_kh_conv = TRUE;
   }

   //Start analysis
   atoms = _queryline_initialize_pointers (ql, STRING);

#ifdef WITH_MECAB
   morpho_expression = _queryline_get_morphology_regexp(ql);
#endif

   length = g_strv_length (atoms);

   //Setup the expression to be used in the base of the regex for kanji-ish strings
   re = ql->re_kanji;
   for (iter = atoms; *iter != NULL && re < (ql->re_kanji + length); iter++)
   {
     atom = *iter;
     expression = NULL;
     expression_low = NULL;

     if (lw_util_is_kanji_ish_str (atom) || lw_util_is_kanji_str (atom)) //Figures out if the string may contain hiragana
     {
       expression = g_strdup_printf ("(%s)", atom);

       if (lw_util_is_yojijukugo_str (atom))  //Check for yojijukugo
       {
          //First half of the yojijukugo
          half = g_strndup (atom, g_utf8_next_char(g_utf8_next_char(atom)) - atom);
          if (strcmp(half, "..") != 0)
          {
            temp = g_strdup_printf ("%s|(%s)", expression, half);
            g_free (expression);
            expression = temp;
          }
          g_free (half);

          //Second half of the yojijukugo
          half = g_strdup (g_utf8_next_char(g_utf8_next_char(atom)));
          if (strcmp(half, "..") != 0)
          {
            temp = g_strdup_printf ("%s|(%s)", expression, half);
            g_free (expression);
            expression = temp;
          }
          g_free (half);
       }
     }

#ifdef WITH_MECAB
     if (morpho_expression && iter == atoms) {
         // Stuff morphology regexp to the first atom
         if (expression == NULL) {
             expression_low = g_strdup (morpho_expression);
         }
         else {
             expression_low = g_strdup_printf ("%s|%s", expression, morpho_expression);
         }
     }
#endif

     if (expression_low && expression == NULL) {
         expression = g_strdup ("----------------");
     }

     if (expression) {
       //Compile the regexes
       temp = expression;
       for (i = 0; i < LW_RELEVANCE_TOTAL; i++) {
         if (expression_low && i == LW_RELEVANCE_LOW)
             temp = expression_low;
         if (((*re)[i] = lw_regex_kanji_new (temp, LW_DICTTYPE_EDICT, i, error)) == NULL) all_regex_built = FALSE;
       }

       g_free (expression);
       if (expression_low)
           g_free (expression_low);
       re++;
     }
   }


   //Setup the expression to be used in the base of the regex for furigana strings
   re = ql->re_furi;
   for (iter = atoms; *iter != NULL && re < (ql->re_furi + length); iter++)
   {
     atom = *iter;
     expression = NULL;
     expression_low = NULL;

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
     }

#ifdef WITH_MECAB
     if (morpho_expression && iter == atoms) {
         // Stuff morphology regexp to the first atom
         if (expression == NULL) {
             expression_low = g_strdup(morpho_expression);
         }
         else {
             expression_low = g_strdup_printf("%s|%s", expression, morpho_expression);
         }
     }
#endif

     if (expression_low && expression == NULL) {
         expression = g_strdup("----------------");
     }

     if (expression) {
       //Compile the regexes
       temp = expression;
       for (i = 0; i < LW_RELEVANCE_TOTAL; i++) {
         if (expression_low && i == LW_RELEVANCE_LOW)
             temp = expression_low;
         if (((*re)[i] = lw_regex_furi_new (temp, LW_DICTTYPE_EDICT, i, error)) == NULL) all_regex_built = FALSE;
       }

       g_free (expression);
       if (expression_low)
           g_free (expression_low);
       re++;
     }
   }


   //Setup the expression to be used in the base of the regex
   re = ql->re_roma;
   for (iter = atoms; *iter != NULL && re < (ql->re_roma + length); iter++)
   {
     atom = *iter;
     g_strstrip(atom);
     if (strlen(atom) > 0 && lw_util_is_romaji_str (atom) && g_regex_match (lw_re[LW_RE_NUMBER], atom, 0, NULL) == FALSE)
     {
       expression = g_strdup (atom);

       //Compile the regexes
       for (i = 0; i < LW_RELEVANCE_TOTAL; i++)
         if (((*re)[i] = lw_regex_romaji_new (expression, LW_DICTTYPE_EDICT, i, error)) == NULL) all_regex_built = FALSE;

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
       for (i = 0; i < LW_RELEVANCE_TOTAL; i++)
         if (((*re)[i] = lw_regex_mix_new (expression, LW_DICTTYPE_EDICT, i, error)) == NULL) all_regex_built = FALSE;

       g_free (expression);
       re++;
     }
   }

   //Cleanup
   g_strfreev (atoms);
   atoms = NULL;

#ifdef WITH_MECAB
   if (morpho_expression)
       g_free(morpho_expression);
#endif

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
    int i;

    //Initializations
    all_regex_built = TRUE;

    //Search the query text for the specfic search terms
    g_regex_match (lw_re[INDEX], subject, 0, &match_info);

    //Allocate the memory depending on how many were found
    re = _queryline_allocate_pointers (g_match_info_get_match_count (match_info));

    //Investigate the matches and compile the new regexes
    for (iter = re; all_regex_built && iter - re < LW_QUERYLINE_MAX_ATOMS && g_match_info_matches (match_info); iter++)
    {
      match_text = g_match_info_fetch (match_info, 0);
      expression = g_strdup_printf("\\b%s\\b", match_text + 1);

      for (i = 0; all_regex_built && i < LW_RELEVANCE_TOTAL; i++)
        if (((*iter)[i] = lw_regex_new (expression, LW_DICTTYPE_KANJI, i, error)) == NULL) all_regex_built = FALSE;

      g_free (expression);
      g_free (match_text);

      g_match_info_next (match_info, NULL);
    }
    g_match_info_free (match_info);
    (*iter) = NULL;

    //Cleanup if there was an error
    for (iter = re; !all_regex_built && iter - re < LW_QUERYLINE_MAX_ATOMS; iter++)
    {
      for (i = 0; i < LW_RELEVANCE_TOTAL; i++)
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
//! @param ql Pointer to a LwQueryLine object ot parse a query string into.
//! @param pm A LwPreferences object to load preferences from
//! @param STRING constant string that is the raw query.
//! @param error A Pointer to a GError to load errors into or NULL
//! @returns Returns true if the string was successfully parsed.
//!
gboolean 
lw_queryline_parse_kanjidict_string (LwQueryLine *ql, LwPreferences *pm, const char* STRING, GError **error)
{
    //Sanity check
    if (error != NULL && *error != NULL) return FALSE;

    //Make sure it isn't already created
    if (ql->string != NULL && strcmp(ql->string, STRING) == 0) return TRUE;

    //Free previously used memory
    _queryline_free_pointers (ql);

    //Declarations
    GMatchInfo *match_info;
    int i;
    int length;
    char **atoms;
    char **iter;
    char *atom;
    char *ptr;
    char *start;
    char *end;
    GRegex ***re;
    gboolean all_regex_built;
    gunichar character;
    GUnicodeScript script;

    //Initializations
    all_regex_built = TRUE;
    if (ql->string != NULL) g_free (ql->string);
    ql->string = lw_util_prepare_query (STRING, FALSE);

    //Get stroke
    if ((ql->re_strokes = _compile_and_allocate_number_search_regex (STRING, LW_RE_STROKES, error)) == NULL)
      all_regex_built = FALSE;

    //Get frequency
    if ((ql->re_frequency = _compile_and_allocate_number_search_regex (STRING, LW_RE_FREQUENCY, error)) == NULL)
      all_regex_built = FALSE;

    //Get grade level
    if ((ql->re_grade = _compile_and_allocate_number_search_regex (STRING, LW_RE_GRADE, error)) == NULL)
      all_regex_built = FALSE;

    //Get JLPT level
    if ((ql->re_jlpt = _compile_and_allocate_number_search_regex (STRING, LW_RE_JLPT, error)) == NULL)
      all_regex_built = FALSE;

    //Get Kanji
    length = 0;
    for (ptr = ql->string; *ptr != '\0'; ptr = g_utf8_next_char (ptr))
    {
      character = g_utf8_get_char (ptr);
      script = g_unichar_get_script (character);
      if (script == G_UNICODE_SCRIPT_HAN) length++;
    }
    ql->re_kanji = _queryline_allocate_pointers (length);

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
        for (i = 0; all_regex_built && i < LW_RELEVANCE_TOTAL; i++)
          (*re)[i] = lw_regex_kanji_new (atom, LW_DICTTYPE_KANJI, i, error);
        g_free (atom);
        re++;
      }
    }

    //Get Hiragana and Katakana (aka Furigana)
    atoms = lw_util_get_furigana_atoms_from_string (ql->string);
    length = g_strv_length (atoms);
    ql->re_furi = _queryline_allocate_pointers (length);
    re = ql->re_furi;
    ptr = ql->string;

    for (iter = atoms; *iter != NULL; iter++)
    {
      atom = *iter;

      for (i = 0; all_regex_built && i < LW_RELEVANCE_TOTAL; i++)
        if (((*re)[i] = lw_regex_furi_new (atom, LW_DICTTYPE_KANJI, i, error)) == NULL) all_regex_built = FALSE;

      re++;
    }

    g_strfreev (atoms);

    //Romaji
    atoms = lw_util_get_romaji_atoms_from_string (ql->string);
    length = g_strv_length (atoms);
    ql->re_roma = _queryline_allocate_pointers (length);
    re = ql->re_roma;
    ptr = ql->string;

    for (iter = atoms; *iter != NULL; iter++)
    {
      atom = *iter;
      int match_start_byte_offset;
      int match_end_byte_offset;

      //This was a special regex that shouldn't be placed in the romaji area
      if (g_regex_match (lw_re[LW_RE_NUMBER], atom, 0, &match_info) == TRUE)
      {
        //Attepm to remove it
        g_match_info_fetch_pos (match_info, 0, &match_start_byte_offset, &match_end_byte_offset);
        for (i = match_start_byte_offset; i < match_end_byte_offset; i++)
          atom[i] = ' ';
        g_strstrip (atom);

        for (i = 0; strlen(atom) > 0 && all_regex_built && i < LW_RELEVANCE_TOTAL; i++)
          (*re)[i] = NULL;;

      }
      //Do the normal regex building
      else
      {
        for (i = 0; all_regex_built && i < LW_RELEVANCE_TOTAL; i++)
          if (((*re)[i] = lw_regex_romaji_new (atom, LW_DICTTYPE_KANJI, i, error)) == NULL) all_regex_built = FALSE;
      }

      g_match_info_free (match_info);
      re++;
    }

    g_strfreev (atoms);

    return all_regex_built;
}


//!
//! @brief Parses a query using the example style
//! @param ql Pointer to a LwQueryLine object ot parse a query string into.
//! @param pm A LwPreferences object to load preferences from
//! @param STRING constant string that is the raw query.
//! @param error A Pointer to a GError to load errors into or NULL
//! @returns Returns true if the string was successfully parsed.
//!
gboolean 
lw_queryline_parse_exampledict_string (LwQueryLine *ql, LwPreferences* pm, const char* STRING, GError **error)
{
    //Sanity check
    if (error != NULL && *error != NULL) return FALSE;

    //Make sure it isn't already created
    if (ql->string != NULL && strcmp(ql->string, STRING) == 0) return TRUE;

    //Free previously used memory
    _queryline_free_pointers (ql);

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
    if (pm != NULL)
    {
      rk_conv_pref = lw_preferences_get_int_by_schema (pm, LW_SCHEMA_BASE, LW_KEY_ROMAN_KANA);
      want_rk_conv = (rk_conv_pref == 0 || (rk_conv_pref == 2 && !lw_util_is_japanese_locale()));
      want_hk_conv = lw_preferences_get_boolean_by_schema (pm, LW_SCHEMA_BASE, LW_KEY_HIRA_KATA);
      want_kh_conv = lw_preferences_get_boolean_by_schema (pm, LW_SCHEMA_BASE, LW_KEY_KATA_HIRA);
      all_regex_built = TRUE;
    }
    else
    {
      rk_conv_pref = 1;
      want_rk_conv = TRUE;
      want_hk_conv = TRUE;
      want_kh_conv = TRUE;
    }
 
    atoms = _queryline_initialize_pointers (ql, STRING);
    length = g_strv_length (atoms);

    re = ql->re_kanji;
    for (iter = atoms; *iter != NULL && re < (ql->re_furi + length); iter++)
    {
      atom = *iter;
      if (lw_util_is_kanji_ish_str (atom) || lw_util_is_kanji_str (atom))
      {
        for (i = 0; all_regex_built && i < LW_RELEVANCE_TOTAL; i++)
          (*re)[i] = lw_regex_kanji_new (atom, LW_DICTTYPE_EXAMPLES, i, error);
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
        for (i = 0; i < LW_RELEVANCE_TOTAL; i++)
          if (((*re)[i] = lw_regex_furi_new (expression, LW_DICTTYPE_EDICT, i, error)) == NULL) all_regex_built = FALSE;

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
        for (i = 0; i < LW_RELEVANCE_TOTAL; i++)
          if (((*re)[i] = lw_regex_furi_new (expression, LW_DICTTYPE_EDICT, i, error)) == NULL) all_regex_built = FALSE;

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
      if (strlen(atom) > 0 && lw_util_is_romaji_str (atom) && g_regex_match (lw_re[LW_RE_NUMBER], atom, 0, NULL) == FALSE)
      {
        expression = g_strdup (atom);
 
       //Compile the regexes
        for (i = 0; i < LW_RELEVANCE_TOTAL; i++)
          if (((*re)[i] = lw_regex_romaji_new (expression, LW_DICTTYPE_EDICT, i, error)) == NULL) all_regex_built = FALSE;
 
        g_free (expression);
        re++;
      }
    }  

    //Cleanup
    g_strfreev (atoms);
    atoms = NULL;

    return all_regex_built;
}


