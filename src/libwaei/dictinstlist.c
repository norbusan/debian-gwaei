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
//!  @file dictinstlist.c
//!


#include <../private.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <glib.h>
#include <curl/curl.h>

#include <libwaei/libwaei.h>


//!
//! @brief Sets up the built-in installabale dictionaries
//!
LwDictInstList* 
lw_dictinstlist_new (LwPreferences *pm)
{
  curl_global_init (CURL_GLOBAL_ALL);

  LwDictInst *di;
  LwDictInstList *temp;

  temp = (LwDictInstList*) malloc(sizeof(LwDictInstList));

  if (temp != NULL)
  {
    temp->list = NULL;
    temp->cancel = FALSE;

    di = lw_dictinst_new_using_pref_uri (
      "English",
      gettext("English"),
      gettext("English Dictionary"),
      gettext("The venerable Japanese-English Dictionary developed by Jim Breen."),
      pm,
      LW_SCHEMA_DICTIONARY,
      LW_KEY_ENGLISH_SOURCE,
      LW_DICTTYPE_EDICT,
      LW_COMPRESSION_GZIP,
      LW_ENCODING_EUC_JP,
      FALSE,
      FALSE,
      TRUE 
    );
    temp->list = g_list_append (temp->list, di);

    di = lw_dictinst_new_using_pref_uri (
      "Kanji",
      gettext("Kanji"),
      gettext("Kanji Dictionary"),
      gettext("A Kanji dictionary based off of kanjidic with radical information combined."),
      pm,
      LW_SCHEMA_DICTIONARY,
      LW_KEY_KANJI_SOURCE,
      LW_DICTTYPE_KANJI,
      LW_COMPRESSION_GZIP,
      LW_ENCODING_EUC_JP,
      FALSE,
      TRUE,
      TRUE 
    );
    temp->list = g_list_append (temp->list, di);

    di = lw_dictinst_new_using_pref_uri (
      "Names and Places",
      gettext("Names and Places"),
      gettext("Names and Places Dictionary"),
      gettext("Based off of Enamdic, but with the names split from the places for 2 separate dictionaries."),
      pm,
      LW_SCHEMA_DICTIONARY,
      LW_KEY_NAMES_PLACES_SOURCE,
      LW_DICTTYPE_EDICT,
      LW_COMPRESSION_GZIP,
      LW_ENCODING_EUC_JP,
      TRUE,
      FALSE,
      TRUE 
    );
    temp->list = g_list_append (temp->list, di);

    di = lw_dictinst_new_using_pref_uri (
      "Examples",
      gettext("Examples"),
      gettext("Examples Dictionary"),
      gettext("A collection of Japanese/English sentences initially compiled "
              "by Professor Yasuhito Tanaka at Hyogo University and his students."),
      pm,
      LW_SCHEMA_DICTIONARY,
      LW_KEY_EXAMPLES_SOURCE,
      LW_DICTTYPE_EXAMPLES,
      LW_COMPRESSION_GZIP,
      LW_ENCODING_EUC_JP,
      FALSE,
      FALSE,
      TRUE 
    );
    temp->list = g_list_append (temp->list, di);

    di = lw_dictinst_new (
      "",
      gettext("Other"),
      gettext("Other Dictionary"),
      gettext("Install a custom dictionary."),
      "",
      LW_DICTTYPE_UNKNOWN,
      LW_COMPRESSION_NONE,
      LW_ENCODING_UTF8,
      FALSE,
      FALSE,
      FALSE
    );
    temp->list = g_list_append (temp->list, di);

  }

  return temp;
}


void 
lw_dictinstlist_free (LwDictInstList* dil)
{
    GList *iter;
    LwDictInst *di;

    for (iter = dil->list; iter != NULL; iter = iter->next)
    {
      di = LW_DICTINST (iter->data);

      if (di != NULL)
      {
        lw_dictinst_free (di);
        iter->data = NULL;
      }
    }
    g_list_free (dil->list);

    free(dil);

    curl_global_cleanup ();
}


//!
//! @brief Checks to see if the current DictInstList is installation ready
//!
gboolean 
lw_dictinstlist_data_is_valid (LwDictInstList *dil)
{
    //Declarations
    GList *iter;
    LwDictInst* di;
    int number_selected;

    //Initializations
    number_selected = 0;

    for (iter = dil->list; iter != NULL; iter = iter->next)
    {
      di = (LwDictInst*) iter->data;
      if (!lw_dictinst_data_is_valid (di) && di->selected) return FALSE;
      if (di->selected) number_selected++;
    }
    return (number_selected > 0);
}


//!
//!  @brief  Gets a LwDictInst object by a fuzzy string description.
//!          It can be either of the form "parser/dictionary" or 
//!          just be the dictionary name.  Case is ignored.
//!  @param FUZZY_DESCRIPTION A fuzzy description of the wanted dictionary.
//!  @returns A matching LwDictInst object or NULL
//!
LwDictInst* 
lw_dictinstlist_get_dictinst_fuzzy (LwDictInstList *dil, const char* FUZZY_DESCRIPTION)
{
    //Declarations
    LwDictInst *di;

    //Initializations
    di = NULL;

    //Try getting the first dictionary if none is specified
    if (FUZZY_DESCRIPTION == NULL )
    {
      if (g_list_length (dil->list))
        di = dil->list->data;
      else
        di = NULL;
    }

    //Otherwise try getting a dictionary using a few different string parsers
    else
    {
      if (di == NULL)
        di = lw_dictinstlist_get_dictinst_by_idstring (dil, FUZZY_DESCRIPTION);
      if (di == NULL)
        di = lw_dictinstlist_get_dictinst_by_filename (dil, FUZZY_DESCRIPTION);
    }

    return di;
}

//!
//! @brief Grabs the first dictionary with a matching dictionary 
//!        filename.  If you have dictionaries with different 
//!        parsers but the same name, the others will not 
//!        be accessible with this function.
//! @param NAME A constant string to search for in the dictionary names.  
//!             This is a fuzzy search, ignoring DICTTYPE and case
//! @returns The requested LwDictInst object if found or null.
//!
LwDictInst* 
lw_dictinstlist_get_dictinst_by_filename (LwDictInstList *dil, const char* FILENAME)
{
    //Declarations
    GList *iter;
    LwDictInst *di;

    for (iter = dil->list; iter != NULL; iter = iter->next)
    {
      di = (LwDictInst*) iter->data;
      if (g_ascii_strcasecmp (di->filename, FILENAME) == 0)
        break;
      di = NULL;
    }

    return di;
}


//!
//! @brief Finds a dictionary by using an id string of the form of 
//!        "engine/dictionary". Case is ignored.
//! @param ENGINE_AND_FILENAME A string in the form "engine/dictionary"
//!                            used to search for a dictionary
//! @returns The requested LwDictInst object if found or NULL.
//!
LwDictInst* 
lw_dictinstlist_get_dictinst_by_idstring (LwDictInstList *dil, const char* ENGINE_AND_FILENAME)
{
    //Declarations
    GList *iter;
    LwDictInst *di;
    char **tokens;
    char *filename;
    LwDictType engine;

    //Initializations
    iter = NULL;
    di = NULL;
    tokens = g_strsplit (ENGINE_AND_FILENAME, "/", 2);

    if (g_strv_length (tokens) == 2)
    {
      engine = lw_util_get_dicttype_from_string (tokens[0]);
      filename = tokens[1];

      for (iter = dil->list; iter != NULL; iter = iter->next)
      {
        di = (LwDictInst*) iter->data;
        if (di->type == engine && g_ascii_strcasecmp (di->filename, filename) == 0)
          break;
        di = NULL;
      }
    }

    g_strfreev (tokens);

    return di;
}


void 
lw_dictinstlist_set_cancel_operations (LwDictInstList *dil, gboolean state)
{
    LwDictInst *di;
    GList *iter;

    for (iter = dil->list; iter != NULL; iter = iter->next)
    {
      di = (LwDictInst*) iter->data;
      if (di != NULL)
        lw_dictinst_set_cancel_operations (di, state);
    }

    dil->cancel = state;
}


