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
//! @file query.c
//!

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <glib.h>

#include <libwaei/libwaei.h>
#include <libwaei/morphology.h>


LwQuery* 
lw_query_new ()
{
    LwQuery *temp;

    temp = g_new0 (LwQuery, 1);

    return temp;
}


void 
lw_query_free (LwQuery* query)
{
    if (query == NULL) return;

    lw_query_clear (query);
    if (query->text != NULL) g_free (query->text); query->text = NULL;

    g_free (query);
}


static void 
lw_query_clear_tokens (LwQuery *query)
{
    //Sanity check
    g_return_if_fail (query != NULL);

    //Declarations
    gint i;
    gint j;

    if (query->tokenlist != NULL)
    {
      for (i = 0; i < TOTAL_LW_QUERY_TYPES; i++)
      {
        if (query->tokenlist[i] != NULL) 
        {
          for (j = 0; query->tokenlist[i][j] != NULL; j++)
          {
            g_free (query->tokenlist[i][j]);
            query->tokenlist[i][j] = NULL;
          }
          g_free (query->tokenlist[i]); 
          query->tokenlist[i] = NULL;
        }
      }
      g_free (query->tokenlist); query->tokenlist = NULL;
    }
}


void
lw_query_init_tokens (LwQuery *query)
{
    //Sanity check
    g_return_if_fail (query != NULL);

    lw_query_clear_tokens (query);
    query->tokenlist = g_new0 (gchar**, TOTAL_LW_QUERY_TYPES);
}


static void 
lw_query_clear_regexgroup (LwQuery *query)
{
    //Sanity check
    g_return_if_fail (query != NULL);

    //Declarations
    gint i;
    gint j;

    if (query->regexgroup != NULL)
    {
      for (i = 0; i < TOTAL_LW_QUERY_TYPES; i++)
      {
        if (query->regexgroup[i] == NULL) continue;
        for (j = 0; j < TOTAL_LW_RELEVANCE; j++)
        {
          if (query->regexgroup[i][j] == NULL) continue;
          g_list_foreach (query->regexgroup[i][j], (GFunc) g_regex_unref, NULL);
          query->regexgroup[i][j] = NULL;
        }
        g_free (query->regexgroup[i]); query->regexgroup[i] = NULL;
      }
      g_free (query->regexgroup); query->regexgroup = NULL;
    }
}


void
lw_query_clear_rangelist (LwQuery *query)
{
    gint i;

    if (query->rangelist != NULL) 
    {
      for (i = 0; i < TOTAL_LW_QUERY_RANGE_TYPES; i++)
      {
        lw_range_free (query->rangelist[i]); query->rangelist[i] = NULL;
      }
      g_free (query->rangelist); query->rangelist = NULL;
    }
}


void
lw_query_init_rangelist (LwQuery *query)
{
    //Sanity check
    g_return_if_fail (query != NULL);

    lw_query_clear_rangelist (query);
    query->rangelist = g_new0 (LwRange*, TOTAL_LW_QUERY_RANGE_TYPES);
}


void 
lw_query_clear (LwQuery* query)
{
    //Sanity check
    g_return_if_fail (query != NULL);

    lw_query_clear_tokens (query);
    lw_query_clear_regexgroup (query);
    lw_query_clear_rangelist (query);

    query->parsed = FALSE;
}


const gchar* 
lw_query_get_text (LwQuery *query)
{
    return query->text;
}


gboolean 
lw_query_is_parsed (LwQuery *query)
{
    return (query->parsed);
}


/* TODO
gboolean 
lw_query_is_sane (const char* query)
{
    //Declarations
    char *q;
    gboolean is_sane;

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
*/


void
lw_query_tokenlist_append_primary (LwQuery     *query, 
                                   LwQueryType  type, 
                                   const gchar *TOKEN)
{
    //Sanity checks
    g_return_if_fail (query != NULL);
    g_return_if_fail (TOKEN != NULL);

    //Declarations
    gint length;
    gint index;

    //Initializations
    if (query->tokenlist[type] == NULL) 
    {
      length = 1;
      index = 0;
      query->tokenlist[type] = (gchar**) g_malloc0 (sizeof(gchar*) * length);
    }
    else
    {
      length = g_strv_length (query->tokenlist[type]) + 1;
      index = length - 1;
    }

    query->tokenlist[type][index] = g_strdup (TOKEN);

    length++;
    index++;
    query->tokenlist[type] = (gchar**) g_realloc (query->tokenlist[type], sizeof(gchar*) * length);
    query->tokenlist[type][index] = NULL;
}


static gchar* 
lw_query_tokenlist_build_kanji_supplimentary (LwQuery      *query,
                                              const gchar  *TOKEN,
                                              LwQueryType  *new_type)
{
    //Sanity checks
    g_return_val_if_fail (query != NULL, NULL);
    g_return_val_if_fail (TOKEN != NULL, NULL);
    g_return_val_if_fail (new_type != NULL, NULL);

    gchar *supplimentary;
    gchar *temp;

    *new_type = LW_QUERY_TYPE_KANJI;
    supplimentary = g_strdup (TOKEN);

#ifdef WITH_MECAB
    {
      LwMorphologyEngine *engine;
      LwMorphology *morphology;
      GList *resultlist;
      GList *resultlink;

      engine = lw_morphologyengine_get_default ();
      resultlink = resultlist = lw_morphologyengine_analyze (engine, TOKEN);

      while (resultlink != NULL)
      {
        morphology = LW_MORPHOLOGY (resultlink->data);
        resultlink = resultlink->next;

        if (morphology->base_form != NULL && g_utf8_strlen (morphology->base_form, -1) > 1 && strcmp(TOKEN, morphology->base_form) != 0)
        {
          temp = g_strjoin (LW_QUERY_DELIMITOR_SUPPLIMENTARY_STRING, supplimentary, morphology->base_form, NULL);
          g_free (supplimentary); supplimentary = temp; temp = NULL;
        }
      }

      if (resultlist != NULL) lw_morphologylist_free (resultlist); resultlist = NULL;
    }
#endif

    return supplimentary;
}


static gchar*
lw_query_tokenlist_build_furigana_supplimentary (LwQuery      *query,
                                                 const gchar  *TOKEN,
                                                 LwQueryType  *new_type)
{
    //Sanity checks
    g_return_val_if_fail (query != NULL, NULL);
    g_return_val_if_fail (TOKEN != NULL, NULL);
    g_return_val_if_fail (new_type != NULL, NULL);

    //Declarations
    gboolean hiragana_to_katakana;
    gboolean katakana_to_hiragana;
    gboolean is_hiragana;
    gboolean is_katakana;
    gchar *supplimentary;
    gchar buffer[100];
    gchar *temp;

    //Initializations
    hiragana_to_katakana = query->flags & LW_QUERY_FLAG_HIRAGANA_TO_KATAKANA;
    katakana_to_hiragana = query->flags & LW_QUERY_FLAG_KATAKANA_TO_HIRAGANA;
    is_hiragana = lw_util_is_hiragana_str (TOKEN);
    is_katakana = lw_util_is_katakana_str (TOKEN);
    supplimentary = g_strdup (TOKEN);
    *new_type = LW_QUERY_TYPE_FURIGANA;

#ifdef WITH_MECAB
    {
      LwMorphologyEngine *engine;
      LwMorphology *morphology;
      GList *resultlist;
      GList *resultlink;

      engine = lw_morphologyengine_get_default ();
      resultlink = resultlist = lw_morphologyengine_analyze (engine, TOKEN);

      while (resultlink != NULL)
      {
        morphology = LW_MORPHOLOGY (resultlink->data);
        resultlink = resultlink->next;

        if (morphology->base_form != NULL && g_utf8_strlen (morphology->base_form, -1) > 1 && strcmp(TOKEN, morphology->base_form) != 0)
        {
          temp = g_strjoin (LW_QUERY_DELIMITOR_SUPPLIMENTARY_STRING, supplimentary, morphology->base_form, NULL);
          g_free (supplimentary); supplimentary = temp; temp = NULL;
        }
      }

      if (resultlist != NULL) lw_morphologylist_free (resultlist); resultlist = NULL;
    }
#endif

    if (hiragana_to_katakana && is_hiragana)
    {
      strcpy (buffer, TOKEN);
      lw_util_str_shift_hira_to_kata (buffer);
      temp = g_strjoin (LW_QUERY_DELIMITOR_SUPPLIMENTARY_STRING, supplimentary, buffer, NULL);
      g_free (supplimentary); supplimentary = temp; temp = NULL;
    }
    else if (katakana_to_hiragana && is_katakana)
    {
      strcpy (buffer, TOKEN);
      lw_util_str_shift_kata_to_hira (buffer);
      temp = g_strjoin (LW_QUERY_DELIMITOR_SUPPLIMENTARY_STRING, supplimentary, buffer, NULL);
      g_free (supplimentary); supplimentary = temp; temp = NULL;
    }

    return supplimentary;
}


static gchar* 
lw_query_tokenlist_build_romaji_supplimentary (LwQuery      *query,
                                               const gchar  *TOKEN,
                                               LwQueryType  *new_type)
{
    //Sanity checks
    g_return_val_if_fail (query != NULL, NULL);
    g_return_val_if_fail (TOKEN != NULL, NULL);
    g_return_val_if_fail (new_type != NULL, NULL);

    //Declarations
    gboolean romaji_to_furigana;
    gboolean hiragana_to_katakana;
    gboolean is_romaji;
    gchar *supplimentary;
    const gint LENGTH = 100;
    gchar buffer[LENGTH];
    gchar *temp;
    gboolean convertable;

    //Initializations
    romaji_to_furigana = query->flags & LW_QUERY_FLAG_ROMAJI_TO_FURIGANA;
    hiragana_to_katakana = query->flags & LW_QUERY_FLAG_HIRAGANA_TO_KATAKANA;
    is_romaji = lw_util_is_romaji_str (TOKEN);
    supplimentary = g_strdup (TOKEN);
    buffer[0] = '\0';
    *new_type = LW_QUERY_TYPE_ROMAJI;
    convertable = lw_util_str_roma_to_hira (TOKEN, buffer, LENGTH);
    
    if (romaji_to_furigana && is_romaji && convertable)
    {
      *new_type = LW_QUERY_TYPE_MIX;
      temp = g_strjoin (LW_QUERY_DELIMITOR_SUPPLIMENTARY_STRING, supplimentary, buffer, NULL);
      g_free (supplimentary); supplimentary = temp; temp = NULL;

#ifdef WITH_MECAB
      {
        LwMorphologyEngine *engine;
        LwMorphology *morphology;
        GList *resultlist;
        GList *resultlink;

        engine = lw_morphologyengine_get_default ();
        resultlink = resultlist = lw_morphologyengine_analyze (engine, TOKEN);

        while (resultlink != NULL)
        {
          morphology = LW_MORPHOLOGY (resultlink->data);
          resultlink = resultlink->next;

          if (morphology->base_form != NULL && g_utf8_strlen (morphology->base_form, -1) > 1 && strcmp(TOKEN, morphology->base_form) != 0)
          {
            temp = g_strjoin (LW_QUERY_DELIMITOR_SUPPLIMENTARY_STRING, supplimentary, morphology->base_form, NULL);
            g_free (supplimentary); supplimentary = temp; temp = NULL;
          }
        }

        if (resultlist != NULL) lw_morphologylist_free (resultlist); resultlist = NULL;
      }
#endif
    }
    else
    {
      buffer[0] = '\0';
    }
    if (hiragana_to_katakana && buffer[0] != '\0' && lw_util_is_hiragana_str (buffer))
    {
      lw_util_str_shift_hira_to_kata (buffer);
      temp = g_strjoin (LW_QUERY_DELIMITOR_SUPPLIMENTARY_STRING, supplimentary, buffer, NULL);
      g_free (supplimentary); supplimentary = temp; temp = NULL;
    }

    return supplimentary;
}

//!
//! @brief Returns the delimited supplimentary tokens and changes the supplimentary type if necessary
//! @param type primary token type to lookup
//! @param index primary token index to lookup
//! @param supplimentary_tokens Where to store the supplimentary tokens
//! @param supplimentary_type Where to store the supplimentary type
//!
gchar*
lw_query_get_supplimentary (LwQuery      *query, 
                            LwQueryType   type, 
                            const gchar  *token,
                            LwQueryType  *new_type)
{
    //Sanity checks
    g_return_val_if_fail (query != NULL, NULL);
    g_return_val_if_fail (new_type != NULL, NULL);

    //Declarations
    gchar *supplimentary;

    //Initializations
    *new_type = type;

    switch (type)
    {
      case LW_QUERY_TYPE_KANJI:
        supplimentary = lw_query_tokenlist_build_kanji_supplimentary (query, token, new_type);
      break;
    case LW_QUERY_TYPE_FURIGANA:
      supplimentary = lw_query_tokenlist_build_furigana_supplimentary (query, token, new_type);
      break;
    case LW_QUERY_TYPE_ROMAJI:
      supplimentary = lw_query_tokenlist_build_romaji_supplimentary (query, token, new_type);
      break;
    default:
      supplimentary = g_strdup (token);
      break;
  }

  return supplimentary;
}


gchar**
lw_query_tokenlist_get (LwQuery *query, LwQueryType type)
{
    //Sanity checks
    g_return_val_if_fail (query != NULL, NULL);

    return (query->tokenlist[type]);
}


void
lw_query_rangelist_set (LwQuery          *query, 
                        LwQueryRangeType  type, 
                        LwRange          *range)
{
    //Sanity checks
    g_return_if_fail (query != NULL);
    g_return_if_fail (range != NULL);
    g_return_if_fail (query->rangelist != NULL);

    if (query->rangelist[type] != NULL) lw_range_free (query->rangelist[type]);
    query->rangelist[type] = range;
}


LwRange*
lw_query_rangelist_get (LwQuery *query, LwQueryRangeType type)
{
    //Sanity checks
    g_return_val_if_fail (query != NULL, NULL);
    g_return_val_if_fail (query->rangelist != NULL, NULL);

    return query->rangelist[type];
}


GList*
lw_query_regexgroup_get (LwQuery     *query, 
                         LwQueryType  type, 
                         LwRelevance  relevance)
{
    g_return_val_if_fail (query != NULL, NULL);

    if (query->regexgroup == NULL) return NULL;
    if (query->regexgroup[type] == NULL) return NULL;

    return query->regexgroup[type][relevance];
}


void
lw_query_regexgroup_append (LwQuery     *query, 
                            LwQueryType  type,
                            LwRelevance  relevance, 
                            GRegex      *regex)
{
    //Sanity checks
    g_return_if_fail (query != NULL);
    g_return_if_fail (regex != NULL);

    if (query->regexgroup == NULL) query->regexgroup = g_new0 (GList**, TOTAL_LW_QUERY_TYPES);
    if (query->regexgroup[type] == NULL) query->regexgroup[type] = g_new0 (GList*, TOTAL_LW_RELEVANCE);

    query->regexgroup[type][relevance] = g_list_append (query->regexgroup[type][relevance], regex);
}


