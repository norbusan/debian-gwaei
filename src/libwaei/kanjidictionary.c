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
//!  @file kanjidictionary.c
//!
//!  @brief LwKanjiDictionary objects represent a loaded dictionary that the program
//!         can use to carry out searches.  You can uninstall dictionaries
//!         by using the object, but you cannot install them. LwDictInst
//!         objects exist for that purpose.
//!

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <glib.h>

#include <libwaei/gettext.h>
#include <libwaei/libwaei.h>
#include <libwaei/dictionary-private.h>

G_DEFINE_TYPE (LwKanjiDictionary, lw_kanjidictionary, LW_TYPE_DICTIONARY)

static gboolean lw_kanjidictionary_parse_query (LwDictionary*, LwQuery*, const gchar*, GError **);
static gint lw_kanjidictionary_parse_result (LwDictionary*, LwResult*, FILE*);
static gboolean lw_kanjidictionary_compare (LwDictionary *dictionary, LwQuery*, LwResult*, const LwRelevance);
static gboolean lw_kanjidictionary_installer_postprocess (LwDictionary*, gchar**, gchar**, LwIoProgressCallback, gpointer, GCancellable*, GError**);

static void lw_kanjidictionary_create_primary_tokens (LwDictionary*, LwQuery*);

LwDictionary* lw_kanjidictionary_new (const gchar *FILENAME)
{
    //Declarations
    LwDictionary *dictionary;

    //Initializations
    dictionary = LW_DICTIONARY (g_object_new (LW_TYPE_KANJIDICTIONARY,
                                "filename", FILENAME,
                                NULL));

    return dictionary;
}


static void 
lw_kanjidictionary_init (LwKanjiDictionary *dictionary)
{
}


static void
lw_kanjidictionary_constructed (GObject *object)
{
    //Chain the parent class
    {
      G_OBJECT_CLASS (lw_kanjidictionary_parent_class)->constructed (object);
    }

    LwDictionary *dictionary;
    LwDictionaryPrivate *priv;

    dictionary = LW_DICTIONARY (object);
    priv = dictionary->priv;

    if (strcmp(priv->filename, "Kanji") == 0)
    {
      if (priv->name != NULL) g_free (priv->name); priv->name = NULL;
      priv->name = g_strdup (gettext("Kanji"));
    }

}


static void 
lw_kanjidictionary_finalize (GObject *object)
{
    G_OBJECT_CLASS (lw_kanjidictionary_parent_class)->finalize (object);
}


static void
lw_kanjidictionary_class_init (LwKanjiDictionaryClass *klass)
{
    //Declarations
    GObjectClass *object_class;
    LwDictionaryClass *dictionary_class;
    gint i;

    //Initializations
    object_class = G_OBJECT_CLASS (klass);
    object_class->finalize = lw_kanjidictionary_finalize;
    object_class->constructed = lw_kanjidictionary_constructed;

    dictionary_class = LW_DICTIONARY_CLASS (klass);
    dictionary_class->parse_query = lw_kanjidictionary_parse_query;
    dictionary_class->parse_result = lw_kanjidictionary_parse_result;
    dictionary_class->compare = lw_kanjidictionary_compare;
    dictionary_class->installer_postprocess = lw_kanjidictionary_installer_postprocess;

    dictionary_class->patterns = g_new0 (gchar**, TOTAL_LW_QUERY_TYPES + 1);
    for (i = 0; i < TOTAL_LW_QUERY_TYPES; i++)
    {
      dictionary_class->patterns[i] = g_new0 (gchar*, TOTAL_LW_RELEVANCE + 1);
    }

    dictionary_class->patterns[LW_QUERY_TYPE_KANJI][LW_RELEVANCE_LOW] = "(%s)";
    dictionary_class->patterns[LW_QUERY_TYPE_KANJI][LW_RELEVANCE_MEDIUM] = "(%s)";
    dictionary_class->patterns[LW_QUERY_TYPE_KANJI][LW_RELEVANCE_HIGH] = "^(%s)$";

    dictionary_class->patterns[LW_QUERY_TYPE_FURIGANA][LW_RELEVANCE_LOW] = "(^|\\s)(%s)(\\s|$)";
    dictionary_class->patterns[LW_QUERY_TYPE_FURIGANA][LW_RELEVANCE_MEDIUM] = "(^|\\s)(%s)(\\s|$)";
    dictionary_class->patterns[LW_QUERY_TYPE_FURIGANA][LW_RELEVANCE_HIGH] = "(^|\\s)(%s)(\\s|$)";

    dictionary_class->patterns[LW_QUERY_TYPE_ROMAJI][LW_RELEVANCE_LOW] = "(%s)";
    dictionary_class->patterns[LW_QUERY_TYPE_ROMAJI][LW_RELEVANCE_MEDIUM] = "\\b(%s)\\b";
    dictionary_class->patterns[LW_QUERY_TYPE_ROMAJI][LW_RELEVANCE_HIGH] =  "\\{(%s)\\}";

    dictionary_class->patterns[LW_QUERY_TYPE_MIX][LW_RELEVANCE_LOW] = "(%s)";
    dictionary_class->patterns[LW_QUERY_TYPE_MIX][LW_RELEVANCE_MEDIUM] = "(%s)";
    dictionary_class->patterns[LW_QUERY_TYPE_MIX][LW_RELEVANCE_HIGH] = "(%s)";
}


static gboolean 
lw_kanjidictionary_parse_query (LwDictionary *dictionary, LwQuery *query, const gchar *TEXT, GError **error)
{
    //Sanity checks
    g_return_val_if_fail (dictionary != NULL, FALSE);
    g_return_val_if_fail (query != NULL, FALSE);
    g_return_val_if_fail (query->tokenlist != NULL, FALSE);
    g_return_val_if_fail (TEXT != NULL, FALSE);
    g_return_val_if_fail (error != NULL, FALSE);
    if (error != NULL && *error != NULL) return FALSE;

    //Sanity check
    g_return_val_if_fail (dictionary != NULL && query != NULL && TEXT != NULL, FALSE);

    lw_query_init_rangelist (query);
    lw_kanjidictionary_create_primary_tokens (dictionary, query);
    lw_dictionary_build_regex (dictionary, query, error);

    return (error == NULL || *error == NULL);
}


//!
//! @brief, Retrieve a line from FILE, parse it according to the LwKanjiDictionary rules and put the results into the LwResult
//!
static gint
lw_kanjidictionary_parse_result (LwDictionary *dictionary, LwResult *result, FILE *fd)
{
    //Declarations
    GMatchInfo* match_info;
    gint start[LW_RE_TOTAL];
    gint end[LW_RE_TOTAL];
    GUnicodeScript script;
    gchar *ptr = result->text;
    gint bytes_read = 0;

    lw_result_clear (result);

    //Read the next line
    do {
      ptr = fgets(result->text, LW_IO_MAX_FGETS_LINE, fd);
      if (ptr != NULL) bytes_read += strlen(result->text);
    } while (ptr != NULL && *ptr == '#');

    if (ptr == NULL) return bytes_read;
    bytes_read += strlen(result->text);


    //First generate the grade, stroke, frequency, and jlpt fields

    //Get strokes
    result->strokes = NULL;
    g_regex_match (lw_re[LW_RE_STROKES], ptr, 0, &match_info);
    if (g_match_info_matches (match_info))
    {
      g_match_info_fetch_pos (match_info, 0, &start[LW_RE_STROKES], &end[LW_RE_STROKES]);
      result->strokes = ptr + start[LW_RE_STROKES] + 1;
    }
    g_match_info_free (match_info);

    //Get frequency
    result->frequency = NULL;
    g_regex_match (lw_re[LW_RE_FREQUENCY], ptr, 0, &match_info);
    if (g_match_info_matches (match_info))
    {
      g_match_info_fetch_pos (match_info, 0, &start[LW_RE_FREQUENCY], &end[LW_RE_FREQUENCY]);
      result->frequency = ptr + start[LW_RE_FREQUENCY] + 1;
    }
    g_match_info_free (match_info);


    //Get grade level
    result->grade = NULL;
    g_regex_match (lw_re[LW_RE_GRADE], ptr, 0, &match_info);
    if (g_match_info_matches (match_info))
    {
      g_match_info_fetch_pos (match_info, 0, &start[LW_RE_GRADE], &end[LW_RE_GRADE]);
      result->grade = ptr + start[LW_RE_GRADE] + 1;
    }
    g_match_info_free (match_info);

    //Get JLPT level
    result->jlpt = NULL;
    g_regex_match (lw_re[LW_RE_JLPT], ptr, 0, &match_info);
    if (g_match_info_matches (match_info))
    {
      g_match_info_fetch_pos (match_info, 0, &start[LW_RE_JLPT], &end[LW_RE_JLPT]);
      result->jlpt = ptr + start[LW_RE_JLPT] + 1;
    }
    g_match_info_free (match_info);


    //Get the kanji character
    result->kanji = ptr;
    ptr = g_utf8_strchr (ptr, -1, g_utf8_get_char (" "));
    if (ptr == NULL)
    {
      fprintf(stderr, "This dictionary is incorrectly formatted\n");
      exit (1);
    }
    *ptr = '\0';
    ptr++;

    //Test if the radicals information is present
    result->radicals = NULL;
    script = g_unichar_get_script (g_utf8_get_char (ptr));
    if (script != G_UNICODE_SCRIPT_LATIN)
    {
      result->radicals = ptr;
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
    result->readings[0] = ptr;

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
          result->readings[1] = ptr;
        }
        else if (g_utf8_get_char (ptr) == g_utf8_get_char ("2"))
        {
          *(ptr - 1) = '\0';
          ptr = g_utf8_next_char (ptr);
          ptr = g_utf8_next_char (ptr);
          result->readings[2] = ptr;
        }
      }
      else
      {
        ptr = g_utf8_next_char (ptr);
      }
    }
    *(ptr - 1) = '\0';

    result->meanings = ptr;

    if ((ptr = g_utf8_strrchr (ptr, -1, g_utf8_get_char ("\n"))) != NULL)
      *ptr = '\0';

    if (result->strokes)   *(result->text + end[LW_RE_STROKES]) = '\0';
    if (result->frequency) *(result->text + end[LW_RE_FREQUENCY]) = '\0';
    if (result->grade)     *(result->text + end[LW_RE_GRADE]) = '\0';
    if (result->jlpt)      *(result->text + end[LW_RE_JLPT]) = '\0';

    return bytes_read;
}



static gboolean 
lw_kanjidictionary_compare (LwDictionary *dictionary, LwQuery *query, LwResult *result, const LwRelevance RELEVANCE)
{
    //Declarations
    GList *link;
    gboolean found;
    gboolean checked;
    LwRange *range;
    GRegex *regex;
    gint i;

    //Initializations
    found = FALSE;
    checked = FALSE;

    //Calculate the strokes check
    range = lw_query_rangelist_get (query, LW_QUERY_RANGE_TYPE_STROKES);
    if (result->strokes != NULL && range != NULL)
    {
      checked = TRUE;
      found = lw_range_string_is_in_range (range, result->strokes);
      if (found == FALSE) return FALSE;
    }

    //Calculate the frequency check
    range = lw_query_rangelist_get (query, LW_QUERY_RANGE_TYPE_FREQUENCY);
    if (result->frequency != NULL && range != NULL)
    {
      checked = TRUE;
      found = lw_range_string_is_in_range (range, result->frequency);
      if (found == FALSE) return FALSE;
    }

    //Calculate the grade check
    range = lw_query_rangelist_get (query, LW_QUERY_RANGE_TYPE_GRADE);
    if (result->grade != NULL && range != NULL)
    {
      checked = TRUE;
      found = lw_range_string_is_in_range (range, result->grade);
      if (found == FALSE) return FALSE;
    }

    //Calculate the jlpt check
    range = lw_query_rangelist_get (query, LW_QUERY_RANGE_TYPE_JLPT);
    if (result->jlpt != NULL && range != NULL)
    {
      checked = TRUE;
      found = lw_range_string_is_in_range (range, result->jlpt);
      if (found == FALSE) return FALSE;
    }

    //Compare romaji atoms
    link = lw_query_regexgroup_get (query, LW_QUERY_TYPE_ROMAJI, RELEVANCE);
    while (link != NULL)
    {
      regex = link->data;
      if (regex == NULL && result->meanings == NULL) return FALSE;

      checked = TRUE;
      found = g_regex_match (regex, result->meanings, 0, NULL);
      if (found == FALSE) return found;

      link = link->next;
    }

    //Compare furigana atoms
    link = lw_query_regexgroup_get (query, LW_QUERY_TYPE_FURIGANA, RELEVANCE);
    while (link != NULL)
    {
      regex = link->data;
      if (regex == NULL) return FALSE;
      checked = TRUE;

      for (i = 0; i < 3 && result->readings[i] != NULL; i++)
      {
        found = g_regex_match (regex, result->readings[i], 0, NULL);
        if (found == TRUE) break;
      }

      if (found == FALSE) return found;
      link = link->next;
    }

    //Compare kanji atoms
    link = lw_query_regexgroup_get (query, LW_QUERY_TYPE_KANJI, RELEVANCE);
    while (link != NULL)
    {
      regex = link->data;
      if (regex == NULL || result->kanji == NULL) return FALSE;
      checked = TRUE;

      found = g_regex_match (regex, result->kanji, 0, NULL);
      if (found == FALSE) found = g_regex_match (regex, result->radicals, 0, NULL);
      if (found == FALSE) return FALSE;

      link = link->next;
    }

    //Return our results
    return (checked && found);
}


static gboolean
lw_kanjidictionary_installer_postprocess (LwDictionary *dictionary, 
                                          gchar **sourcelist, 
                                          gchar **targetlist, 
                                          LwIoProgressCallback cb,
                                          gpointer data,
                                          GCancellable *cancellable,
                                          GError **error)
{
    g_return_val_if_fail (dictionary != NULL, FALSE);
    g_return_val_if_fail (g_strv_length (sourcelist) > 1, FALSE);
    g_return_val_if_fail (g_strv_length (targetlist) > 0, FALSE);

    return lw_io_create_mix_dictionary (targetlist[0], sourcelist[0], sourcelist[1], cb, data, cancellable, error);
}


static void
lw_kanjidictionary_create_primary_tokens (LwDictionary *dictionary, LwQuery *query)
{
    //Declarations
    gchar *temp;
    gchar *delimited;
    gchar **tokens;
    gchar **tokeniter;
    gboolean split_script_changes, split_whitespace;
    
    //Initializations
    delimited = lw_util_prepare_query (lw_query_get_text (query), TRUE);
    split_script_changes = split_whitespace = TRUE;

    if (split_script_changes)
    {
      temp = lw_util_delimit_script_changes (LW_QUERY_DELIMITOR_PRIMARY_STRING, delimited, TRUE);
      g_free (delimited); delimited = temp; temp = NULL;
    }

    if (split_whitespace)
    {
      temp = lw_util_delimit_whitespace (LW_QUERY_DELIMITOR_PRIMARY_STRING, delimited);
      g_free (delimited); delimited = temp; temp = NULL;
    }

    temp = lw_util_delimit_radicals (LW_QUERY_DELIMITOR_PRIMARY_STRING, delimited);
    g_free (delimited); delimited = temp; temp = NULL;

    tokeniter = tokens = g_strsplit (delimited, LW_QUERY_DELIMITOR_PRIMARY_STRING, -1);

    if (tokens != NULL)
    {
      while (*tokeniter != NULL)
      {
        if (lw_range_pattern_is_valid (*tokeniter))
        {
          LwRange *range = lw_range_new_from_pattern (*tokeniter);
          if (**tokeniter == 's' || **tokeniter == 'S')
            lw_query_rangelist_set (query, LW_QUERY_RANGE_TYPE_STROKES, range);
          else if (**tokeniter == 'g' || **tokeniter == 'G')
            lw_query_rangelist_set (query, LW_QUERY_RANGE_TYPE_GRADE, range);
          else if (**tokeniter == 'f' || **tokeniter == 'f')
            lw_query_rangelist_set (query, LW_QUERY_RANGE_TYPE_FREQUENCY, range);
          else if (**tokeniter == 'j' || **tokeniter == 'J')
            lw_query_rangelist_set (query, LW_QUERY_RANGE_TYPE_JLPT, range);
          else
            lw_range_free (range);
        }
        else if (lw_util_is_furigana_str (*tokeniter))
          lw_query_tokenlist_append_primary (query, LW_QUERY_TYPE_FURIGANA, *tokeniter);
        else if (lw_util_is_kanji_ish_str (*tokeniter))
          lw_query_tokenlist_append_primary (query, LW_QUERY_TYPE_KANJI, *tokeniter);
        else if (lw_util_is_romaji_str (*tokeniter))
          lw_query_tokenlist_append_primary (query, LW_QUERY_TYPE_ROMAJI, *tokeniter);
        else
          lw_query_tokenlist_append_primary (query, LW_QUERY_TYPE_MIX, *tokeniter);
        tokeniter++;
      }
      g_strfreev (tokens); tokens = NULL;
    }

    if (delimited != NULL) g_free (delimited); delimited = NULL;
}

