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
//! @file src/dictlist-object.c
//!
//! @brief Management of dictionary objects
//!
//! The functions her generally manage the creation, destruction, and searching
//! of dictionaries.  The GwDictInfo objects also are used as a convenient
//! container for variables pointing towards download locations, install locations
//! etc.
//!


#include <string.h>
#include <regex.h>
#include <stdlib.h>
#include <stdio.h>
#include <libintl.h>

#include <glib.h>
#include <glib/gstdio.h>

#include <gwaei/definitions.h>
#include <gwaei/regex.h>
#include <gwaei/utilities.h>
#include <gwaei/io.h>
#include <gwaei/dictlist-object.h>
#include <gwaei/preferences.h>


static GwDictList *dictionaries;

//!
//! @brief Constructor for a dictionary list object.
//!
//! This object is used to help manage groups of dictionaries in a sane way.
//!
//! @return An allocated GwDictList that will be needed to be freed by gw_dictlist_free ()
//!
GwDictList* gw_dictlist_new ()
{
    GwDictList *temp;

    //Allocate some memory
    if ((temp = malloc(sizeof(GwDictList))) == NULL) return NULL;

    temp->list = NULL;
    temp->selected = NULL;
    return temp;
}


//!
//! @brief Get the currently selected GwDictInfo object
//!
//! A function used for abstracting what is the current dictionary for the GUI
//! instead of relying on the status of a particular widget.
//!
//! @return The position in the GwDictList of the GwDictInfo
//!
GList* gw_dictlist_get_selected()
{
    return dictionaries->selected;
}


//!
//! @brief Gets the dictionary by load position in the GUI
//!
//! Each dictionary has a load position recorded in the GUI when it is added to
//! it.  This function makes it easy for a GUI callback to find that exact
//! dictionary in the GwDictList.
//!
//! @param request The GUI load position of the desired dictionary
//! @return The position in the GwDictList of the GwDictInfo
//!
GList* gw_dictlist_get_dict_by_load_position (int request)
{
    GList* current = gw_dictlist_get_list();
    GwDictInfo *di = NULL;
    do
    {
       di = (GwDictInfo*) current->data;
       if (current && di && di->load_position == request)
         break;
       current = g_list_next (current);
    } while (current);
    return current;
}

//!
//! @brief Gets the dictionary by load position in the GUI
//!
//! Each dictionary has a load position recorded in the GUI when it is added to
//! it.  This function makes it easy for a GUI callback to find that exact
//! dictionary in the GwDictList.
//!
//! @param request The GUI load position of the desired dictionary
//! @return The position in the GwDictList of the GwDictInfo
//!
GList* gw_dictlist_set_selected_by_load_position(int request)
{
    GList* current_dictionary = gw_dictlist_get_list();
    do
    {
       if (((GwDictInfo*)current_dictionary->data)->load_position == request)
         break;
       current_dictionary = g_list_next (current_dictionary);
    } while (current_dictionary != NULL);

    dictionaries->selected = current_dictionary;
    return current_dictionary;
}


//!
//! @brief Adds a dictionary to the GwDictList
//!
//! This function does not install files or anything.  It just looks for the
//! dictionary by the name specified in the .waei folder and then sets up it's
//! initial state using gw_dictinfo_new().  It will also make sure the
//! dictionary is not added twice.
//!
//! @param name Name of the dictionary to add
//!
void gw_dictlist_add_dictionary(char *name)
{
    GwDictInfo *di;
    if  (gw_dictlist_check_if_loaded_by_name (name) == FALSE)
      di = gw_dictinfo_new (name);
    if (di != NULL)
      dictionaries->list = g_list_append (dictionaries->list, di);
}


//!
//! @brief Removes the first dictionary in the GwDictList
//!
//! Does the work of asking for the GwDictInfo object to free it's memory
//! and then removing the empty object from the GwDictList dictionary list.
//!
//! @param name Name of the dictionary to add
//! @return Returns the dictionary object freed or null.
//!
GList* gw_dictlist_remove_first()
{
    GList *list;
    list = dictionaries->list;

    gw_dictinfo_free(list->data);
    return g_list_delete_link(list, list);
}


//!
//! @brief Frees up the GwDictList dictionary list
//!
//! The work of freeing each individual dictionary is automatically handled,
//! removing the chance for mistakes.
//!
void gw_dictlist_free ()
{
    while (dictionaries->list != NULL)
      dictionaries->list = gw_dictlist_remove_first();

    g_free(dictionaries);
    dictionaries = NULL;
}

//!
//! @brief Searchs for a specific dictionary by name
//!
//! The function will go through each dictionary until it matches the requested
//! name.
//!
//! @param request a const string to search for in the dictionary names
//! @return returns the GwDictInfo dictionary object of the result or null.
//!
GwDictInfo* gw_dictlist_get_dictinfo_by_name (const char* request)
{
    GList *current;
    current = dictionaries->list;
    GwDictInfo *di;
    di = NULL;

    while (current != NULL)
    {
      di = (GwDictInfo*) current->data;
      if (strcmp (di->name, request) == 0)
        break;
      current = current->next;
      di = NULL;
    }

    return di;
}


//!
//! @brief Searchs for a specific dictionary by alias
//!
//! The function will go through each dictionary until it matches the requested
//! name.  If the name equates to an alias, the alias dictionary is returned
//! instead if it is installed.
//!
//! @param request a const string to search for in the dictionary names
//! @return returns the GwDictInfo dictionary object of the result or null.
//!
GwDictInfo* gw_dictlist_get_dictinfo_by_alias (const char* request)
{
    if ((strcmp (request, "Radicals") == 0 || strcmp (request, "Kanji") == 0) &&
         gw_dictlist_dictionary_get_status_by_id (GW_DICT_ID_MIX) == GW_DICT_STATUS_INSTALLED )
      return gw_dictlist_get_dictinfo_by_id (GW_DICT_ID_MIX);
    else 
      return gw_dictlist_get_dictinfo_by_name (request);
}


//!
//! @brief Searchs for a specific dictionary by name to see if it is installed
//!
//! This as a convenience function to see if a dictionary is installed,
//! negating the need to see if it was added to the dictionary list and if
//! it has the GW_DICT_STATUS_INSTALLED status set.
//!
//! @param request a const string to search for in the dictionary names
//! @return returns true if the dictionary is installed
//!
gboolean gw_dictlist_check_if_loaded_by_name (char* name)
{
    GList *current = dictionaries->list;
    GwDictInfo *di;

    while (current != NULL)
    {
      di = (GwDictInfo*) current->data;
      if (strcmp (di->name, name) == 0 && di->status == GW_DICT_STATUS_INSTALLED)
        return TRUE;
      current = current->next;
    }

    return FALSE;
}


//!
//! @brief Counts how many dictionaries in the dictionary list have a specific status
//!
//! The function will loop through each item in the dictionary list,
//! incrementing its count by 1 for each found dictionary, then returning the
//! number. The statuses include: GW_DICT_STATUS_INSTALLING, GW_DICT_STATUS_INSTALLED, GW_DICT_STATUS_NOT_INSTALLED,
//! GW_DICT_STATUS_UPDATING, GW_DICT_STATUS_UPDATED, and GW_DICT_STATUS_CANCELING. See dictionaries.h for the current list.
//!
//! @param status the integer status to check for
//! @return returns the number of dictionaries with the status
//!
int gw_dictlist_get_total_with_status (GwDictStatus status)
{
    GwDictInfo *di;
    GList *current = dictionaries->list;
    int i = 0;

    while (current != NULL)
    {
      di = (GwDictInfo*) current->data;
      if (di->status == status)
      {
        i++;
      }
      current = current->next;
    }

    return i;
}


//!
//! @brief Gets a dictionary status from the dictionary list by its id
//!
//! The function will loop the dictionary list looking for a specific id,
//! returning it's status.  Ids include GW_DICT_ENGLISH, GW_DICT_KANJI, GW_DICT_RADICALS, GW_DICT_NAMES, GW_DICT_PLACES
//! and GW_DICT_MIX. See dictionaries.h for the current list.
//!
//! @param id Id attribute of the wanted dictionary
//! @return Returns the status of the dictionary
//!
GwDictStatus gw_dictlist_dictionary_get_status_by_id (GwDictId id)
{
    GList *current = dictionaries->list;
    GwDictInfo *di = (GwDictInfo*) current->data;

    while (current != NULL && di->id != id)
    {
      di = (GwDictInfo*) current->data;
      current = current->next;
    }
    
    if (di->id == id)
      return di->status;
    else
      return GW_DICT_STATUS_NOT_INSTALLED;
}


//!
//! @brief Gets a dictionary from the dictionary list by its id
//!
//! The function will loop the dictionary list looking for a specific id,
//! returning a reference to the dictionary.
//!
//! @param id Id attribute of the wanted dictionary
//! @return Returns a pointer to the dictionary if available
//!
GwDictInfo* gw_dictlist_get_dictinfo_by_id (GwDictId id)
{
    GList *current = dictionaries->list;
    GwDictInfo *di = (GwDictInfo*) current->data;

    while (current != NULL && di->id != id)
    {
      di = (GwDictInfo*) current->data;
      current = current->next;
    }
    
    return di;
}


//!
//! @brief Returns the number of dictionaries in the dictionary list
//!
//! This is not the number of dictionaries that are active.  It shows
//! how many dictionary names are recorded in the dictionary list.
//! By default, the default dictionaries appended to the list with
//! an UNGW_DICT_STATUS_INSTALLED status if they are unavailable. If the GW_DICT_MIX dictionary
//! is installed, Kanji and Radicals disappear from the GUI, but are still
//! in this list.
//!
//! @return Integer representing the number of installed dictionaries
//!
int gw_dictlist_get_total()
{
    return g_list_length(dictionaries->list);
}


//!
//! @brief Returns a pointer to the list of dictionaries in GwDictList
//!
//! Gets the list variable inside of the GwDictList object and returns it.
//!
//! @return GList of the added dictionaries to the list.
//!
GList* gw_dictlist_get_list()
{
    return dictionaries->list;
}


//!
//! @brief Set the initial status of the dictionary list
//!
//! The built in dictionaries are set up, then any additional manually installed
//! user dictionaries are searched for and set up.
//!
void gw_dictionaries_initialize_dictionary_list ()
{
    if (dictionaries != NULL)
      gw_dictlist_free ();

    dictionaries = gw_dictlist_new ();
       
    //Dictionaries having to do with gui elements
    gw_dictlist_add_dictionary ("English");
    gw_dictlist_add_dictionary ("Mix");
    gw_dictlist_add_dictionary ("Kanji");
    gw_dictlist_add_dictionary ("Radicals");
    gw_dictlist_add_dictionary ("Names");
    gw_dictlist_add_dictionary ("Places");
    gw_dictlist_add_dictionary ("Examples");
    gw_dictlist_add_dictionary ("French");
    gw_dictlist_add_dictionary ("German");
    gw_dictlist_add_dictionary ("Spanish");

    if (gw_util_get_waei_directory () == NULL) return;

    //Path variables
    char path[FILENAME_MAX];
    strncpy(path, gw_util_get_waei_directory (), FILENAME_MAX);
    char *filename = &path[strlen (path)];

    //Directory variables
    GDir *directory = NULL;
    const gchar *input = NULL;

    //Open the ~/.waei directory
    directory = g_dir_open (path, 0, NULL);
    if (directory != NULL)
    {
      while ((input = g_dir_read_name (directory)) != NULL)
      {
        strcpy (filename, input);
        if (g_file_test(path, G_FILE_TEST_IS_REGULAR) == TRUE &&
            regexec (gw_re[GW_RE_FILENAME_GZ],  filename, 1, NULL, 0 ) != 0     &&
            strcmp  (filename, "English"           ) != 0     &&
            strcmp  (filename, "Kanji"             ) != 0     &&
            strcmp  (filename, "Names"             ) != 0     &&           
            strcmp  (filename, "Places"            ) != 0     &&        
            strcmp  (filename, "Radicals"          ) != 0     &&     
            strcmp  (filename, "Examples"          ) != 0     &&     
            strcmp  (filename, "French"            ) != 0     &&     
            strcmp  (filename, "German"            ) != 0     &&     
            strcmp  (filename, "Spanish"           ) != 0     &&     
            strcmp  (filename, "Mix"               ) != 0       )
        {
          gw_dictlist_add_dictionary (filename);
        }
      }
    }
    g_dir_close (directory);
}


//!
//! @brief Does the required post processing to create the Mix dictionary
//!
//! THIS IS A PRIVATE FUNCTION. The function removes the current Mix dictionary
//! if it is there, then attempts to create a new mix dictionary.
//!
//! @see gw_io_create_mix_dictionary ()
//! @returns Returns true on success
//!
static gboolean create_mix_dictionary()
{
    GwDictInfo* mix;
    mix = gw_dictlist_get_dictinfo_by_id(GW_DICT_ID_MIX);
    g_remove (mix->path);
    mix->status = GW_DICT_STATUS_NOT_INSTALLED;

    GwDictInfo* kanji;
    kanji = gw_dictlist_get_dictinfo_by_id(GW_DICT_ID_KANJI);
    GwDictInfo* radicals;
    radicals = gw_dictlist_get_dictinfo_by_id(GW_DICT_ID_RADICALS);

    char *mpath = mix->path;
    char *kpath = kanji->path;
    char *rpath = radicals->path;

    mix->status = GW_DICT_STATUS_INSTALLING;

    gboolean ret;
    ret = gw_io_create_mix_dictionary(mpath, kpath, rpath);
   
    if (ret)
    {
      mix->status = GW_DICT_STATUS_INSTALLED;
      mix->total_lines =  gw_io_get_total_lines_for_path (mix->path);
    }
    else
      mix->status = GW_DICT_STATUS_ERRORED;

    return ret;
}


//!
//! @brief Splits out the place names from the person names
//!
//! THIS IS A PRIVATE FUNCTION.  Function checks to see the Names dictionary
//! is installed, then tries to split out the Names and Places dictionaries.
//! If there is a failure, false is returned and the GError error gets set.
//!
//! @see gw_io_split_places_from_names_dictionary ()
//! @param error set a GError to the pointer when an error occurs
//! @returns Returns true on success
//!
static gboolean split_places_from_names_dictionary(GError **error)
{
    GwDictInfo* di_places;
    di_places = gw_dictlist_get_dictinfo_by_id (GW_DICT_ID_PLACES);
    GwDictInfo* di_names;
    di_names = gw_dictlist_get_dictinfo_by_id (GW_DICT_ID_NAMES);

    if (di_names->status == GW_DICT_STATUS_NOT_INSTALLED) return FALSE;

    char *raw = di_names->sync_path;
    char source[FILENAME_MAX];
    strncpy (source, di_names->path, FILENAME_MAX);
    strncat (source, ".new", FILENAME_MAX - strlen (di_names->path));

    char *names = di_names->path;
    char *places = di_places->path;
    
    gboolean ret = TRUE;

    if (ret)
      ret = gw_io_copy_with_encoding(raw, source, "EUC-JP","UTF-8", error);
       
    if (ret)
    {
      remove(names);
      remove(places);
      ret = gw_io_split_places_from_names_dictionary (source, names, places);
    }


    if (ret)
    {
      g_remove(source);
      di_places->status = GW_DICT_STATUS_INSTALLED;
      di_places->total_lines =  gw_io_get_total_lines_for_path (di_places->path);
      di_names->status  = GW_DICT_STATUS_INSTALLED;
      di_names->total_lines =  gw_io_get_total_lines_for_path (di_names->path);
    }
    else
    {
      g_remove(source);
      g_remove(names);
      remove(places);
      di_places->status = GW_DICT_STATUS_ERRORED;
      di_names->status  = GW_DICT_STATUS_ERRORED;
    }
    
    return ret;
}


//!
//! @brief Figures out the postprocessing required by the dictionary name
//!
//! The name is parsed then it is decided if create_mix_dictionary () or
//! split_places_from_names_dictionary () should be called. The function
//! returns a GError on failure through the error parameter.
//!
//! @see create_mix_dictionary ()
//! @see split_places_from_names_dictionary ()
//! @param name Name of the dictionary that should be postprocessesed
//! @param error set a GError to the pointer when an error occurs
//! @returns Returns true on success
//!
void gw_dictlist_preform_postprocessing_by_name (char* name, GError **error)
{
    //Sanity check
    GwDictInfo* di;
    di = gw_dictlist_get_dictinfo_by_name (name);
    if (di->status != GW_DICT_STATUS_INSTALLING &&
        di->status != GW_DICT_STATUS_UPDATING &&
        di->status != GW_DICT_STATUS_REBUILDING)
      return;
    if (di->status == GW_DICT_STATUS_CANCELING)
      return;

    //Setup some pointers
    GwDictInfo* k_di = gw_dictlist_get_dictinfo_by_id (GW_DICT_ID_KANJI);
    GwDictInfo* r_di = gw_dictlist_get_dictinfo_by_id (GW_DICT_ID_RADICALS);
    GwDictInfo* n_di = gw_dictlist_get_dictinfo_by_id (GW_DICT_ID_NAMES);

    //Rebuild the mix dictionary
    if ((di->id == GW_DICT_ID_RADICALS && k_di->status == GW_DICT_STATUS_INSTALLED) || 
        (di->id == GW_DICT_ID_KANJI    && r_di->status == GW_DICT_STATUS_INSTALLED) ||
        (di->id == GW_DICT_ID_MIX && k_di->status == GW_DICT_STATUS_INSTALLED && r_di->status == GW_DICT_STATUS_INSTALLED)
       )
    {
      di->status = GW_DICT_STATUS_REBUILDING;
      create_mix_dictionary ();
    }
    //Rebuild the names dictionary
    else if(di->id == GW_DICT_ID_NAMES)
    {
      di->status = GW_DICT_STATUS_REBUILDING;
      split_places_from_names_dictionary(error);
    }

    //Restore the previous state if the install wasn't canceled
    if (di->status != GW_DICT_STATUS_CANCELING)
    {
      di->status = GW_DICT_STATUS_INSTALLED;
    }
}


//!
//! @brief All dictionaries with a specific status get switched to the requested one.
//!
//! Each dictionary in the dictionary list is examined and if it matches a
//! a specific status, it is switched to the requested one.  This is a good way
//! to quickly cleanup after possible installation errors to reset the
//! dictionaries to an uninstalled status state. The possible statuses include:
//! GW_DICT_STATUS_INSTALLING, GW_DICT_STATUS_INSTALLED, GW_DICT_STATUS_NOT_INSTALLED, GW_DICT_STATUS_UPDATING, GW_DICT_STATUS_UPDATED, and GW_DICT_STATUS_CANCELING.
//! See dictionaries.h for the current list.
//!
//! @param OLD const int of the current install status
//! @param NEW const int of the new install status
//!
void gw_dictlist_normalize_all_status_from_to (const GwDictStatus OLD, const GwDictStatus NEW)
{
    GList *current = dictionaries->list;
    GwDictInfo *di;

    while (current != NULL)
    {
      di = (GwDictInfo*) current->data;
      if (di->status == OLD)
        di->status = NEW;
      current = current->next;
    }
}


//!
//! @brief Does an rsync operation on the requested dictionary object
//!
//! The program does a system call to rsync the dictionary file in the raw
//! path.  It then copies the file and converts it to UTF-8 from EUC-JP.
//!
//! @see gw_io_copy_with_encoding ()
//! @param di pointer to the dictionary object that needs updating
//! @param error set a GError to the pointer when an error occurs
//!
void gw_dictlist_sync_dictionary (GwDictInfo *di, GError **error)
{
    GQuark quark;
    quark = g_quark_from_string (GW_GENERIC_ERROR);

    char *path = di->path;
    char *sync_path = di->sync_path;

    if (di->status != GW_DICT_STATUS_INSTALLED || strlen(di->rsync) < 2)
      return;

    printf("*  ");
    // TRANSLATORS: The %s stands for the internal (untranslated) short name of the dictionary
    printf(gettext("Syncing %s dictionary"), di->name);
    printf("---------------------\n");

    if (system(di->rsync) != 0)
    {
      const char *message = gettext("File read failed");
      if (*error != NULL)
        *error = g_error_new_literal (quark, GW_FILE_ERROR, message);
    }
    
    if (*error == NULL)
      gw_io_copy_with_encoding(sync_path, path, "EUC-JP","UTF-8", error);

    //Special dictionary post processing
    if (*error == NULL)
      gw_dictlist_preform_postprocessing_by_name(di->name, error);

    if (*error == NULL)
      di->status = GW_DICT_STATUS_UPDATED;
    else
      di->status = GW_DICT_STATUS_ERRORED;

    if (error != NULL && *error != NULL)
    {
      g_error_free (*error);
      *error = NULL;
    }

    printf("\n");
    if (*error == NULL)
      printf("%s\n", gettext("Success"));
}


void gw_dictlist_update_dictionary_order_list ()
{
    //Get the pref string
    char order[5000];
    gw_pref_get_string (order, GCKEY_GW_LOAD_ORDER, GW_LOAD_ORDER_FALLBACK, 5000);

    //Make sure all the dictionaries are in there
    GList* list = gw_dictlist_get_list();
    GwDictInfo* di = NULL;
    while (list != NULL)
    {
      di = list->data;
      if (strstr(di->name, order) != 0)
      {
         strcat(order, ",");
         strcat(order, di->name);
      }
    }

    //Parse the string
    char *names[50];
    char *mix_name = NULL, *kanji_name = NULL, *radicals_name = NULL;
    names[0] = order;
    int i = 0;
    while ((names[i + 1] = g_utf8_strchr (names[i], -1, L',')) && i < 50)
    {
      i++;
      *names[i] = '\0';
      names[i]++;
    }
    names[i + 1] = NULL;

    //Remove not installed dictionaries from the list
    i = 0;
    while (names[i] != NULL)
    {
      di = gw_dictlist_get_dictinfo_by_name (names[i]);
      if (di == NULL || di->status != GW_DICT_STATUS_INSTALLED)
        *names[i] = '\0';
      if (di && di->id == GW_DICT_ID_MIX) mix_name = names[i];
      if (di && di->id == GW_DICT_ID_KANJI) kanji_name = names[i];
      if (di && di->id == GW_DICT_ID_RADICALS) radicals_name = names[i];
      i++;
    }

    //Remove kanji and radicals if mix exists
    if (mix_name)
    {
      if (kanji_name) *kanji_name = '\0';
      if (radicals_name) *radicals_name = '\0';
    }

    int j = 0;
    i = 0;
    char new_order[5000];
    new_order[0] = '\0';
    while (names[i] != NULL && names[j] != NULL)
    {
      if (*names[j] == '\0')
      {
        j++;
      }
      else if (*names[j] != '\0')
      {
        names[i] = names[j];
        strcat(new_order, names[j]);
        strcat(new_order, ",");
        i++;
        j++;
      }
    }
    new_order[strlen(new_order) - 1] = '\0';
    names[i] = NULL;
    gw_pref_set_string (GCKEY_GW_LOAD_ORDER, new_order);
}


