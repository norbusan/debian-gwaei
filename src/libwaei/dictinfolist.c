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
//! @file dictinfolist.c
//!


#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <glib.h>
#include <glib/gstdio.h>

#include <libwaei/libwaei.h>


//Private methods
static gint     _dictinfolist_load_order_compare_function (gconstpointer, gconstpointer);
static void     _dictinfolist_sort_and_normalize_order (LwDictInfoList*);


//!
//! @brief Constructor for a dictionary list object.
//! @return An allocated LwDictInfoList that will be needed to be freed by lw_dictinfolist_free ()
//!
LwDictInfoList* 
lw_dictinfolist_new (const int MAX, LwPreferences *pm)
{
    LwDictInfoList *temp;
    temp = (LwDictInfoList*) malloc(sizeof(LwDictInfoList));

    if (temp != NULL)
    {
      lw_dictinfolist_init (temp, MAX, pm);
    }

    return temp;
}


//!
//! @brief Frees up the LwDictInfoList dictionary list
//! The work of freeing each individual dictionary is automatically handled,
//! removing the chance for mistakes.
//!
void 
lw_dictinfolist_free (LwDictInfoList *dil)
{
    lw_dictinfolist_deinit (dil);
    free(dil);
}



void 
lw_dictinfolist_init (LwDictInfoList *dil, const int MAX, LwPreferences* pm)
{
    dil->list = NULL;
    dil->mutex = g_mutex_new();
    dil->max = MAX;

    lw_dictinfolist_reload (dil);
    lw_dictinfolist_load_dictionary_order_from_pref (dil, pm);
}


void 
lw_dictinfolist_deinit (LwDictInfoList *dil)
{
    lw_dictinfolist_clear (dil);
    g_mutex_free (dil->mutex);
}


void 
lw_dictinfolist_clear (LwDictInfoList *dil)
{
    LwDictInfo *di;
    GList *iter;

    if (dil->list != NULL)
    {
      for (iter = dil->list; iter != NULL; iter = iter->next)
      {
        di = LW_DICTINFO (iter->data);
        if (di != NULL)
        {
          lw_dictinfo_free (di);
        }
      }
      g_list_free (dil->list);
      dil->list = NULL;
    }
}

void 
lw_dictinfolist_reload (LwDictInfoList *dil)
{
    //Declarations
    gchar** dictionarylist;
    gchar** pair;
    LwDictType engine;
    char *dictionary;
    int i;

    lw_dictinfolist_clear (dil);

    //Create a new list
    dictionarylist = lw_io_get_dictionary_file_list (dil->max);
    for (i = 0; dictionarylist != NULL && dictionarylist[i] != NULL; i++)
    {
      pair = g_strsplit_set (dictionarylist[i], "/", 2);
      if (pair != NULL && pair[0] != NULL && pair[1] != NULL) 
      {
        engine = lw_util_get_dicttype_from_string (pair[0]);
        dictionary = pair[1];
        lw_dictinfolist_add_dictionary (dil, engine, dictionary);
      }
      g_strfreev (pair);
    }
    g_strfreev(dictionarylist);
}



//!
//! @brief Gets the dictionary by load position in the GUI
//! @param request The GUI load position of the desired dictionary
//! @return The position in the LwDictInfoList of the LwDictInfo
//!
LwDictInfo* 
lw_dictinfolist_get_dictinfo_by_load_position (LwDictInfoList* dil, int request)
{
    GList *iter;
    LwDictInfo *di;

    di = NULL;

    for (iter = dil->list; iter != NULL; iter = iter->next)
    {
       di = (LwDictInfo*) iter->data;
       if (di != NULL && di->load_position == request)
       {
         break;
       }
       di = NULL;
    }

    return di;
}


//!
//! @brief Adds a dictionary to the LwDictInfoList with sanity checks
//! @param DICTTYPE Engine of the dictionary to add
//! @param FILENAME Name of the dictionary to add
//!
void 
lw_dictinfolist_add_dictionary (LwDictInfoList *dil, const LwDictType DICTTYPE, const char *FILENAME)
{
    //Sanity check
    if (lw_dictinfolist_check_if_loaded (dil, DICTTYPE, FILENAME) == TRUE) return;

    //Declarations
    LwDictInfo *di;

    //Initializations
    di = lw_dictinfo_new (DICTTYPE, FILENAME);

    //Append to the dictionary list if was loadable
    if (di != NULL) dil->list = g_list_append (dil->list, di);
}


//!
//! @brief Searchs for a specific dictionary by name
//! The function will go through each dictionary until it matches the requested
//! name.
//!
//! @param DICTTYPE The parsing engine of the dictionary wanted.  There can be
//!               dictionaries with the same name, but different engines.
//! @param NAME A constant string to search for in the dictionary names.
//! @returns The requested LwDictInfo object if found or null.
//!
LwDictInfo* 
lw_dictinfolist_get_dictinfo (LwDictInfoList *dil, const LwDictType DICTTYPE, const char* FILENAME)
{
    //Sanity checks
    g_assert (DICTTYPE >= 0 && FILENAME != NULL);

    //Declarations
    GList *iter;
    LwDictInfo *di;

    //Initializations
    di = NULL;

    for (iter = dil->list; iter != NULL; iter = iter->next)
    {
      di = LW_DICTINFO (iter->data);
      if (di != NULL && di->type == DICTTYPE && strcmp (di->filename, FILENAME) == 0)
      {
        break;
      }
      di = NULL;
    }

    return di;
}

//!
//!  @brief  Gets a LwDictInfo object by a fuzzy string description.
//!          It can be either of the form "parser/dictionary" or 
//!          just be the dictionary name.  Case is ignored.
//!  @param FUZZY_DESCRIPTION A fuzzy description of the wanted dictionary.
//!  @returns A matching LwDictInfo object or NULL
//!
LwDictInfo* 
lw_dictinfolist_get_dictinfo_fuzzy (LwDictInfoList *dil, const char* FUZZY_DESCRIPTION)
{
    //Declarations
    LwDictInfo *di;

    //Initializations
    di = NULL;

    //Try getting the first dictionary if none is specified
    if (FUZZY_DESCRIPTION == NULL )
    {
      if (dil->list != NULL)
        di = (LwDictInfo*) dil->list->data;
      else
        di = NULL;
    }

    //Otherwise try getting a dictionary using a few different string parsers
    else
    {
      if (di == NULL)
        di = lw_dictinfolist_get_dictinfo_by_idstring (dil, FUZZY_DESCRIPTION);
      if (di == NULL)
        di = lw_dictinfolist_get_dictinfo_by_filename (dil, FUZZY_DESCRIPTION);
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
//! @returns The requested LwDictInfo object if found or null.
//!
LwDictInfo* 
lw_dictinfolist_get_dictinfo_by_filename (LwDictInfoList *dil, const char* FILENAME)
{
    //Sanity checks
    g_assert (FILENAME != NULL);

    //Declarations
    GList *iter;
    LwDictInfo *di;

    //Initializations
    di = NULL;

    for (iter = dil->list; iter != NULL; iter = iter->next)
    {
      di = (LwDictInfo*) iter->data;
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
//! @returns The requested LwDictInfo object if found or NULL.
//!
LwDictInfo* 
lw_dictinfolist_get_dictinfo_by_idstring (LwDictInfoList *dil, const char* ENGINE_AND_FILENAME)
{
    //Sanity checks
    g_assert (ENGINE_AND_FILENAME != NULL);

    //Declarations
    GList *iter;
    LwDictInfo *di;
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
        di = (LwDictInfo*) iter->data;
        if (di->type == engine && g_ascii_strcasecmp (di->filename, filename) == 0)
          break;
        di = NULL;
      }
    }

    g_strfreev (tokens);

    return di;
}



//!
//! @brief Searchs for a specific dictionary by name to see if it is installed
//! @param NAME request a const string to search for in the dictionary names
//! @return returns true if the dictionary is installed
//!
gboolean 
lw_dictinfolist_check_if_loaded (LwDictInfoList *dil, const LwDictType DICTTYPE, const char* FILENAME)
{
    //Sanity checks
    g_assert (DICTTYPE >= 0 && FILENAME != NULL);

    //Declarations
    GList *iter;
    LwDictInfo *di;

    //Return true if the dictionary exists
    for (iter = dil->list; iter != NULL; iter = iter->next)
    {
      di = (LwDictInfo*) iter->data;
      if (di->type == DICTTYPE && strcmp (di->filename, FILENAME) == 0) 
        return TRUE;
    }

    //Otherwise it doesn't
    return FALSE;
}


//!
//! @brief Returns the number of dictionaries in the dictionary list
//!
//! This is not the number of dictionaries that are active.  It shows
//! how many dictionary names are recorded in the dictionary list.
//! By default, the default dictionaries appended to the list with
//! an UNLW_DICT_STATUS_INSTALLED status if they are unavailable. If the LW_DICT_MIX dictionary
//! is installed, Kanji and Radicals disappear from the GUI, but are still
//! in this list.
//!
//! @return Integer representing the number of installed dictionaries
//!
int 
lw_dictinfolist_get_total (LwDictInfoList *dil)
{
    if (dil == NULL) return 0;

    return g_list_length (dil->list);
}


//
//! @brief Saves the current load order to the preferences
//
void 
lw_dictinfolist_save_dictionary_order_pref (LwDictInfoList *dil, LwPreferences *pm)
{
    //Make sure things are sorted and normal
    _dictinfolist_sort_and_normalize_order (dil);

    //Declarations
    char *load_order;
    LwDictInfo *di;
    GList *iter;
    char **atom;
    int i;

    //Initializations;
    atom = (char**) malloc((dil->max + 1) * sizeof(char*));
    i = 0;

    //Create the string to write to the prefs with the last one NULL terminated
    for (iter = dil->list; iter != NULL && i < dil->max; iter = iter->next)
    {
      di = (LwDictInfo*) iter->data;
      atom[i] = g_strdup_printf ("%s/%s", lw_util_dicttype_to_string (di->type), di->filename);
      if (atom == NULL) { fprintf(stderr, "Out of memory\n"); exit(1); }
      i++;
    }
    atom[i] = NULL;

    load_order = g_strjoinv (";", atom);
    lw_preferences_set_string_by_schema (pm, LW_SCHEMA_DICTIONARY, LW_KEY_LOAD_ORDER, load_order);

    //Free the used memory
    g_strfreev (atom);
    atom = NULL;
    g_free (load_order);
    load_order = NULL;
}


//
//! @brief Loads the load order from the preferences
//
void 
lw_dictinfolist_load_dictionary_order_from_pref (LwDictInfoList *dil, LwPreferences *pm)
{
    if (pm == NULL) return; 

    char load_order[1000];
    char **load_order_array;
    char **engine_name_array;
    char **iter = NULL;
    LwDictType engine;
    char *name;
    LwDictInfo *di = NULL;
    int load_position = 0;
    
    lw_preferences_get_string_by_schema (pm, load_order, LW_SCHEMA_DICTIONARY, LW_KEY_LOAD_ORDER, 1000);
    load_order_array = g_strsplit_set (load_order, ";", dil->max);
    
    for (iter = load_order_array; *iter != NULL; iter++)
    {
      //Sanity checking
      if (*iter == NULL || **iter == '\0') { 
        fprintf(stderr, "WARNING: failed sanity check 1. Resetting dictionary order prefs.\n");
        lw_preferences_reset_value_by_schema (pm, LW_SCHEMA_DICTIONARY, LW_KEY_LOAD_ORDER);
        lw_dictinfolist_load_dictionary_order_from_pref (dil, pm);
        return;
      }

      engine_name_array = g_strsplit_set (*iter, "/", -1); 

      //Sanity checking
      if (engine_name_array[0] == NULL || engine_name_array[1] == NULL)
      {
        fprintf(stderr, "WARNING: failed sanity check 2. Resetting dictionary order prefs.\n");
        lw_preferences_reset_value_by_schema (pm, LW_SCHEMA_DICTIONARY, LW_KEY_LOAD_ORDER);
        lw_dictinfolist_load_dictionary_order_from_pref (dil, pm);
        return;
      }

      engine = lw_util_get_dicttype_from_string (engine_name_array[0]);
      name = engine_name_array[1];

      //Sanity Checking
      if ((di = lw_dictinfolist_get_dictinfo (dil, engine, name)) != NULL)
      {
        di->load_position = load_position;
        load_position++;
      }

      g_strfreev (engine_name_array);
      engine_name_array = NULL;
    }

    g_strfreev (load_order_array);
    load_order_array = NULL;

    _dictinfolist_sort_and_normalize_order (dil);
}


//!
//! @brief All dictionaries with a specific status get switched to the requested one
//! @param a Pointer to LwDictInfo object a
//! @param b Pointer to LwDictInfo object b
//! @returns Whether the position of a is less than (-1), equal (0) or greater than b (1)
//!
static gint _dictinfolist_load_order_compare_function (gconstpointer a, gconstpointer b)
{
    //Declarations and initializations
    LwDictInfo *di_a = (LwDictInfo*) a;
    LwDictInfo *di_b = (LwDictInfo*) b;
    int lpa = di_a->load_position;
    int lpb = di_b->load_position;

    //Exception cases for positions less than 0.
    //We want negative numbers after everything else
    if (lpa < 0 && lpb >= 0)  return 1;
    if (lpa >= 0 && lpb < 0)  return -1;

    //Normal ordering
    if (lpa < lpb) return -1;
    else if (lpa == lpb) return 0;
    else return 1;
}


//
//! @brief Sorts the dictionaries by their load order and makes the numbers clean with no holes
//
static void _dictinfolist_sort_and_normalize_order (LwDictInfoList *dil)
{
    //Declarations
    LwDictInfo *di;
    int load_position;
    GList *iter;

    //Initializations
    load_position = 0;

    //Sort the list
    dil->list = g_list_sort (dil->list, _dictinfolist_load_order_compare_function);

    //Make sure there is no number skipping
    for (iter = dil->list; iter != NULL; iter = iter->next)
    {
      di = (LwDictInfo*) iter->data;
      di->load_position = load_position;
      load_position++;
    }
}
