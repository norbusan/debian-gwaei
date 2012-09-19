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
//!  @file src/backend/dictinst.c
//!
//!  @brief Basic construct that hold data needed for installing a LwDictInst item
//!


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <libintl.h>

#include <glib.h>
#include <glib/gstdio.h>

#include <libwaei/libwaei.h>

static gboolean _cancel = FALSE;


//!
//! @brief Updates the LwDictInst source uri when the pref changes
//! @param settting A GSetting object
//! @param key The key of the pref
//! @param data User data passed to the preference listener
//!
static void _update_dictinst_source_uri_cb (GSettings *settings, char* key, gpointer data)
{
    LwDictInst *di = (LwDictInst*) data;
    char source_uri[200];

    lw_pref_get_string_by_schema (source_uri, di->schema, di->key, 200);
    g_free (di->uri[GW_DICTINST_NEEDS_DOWNLOADING]);
    di->uri[GW_DICTINST_NEEDS_DOWNLOADING] = g_strdup (source_uri);
}


//!
//! @brief Creates a new LwDictInst object.  It also connects a listener to the preference so it updates automatically.
//!
LwDictInst* lw_dictinst_new_using_pref_uri (const char* filename,
                                            const char* shortname,
                                            const char* longname,
                                            const char* description,
                                            const char* schema,
                                            const char* key,
                                            const LwEngine ENGINE,
                                            const LwCompression COMPRESSION,
                                            const LwEncoding ENCODING,
                                            gboolean split, gboolean merge, gboolean builtin)
{
    char source_uri[200];
    lw_pref_get_string_by_schema (source_uri, schema, key, 200);
    LwDictInst *di = NULL;

    di = lw_dictinst_new (
      filename,
      shortname, 
      longname, 
      description, 
      source_uri, 
      ENGINE, 
      COMPRESSION, 
      ENCODING, 
      split, 
      merge,
      builtin
    );

    di->schema = g_strdup (schema);
    di->key = g_strdup (key);
    if (di->listenerid_is_set == TRUE)
      lw_pref_remove_change_listener_by_schema (schema, di->listenerid);
    di->listenerid = lw_pref_add_change_listener_by_schema (schema, key, _update_dictinst_source_uri_cb, di);

    return di;
}



//!
//! @brief Creates a LwDictInst Object
//! 

LwDictInst* lw_dictinst_new (const char* filename,
                             const char* shortname,
                             const char* longname,
                             const char* description,
                             const char* source_uri,
                             const LwEngine ENGINE,
                             const LwCompression COMPRESSION,
                             const LwEncoding ENCODING,
                             gboolean split, gboolean merge, gboolean builtin)
{
    //Create the temp object to fill
    LwDictInst *temp = NULL;
    temp = (LwDictInst*) malloc (sizeof(LwDictInst));
    if (temp == NULL) g_error ("Out of memory. Could not create LwDictInst\n");

    //Initialize the variables for safety
    temp->filename = NULL;
    temp->shortname = NULL;
    temp->longname = NULL;
    temp->description = NULL;
    int i = 0;
    for (i = 0; i < GW_DICTINST_TOTAL_URIS; i++)
      temp->uri[i] = NULL;
    temp->schema = NULL;
    temp->key = NULL;
    temp->progress = 0;
    temp->selected = FALSE;
    temp->listenerid = 0;
    temp->listenerid_is_set = FALSE;
    temp->compression = COMPRESSION;    //!< Path to the gziped dictionary file
    temp->encoding = ENCODING;          //!< Path to the raw unziped dictionary file
    temp->engine = ENGINE;
    temp->uri_group_index = -1;
    temp->uri_atom_index = -1;
    temp->builtin = builtin;
    temp->split = split;
    temp->merge = merge;
    temp->mutex = g_mutex_new ();
    temp->current_source_uris = NULL;
    temp->current_target_uris = NULL;

    //Set the values
    temp->filename = NULL;
    temp->shortname = g_strdup (shortname);
    temp->longname = g_strdup (longname);
    temp->description = g_strdup (description);
    
    lw_dictinst_set_filename (temp, filename);
    lw_dictinst_set_download_source (temp, source_uri);


    return temp;
}


//!
//! @brief Frees a LwDictInst object
//! 
void lw_dictinst_free (LwDictInst* di)
{
    if (di->listenerid_is_set == TRUE)
      lw_pref_remove_change_listener_by_schema (di->schema, di->listenerid);

    g_free(di->filename);
    g_free(di->shortname);
    g_free(di->longname);
    g_free(di->description);
    g_strfreev (di->current_source_uris);
    g_strfreev (di->current_target_uris);

    int i = 0;
    while (i < GW_DICTINST_TOTAL_URIS)
    {
      g_free(di->uri[i]);
      i++;
    }
    di->progress = 0;
    g_free (di->schema);
    g_free (di->key);
    di->compression = 0;    //!< Path to the gziped dictionary file
    di->encoding = 0;          //!< Path to the raw unziped dictionary file
    di->engine = 0;
    di->split = FALSE;
    di->merge = FALSE;
    g_mutex_free (di->mutex);
    free (di);
}


//!
//! @brief Updates the filename save targets of the LwDictInst.
//! @param di The LwDictInst object to set the filename to
//! @param filename to copy to the LwDictInst object and use to generate uris
//!
void lw_dictinst_set_filename (LwDictInst *di, const char *filename)
{
    g_free (di->filename);
    di->filename = g_strdup (filename);

    lw_dictinst_regenerate_save_target_uris (di);
}


//!
//! @brief Updates the engine of the LwDictInst
//! @param The LwDictInst object to set the ENGINE to
//! @param ENGINE the engine that you want to set
//!
void lw_dictinst_set_engine (LwDictInst *di, const LwEngine ENGINE)
{
    di->engine = ENGINE;

    lw_dictinst_regenerate_save_target_uris (di);
}


//!
//! @brief Updates the encoding of the LwDictInst
//! @param di The LwDictInfo object to set the ENCODING to
//! @param ENCODING Tells the LwDictInfo object what the initial character encoding of the downloaded file will be
//!
void lw_dictinst_set_encoding (LwDictInst *di, const LwEncoding ENCODING)
{
    di->encoding = ENCODING;

    lw_dictinst_regenerate_save_target_uris (di);
}


//!
//! @brief Updates the compression of the LwDictInst
//! @param di The LwDictInfo objcet to set the COMPRESSION variable on
//! @param COMPRESSION Tells the LwDictInfo object what kind of compression the downloaded dictionary file will have.
//!
void lw_dictinst_set_compression (LwDictInst *di, const LwCompression COMPRESSION)
{
    di->compression = COMPRESSION;

    lw_dictinst_regenerate_save_target_uris (di);
}




//!
//! @brief Updates the download source of the LwDictInst object
//!
void lw_dictinst_set_download_source (LwDictInst *di, const char *SOURCE)
{
    g_free (di->uri[GW_DICTINST_NEEDS_DOWNLOADING]);
    di->uri[GW_DICTINST_NEEDS_DOWNLOADING] = g_strdup (SOURCE);
}


//!
//! @brief Updates the merge state of the LwDictInst
//!
void lw_dictinst_set_merge (LwDictInst *di, const gboolean MERGE)
{
    di->merge = MERGE;

    lw_dictinst_regenerate_save_target_uris (di);
}


//!
//! @brief Updates the split state of the LwDictInst
//!
void lw_dictinst_set_split (LwDictInst *di, const gboolean SPLIT)
{
    di->split = SPLIT;

    lw_dictinst_regenerate_save_target_uris (di);
}


//!
//! @brief This method should be called after the filename, engine, compression,
//!        or encoding members of the LwDictInst is changed to sync the new paths
//!
void lw_dictinst_regenerate_save_target_uris (LwDictInst *di)
{
    //Sanity check
    g_assert (di != NULL && di->filename != NULL);

    //Declarations
    char *cache_filename;
    char *engine_filename;
    const char *compression_ext;
    const char *encoding_ext;
    char *temp[2][GW_DICTINST_TOTAL_URIS];
    char *radicals_cache_filename;
    int i, j;

    //Remove the previous contents
    g_free (di->uri[GW_DICTINST_NEEDS_DECOMPRESSION]);
    g_free (di->uri[GW_DICTINST_NEEDS_TEXT_ENCODING]);
    g_free (di->uri[GW_DICTINST_NEEDS_POSTPROCESSING]);
    g_free (di->uri[GW_DICTINST_NEEDS_FINALIZATION]);
    g_free (di->uri[GW_DICTINST_NEEDS_NOTHING]);

    //Initialize the array
    for (i = 0; i < 2; i++)
      for (j = 1; j < GW_DICTINST_TOTAL_URIS; j++)
        temp[i][j] = NULL;

    //Initializations
    cache_filename = g_build_filename (lw_util_get_directory (GW_PATH_CACHE), di->filename, NULL);
    engine_filename = g_build_filename (lw_util_get_directory_for_engine (di->engine), di->filename, NULL);
    compression_ext = lw_util_get_compression_name (di->compression);
    encoding_ext = lw_util_get_encoding_name (di->encoding);

    temp[0][GW_DICTINST_NEEDS_DECOMPRESSION] =  g_strjoin (".", cache_filename, compression_ext, NULL);
    temp[0][GW_DICTINST_NEEDS_TEXT_ENCODING] =   g_strjoin (".", cache_filename, encoding_ext, NULL);
    temp[0][GW_DICTINST_NEEDS_POSTPROCESSING] =   g_strjoin (".", cache_filename, "UTF8", NULL);
    temp[0][GW_DICTINST_NEEDS_FINALIZATION] =  g_strdup (cache_filename);
    temp[0][GW_DICTINST_NEEDS_NOTHING] =  g_strdup (engine_filename);

    //Adjust the uris for the split dictionary exception case
    if (di->split)
    {
      g_free (temp[0][GW_DICTINST_NEEDS_FINALIZATION]);
      temp[0][GW_DICTINST_NEEDS_FINALIZATION] = g_build_filename (lw_util_get_directory (GW_PATH_CACHE), "Names", NULL);
      temp[1][GW_DICTINST_NEEDS_FINALIZATION] = g_build_filename (lw_util_get_directory (GW_PATH_CACHE), "Places", NULL);

      g_free (temp[0][GW_DICTINST_NEEDS_NOTHING]);
      temp[0][GW_DICTINST_NEEDS_NOTHING] = g_build_filename (lw_util_get_directory_for_engine (di->engine), "Names", NULL);
      temp[1][GW_DICTINST_NEEDS_NOTHING] = g_build_filename (lw_util_get_directory_for_engine (di->engine), "Places", NULL);
    }
    //Adjust the uris for the merge dictionary exception case
    else if (di->merge)
    {
      radicals_cache_filename = g_build_filename (lw_util_get_directory (GW_PATH_CACHE), "Radicals", NULL);
      temp[1][GW_DICTINST_NEEDS_DECOMPRESSION] =  g_strjoin (".", radicals_cache_filename, "gz", NULL);
      temp[1][GW_DICTINST_NEEDS_TEXT_ENCODING] =   g_strjoin (".", radicals_cache_filename, "EUC-JP", NULL);
      temp[1][GW_DICTINST_NEEDS_POSTPROCESSING] =   g_strjoin (".", radicals_cache_filename, "UTF8", NULL);
      g_free (radicals_cache_filename);
    }

    //Join the strings if appropriate
    for (i = 1; i < GW_DICTINST_TOTAL_URIS; i++)
    {
      di->uri[i] = g_strjoin (";", temp[0][i], temp[1][i], NULL);
    }

    //Cleanup
    for (i = 0; i < 2; i++)
      for (j = 1; j < GW_DICTINST_TOTAL_URIS; j++)
        if (temp[i][j] != NULL) g_free (temp[i][j]);

    g_free (cache_filename);
    g_free (engine_filename);

/*
    for (i = 1; i < GW_DICTINST_TOTAL_URIS; i++)
      printf("%s\n", di->uri[i]);
      printf("\n");
*/
}


//!
//! @brief Tells the installer mechanism if it is going to fail if it tries installing because of missing info
//!
gboolean lw_dictinst_data_is_valid (LwDictInst *di)
{
    //Declarations
    char *ptr;
    char **temp_string_array;
    int total_download_arguments;

    ptr = di->filename;
    if (ptr == NULL || strlen (ptr) == 0) return FALSE;

    ptr = di->uri[GW_DICTINST_NEEDS_DOWNLOADING];
    if (ptr == NULL || strlen (ptr) == 0) return FALSE;

    //Make sure the correct number of download arguments are available
    temp_string_array = g_strsplit (ptr, ";", -1);
    total_download_arguments = g_strv_length (temp_string_array);
    g_strfreev (temp_string_array);

    if (di->merge && total_download_arguments != 2) return FALSE;
    if (!di->merge && total_download_arguments != 1) return FALSE;

    ptr = di->uri[GW_DICTINST_NEEDS_DECOMPRESSION];
    if (ptr == NULL || strlen (ptr) == 0) return FALSE;

    ptr = di->uri[GW_DICTINST_NEEDS_TEXT_ENCODING];
    if (ptr == NULL || strlen (ptr) == 0) return FALSE;

    ptr = di->uri[GW_DICTINST_NEEDS_POSTPROCESSING];
    if (ptr == NULL || strlen (ptr) == 0) return FALSE;

    ptr = di->uri[GW_DICTINST_NEEDS_FINALIZATION];
    if (ptr == NULL || strlen (ptr) == 0) return FALSE;

    ptr = di->uri[GW_DICTINST_NEEDS_NOTHING];
    if (ptr == NULL || strlen (ptr) == 0) return FALSE;

    if (di->engine < 0 || di->engine >= GW_ENGINE_TOTAL) return FALSE;
    if (di->compression < 0 || di->compression >= GW_COMPRESSION_TOTAL) return FALSE;
    if (di->encoding < 0 || di->encoding >= GW_ENCODING_TOTAL) return FALSE;

    return TRUE;
}


//!
//! @brief Downloads or copies the file to the dictionary directory to be worked on
//!        This function should normally only be used in the lw_dictinst_install function.
//! @param path String representing the path of the file to gunzip.
//! @param error Error handling
//! @see lw_dictinst_download
//! @see lw_dictinst_convert_encoding
//! @see lw_dictinst_postprocess
//! @see lw_dictinst_install
//!
gboolean lw_dictinst_download (LwDictInst *di, LwIoProgressCallback cb, GError **error)
{
    //Sanity check
    if (error != NULL && *error != NULL) return FALSE;
    if (_cancel) return FALSE;
    g_assert (di != NULL);

    //Declarations
    char *source;
    char *target;
    gpointer data;
    int i;
    LwDictInstUri group_index;

    //Initializations
    group_index = GW_DICTINST_NEEDS_DOWNLOADING;
    i = 0;
    data = di;

    while ((source = lw_dictinst_get_source_uri (di, group_index, i)) != NULL &&
           (target = lw_dictinst_get_target_uri (di, group_index, i)) != NULL
          )
    {
      //File is located locally so copy it
      if (g_file_test (source, G_FILE_TEST_IS_REGULAR))
        lw_io_copy (source, target, (LwIoProgressCallback) cb, data, error);
      //Download the file
      else
        lw_io_download (source, target, cb, data, error);
      i++;
    }

    return (*error == NULL);
}


//!
//! @brief Detects the compression scheme of a file and decompresses it using the approprate function.
//!        This function should normally only be used in the lw_dictinst_install function.
//! @param path String representing the path of the file to gunzip.
//! @param error Error handling
//! @see lw_dictinst_download
//! @see lw_dictinst_convert_encoding
//! @see lw_dictinst_postprocess
//! @see lw_dictinst_install
//!
gboolean lw_dictinst_decompress (LwDictInst *di, LwIoProgressCallback cb, GError **error)
{
    //Sanity check
    if (error != NULL && *error != NULL) return FALSE;
    if (_cancel) return FALSE;
    g_assert (di != NULL);

    //Declarations
    gint status;
    gpointer data;
    char *source;
    char *target;
    int i;
    LwDictInstUri group_index;

    //Initializations
    data = di;
    i = 0;
    group_index = GW_DICTINST_NEEDS_DECOMPRESSION;

    if ((source = lw_dictinst_get_source_uri (di, group_index, 0)) != NULL &&
        (target = lw_dictinst_get_target_uri (di, group_index, 0)) != NULL
       )
    {
      //Sanity check
      g_assert (g_file_test (source, G_FILE_TEST_IS_REGULAR));

      //Preform the correct decompression
      switch (di->compression)
      {
        case GW_COMPRESSION_GZIP:
          status = lw_io_gunzip_file (source, target, cb, data, error);
          break;
        case GW_COMPRESSION_NONE:
          status =  lw_io_copy (source, target, cb, data, error);
          break;
        default:
          break;
      }

      //If there is another path, it is assumed to be the radicals dictionary
      if ((source = lw_dictinst_get_source_uri (di, group_index, 1)) != NULL &&
          (target = lw_dictinst_get_target_uri (di, group_index, 1)) != NULL
         )
      {
        //Sanity check
        g_assert (g_file_test (source, G_FILE_TEST_IS_REGULAR));

        status = lw_io_gunzip_file (source, target, cb, data, error);
      }
    }

    return (error == NULL && *error == NULL);
}


//!
//! @brief Converts the encoding to UTF8 for the file
//!        This function should normally only be used in the lw_dictinst_install function.
//! @param path String representing the path of the file to gunzip.
//! @param error Error handling
//! @see lw_dictinst_download
//! @see lw_dictinst_convert_encoding
//! @see lw_dictinst_postprocess
//! @see lw_dictinst_install
//!
gboolean lw_dictinst_convert_encoding (LwDictInst *di, LwIoProgressCallback cb, GError **error)
{
    //Sanity check
    if (error != NULL && *error != NULL) return FALSE;
    if (_cancel) return FALSE;
    g_assert (di != NULL);

    //Declarations
    char *source;
    char *target;
    const char *encoding_name;
    gpointer data;
    LwDictInstUri group_index;

    //Initializations
    group_index = GW_DICTINST_NEEDS_TEXT_ENCODING;
    encoding_name = lw_util_get_encoding_name (di->encoding);
    data = di;

    if ((source = lw_dictinst_get_source_uri (di, group_index, 0)) != NULL &&
        (target = lw_dictinst_get_target_uri (di, group_index, 0)) != NULL
       )
    {
      if (di->encoding == GW_ENCODING_UTF8)
        lw_io_copy (source, target, cb, data, error);
      else
        lw_io_copy_with_encoding (source, target, encoding_name, "UTF-8", cb, data, error);

      //If there is another path, it is assumed to be the radicals dictionary
      if ((source = lw_dictinst_get_source_uri (di, group_index, 1)) != NULL &&
          (target = lw_dictinst_get_target_uri (di, group_index, 1)) != NULL
         )
        lw_io_copy_with_encoding (source, target, "EUC-JP", "UTF-8", cb, data, error);
    }

    return (*error == NULL);
}


//!
//! @brief does the required postprocessing on a dictionary
//!        This function should normally only be used in the lw_dictinst_install function.
//! @param path String representing the path of the file to gunzip.
//! @param error Error handling
//! @see lw_dictinst_download
//! @see lw_dictinst_convert_encoding
//! @see lw_dictinst_postprocess
//! @see lw_dictinst_install
//!
gboolean lw_dictinst_postprocess (LwDictInst *di, LwIoProgressCallback cb, GError **error)
{
    //Sanity check
    if (error != NULL && *error != NULL) return FALSE;
    if (_cancel) return FALSE;
    g_assert (di != NULL);

    //Declarations
    gpointer data;
    gchar *source;
    gchar *source2;
    gchar *target;
    gchar *target2;
    LwDictInstUri group_index;

    //Initializations
    data = di;
    group_index = GW_DICTINST_NEEDS_POSTPROCESSING;


    //Rebuild the mix dictionary
    if (di->merge)
    {
      if ((source = lw_dictinst_get_source_uri (di, group_index, 0)) != NULL &&
          (source2 = lw_dictinst_get_source_uri (di, group_index, 1)) != NULL &&
          (target = lw_dictinst_get_target_uri (di, group_index, 0)) != NULL
         )
      {
      lw_io_create_mix_dictionary (target, source, source2, cb, data, error);
      }
    }

    //Rebuild the names dictionary
    else if(di->split)
    {
      if ((source = lw_dictinst_get_source_uri (di, group_index, 0)) != NULL &&
          (target = lw_dictinst_get_target_uri (di, group_index, 0)) != NULL &&
          (target2 = lw_dictinst_get_target_uri (di, group_index, 1)) != NULL
         )
      lw_io_split_places_from_names_dictionary (target, target2, source, cb, data, error);
    }

    //Just copy the file no postprocessing required
    else
    {
      if ((source = lw_dictinst_get_source_uri (di, group_index, 0)) != NULL &&
          (target = lw_dictinst_get_target_uri (di, group_index, 0)) != NULL
         )
      lw_io_copy (source, target, cb, data, error);
    }

    //Finish
    return (*error == NULL);
}


//!
//! @brief does the required postprocessing on a dictionary
//!        This function should normally only be used in the lw_dictinst_install function.
//! @param path String representing the path of the file to gunzip.
//! @param error Error handling
//! @see lw_dictinst_download
//! @see lw_dictinst_convert_encoding
//! @see lw_dictinst_postprocess
//! @see lw_dictinst_install
//!
gboolean lw_dictinst_finalize (LwDictInst *di, LwIoProgressCallback cb, GError **error)
{
    //Sanity check
    if (error != NULL && *error != NULL) return FALSE;
    if (_cancel) return FALSE;
    g_assert (di != NULL);

    //Declarations
    char *source;
    char *target;
    gpointer data;
    LwDictInstUri group_index;
    int atom_index;
    int i;

    //Initializations
    data = di;
    group_index = GW_DICTINST_NEEDS_FINALIZATION;
    i = 0;

    while ((source = lw_dictinst_get_source_uri (di, group_index, i)) != NULL &&
           (target = lw_dictinst_get_target_uri (di, group_index, i)) != NULL
          )
    {
      lw_io_copy (source, target, cb, data, error);
      i++;
    }

    //Finish
    return (*error == NULL);
}


//!
//! @brief removes temporary files created by installation in the dictionary cache folder
//!
void lw_dictinst_clean (LwDictInst *di, LwIoProgressCallback cb)
{
    //Declarations
    LwDictInstUri group_index;
    int i;
    char *uri;
    char *source;

    //Initializations
    group_index = 0;
    uri = NULL;

    //Loop through all of the uris except the final destination
    while (group_index < GW_DICTINST_NEEDS_NOTHING)
    {
      i = 0;
      while ((source = lw_dictinst_get_source_uri (di, group_index, i)) != NULL)
      {
        g_remove (source);
        i++;
      }
      group_index++;
    }

    if (di->current_source_uris != NULL) g_strfreev (di->current_source_uris);
    if (di->current_target_uris != NULL) g_strfreev (di->current_target_uris);
    di->current_source_uris = NULL;
    di->current_target_uris = NULL;
    di->uri_group_index = -1;
    di->uri_atom_index = -1;
}


//!
//! @brief Installs a LwDictInst object using the provided gui update callback
//!        This function should normally only be used in the lw_dictinst_install function.
//! @param path String representing the path of the file to gunzip.
//! @param error Error handling
//! @see lw_dictinst_download
//! @see lw_dictinst_convert_encoding
//! @see lw_dictinst_postprocess
//! @see lw_dictinst_install
//!
gboolean lw_dictinst_install (LwDictInst *di, LwIoProgressCallback cb, GError **error)
{
    g_assert (*error == NULL && di != NULL);

    lw_dictinst_download (di, cb, error);
    lw_dictinst_decompress (di, cb, error);
    lw_dictinst_convert_encoding (di, cb, error);
    lw_dictinst_postprocess (di, cb, error);
    lw_dictinst_finalize (di, cb, error);
    lw_dictinst_clean (di, cb);

    return (*error == NULL);
}


//!
//! @brief Returns a status string describing the current process being taken
//!        on a LwDictInst object
//! @param di A LwDictInst to get the status of
//! @param long_form Whether you want the long or short form of the status messages.
//! @returns An allocated string that should be freed with gfree when finished
//!
gchar* lw_dictinst_get_status_string (LwDictInst *di, gboolean long_form)
{
    //Declarations
    gchar *string;

    switch (di->uri_group_index) {
      case GW_DICTINST_NEEDS_DOWNLOADING:
        if (long_form)
          string = g_strdup_printf (gettext("Downloading %s..."), di->longname);
        else
          string = g_strdup_printf (gettext("Downloading..."));
        break;
      case GW_DICTINST_NEEDS_TEXT_ENCODING:
        if (long_form)
          string = g_strdup_printf (gettext("Converting the encoding of %s from %s to UTF-8..."), di->longname, lw_util_get_encoding_name (di->encoding));
        else
          string = g_strdup_printf (gettext("Converting the encoding to UTF-8..."));
        break;
      case GW_DICTINST_NEEDS_DECOMPRESSION:
        if (long_form)
          string = g_strdup_printf (gettext("Decompressing %s from %s file..."), di->longname, lw_util_get_compression_name (di->compression));
        else
          string = g_strdup_printf (gettext("Decompressing..."));
        break;
      case GW_DICTINST_NEEDS_POSTPROCESSING:
        if (long_form)
          string = g_strdup_printf (gettext("Doing postprocessing on %s..."), di->longname);
        else
          string = g_strdup_printf (gettext("Postprocessing..."));
        break;
      case GW_DICTINST_NEEDS_FINALIZATION:
        string = g_strdup_printf (gettext("Finalizing installation of %s..."), di->longname);
        break;
      case GW_DICTINST_NEEDS_NOTHING:
        string = g_strdup_printf (gettext("Installed."));
        break;
      default:
        string = g_strdup_printf (" ");
        break;
    }

    return string;
}


double lw_dictinst_get_process_progress (LwDictInst *di, double fraction)
{
    //Declarations
    double current;
    double final;
    double output_fraction;
    char *ptr;
    
    //Initializations
    current = 0.0;
    final = 0.0;
    output_fraction = 0.0;
    di->progress = fraction;

    //Get the current progress
    current = fraction + ((double) di->uri_atom_index);

    //Calculate the amount needed for the whole process to finish
    for (ptr = di->uri[di->uri_group_index]; ptr != NULL; ptr = strchr(ptr, ';'))
    {
      final += 1.0;
      ptr++;
    }

    if (final > 0.0)
      output_fraction = current / final;

    return output_fraction;
}


double lw_dictinst_get_total_progress (LwDictInst *di, double fraction)
{
    //Declarations
    double output_fraction, current, final;
    int i;
    char *ptr;

    //Definitions
    output_fraction = 0.0;
    current = 0.0;
    final = 0.0;
    di->progress = fraction;
    const double DOWNLOAD_WEIGHT = 3.0;

    //Calculate the already completed activities
    for (i = 0; i < di->uri_group_index && i < GW_DICTINST_NEEDS_NOTHING; i++)
    {
      for (ptr = di->uri[i]; ptr != NULL; ptr = strchr(ptr, ';'))
      {
        if (i == GW_DICTINST_NEEDS_DOWNLOADING)
          current += 1.0 * DOWNLOAD_WEIGHT;
        else
          current += 1.0;
        ptr++;
      }
    }
    //Add the current in progress activity
    if (i == GW_DICTINST_NEEDS_DOWNLOADING)
      current += (fraction + (double) di->uri_atom_index) * DOWNLOAD_WEIGHT;
    else
      current += fraction + (double) di->uri_atom_index;

    //Calculate the amount needed for the whole process to finish
    for (i = 0; i < GW_DICTINST_NEEDS_NOTHING; i++)
    {
      for (ptr = di->uri[i]; ptr != NULL; ptr = strchr(ptr, ';'))
      {
        if (i == GW_DICTINST_NEEDS_DOWNLOADING)
          final += 1.0 * DOWNLOAD_WEIGHT;
        else
          final += 1.0;
        ptr++;
      }
    }

    if (final > 0.0)
      output_fraction = current / final;

    return output_fraction;
}


//!
//! @brief Gets the uris saved in the LwDictInfo in the form of an array.  It should not be freed.
//! @param INDEX The LwDictInstUri index
//! @returns An allocated string internal to the LwDictInst that should not be freed
//!
char* lw_dictinst_get_source_uri (LwDictInst *di, const LwDictInstUri GROUP_INDEX, const int ATOM_INDEX)
{
    //Sanity check
    g_assert (GROUP_INDEX >= 0 && GROUP_INDEX < GW_DICTINST_NEEDS_NOTHING);

    //Declarations
    char *uri;

    //Set up the backbone if it isn't already
    if (GROUP_INDEX != di->uri_group_index)
    {
      if (di->current_source_uris != NULL) g_strfreev (di->current_source_uris);
      if (di->current_target_uris != NULL) g_strfreev (di->current_target_uris);
      di->current_source_uris = g_strsplit (di->uri[GROUP_INDEX], ";", -1);
      di->current_target_uris = g_strsplit (di->uri[GROUP_INDEX + 1], ";", -1);
    }

    //Get the information we came here for
    if (ATOM_INDEX >= 0 && ATOM_INDEX < g_strv_length (di->current_source_uris))
    {
      di->uri_group_index = GROUP_INDEX;
      di->progress = 0.0;
      di->uri_atom_index = ATOM_INDEX;
      uri = di->current_source_uris[ATOM_INDEX];
    }
    else
    {
      uri = NULL;
    }

    return uri;
}


//!
//! @brief Gets the uris saved in the LwDictInfo in the form of an array.  It should not be freed.
//! @param INDEX The LwDictInstUri index
//! @returns An allocated string internal to the LwDictInst that should not be freed
//!
char* lw_dictinst_get_target_uri (LwDictInst *di, const LwDictInstUri GROUP_INDEX, const int ATOM_INDEX)
{
    //Sanity check
    g_assert (GROUP_INDEX >= 0 && GROUP_INDEX < GW_DICTINST_NEEDS_NOTHING);

    //Declarations
    char *uri;

    //Set up the backbone if it isn't already
    if (GROUP_INDEX != di->uri_group_index)
    {
      di->uri_group_index = GROUP_INDEX;
      //di->progress = 0.0;
      if (di->current_source_uris != NULL) g_strfreev (di->current_source_uris);
      if (di->current_target_uris != NULL) g_strfreev (di->current_target_uris);
      di->current_source_uris = g_strsplit (di->uri[GROUP_INDEX], ";", -1);
      di->current_target_uris = g_strsplit (di->uri[GROUP_INDEX + 1], ";", -1);
    }

    //Get the information we came here for
    if (ATOM_INDEX >= 0 && ATOM_INDEX < g_strv_length (di->current_target_uris))
    {
      //di->uri_atom_index = ATOM_INDEX; //Only source sets this variable.  not target.
      uri = di->current_target_uris[ATOM_INDEX];
    }
    else
    {
      uri = NULL;
    }

    return uri;
}


void lw_dictinst_set_cancel_operations (LwDictInst *di, gboolean state)
{
    _cancel = state;
    lw_io_set_cancel_operations (state);
}

