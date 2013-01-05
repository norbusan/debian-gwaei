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
//!  @file edictionary.c
//!
//!  @brief LwEDictionary objects represent a loaded dictionary that the program
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

G_DEFINE_TYPE (LwEDictionary, lw_edictionary, LW_TYPE_DICTIONARY)

static gchar* FIRST_DEFINITION_PREFIX_STR = "(1)";
static gboolean lw_edictionary_parse_query (LwDictionary*, LwQuery*, const gchar*, GError**);
static gboolean lw_edictionary_parse_result (LwDictionary*, LwResult*, FILE*);
static gboolean lw_edictionary_compare (LwDictionary*, LwQuery*, LwResult*, const LwRelevance);
static gboolean lw_edictionary_installer_postprocess (LwDictionary*, gchar**, gchar**, LwIoProgressCallback, gpointer, GCancellable*, GError**);
static void lw_edictionary_create_primary_tokens (LwDictionary*, LwQuery*);


LwDictionary* lw_edictionary_new (const gchar *FILENAME)
{
    //Declarations
    LwDictionary *dictionary;

    //Initializations
    dictionary = LW_DICTIONARY (g_object_new (LW_TYPE_EDICTIONARY,
                                "filename", FILENAME,
                                NULL));

    return dictionary;
}


static void 
lw_edictionary_init (LwEDictionary *dictionary)
{
}


static void
lw_edictionary_constructed (GObject *object)
{
    //Chain the parent class
    {
      G_OBJECT_CLASS (lw_edictionary_parent_class)->constructed (object);
    }

    LwDictionary *dictionary;
    LwDictionaryPrivate *priv;

    dictionary = LW_DICTIONARY (object);
    priv = dictionary->priv;

    if (strcmp(priv->filename, "English") == 0)
    {
      if (priv->name != NULL) g_free (priv->name); priv->name = NULL;
      priv->name = g_strdup (gettext("English"));
    }
    else if (strcmp(priv->filename, "Names") == 0)
    {
      if (priv->name != NULL) g_free (priv->name); priv->name = NULL;
      priv->name = g_strdup (gettext("Names"));
    }
    else if (strcmp(priv->filename, "Places") == 0)
    {
      if (priv->name != NULL) g_free (priv->name); priv->name = NULL;
      priv->name = g_strdup (gettext("Places"));
    }
    else if (strcmp(priv->filename, "Names and Places") == 0)
    {
      if (priv->name != NULL) g_free (priv->name); priv->name = NULL;
      priv->name = g_strdup (gettext("Names and Places"));
    }
}


static void 
lw_edictionary_finalize (GObject *object)
{
    G_OBJECT_CLASS (lw_edictionary_parent_class)->finalize (object);
}


static void
lw_edictionary_class_init (LwEDictionaryClass *klass)
{
    //Declarations
    GObjectClass *object_class;
    LwDictionaryClass *dictionary_class;
    gint i;

    //Initializations
    object_class = G_OBJECT_CLASS (klass);
    object_class->finalize = lw_edictionary_finalize;
    object_class->constructed = lw_edictionary_constructed;

    dictionary_class = LW_DICTIONARY_CLASS (klass);
    dictionary_class->parse_query = lw_edictionary_parse_query;
    dictionary_class->parse_result = lw_edictionary_parse_result;
    dictionary_class->compare = lw_edictionary_compare;
    dictionary_class->installer_postprocess = lw_edictionary_installer_postprocess;

    dictionary_class->patterns = g_new0 (gchar**, TOTAL_LW_QUERY_TYPES + 1);
    for (i = 0; i < TOTAL_LW_QUERY_TYPES; i++)
    {
      dictionary_class->patterns[i] = g_new0 (gchar*, TOTAL_LW_RELEVANCE + 1);
    }

    dictionary_class->patterns[LW_QUERY_TYPE_KANJI][LW_RELEVANCE_LOW] = "(%s)";
//TODO
    //dictionary_class->patterns[LW_QUERY_TYPE_KANJI][LW_RELEVANCE_MEDIUM] = "^(お|を|に|で|は|と|)(%s)(で|が|の|を|に|で|は|と|$)";
    dictionary_class->patterns[LW_QUERY_TYPE_KANJI][LW_RELEVANCE_MEDIUM] = "^(お|を|に|で|は|と|)(%s)(で|が|の|を|に|で|は|と)$";
    dictionary_class->patterns[LW_QUERY_TYPE_KANJI][LW_RELEVANCE_HIGH] = "^(無|不|非|お|御|)(%s)$";

//TODO
//    dictionary_class->patterns[LW_QUERY_TYPE_FURIGANA][LW_RELEVANCE_LOW] = "(\\b|お|を|に|で|は|と)(%s)(で|が|の|を|に|で|は|と|\\b)";
    dictionary_class->patterns[LW_QUERY_TYPE_FURIGANA][LW_RELEVANCE_LOW] = "(%s)";
    dictionary_class->patterns[LW_QUERY_TYPE_FURIGANA][LW_RELEVANCE_MEDIUM] = "(\\b|お|を|に|で|は|と)(%s)(で|が|の|を|に|で|は|と|)";
    dictionary_class->patterns[LW_QUERY_TYPE_FURIGANA][LW_RELEVANCE_HIGH] = "\\b(お|)(%s)\\b";

    dictionary_class->patterns[LW_QUERY_TYPE_ROMAJI][LW_RELEVANCE_LOW] = "(%s)";
    dictionary_class->patterns[LW_QUERY_TYPE_ROMAJI][LW_RELEVANCE_MEDIUM] = "(\\) |/)((\\bto )|(\\bto be )|(\\b))(%s)(( \\([^/]+\\)/)|(/))";
    dictionary_class->patterns[LW_QUERY_TYPE_ROMAJI][LW_RELEVANCE_HIGH] = "(^|\\)|/|^to |\\) )(%s)(\\(|/|$|!| \\()";

    dictionary_class->patterns[LW_QUERY_TYPE_MIX][LW_RELEVANCE_LOW] = "(%s)";
    dictionary_class->patterns[LW_QUERY_TYPE_MIX][LW_RELEVANCE_MEDIUM] = "\\b(%s)\\b";
    dictionary_class->patterns[LW_QUERY_TYPE_MIX][LW_RELEVANCE_HIGH] = "^(%s)$";
}


static gboolean 
lw_edictionary_parse_query (LwDictionary *dictionary, LwQuery *query, const gchar *TEXT, GError **error)
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
    lw_edictionary_create_primary_tokens (dictionary, query);  //fist&&second
    lw_dictionary_build_regex (dictionary, query, error);

    return (error == NULL || *error == NULL);
}


//!
//! @brief, Retrieve a line from FILE, parse it according to the LwEDictionary rules and put the results into the LwResult
//!
static gint 
lw_edictionary_parse_result (LwDictionary *dictionary, LwResult *result, FILE *fd)
{
    gchar *ptr = NULL;
    gchar *next = NULL;
    gchar *nextnext = NULL;
    gchar *nextnextnext = NULL;
    gchar *temp = NULL;
    gint bytes_read = 0;

    lw_result_clear (result);

    //Read the next line
    do {
      ptr = fgets(result->text, LW_IO_MAX_FGETS_LINE, fd);
      if (ptr != NULL) bytes_read += strlen(result->text);
    } while (ptr != NULL && *ptr == '#');

    if (ptr == NULL) return bytes_read;
    bytes_read += strlen(result->text);

    //Remove the final line break
    if ((temp = g_utf8_strchr (result->text, -1, '\n')) != NULL)
    {
        temp--;
        *temp = '\0';
    }

    //Set the kanji pointers
    result->kanji_start = ptr;
    ptr = g_utf8_strchr (ptr, -1, L' ');
    *ptr = '\0';

    //Set the furigana pointer
    ptr++;
    if (g_utf8_get_char(ptr) == L'[' && g_utf8_strchr (ptr, -1, L']') != NULL)
    {
      ptr = g_utf8_next_char(ptr);
      result->furigana_start = ptr;
      ptr = g_utf8_strchr (ptr, -1, L']');
      *ptr = '\0';
    }
    else
    {
      result->furigana_start = NULL;
      ptr--;
    }

    //Find if there is a type description classification
    temp = ptr;
    temp++;
    temp = g_utf8_strchr (temp, -1, L'/');
    if (temp != NULL && g_utf8_get_char(temp + 1) == L'(')
    {
      result->classification_start = temp + 2;
      temp = g_utf8_strchr (temp, -1, L')');
      *temp = '\0';
      ptr = temp;
    }

    //Set the definition pointers
    ptr++;
    ptr = g_utf8_next_char(ptr);
    result->def_start[0] = ptr;
    result->number[0] = FIRST_DEFINITION_PREFIX_STR;
    gint i = 1;

    temp = ptr;
    while ((temp = g_utf8_strchr(temp, -1, L'(')) != NULL && i < 50)
    {
      next = g_utf8_next_char (temp);
      nextnext = g_utf8_next_char (next);
      nextnextnext = g_utf8_next_char (nextnext);
      if (*next != '\0' && *nextnext != '\0' &&
          *next == L'1' && *nextnext == L')')
      {
         result->def_start[0] = result->def_start[0] + 4;
      }
      else if (*next != '\0' && *nextnext != '\0' && *nextnextnext != '\0' &&
               *next >= L'1' && *next <= L'9' && (*nextnext == L')' || *nextnextnext == L')'))
      {
         *(temp - 1) = '\0';
         result->number[i] = temp;
         temp = g_utf8_strchr (temp, -1, L')');
         *(temp + 1) = '\0';
         result->def_start[i] = temp + 2;
         i++;
      }
      temp = temp + 2;
    }
    result->def_total = i;
    result->def_start[i] = NULL;
    result->number[i] = NULL;
    i--;

    //Get the importance
    //temp = result->def_start[i] + strlen(result->def_start[i]) - 4;
    if ((temp = g_utf8_strrchr (result->def_start[i], -1, L'(')) != NULL)
    {
      result->important = (*temp == '(' && *(temp + 1) == 'P' && *(temp + 2) == ')');
      if (result->important) 
      {
        *(temp - 1) = '\0';
      }
    }

    return bytes_read;
}


static gboolean 
lw_edictionary_compare (LwDictionary *dictionary, LwQuery *query, LwResult *result, const LwRelevance RELEVANCE)
{
    //Sanity checks
    g_return_val_if_fail (dictionary != NULL, FALSE);
    g_return_val_if_fail (query != NULL, FALSE);
    g_return_val_if_fail (result != NULL, FALSE);

    //Declarations
    gint j;
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
      if (regex == NULL || result->kanji_start == NULL)  return FALSE;

      checked = TRUE;
      found = g_regex_match (regex, result->kanji_start, 0, NULL);
      if (found == FALSE) return found;

      link = link->next;
    }

    //Compare furigana atoms
    {
      gchar *text;
      if (result->furigana_start != NULL) text = result->furigana_start;
      else text = result->kanji_start;
      link = lw_query_regexgroup_get (query, LW_QUERY_TYPE_FURIGANA, RELEVANCE);

      while (link != NULL && text != NULL)
      {
        regex = link->data;
        if (regex == NULL) return FALSE;

        checked = TRUE;
        found = g_regex_match (regex, text, 0, NULL);
        if (found == FALSE) return found;

        link = link->next;
      }
    }

    //Compare romaji atoms
    link = lw_query_regexgroup_get (query, LW_QUERY_TYPE_ROMAJI, RELEVANCE);
    while (link != NULL)
    {
      regex = link->data;
      if (regex == NULL) return FALSE;

      for (j = 0; result->def_start[j] != NULL; j++)
      {
        checked = TRUE;
        found = g_regex_match (regex, result->def_start[j], 0, NULL);
        if (found == TRUE) break;
      }

      if (found == FALSE) return found;
      link = link->next;
    }

    //Compare mix atoms
    link = lw_query_regexgroup_get (query, LW_QUERY_TYPE_MIX, RELEVANCE);
    while (link != NULL)
    {
      regex = link->data;
      if (regex == NULL || result->text == NULL) return FALSE;
      found = FALSE;

      if (result->kanji_start != NULL) 
      {
        checked = TRUE;
        found = g_regex_match (regex, result->kanji_start, 0, NULL);
      }
      if (result->furigana_start != NULL && !found) 
      {
        checked = TRUE;
        found = g_regex_match (regex, result->furigana_start, 0, NULL);
      }
      if (!found)
      {
        checked = TRUE;
        for (j = 0; result->def_start[j] != NULL; j++)
        {
          found = g_regex_match (regex, result->def_start[j], 0, NULL);
          if (found == TRUE) break;
        }
      }

      if (found == FALSE) return found;

      link = link->next;
    }

    return (checked && found);
}


static gboolean
lw_edictionary_installer_postprocess (LwDictionary *dictionary, 
                                      gchar **sourcelist, 
                                      gchar **targetlist, 
                                      LwIoProgressCallback cb,
                                      gpointer data,
                                      GCancellable *cancellable,
                                      GError **error)
{
    //Sanity checks
    g_return_val_if_fail (dictionary != NULL, FALSE);
    g_return_val_if_fail (sourcelist != NULL, FALSE);
    g_return_val_if_fail (targetlist != NULL, FALSE);
    if (*error != NULL) return FALSE;

    //Declarations
    LwDictionaryPrivate *priv;
    LwDictionaryInstall *install;
    gint i;

    //Initializations
    priv = dictionary->priv;
    install = priv->install;

    if (install->postprocess == FALSE)
    {
      for (i = 0; targetlist[i] != NULL && sourcelist[i] != NULL; i++)
      {
        if (g_file_test (sourcelist[i], G_FILE_TEST_IS_REGULAR) && *error == NULL)
          lw_io_copy (sourcelist[i], targetlist[i], cb, data, cancellable, error);
      }
    }
    else
    {
      g_return_val_if_fail (g_strv_length (sourcelist) > 0, FALSE);
      g_return_val_if_fail (g_strv_length (targetlist) > 1, FALSE);
      return lw_io_split_places_from_names_dictionary (targetlist[0], targetlist[1], sourcelist[0], cb, data, cancellable, error);
    }

    return FALSE;
}


//!
//! @brief Will change a query into a & delimited set of tokens (logical and)
//!
static void
lw_edictionary_create_primary_tokens (LwDictionary *dictionary, LwQuery *query)
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

    if (delimited != NULL) g_free (delimited); delimited = NULL;
}


