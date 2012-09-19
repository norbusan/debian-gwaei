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
//!  @file src/dictinfo-object.c
//!
//!  @brief Management of dictionary objects
//!
//!  The functions her generally manage the creation, destruction, and searching
//!  of dictionaries.  The LwDictInfo objects also are used as a convenient
//!  container for variables pointing towards download locations, install locations
//!  etc.
//!


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <libintl.h>

#include <glib.h>

#include <libwaei/libwaei.h>

//Static declarations
static gboolean _overlay_default_builtin_dictionary_settings (LwDictInfo*);


//!
//! @brief Creates a new LwDictInfo object
//!
//! Memory for a new LwDictInfo object will be allocated, and the name passed
//! to the function as a param will be searched for in the .waei folder.  If 
//! it is a known name, the long name of the object will betranslated and if
//! it is installed, the status variable set to GW_DICT_STATUS_INSTALLED.
//!
//! @param name Name of the object to create
//! @return An allocated LwDictInfo that will be needed to be freed by lw_dictinfo_free ()
//!
LwDictInfo* lw_dictinfo_new (LwEngine ENGINE, const char *FILENAME)
{
    g_assert (ENGINE >= 0 && ENGINE <= GW_ENGINE_TOTAL && FILENAME != NULL);

    LwDictInfo *temp;
    char *uri;

    //Allocate some memory
    if ((temp = malloc(sizeof(LwDictInfo))) == NULL) return NULL;

    temp->load_position = -1;

    //Initialize the members
    temp->filename = NULL;
    temp->shortname = NULL;
    temp->longname = NULL;
    temp->engine = ENGINE;
    temp->filename = g_strdup_printf ("%s", FILENAME);

    uri = lw_dictinfo_get_uri (temp);
    temp->total_lines = lw_io_get_total_lines_for_file (uri);
    g_free (uri);

    if (!_overlay_default_builtin_dictionary_settings (temp))
    {
      temp->longname = g_strdup_printf (gettext("%s Dictionary"), FILENAME);
      temp->shortname = g_strdup_printf ("%s", FILENAME);
      temp->load_position = -1;
    }

    temp->cached_resultlines = NULL;
    temp->current_resultline = NULL;

    //Done
    return temp;
}


//!
//! @brief Releases a LwDictInfo object from memory.
//!
//! Takes care of any of the work needed to release a LwDictInfo object from
//! memory.
//!
//! @param di LwDictInfo object to free
//!
void lw_dictinfo_free (LwDictInfo* di)
{
    g_free (di->filename);
    di->filename = NULL;

    g_free (di->shortname);
    di->shortname = NULL;

    g_free (di->longname);
    di->longname = NULL;

    free (di);
    di = NULL;
}


static gboolean _overlay_default_builtin_dictionary_settings (LwDictInfo *di)
{
    g_assert (di != NULL);

    if (di->engine == GW_ENGINE_EDICT)
    {
      if (strcmp(di->filename, "English") == 0)
      {
        di->longname = g_strdup_printf ("%s", gettext("English Dictionary"));
        di->shortname = g_strdup_printf ("%s", gettext("English"));
        di->load_position = 1;
      }
      else if (strcmp(di->filename, "Names") == 0)
      {
        di->longname = g_strdup_printf ("%s", gettext("Names Dictionary"));
        di->shortname = g_strdup_printf ("%s", gettext("Names"));
        di->load_position = 3;
      }
      else if (strcmp(di->filename, "Places") == 0)
      {
        di->longname = g_strdup_printf ("%s", gettext("Places Dictionary"));
        di->shortname = g_strdup_printf ("%s", gettext("Places"));
        di->load_position = 4;
      }
    }
    else if (di->engine == GW_ENGINE_KANJI)
    {
      if (strcmp(di->filename, "Kanji") == 0)
      {
        di->longname = g_strdup_printf ("%s", gettext("Kanji Dictionary"));
        di->shortname = g_strdup_printf ("%s", gettext("Kanji"));
        di->load_position = 2;
      }
    }
    else if (di->engine == GW_ENGINE_EXAMPLES)
    {
      if (strcmp(di->filename, "Examples") == 0)
      {
        di->longname = g_strdup_printf ("%s", gettext("Examples Dictionary"));
        di->shortname = g_strdup_printf ("%s", gettext("Examples"));
        di->load_position = 5;
      }
    }

    return (di->load_position > -1);
}


//!
//! @brief Installs a LwDictInst object using the provided gui update callback
//!        This function should normally only be used in the lw_dictinfo_uninstall function.
//! @param path String representing the path of the file to gunzip.
//! @param error Error handling
//!
gboolean lw_dictinfo_uninstall (LwDictInfo *di, LwIoProgressCallback cb, GError **error)
{
    //Sanity check
    if (error != NULL && *error != NULL) return FALSE;
    g_assert (di != NULL);

    //Declarations
    char *uri;

    //Initializations
    uri =  g_build_filename (lw_util_get_directory_for_engine (di->engine), di->filename, NULL);

    lw_io_remove (uri, error);
    if (cb != NULL) cb (1.0, di);

    g_free (uri);

    lw_dictinfolist_initialize ();

    return (*error == NULL);
}
 

char* lw_dictinfo_get_uri (LwDictInfo *di)
{
    //Declarations
    const char *directory;
    const char *filename;
    char *path;

    //Initializations
    directory = lw_util_get_directory_for_engine (di->engine);
    filename = di->filename;
    path = g_build_filename (directory, filename, NULL);

    return path;
}

