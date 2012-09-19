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
//! @file io.c
//!


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>

#include <glib.h>
#include <glib/gstdio.h>
#include <curl/curl.h>

#include <libwaei/libwaei.h>


static gchar *_savepath = NULL;
static gboolean _cancel = FALSE;

struct _LwIoProcessFdData {
  const char* uri;         //!< The file path being passed
  int fd;                  //!< The file descriptor to be used with the path
  LwIoProgressCallback cb; //!< Callback to update progress
  gpointer data;           //!< Data to be passed to the LwIoProgressCallback
  GError *error;           //!< A GError
};
typedef struct _LwIoProcessFdData LwIoProcessFdData; //!< Used for passing data to LwIo functions



//!
//! @brief Creates a savepath that is used with the save/save as functions
//! @param PATH a path to save to
//!
void 
lw_io_set_savepath (const gchar *PATH)
{
    if (_savepath != NULL)
    {
      g_free (_savepath);
      _savepath = NULL;
    }

    if (PATH != NULL)
      _savepath = g_strdup (PATH);
}


//!
//! @brief Gets the savepath used with the save/save as functions
//! @returns A constant path string that is not to be freed
//!
const gchar* 
lw_io_get_savepath ()
{
  return _savepath;
}


//!
//! @brief Writes a file using the given text and write mode
//! @param PATH The Path to write the file to
//! @param mode A constant char representing the write mode to be used (w,a)
//! @param text A char pointer to some text to write to the file.
//! @param cb A LwIoProgressCallback function to give progress feedback or NULL
//! @param data A generic pointer to data to pass to the callback
//! @param error A pointer to a GError object to write errors to or NULL
//!
void 
lw_io_write_file (const char* PATH, const char* mode, gchar *text, LwIoProgressCallback cb, gpointer data, GError **error)
{
    //Sanity checks
    g_assert (PATH != NULL && mode != NULL && text != NULL);
    if (*error != NULL) return;

    //Declarations
    gchar *ptr;
    FILE* file;

    //Initializations
    ptr = &text[0];
    file = fopen(_savepath, mode);

    while (*ptr != '\0' && feof(file) == 0 && ferror(file) == 0)
    {
      fputc(*ptr, file);
      ptr++;
    }

    if (feof(file) == 0 && ferror(file) == 0)
    {
      fputc('\n', file);
    }

    //Cleanup
    fclose(file);
    file = NULL;
    ptr = NULL;
}


//!
//! @brief Copies a file and creates a new one using the new encoding
//! @param SOURCE_PATH The source file to change the encoding on.
//! @param TARGET_PATH The place to save the new file with the new encoding.
//! @param SOURCE_ENCODING The encoding of the source file.
//! @param TARGET_ENCODING THe wanted encoding in the new file to be created.
//! @param cb A LwIoProgressCallback to use to give progress feedback or NULL
//! @param data A gpointer to data to pass to the LwIoProgressCallback
//! @param error pointer to a GError to write errors to
//! @return The status of the conversion opertaion
//!
gboolean 
lw_io_copy_with_encoding (const char *SOURCE_PATH, const char *TARGET_PATH,
                                   const char *SOURCE_ENCODING, const char *TARGET_ENCODING,
                                   LwIoProgressCallback cb, gpointer data, GError **error   )
{
    if (*error != NULL) return FALSE;

    //Declarations
    FILE* readfd = NULL;
    FILE* writefd = NULL;
    const int MAX = LW_IO_MAX_FGETS_LINE;
    char buffer[MAX];
    char output[MAX];
    gsize inbytes_left, outbytes_left;
    char *inptr, *outptr;
    char prev_inbytes;
    size_t curpos;
    size_t end;
    GIConv conv;
    double fraction;

    //Initializations
    readfd = fopen (SOURCE_PATH, "r");
    writefd = fopen (TARGET_PATH, "w");
    conv = g_iconv_open (TARGET_ENCODING, SOURCE_ENCODING);
    prev_inbytes = 0;
    end = lw_io_get_filesize (SOURCE_PATH);
    curpos = 0;

    while (ferror(readfd) == 0 && feof(readfd) == 0 && ferror(writefd) == 0 && feof(writefd) == 0)
    {
      fgets(buffer, MAX, readfd);
      fraction = ((double) curpos / (double) end);
      if (cb != NULL) cb (fraction, data);

      curpos += strlen(buffer);
      inptr = buffer; outptr = output;
      inbytes_left = MAX; outbytes_left = MAX;
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
      fwrite(output, 1, strlen(output), writefd); 
    }
    fraction = ((double) curpos / (double) end);
    if (cb != NULL) cb (fraction, data);

    //Cleanup
    g_iconv_close (conv);
    fclose(readfd);
    fclose(writefd);

    return TRUE;
}


//!
//! @brief Private function made to be used with lw_io_download
//! @param ptr TBA
//! @param size TBA
//! @param nmemb TBA
//! @param stream TBA
//!
static size_t _libcurl_write_func (void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    return fwrite(ptr, size, nmemb, stream);
}
 

//!
//! @brief Private function made to be used with lw_io_download
//! @param ptr TBA
//! @param size TBA
//! @param nmemb TBA
//! @param stream TBA
//!
static size_t _libcurl_read_func (void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    return fread (ptr, size, nmemb, stream);
}

//!
//! @brief Private struct made to be used with lw_io_download
//! @param ptr TBA
//! @param size TBA
//! @param nmemb TBA
//! @param stream TBA
//!
static int _libcurl_update_progress (void  *custom,
                                     double dltotal,
                                     double dlnow,
                                     double ultotal,
                                     double ulnow   )
{
    //Declarations
    LwIoProgressCallbackWithData *cbwdata;
    LwIoProgressCallback cb;
    gpointer data;
    double fraction;
    
    //Initializations
    cbwdata = (LwIoProgressCallbackWithData*) custom;
    cb = cbwdata->cb;
    data = cbwdata->data;
    fraction = 0;

    if (dltotal > 0.0)
      fraction = dlnow / dltotal;

    //Update the interface
    if (_cancel) return 1;
    else return cb (fraction, data);
}




//!
//! @brief Downloads a file using libcurl
//! @param source_path String of the source url
//! @param target_path String of the path to save the file locally
//! @param cb A LwIoProgressCallback to use to give progress feedback or NULL
//! @param data gpointer to data to pass to the function pointer
//! @param error Error handling
//!
gboolean 
lw_io_download (char *source_path, char *target_path, LwIoProgressCallback cb,
                gpointer data, GError **error)
{
    if (error != NULL && *error != NULL) return FALSE;

    //Declarations
    GQuark quark;
    CURL *curl;
    CURLcode res;
    FILE *outfile;
    char *message;
    LwIoProgressCallbackWithData cbwdata;

    //Initializations
    curl = curl_easy_init ();
    outfile = fopen(target_path, "wb");
    cbwdata.cb = cb;
    cbwdata.data = data;
    res = 0;

    if (curl != NULL || outfile != NULL)
    {
      curl_easy_setopt(curl, CURLOPT_URL, source_path);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, outfile);
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _libcurl_write_func);
      curl_easy_setopt(curl, CURLOPT_READFUNCTION, _libcurl_read_func);

      if (cb != NULL)
      {
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, _libcurl_update_progress);
        curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &cbwdata);
      }

      res = curl_easy_perform(curl);
    }
     
    fclose(outfile);
    curl_easy_cleanup(curl);

    if (res != 0 && _cancel == FALSE)
    {
      g_remove (target_path);

      if (error != NULL) {
        message = gettext(curl_easy_strerror(res));
        quark = g_quark_from_string (LW_IO_ERROR);
        *error = g_error_new_literal (quark, LW_IO_DOWNLOAD_ERROR, message);
      }
    }

    return (res == 0);
}


//!
//! @brief Copies a local file to another local location
//! @param SOURCE_PATH String of the source url
//! @param TARGET_PATH String of the path to save the file locally
//! @param cb A LwIoProgressCallback to use to give progress feedback or NULL
//! @param data A gpointer to data to pass to the LwIoProgressCallback
//! @param error Error handling
//!
gboolean 
lw_io_copy (const char *SOURCE_PATH, const char *TARGET_PATH, 
            LwIoProgressCallback cb, gpointer data, GError **error)
{
    if (*error != NULL) return FALSE;

    //Declarations
    FILE *in;
    FILE *out;
    size_t chunk;
    size_t end;
    size_t curpos;
    const int MAX_CHUNK = 128;
    char buffer[MAX_CHUNK];
    double fraction;

    //Initalizations
    in = fopen(SOURCE_PATH, "rb");
    out = fopen(TARGET_PATH, "wb");
    chunk = 1;
    end = lw_io_get_filesize (SOURCE_PATH);
    curpos = 0;
    fraction = 0.0;

    while (chunk && !_cancel)
    {
      fraction = ((double) curpos) / ((double) end);
      if (cb != NULL) cb (fraction, data);
      chunk = fread(buffer, sizeof(char), MAX_CHUNK, in);
      chunk = fwrite(buffer, sizeof(char), chunk, out);
      curpos += chunk;
    }
    fraction = 1.0;
    if (cb != NULL) cb (fraction, data);

    //Cleanup
    fclose(in);
    fclose(out);

    return (error == NULL && *error == NULL);;
}


//!
//! @brief Creates a single dictionary containing both the radical dict and kanji dict
//! @param output_path Mix dictionary path to write to
//! @param kanji_dictionary_path Kanjidic dictionary path
//! @param radicals_dictionary_path raddic dictionary path
//! @param cb A LwIoProgressCallback to use to give progress feedback or NULL
//! @param data A gpointer to data to pass to the LwIoProgressCallback
//! @param error pointer to a GError to write errors to
//!
gboolean 
lw_io_create_mix_dictionary (const char *output_path, 
                             const char *kanji_dictionary_path, 
                             const char *radicals_dictionary_path, 
                             LwIoProgressCallback cb,
                             gpointer data,
                             GError **error)
{
    //Sanity check
    if (*error != NULL) return FALSE;

    //Declarations
    FILE *output_file, *kanji_file, *radicals_file;
    char radicals_input[LW_IO_MAX_FGETS_LINE];
    char kanji_input[LW_IO_MAX_FGETS_LINE];
    char output[LW_IO_MAX_FGETS_LINE * 2];
    char *radicals_ptr, *kanji_ptr, *output_ptr, *temp_ptr;

    size_t curpos;
    size_t end;
    double fraction;

    //Initializations
    kanji_file =  fopen(kanji_dictionary_path, "r");
    radicals_file = fopen(radicals_dictionary_path, "r");
    output_file = fopen(output_path, "w");
    radicals_ptr = NULL;
    kanji_ptr = NULL;
    output_ptr = NULL;

    curpos = 0;
    end = lw_io_get_filesize (kanji_dictionary_path);
    fraction = 0.0;

    //Loop through the kanji file
    while (fgets(kanji_input, LW_IO_MAX_FGETS_LINE, kanji_file) != NULL && !_cancel)
    {
      fraction = ((double) curpos)/((double) end);
      if (cb != NULL) cb (fraction, data);

      curpos += strlen (kanji_input);

      if (kanji_input[0] == '#') continue;

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
      rewind (radicals_file);
      while (fgets(radicals_input, LW_IO_MAX_FGETS_LINE, radicals_file) != NULL)
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
    fraction = 1.0;
    if (cb != NULL) cb (fraction, data);

    //Cleanup
    fclose(kanji_file);
    fclose(output_file);
    fclose(radicals_file);

    return TRUE;
}


//!
//! @brief Splits the Names 
//! @param OUTPUT_NAMES_PATH The path to write the new Names dictionary to
//! @param OUTPUT_PLACES_PATH The path to write the new Places dictionary to
//! @param INPUT_NAMES_PLACES_PATH The file to use to generate the split dictionaries
//! @param cb A LwIoProgressCallback to use to give progress feedback or NULL
//! @param data A gpointer to data to pass to the LwIoProgressCallback
//! @param error pointer to a GError to write errors to
//!
gboolean 
lw_io_split_places_from_names_dictionary (const char *OUTPUT_NAMES_PATH, 
                                          const char* OUTPUT_PLACES_PATH,
                                          const char* INPUT_NAMES_PLACES_PATH,
                                          LwIoProgressCallback cb,
                                          gpointer data,
                                          GError **error                    )
{
    if (error != NULL && *error != NULL) return FALSE;

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

    //Declarations
    char buffer[LW_IO_MAX_FGETS_LINE];
    FILE *inputf;
    size_t curpos;
    size_t end;
    double fraction;

    FILE *placesf;
    GRegex *re_place;
    const char *place_pattern = "([\\(,])((p)|(st))([\\),])";
    int  place_write_error;

    FILE *namesf;
    GRegex *re_name;
    const char *name_pattern = "([\\(,])((s)|(u)|(g)|(f)|(m)|(h)|(pr)|(co))([\\),])";
    int  name_write_error;

    //Initializations
    inputf = fopen(INPUT_NAMES_PLACES_PATH, "r");
    curpos = 0;
    end = lw_io_get_filesize (INPUT_NAMES_PLACES_PATH);
    fraction = 0.0;

    re_place = g_regex_new (place_pattern,  LW_RE_COMPILE_FLAGS, LW_RE_LOCATE_FLAGS, error);
    placesf = fopen(OUTPUT_PLACES_PATH, "w");
    place_write_error = 0;

    re_name = g_regex_new (name_pattern,  LW_RE_COMPILE_FLAGS, LW_RE_LOCATE_FLAGS, error);
    namesf = fopen(OUTPUT_NAMES_PATH, "w");
    name_write_error  = 0;


    //Start writing the child files
    while (fgets(buffer, LW_IO_MAX_FGETS_LINE, inputf) != NULL &&
           place_write_error != EOF &&
           name_write_error  != EOF &&
           *error == NULL &&
           !_cancel)
    {
      fraction = ((double) curpos) / ((double) end);
      if (cb != NULL) cb (fraction, data);

      if (placesf != NULL && g_regex_match (re_place, buffer, 0, NULL))
        place_write_error = fputs(buffer, placesf);
      if (namesf != NULL && g_regex_match(re_name, buffer, 0, NULL))
        name_write_error =  fputs(buffer, namesf);
      curpos += strlen(buffer);
    }
    fraction = 1.0;
    if (cb != NULL) cb (fraction, data);

    //Cleanup
    fclose(inputf);
    fclose(placesf);
    fclose(namesf);
    g_regex_unref (re_place);
    g_regex_unref (re_name);

    return (place_write_error != EOF && name_write_error != EOF);
}


//!
//! @brief Decompresses a gzip file
//! @param SOURCE_PATH The path to the file that is gzipped
//! @param TARGET_PATH The path to write the uncompressed file to
//! @param cb A LwIoProgressCallback function to give progress feedback or NULL
//! @param data A generic pointer to data to pass to the LwIoProgressCallback
//! @param error A pointer to a GError object to write an error to or NULL
//!
gboolean 
lw_io_gunzip_file (const char *SOURCE_PATH, const char *TARGET_PATH,
                   LwIoProgressCallback cb, gpointer data, GError **error)
{
    if (error != NULL && *error != NULL) return FALSE;

    //Declarations
    char *argv[] = { GZIP, "-cd", NULL };

    lw_io_pipe_data (argv, SOURCE_PATH, TARGET_PATH, cb, data, error);

    return (error == NULL || *error == NULL);
} 


//!
//! @brief Decompresses a zip file
//! @param SOURCE_PATH The path to the file that is gzipped
//! @param cb A LwIoProgressCallback function to give progress feedback or NULL
//! @param data A generic pointer to data to pass to the LwIoProgressCallback
//! @param error A pointer to a GError object to write an error to or NULL
//!
gboolean 
lw_io_unzip_file (char *SOURCE_PATH, LwIoProgressCallback cb, gpointer data, GError **error)
{
    return TRUE;
}


//!
//! @brief Gets a list of the currently installed dictionaries as an array of strings.
//! The format will be DICTTYPE/FILENAME and the array is null terminated.  Both the array
//! and string themselves must be freed after.
//!
//! @returns An array of strings that must be freed.  We recommend g_strfreev() from glib
//!
char** 
lw_io_get_dictionary_file_list (const int MAX)
{
    //Declarations and initializations
    LwDictType type;
    GDir *dir;
    const char* typename;
    const char* filename;
    char *directory;
    char** atoms = (char**) malloc((MAX + 1) * sizeof(int));
    int i = 0;

    //Go through each engine folder looking for dictionaries
    for (type = 0; type < TOTAL_LW_DICTTYPES && i < MAX; type++)
    {
      typename = lw_util_dicttype_to_string (type);
      directory = lw_util_build_filename_by_dicttype (type, NULL);
      if (directory != NULL)
      {
        dir = g_dir_open (directory, 0, NULL);

        //Look for files in the directory and stop if we reached the max for the program
        while (dir != NULL && (filename =  g_dir_read_name (dir)) != NULL && i < MAX)
        {
          atoms[i] = g_strdup_printf("%s/%s", typename, filename);
          i++;
        }
        g_dir_close(dir);
        g_free (directory);
      }
    }
    atoms[i] = NULL;

    return atoms;
}


//!
//! @brief Gets the size of a file in bytes
//! @param URI The path to the file to calculate the size of
//! @returns The size of the file in bytes
//!
size_t 
lw_io_get_filesize (const char *URI)
{
    //Sanity check
    g_assert (g_file_test (URI, G_FILE_TEST_IS_REGULAR));

    //Declarations
    const int MAX_CHUNK = 128;
    char buffer[MAX_CHUNK];
    FILE *file;
    size_t size;

    //Initializations
    file = fopen(URI, "rb");
    size = 0;

    while (file != NULL && ferror(file) == 0 && feof(file) == 0)
        size += fread(buffer, sizeof(char), MAX_CHUNK, file);

    //Cleanup
    fclose(file);

    return size;
}


//!
//! @brief Callback function to read data in from a file and send it to a stream
//! @param data A pointer to a LwIoProcessFdData object
//! @returns Returns if there was an error
//!
gpointer _stdin_func (gpointer data)
{
    //Declarations
    const int MAX_CHUNK = 128;
    char buffer[MAX_CHUNK];
    size_t chunk;
    size_t curpos;
    size_t end;
    double fraction;
    FILE *file;
    FILE *stream;
    LwIoProcessFdData* in;
    const char *message;
    GQuark domain;

    //Initalizations
    in = data;
    chunk = 0;
    curpos = 0;
    end = lw_io_get_filesize (in->uri);
    fraction = 0.0;
    file = fopen(in->uri, "rb");
    stream = fdopen(in->fd, "ab");

    while (file != NULL && ferror(file) == 0 && feof(file) == 0 && !_cancel)
    {
        fraction = ((double) curpos / (double) end);
        if (in->cb != NULL) in->cb (fraction, in->data);

        chunk = fread(buffer, sizeof(char), MAX_CHUNK, file);
        curpos += chunk;
        chunk = fwrite(buffer, sizeof(char), chunk, stream);
        fflush(stream);
    }
    fraction = 1.0;
    if (in->cb != NULL) in->cb (fraction, in->data);

    if (ferror(file) != 0)
    {
      domain = g_quark_from_string (LW_IO_ERROR);
      message = gettext("Unable to read data from the input file.");
      in->error = g_error_new (domain, LW_IO_READ_ERROR, message);
    }
    else if(ferror(stream) != 0)
    {
      domain = g_quark_from_string (LW_IO_ERROR);
      message = gettext("Unable to write to the external program's input stream.");
      in->error = g_error_new (domain, LW_IO_WRITE_ERROR, message);
    }

    //Cleanup
    fclose(file);
    fclose(stream);

    return (in->error);
}


//!
//! @brief Callback function to read data from a stream and write it to a file
//! @param data A pointer to a LwIoProcessFdData object
//! @returns Returns if there was an error
//!
gpointer _stdout_func (gpointer data)
{
    //Declarations
    const int MAX_CHUNK = 128;
    char buffer[MAX_CHUNK];
    size_t chunk;
    size_t curpos;
    FILE *file;
    FILE *stream;
    LwIoProcessFdData* out;
    const char *message;
    GQuark domain;

    //Initalizations
    out = data;
    chunk = 1;
    curpos = 0;
    file = fopen(out->uri, "wb");
    stream = fdopen(out->fd, "rb");

    while (file != NULL && ferror(file) == 0 && feof(file) == 0 && chunk != 0)
    {
        chunk = fread(buffer, sizeof(char), MAX_CHUNK, stream);
        curpos += chunk;
        chunk = fwrite(buffer, sizeof(char), chunk, file);
    }

    if (ferror(stream) != 0)
    {
      domain = g_quark_from_string (LW_IO_ERROR);
      message = gettext("Unable to read data from the external program's pipe.");
      out->error = g_error_new (domain, LW_IO_READ_ERROR, message);
    }
    else if(ferror(stream) != 0)
    {
      domain = g_quark_from_string (LW_IO_ERROR);
      message = gettext("Unable to write the stream's output to a file.");
      out->error = g_error_new (domain, LW_IO_WRITE_ERROR, message);
    }

    //Cleanup
    fclose(stream);
    fclose(file);

    return (out->error);
}



//!
//! @brief Pipes a source file through a program and sends the output to a target file.
//! @param argv An array of strings representing a program path and flags
//! @param SOURCE_PATH The path to the file file to be streamed to the program
//! @param TARGET_PATH The path to write the uncompressed file to
//! @param cb A LwIoProgressCallback function to give progress feedback or NULL
//! @param data A generic pointer to data to pass to the LwIoProgressCallback
//! @param error A pointer to a GError object to write an error to or NULL
//!
gboolean 
lw_io_pipe_data (char                 **argv, 
                 const char            *SOURCE_PATH, 
                 const char            *TARGET_PATH, 
                 LwIoProgressCallback   cb, 
                 gpointer               data, 
                 GError               **error          )
{
    //Sanity check
    if (error != NULL && *error != NULL) return FALSE;

    //Declarations
    gint stdin_fd;
    gint stdout_fd;
    GPid pid;
    GThread *stdin_thread;
    GThread *stdout_thread;
    LwIoProcessFdData stdin_data;
    LwIoProcessFdData stdout_data;

    //Initalizations
    g_spawn_async_with_pipes (
          NULL,
          argv, 
          NULL, 
          G_SPAWN_STDERR_TO_DEV_NULL, 
          NULL, 
          NULL, 
          &pid, 
          &stdin_fd, 
          &stdout_fd, 
          NULL, 
          error
    );

    stdin_data.uri = SOURCE_PATH;
    stdin_data.fd = stdin_fd;
    stdin_data.cb = cb;
    stdin_data.data = data;
    stdin_data.error = NULL;

    stdout_data.uri = TARGET_PATH;
    stdout_data.fd = stdout_fd;
    stdout_data.cb = cb;
    stdout_data.data = data;
    stdout_data.error = NULL;

    //Process the data through the external program
    stdin_thread = g_thread_create (_stdin_func, &stdin_data, TRUE, error);
    stdout_thread = g_thread_create (_stdout_func, &stdout_data, TRUE, error);

    //Wait for it to finish
    g_thread_join (stdin_thread);
    g_thread_join (stdout_thread);

    //Set the first error from the streams if there or no ther ones already there
    if (stdin_data.error != NULL)
    {
      if (error != NULL && *error == NULL)
        *error = stdin_data.error;
      else
        g_error_free (stdin_data.error);
    }
    if (stdout_data.error != NULL)
    {
      if (error != NULL && *error == NULL)
        *error = stdout_data.error;
      else
        g_error_free (stdout_data.error);
    }

    //Cleanup
    g_spawn_close_pid (pid);

    return (*error == NULL);
} 


//!
//! @brief Deletes a file from the filesystem
//! @param URI The path to the file to delet
//! @param error A pointer to a GError object to write errors to or NULL
//!
gboolean 
lw_io_remove (const char *URI, GError **error)
{
  if (error != NULL && *error != NULL) return FALSE;

  g_remove (URI);

  return (error == NULL && *error == NULL);
}


//!
//! @brief Sets the global IO state to cancel all operations
//! @param state The cancel request state wanted
//!
void 
lw_io_set_cancel_operations (gboolean state)
{
    _cancel = state;
}


//!
//! @brief A quick way to get the number of lines in a file for use in progress functions
//! @param FILENAME The path to the file to see how many lines it has
//!
long 
lw_io_get_size_for_uri (const char *URI)
{
    //Declarations
    FILE *file;
    long length;

    //Initializations
    file = fopen (URI, "r");
    length = 0L;

    if (file != NULL)
    {
      fseek (file, 0L, SEEK_END);
      length = ftell (file);
      fclose(file);
    }
   
    return length;
}

