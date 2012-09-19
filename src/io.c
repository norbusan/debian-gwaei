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
//! @file src/io.c
//!
//! @brief File reading and writing.
//!
//! Functions that mostly deal with reading and writing files.
//!


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <locale.h>
#include <libintl.h>

#include <glib.h>
#include <glib/gstdio.h>
#include <curl/curl.h>

#include <gwaei/definitions.h>
#include <gwaei/regex.h>
#include <gwaei/utilities.h>
#include <gwaei/dictionary-objects.h>
#include <gwaei/preferences.h>


char save_path[FILENAME_MAX] = { '\0' };


//!
//! \brief Writes a file using the given text and write mode
//!
//! This function is an extension of the save as/append buttons.
//! Thus, the program currently only uses the write and append write modes.
//! The save path is gathered using some's build in function for a dialog, and the
//! path is saved for as long as the program is running, so it doesn't need to be
//! asked a second time.
//!
//! @param write_mode A constant char representing the write mode to be used (w,a)
//! @param text A char pointer to some text to save.
//!
void gw_io_write_file(const char* write_mode, gchar *text)
{
    if (save_path[0] != '\0')
    {
      //Get the data for the file
      gchar *text_ptr;
      int status = 0;

      text_ptr = &text[0];

      //Write it
      FILE* fd;
      fd = fopen(save_path, write_mode);

      while (*text_ptr != '\0' && (status = fputc(*text_ptr, fd)) != EOF)
        text_ptr++;

      if (status != EOF) fputc('\n', fd);
      //if (status != EOF) fputc('\0', fd);

      fclose(fd);
      fd = NULL;

      //Cleanup
      text_ptr = NULL;
    }
}


//!
//! \brief Checks to see if rsync is available
//!
//! The variable that this checks is set at compile time, so if the program
//! was compiled when rsync wasn't available, the option will be preminiently
//! disabled.
//!
//! @return The status of the existance of rsync
//!
gboolean gw_io_check_for_rsync ()
{
    gboolean rsync_exists;
    rsync_exists = ( RSYNC != NULL     &&
                     strlen(RSYNC) > 0 &&
                     g_file_test(RSYNC, G_FILE_TEST_IS_EXECUTABLE)
                   );
    return rsync_exists;
}


//!
//! \brief Copies a file and creates a new one using the new encoding
//!
//! This function is made to be risilient to errors unlike the built in easy
//! to use g_convert.  It will skip over any characters it has problems converting.
//!
//! @param source_path The source file to change the encoding on.
//! @param target_path The place to save the new file with the new encoding.
//! @param source_encoding The encoding of the source file.
//! @param target_encoding THe wanted encoding in the new file to be created.
//!
//! @return The status of the conversion opertaion
//!
gboolean gw_io_copy_with_encoding( char *source_path,     char *target_path,
                                  char *source_encoding, char *target_encoding,
                                  GError **error)
{
    if (*error != NULL) return FALSE;

    FILE* readfd = NULL;
    readfd = fopen (source_path, "r");
    if (readfd == NULL) exit(0);

    FILE* writefd = NULL;
    writefd = fopen (target_path, "w");
    if (writefd == NULL) exit(0);

    int length = MAX_LINE;
    char buffer[length];
    char output[length];
    gsize inbytes_left, outbytes_left;
    char *inptr, *outptr;
    char prev_inbytes = 0;

    size_t written;
    GIConv conv = g_iconv_open (target_encoding, source_encoding);
    while (fgets(buffer, length, readfd) != NULL)
    {
      inptr = buffer; outptr = output;
      inbytes_left = length; outbytes_left = length;
      while (inbytes_left && outbytes_left)
      {
        if (g_iconv (conv, &inptr, &inbytes_left, &outptr, &outbytes_left) == -1) break;

        //Force increment if there is something wrong
        if (prev_inbytes == inbytes_left)
        {
          inptr++;
          inbytes_left--;
        }
        //Normal operation
        prev_inbytes = inbytes_left;
        inptr = inptr + strlen(inptr) - inbytes_left;
        outptr = outptr + strlen(outptr) - outbytes_left;
      }
      written = fwrite(output, 1, strlen(output), writefd); 
    }
    g_iconv_close (conv);

    fclose(readfd);
    fclose(writefd);

    return TRUE;
}


//!
//! \brief Private function made to be used with gw_io_download_file
//!
//! @param ptr TBA
//! @param size TBA
//! @param nmemb TBA
//! @param stream TBA
//!
static size_t libcurl_write_func (void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    return fwrite(ptr, size, nmemb, stream);
}
 

//!
//! \brief Private function made to be used with gw_io_download_file
//!
//! @param ptr TBA
//! @param size TBA
//! @param nmemb TBA
//! @param stream TBA
//!
static size_t libcurl_read_func (void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    return fread (ptr, size, nmemb, stream);
}


//!
//! \brief Private struct made to be used with gw_io_download_file
//!
//! @param ptr TBA
//! @param size TBA
//! @param nmemb TBA
//! @param stream TBA
//!
typedef struct libcurl_callback_func_with_data {
    int (*callback_function) (char*, int, gpointer);
    gpointer data;
} libcurl_callback_func_with_data; 


//!
//! \brief Private struct made to be used with gw_io_download_file
//!
//! @param ptr TBA
//! @param size TBA
//! @param nmemb TBA
//! @param stream TBA
//!
int libcurl_update_progressbar (void   *data,
                                double  dltotal,
                                double  dlnow,
                                double  ultotal,
                                double  ulnow   )
{
    int percent = 0;
    if (dltotal != 0.0)
      percent = (int) (dlnow / dltotal * 100.0);
    libcurl_callback_func_with_data *curldata = (libcurl_callback_func_with_data*) data;
    return (curldata->callback_function) (NULL, percent, curldata->data);
}


//!
//! \brief Downloads a file using libcurl
//!
//! @param source_path String of the source url
//! @param save_path String of the path to save the file locally
//! @param func Pointer to a function to update
//! @param data gpointer to data to pass to the function pointer
//! @param error Error handling
//!
gboolean gw_io_download_file (char *source_path, char *save_path,
                              int (*callback_function) (char*, int, gpointer), gpointer data, GError **error)
{
    if (*error != NULL) return FALSE;

    CURL *curl;
    CURLcode res;
    FILE *outfile = NULL;
    GwDictInfo *di = (GwDictInfo*) data;

    curl = curl_easy_init ();

    if (curl == NULL) return FALSE;

    outfile = fopen(save_path, "w");

    curl_easy_setopt(curl, CURLOPT_URL, source_path);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, outfile);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, libcurl_write_func);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, libcurl_read_func);

    libcurl_callback_func_with_data libcurl_data;
    libcurl_data.callback_function = callback_function;
    libcurl_data.data = data;

    if (callback_function != NULL)
    {
      curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
      curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, libcurl_update_progressbar);
      curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &libcurl_data);
    }

    res = curl_easy_perform(curl);
     
    fclose(outfile);
    curl_easy_cleanup(curl);

    if (res != 0)
    {
      g_remove(save_path);
      if (di->status != GW_DICT_STATUS_CANCELING)
      {
        GQuark quark;
        quark = g_quark_from_string (GW_GENERIC_ERROR);
        const char *message = gettext(curl_easy_strerror(res));
        if (error != NULL) *error = g_error_new_literal (quark, GW_FILE_ERROR, message);
      }
    }

    return (res == 0);
}


//!
//! \brief Copies a local file to another local location
//!
//! @param source_path String of the source url
//! @param save_path String of the path to save the file locally
//! @param error Error handling
//!
gboolean gw_io_copy_file (char *source_path, char *target_path, GError **error)
{
    if (*error != NULL) return FALSE;

    GQuark quark;
    quark = g_quark_from_string (GW_GENERIC_ERROR);

    char *contents = NULL;
    gsize length;
    if ( g_file_get_contents(source_path, &contents, &length, NULL) == FALSE ||
         g_file_set_contents(target_path, contents, length, NULL) == FALSE     )
    {
      remove(target_path);
      if (error != NULL) *error = g_error_new_literal (quark, GW_FILE_ERROR, gettext("File copy error"));
      return FALSE;
    }
    if (contents != NULL) g_free(contents);

    return TRUE;
}


//Creates a single dictionary containing both the radical dict and kanji dict
gboolean gw_io_create_mix_dictionary (char *mpath, char *kpath, char *rpath)
{
    FILE* kanji_file =  fopen(kpath, "r");
    FILE* output_file = fopen(mpath, "w");
    FILE *radicals_file = NULL;

    char radicals_input[MAX_LINE];
    char* radicals_ptr = NULL;

    char kanji_input[MAX_LINE];
    char* kanji_ptr = NULL;

    char output[MAX_LINE * 2];
    char* output_ptr = NULL;

    char* temp_ptr;

    //Loop through the kanji file
    while ( fgets(kanji_input, MAX_LINE, kanji_file) != NULL )
    {
      if(kanji_input[0] == '#') continue;

      kanji_ptr = kanji_input;
      output_ptr = output;

      //1. Copy the kanji character from the kanji line
      while (*kanji_ptr != ' ')
      {
        *output_ptr = *kanji_ptr;
        output_ptr++;
        kanji_ptr++;
      }

      //2. Find the relevent radical line and insert it if available
      radicals_file = fopen(rpath, "r");
      while ( fgets(radicals_input, MAX_LINE, radicals_file) != NULL )
      {
        //Check for a match
        temp_ptr = kanji_input;
        radicals_ptr = radicals_input;
        while (*radicals_ptr != ' ' && *radicals_ptr == *temp_ptr)
        {
          temp_ptr++;
          radicals_ptr++;
        }

        //If a match is found...
        if (*radicals_ptr == ' ')
        {
          //Skip over the colon
          radicals_ptr++;
          radicals_ptr++;
   
          //Copy the data
          while (*(radicals_ptr + 1) != '\0')
          {
            *output_ptr = *radicals_ptr;
            output_ptr++;
            radicals_ptr++;
          }

          break;
        }
      }
      fclose(radicals_file);

      //3. Copy the rest of the kanji line to output
      while (*kanji_ptr != '\0')
      {
        *output_ptr = *kanji_ptr;
        output_ptr++;
        kanji_ptr++;
      }

      //4. Close off the string and write it to the file
      *output_ptr = '\0';
      fputs(output, output_file);
      output[0] = '\0';
    }

    //Cleanup
    fclose(kanji_file);
    fclose(output_file);

    return TRUE;
}


gboolean gw_io_split_places_from_names_dictionary (char *spath, char *npath, char *ppath)
{
    /*
      Current composition of the Enamdic dictionary
      ----------------------------------------------
      s - surname (138,500)
      p - place-name (99,500)
      u - person name, either given or surname, as-yet unclassified (139,000) 
      g - given name, as-yet not classified by sex (64,600)
      f - female given name (106,300)
      m - male given name (14,500)
      h - full (family plus given) name of a particular person (30,500)
      pr - product name (55)
      co - company name (34)
      ---------------------------------------------
    */
    int eflags_exist = REG_EXTENDED | REG_NOSUB;
    //Setup the regular expression for Places
    regex_t re_place_line;
    char *place_pattern = "([\\(,])((p)|(st))([\\),])";
    if (regcomp(&re_place_line, place_pattern, eflags_exist) != 0)
    {
      printf("A problem occured while setting the regular expression for place\n");
      return FALSE; 
    }

    //Setup the regular expression for Names
    regex_t re_name_line;
    char *name_pattern = "([\\(,])((s)|(u)|(g)|(f)|(m)|(h)|(pr)|(co))([\\),])";
    if (regcomp(&re_name_line, name_pattern, eflags_exist) != 0)
    {
      printf("A problem occured while setting the regular expression for name\n");
      return FALSE;
    }

    //Buffer
    char buffer[MAX_LINE];

    g_remove (npath);
    g_remove (ppath);

    //Setup the file descriptors
    FILE *input_stream = NULL;
    input_stream = fopen(spath, "r");
    FILE *places_stream;
    places_stream = fopen(ppath, "w");
    FILE *names_stream = NULL;
    names_stream = fopen(npath, "w");
    

    //Error checking
    int  place_write_error = 0;
    long total_place_lines = 0;
    int  name_write_error  = 0;
    long total_name_lines  = 0;

    //Start writing the child files
    while ( fgets(buffer, MAX_LINE, input_stream) != NULL &&
            place_write_error != EOF &&
            name_write_error  != EOF                            )
    {
      if (places_stream != NULL && regexec(&re_place_line, buffer, 1, NULL, 0) == 0)
      {
        place_write_error = fputs(buffer, places_stream);
        total_place_lines++;
      }
      if (names_stream != NULL && regexec(&re_name_line, buffer, 1, NULL, 0) == 0)
      {
        name_write_error =  fputs(buffer, names_stream);
        total_name_lines++;
      }
    }

    //Check for failure
    ;

    //Cleanup
    if (input_stream != NULL)
    {
      fclose(input_stream);
      input_stream = NULL;
    }

    if (places_stream != NULL)
    {
      fclose(places_stream);
      places_stream = NULL;
    }
    regfree(&re_place_line);

    if (names_stream != NULL)
    {
      fclose(names_stream);
      names_stream = NULL;
    }
    regfree(&re_name_line);


    if (total_place_lines == 0 && ppath != NULL)
      g_remove(ppath);
    if (total_name_lines == 0 && npath != NULL)
      g_remove(npath);

    return (place_write_error != EOF && name_write_error != EOF);
}


//!
//! \brief Gunzips a file
//!
//! @param path String representing the path of the file to gunzip
//! @param error Error handling
//!
gboolean gw_io_gunzip_file (char *path, GError **error)
{
    if (*error != NULL) return FALSE;

    GQuark quark;
    quark = g_quark_from_string (GW_GENERIC_ERROR);
    gboolean success;

    char command[FILENAME_MAX]; 
    strcpy(command, GUNZIP);
    strcat(command, " ");
    strcat(command, path);

    success = (system(command) == 0);

    if (success == FALSE) {
      g_remove(path);
      const char *message = gettext("gunzip error");
      if (error != NULL) *error = g_error_new_literal (quark, GW_FILE_ERROR, message);
    }

    return success;
} 


//!
//! \brief Unzips a file
//!
//! @param path String representing the path of the file to unzip
//! @param error Error handling
//!
gboolean gw_io_unzip_file (char *path, GError **error)
{
    GQuark quark;
    quark = g_quark_from_string (GW_GENERIC_ERROR);
    gboolean success;

    char extraction_directory[FILENAME_MAX];
    strcpy(extraction_directory, path);
    *strrchr (extraction_directory, G_DIR_SEPARATOR) = '\0';


    char command[FILENAME_MAX]; 
    strcpy(command, UNZIP);
    strcat(command, " -o ");
    strcat(command, path);
    strcat(command, " -d ");
    strcat(command, extraction_directory);

    success = (system(command) == 0);

    if (success == FALSE) {
      g_remove(path);
      const char *message = gettext("gunzip error");
      if (error != NULL) *error = g_error_new_literal (quark, GW_FILE_ERROR, message);
    }
    return success;
}


//!
//! \brief Returns the total lines in a dictionary file for processing purposes.
//!
//! @param path The string representing path to the dictionary file
//!
int gw_io_get_total_lines_for_path (char *path)
{
    //Calculate the number of lines in the dictionary
    char line[MAX_LINE];
    int total_lines = 0;
    FILE *fd = fopen (path, "r");
    if (fd != NULL)
    {
      while (fgets(line, MAX_LINE, fd) != NULL)
        total_lines++;
      fclose(fd);
    }
    return total_lines;
}


//!
//! \brief Uninstalls a dictionary by it's name and all related dictionaries
//!
//! @param name The string representing the name of the dictionary to uninstall.
//! @param callback_function The function to use to post status messages to.
//! @param data gpointer to the data to pass to the callback_function
//! @param long_messages Whether the messages should be long form or short form.
//!
void gw_io_uninstall_dictinfo (GwDictInfo *di,    int (*callback_function) (char*, int, gpointer),
                                 gpointer data, gboolean long_messages                            )
{
    if (di == NULL) return;

    char *message = NULL;
    if (long_messages)
      // TRANSLATORS: The %s stands for a file path
      message = g_strdup_printf (gettext("Removing %s..."), di->path);
    else
      message = g_strdup_printf (gettext("Removing..."));

    if (callback_function != NULL) {
      callback_function (message, -1, data);
    }

    if (message != NULL)
    {
      g_free (message);
      message = NULL;
    }

    g_remove(di->path);
    g_remove(di->gz_path);
    g_remove(di->sync_path);

    di->status = GW_DICT_STATUS_NOT_INSTALLED;
    di->load_position = -1;


    GwDictInfo *r_di, *m_di, *k_di, *p_di;
    r_di = gw_dictlist_get_dictinfo_by_id (GW_DICT_ID_RADICALS);
    k_di = gw_dictlist_get_dictinfo_by_id (GW_DICT_ID_KANJI);
    m_di = gw_dictlist_get_dictinfo_by_id (GW_DICT_ID_MIX);
    p_di = gw_dictlist_get_dictinfo_by_id (GW_DICT_ID_PLACES);

    if (di->id == GW_DICT_ID_KANJI)
    {
      gw_io_uninstall_dictinfo (r_di, callback_function, data, long_messages);
      gw_io_uninstall_dictinfo (m_di, callback_function, data, long_messages);
    }
    else if (di->id == GW_DICT_ID_NAMES)
    {
      gw_io_uninstall_dictinfo (p_di, callback_function, data, long_messages);
    }
}


//!
//! \brief Installs a dictionary by it's name and all related dictionaries
//!
//! @param name The string representing the name of the dictionary to uninstall.
//! @param callback_function The function to use to post status messages to.
//! @param data gpointer to the data to pass to the callback_function
//! @param long_messages Whether the messages should be long form or short form.
//!
void gw_io_install_dictinfo (GwDictInfo *di,    int (*callback_function) (char*, int, gpointer),
                               gpointer data, gboolean long_messages, GError **error            )
{
    if (*error != NULL) return;

    GQuark quark;
    quark = g_quark_from_string (GW_GENERIC_ERROR);

    if (di == NULL || di->status != GW_DICT_STATUS_NOT_INSTALLED) return;
    di->status = GW_DICT_STATUS_INSTALLING;

    char *message = NULL;

    char fallback_uri[100];
    gw_util_strncpy_fallback_from_key (fallback_uri, di->gckey, 100);

    char uri[100];
    gw_pref_get_string (uri, di->gckey, fallback_uri, 100);

    if (long_messages == TRUE)
    {
      // TRANSLATORS: The %s stands for an URL
      message = g_strdup_printf(gettext("Downloading %s..."), uri);
      if (message != NULL)
      {
        callback_function (message, -1, NULL);
        g_free (message);
        message = NULL;
      }
    }

    //Make sure the download folder exits
    char *download_path = g_build_filename (gw_util_get_waei_directory(), "download", NULL);
    if (download_path != NULL)
    {
      if (*error == NULL)
      {
        if ((g_mkdir_with_parents(download_path, 0755)) != 0 && di->status != GW_DICT_STATUS_CANCELING)
        {
          quark = g_quark_from_string (GW_GENERIC_ERROR);
          *error = g_error_new_literal (quark, GW_FILE_ERROR, gettext("Unable to create dictionary folder"));
        }
      }
      g_free (download_path);
      download_path = NULL;
    }

    //Copy the file if it is a local file
    if (*error == NULL && di->status != GW_DICT_STATUS_CANCELING)
    {
      if (g_file_test (uri, G_FILE_TEST_IS_REGULAR)) //Copy from local drive
      {
        gw_io_copy_file (uri, di->gz_path, error);
      }
      else //Download file over network
      {
        gw_io_download_file (uri, di->gz_path, callback_function, data, error);
      }
    }

    //Decompress the file if necessary
    if (*error == NULL && di->status != GW_DICT_STATUS_CANCELING && strstr(di->gz_path, ".gz"))
    {
      if (callback_function != NULL) {
        callback_function (gettext("Decompressing..."), -1, data);
      }
      gw_io_gunzip_file (di->gz_path, error);
    }
    else if (*error == NULL && di->status != GW_DICT_STATUS_CANCELING && strstr(di->gz_path, ".zip"))
    {
      gw_io_unzip_file(di->gz_path, error);
      if (*error == NULL && di->status != GW_DICT_STATUS_CANCELING)
      {
        quark = g_quark_from_string (GW_GENERIC_ERROR);
        *error = g_error_new_literal (quark, GW_FILE_ERROR, gettext("Unzip Error"));
      }
    }

    //Convert encoding if necessary
    if (*error == NULL && di->status != GW_DICT_STATUS_CANCELING && strstr(di->sync_path, "UTF") == NULL)
    {
      if (callback_function != NULL) {
        callback_function (gettext("Converting encoding..."), -1, data);
      }
      printf("converting encoding...\n");
      gw_io_copy_with_encoding (di->sync_path, di->path, "EUC-JP","UTF-8", error);
    }
    else if (*error == NULL && di->status != GW_DICT_STATUS_CANCELING)
    {
      gw_io_copy_file (di->sync_path, di->path, error);
    }

    //Special dictionary post processing
    if (*error == NULL && di->status != GW_DICT_STATUS_CANCELING)
    {
      if (callback_function != NULL) {
        callback_function (gettext("Postprocessing..."), -1, data);
      }
      gw_dictlist_preform_postprocessing_by_name (di->name, error);
    }
}

