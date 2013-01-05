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
//!  @file unknowndictionary.c
//!
//!  @brief LwDictionary objects represent a loaded dictionary that the program
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

#include <libwaei/libwaei.h>
#include <libwaei/dictionary-private.h>
#include <libwaei/gettext.h>

static gboolean lw_unknowndictionary_parse_query (LwDictionary*, LwQuery*, const gchar*, GError**);
static gint lw_unknowndictionary_parse_result (LwDictionary*, LwResult*, FILE*);
static gboolean lw_unknowndictionary_compare (LwDictionary*, LwQuery*, LwResult*, const LwRelevance);
static void lw_unknowndictionary_create_primary_tokens (LwDictionary*, LwQuery*);
static void lw_unknowndictionary_add_supplimental_tokens (LwDictionary*, LwQuery*);


G_DEFINE_TYPE (LwUnknownDictionary, lw_unknowndictionary, LW_TYPE_DICTIONARY)


LwDictionary* lw_unknowndictionary_new (const gchar *FILENAME)
{
    //Declarations
    LwDictionary *dictionary;

    //Initializations
    dictionary = LW_DICTIONARY (g_object_new (LW_TYPE_UNKNOWNDICTIONARY,
                                "filename", FILENAME,
                                 NULL));

    return dictionary;
}


static void 
lw_unknowndictionary_init (LwUnknownDictionary *dictionary)
{
}


static void
lw_unknowndictionary_constructed (GObject *object)
{
    //Chain the parent class
    {
      G_OBJECT_CLASS (lw_unknowndictionary_parent_class)->constructed (object);
    }

/*
    LwDictionary *dictionary;
    LwDictionaryPrivate *priv;

    dictionary = LW_DICTIONARY (object);
    priv = dictionary->priv;
*/
}


static void 
lw_unknowndictionary_finalize (GObject *object)
{
    G_OBJECT_CLASS (lw_unknowndictionary_parent_class)->finalize (object);
}


static void
lw_unknowndictionary_class_init (LwUnknownDictionaryClass *klass)
{
    //Declarations
    GObjectClass *object_class;
    LwDictionaryClass *dictionary_class;
    gint i;

    //Initializations
    object_class = G_OBJECT_CLASS (klass);
    object_class->finalize = lw_unknowndictionary_finalize;
    object_class->constructed = lw_unknowndictionary_constructed;

    dictionary_class = LW_DICTIONARY_CLASS (klass);
    dictionary_class->parse_query = lw_unknowndictionary_parse_query;
    dictionary_class->parse_result = lw_unknowndictionary_parse_result;
    dictionary_class->compare = lw_unknowndictionary_compare;

    dictionary_class->patterns = g_new0 (gchar**, TOTAL_LW_QUERY_TYPES + 1);
    for (i = 0; i < TOTAL_LW_QUERY_TYPES; i++)
    {
      dictionary_class->patterns[i] = g_new0 (gchar*, TOTAL_LW_RELEVANCE + 1);
    }

    dictionary_class->patterns[LW_QUERY_TYPE_KANJI][LW_RELEVANCE_LOW] = "(%s)";
    dictionary_class->patterns[LW_QUERY_TYPE_KANJI][LW_RELEVANCE_MEDIUM] = "(^|お|を|に|で|は|と|)(%s)(で|が|の|を|に|で|は|と|$)";
    dictionary_class->patterns[LW_QUERY_TYPE_KANJI][LW_RELEVANCE_HIGH] = "^(無|不|非|お|御|)(%s)$";

    dictionary_class->patterns[LW_QUERY_TYPE_FURIGANA][LW_RELEVANCE_LOW] = "(%s)";
    dictionary_class->patterns[LW_QUERY_TYPE_FURIGANA][LW_RELEVANCE_MEDIUM] = "(%s)";
    dictionary_class->patterns[LW_QUERY_TYPE_FURIGANA][LW_RELEVANCE_HIGH] = "(%s)";
    //dictionary_class->patterns[LW_QUERY_TYPE_FURIGANA][LW_RELEVANCE_MEDIUM] = "(^|お|を|に|で|は|と)(%s)(で|が|の|を|に|で|は|と|$)";
    //dictionary_class->patterns[LW_QUERY_TYPE_FURIGANA][LW_RELEVANCE_HIGH] = "^(お|)(%s)$";

    dictionary_class->patterns[LW_QUERY_TYPE_ROMAJI][LW_RELEVANCE_LOW] = "(%s)";
    dictionary_class->patterns[LW_QUERY_TYPE_ROMAJI][LW_RELEVANCE_MEDIUM] = "(\\) |/)((\\bto )|(\\bto be )|(\\b))(%s)(( \\([^/]+\\)/)|(/))";
    dictionary_class->patterns[LW_QUERY_TYPE_ROMAJI][LW_RELEVANCE_HIGH] = "(^|\\)|/|^to |\\) )(%s)(\\(|/|$|!| \\()";

    dictionary_class->patterns[LW_QUERY_TYPE_MIX][LW_RELEVANCE_LOW] = "(%s)";
    dictionary_class->patterns[LW_QUERY_TYPE_MIX][LW_RELEVANCE_MEDIUM] = "(%s)";
    dictionary_class->patterns[LW_QUERY_TYPE_MIX][LW_RELEVANCE_HIGH] = "(%s)";
}



//!
//! @brief Parses a string for an unknown format string
//! @param rl The Resultline object this method works on
//!
static gint
lw_unknowndictionary_parse_result (LwDictionary *dictionary, LwResult *result, FILE *fd)
{
    gchar *ptr = NULL;
    gint bytes_read = 0;

    lw_result_clear (result);

    //Read the next line
    do {
      ptr = fgets(result->text, LW_IO_MAX_FGETS_LINE, fd);
      if (ptr != NULL) bytes_read += strlen(result->text);
    } while (ptr != NULL && *ptr == '#');

    return bytes_read;
}


static gboolean 
lw_unknowndictionary_parse_query (LwDictionary *dictionary, LwQuery *query, const gchar *TEXT, GError **error)
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
 
    lw_unknowndictionary_create_primary_tokens (dictionary, query);
    lw_unknowndictionary_add_supplimental_tokens (dictionary, query);
    lw_dictionary_build_regex (dictionary, query, error);

    return (error == NULL || *error == NULL);
}




static gboolean 
lw_unknowndictionary_compare (LwDictionary *dictionary, LwQuery *query, LwResult *result, const LwRelevance RELEVANCE)
{
    //Sanity checks
    g_return_val_if_fail (dictionary != NULL, FALSE);
    g_return_val_if_fail (query != NULL, FALSE);
    g_return_val_if_fail (result != NULL, FALSE);

    //Declarations
    gboolean found;
    gboolean checked;
    GList *link;
    GRegex *regex;

    //Initializations
    checked = FALSE;
    found = TRUE;

    //Compare kanji atoms
    link = lw_query_regexgroup_get (query, LW_QUERY_TYPE_KANJI, RELEVANCE);
    while (link != NULL)
    {
      regex = link->data;
      if (regex == NULL || result->text == NULL) return FALSE;

      checked = TRUE;
      found = g_regex_match (regex, result->text, 0, NULL);
      if (found == FALSE) return found;

      link = link->next;
    }

    //Compare furigana atoms
    link = lw_query_regexgroup_get (query, LW_QUERY_TYPE_FURIGANA, RELEVANCE);
    while (link != NULL)
    {
      regex = link->data;
      if (regex == NULL || result->text == NULL) return FALSE;

      checked = TRUE;
      found = g_regex_match (regex, result->text, 0, NULL);
      if (found == FALSE) return found;

      link = link->next;
    }

    //Compare romaji atoms
    link = lw_query_regexgroup_get (query, LW_QUERY_TYPE_ROMAJI, RELEVANCE);
    while (link != NULL)
    {
      regex = link->data;
      if (regex == NULL || result->text == NULL) return FALSE;

      checked = TRUE;
      found = g_regex_match (regex, result->text, 0, NULL);
      if (found == FALSE) return found;

      link = link->next;
    }

    return (checked && found);
}


static void
lw_unknowndictionary_create_primary_tokens (LwDictionary *dictionary, LwQuery *query)
{
    //Declarations
    gchar *temp;
    gchar *delimited;
    gboolean split_script_changes, split_whitespace;
    gchar **tokens;
    gchar **tokeniter;
    
    //Initializations
    delimited = lw_util_prepare_query (lw_query_get_text (query), TRUE);
    split_script_changes = split_whitespace = TRUE;

    if (split_script_changes)
    {
      temp = lw_util_delimit_script_changes (LW_QUERY_DELIMITOR_PRIMARY_STRING, delimited, FALSE);
      g_free (delimited); delimited = temp; temp = NULL;
    }

    if (split_whitespace)
    {
      temp = lw_util_delimit_whitespace (LW_QUERY_DELIMITOR_PRIMARY_STRING, delimited);
      g_free (delimited); delimited = temp; temp = NULL;
    }

    tokeniter = tokens = g_strsplit (delimited, LW_QUERY_DELIMITOR_PRIMARY_STRING, -1);

    if (tokens != NULL)
    {
      while (*tokeniter != NULL)
      {
        if (lw_util_is_furigana_str (*tokeniter))
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

    if (temp != NULL) g_free (temp); temp = NULL;
}


static void 
lw_unknowndictionary_add_supplimental_tokens (LwDictionary *dictionary, LwQuery *query)
{
/*
          if (get_japanese_morphology)
          {
            lw_morphology_get_stem ()
            query->tokenlist[LW_QUERY_TYPE_KANJI] = g_list_append (query->tokenlist[LW_QUERY_TYPE_KANJI], tokens[i]);
          }
*/
}

