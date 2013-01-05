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
//!  @file vocabularylist.c
//!

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <libwaei/gettext.h>
#include <libwaei/libwaei.h>


gchar**
lw_vocabulary_get_lists ()
{
    //Declarations
    GDir *dir;
    gchar **atoms;
    gchar *buffer;
    const gchar *name;
    guint chars;
    gchar *uri;

    //Initializations
    chars = 0;
    atoms = NULL;

    if ((uri = lw_util_build_filename (LW_PATH_VOCABULARY, NULL)) != NULL)
    {
      if ((dir = g_dir_open (uri, 0, NULL)) != NULL)
      {
        //Get the size needed for the buffer
        while ((name = g_dir_read_name (dir)) != NULL)
        {
          chars += strlen(name) + 1;
        }

        if (chars > 1 && (buffer = g_new0 (gchar, chars + 1)) != NULL)
        {
          g_dir_rewind (dir);

          //Set the buffer
          while ((name = g_dir_read_name (dir)) != NULL)
          {
            strcat(buffer, name);
            strcat(buffer, ";");
          }
          buffer[chars - 1] = '\0';

          //Split it
          atoms = g_strsplit (buffer, ";", -1);

          g_free (buffer); buffer = NULL;
        }
        g_dir_close (dir); dir = NULL;
      }
      g_free (uri); uri = NULL;
    }

    return atoms;
}

LwVocabulary*
lw_vocabulary_new (const gchar *NAME)
{
    g_assert (NAME != NULL && strlen (NAME) > 0);

    LwVocabulary *vocabulary;

    vocabulary = g_new0 (LwVocabulary, 1);
    if (vocabulary != NULL)
    {
      vocabulary->name = g_strdup (NAME);
    }
    return vocabulary;
}


void
lw_vocabulary_free (LwVocabulary *vocabulary)
{
    if (vocabulary->name != NULL) g_free (vocabulary->name);
    if (vocabulary->items != NULL)
    {
      g_list_foreach (vocabulary->items, (GFunc) lw_word_free, NULL);
      g_list_free (vocabulary->items); vocabulary->items = NULL;
    }
    g_free (vocabulary);
}


void
lw_vocabulary_load (LwVocabulary *vocabulary, const gchar *FILENAME, LwIoProgressCallback cb)
{
    LwWord *word;
    gchar *uri;
    FILE *stream;
    const gint MAX = 512;
    gchar buffer[MAX + 1];

    if (FILENAME != NULL)
      uri = lw_util_build_filename (LW_PATH_VOCABULARY, FILENAME);
    else
      uri = lw_util_build_filename (LW_PATH_VOCABULARY, vocabulary->name);

    if (uri != NULL)
    {
      stream = fopen (uri, "r");
      if (stream != NULL)
      {
        while (feof(stream) == 0)
        {
          if (fgets (buffer, MAX, stream) != NULL)
          {
            buffer[MAX] = '\0';
            word = lw_word_new_from_string (buffer);
            if (word != NULL)
            {
              vocabulary->items = g_list_append (vocabulary->items, word);
            }
          }
          if (strchr(buffer, '\n') == NULL && feof(stream) == 0)
          {
            while (fgetc(stream) != '\n' && feof(stream) == 0);
          }
        }
        fclose(stream); stream = NULL;
      }
      g_free (uri); uri = NULL;
    }
}

void
lw_vocabulary_save (LwVocabulary *vocabulary, const gchar *FILENAME, LwIoProgressCallback cb)
{
    //Declarations
    LwWord *word;
    GList *iter;
    gint i;
    gchar *uri;
    FILE *stream;

    if (FILENAME != NULL)
      uri = g_strdup (FILENAME);
    else
      uri = lw_util_build_filename (LW_PATH_VOCABULARY, vocabulary->name);

    if (uri != NULL)
    {
      stream = fopen (uri, "w");
      if (stream != NULL)
      {
        for (iter = vocabulary->items; iter != NULL; iter = iter->next)
        {
          word = LW_WORD (iter->data);
          if (word != NULL)
          {
            for (i = 0; i < TOTAL_LW_WORD_FIELDS - 1 && feof (stream) == 0; i++)
            {
              if (word->fields[i] != NULL)
              {
                fputs(word->fields[i], stream);
                fputc(';', stream);
              }
            }
            if (word->fields[i] != NULL) fputs(word->fields[i], stream);
            fputc('\n', stream);
          }
        }
        fclose(stream);
      }
      g_free (uri);
    }
}

void
lw_vocabulary_set_name (LwVocabulary *vocabulary, const gchar *name)
{
    if (vocabulary->name != NULL)
      g_free (vocabulary->name);
    vocabulary->name = g_strdup (name);
}

const gchar*
lw_vocabulary_get_name (LwVocabulary *vocabulary)
{
    return vocabulary->name;
}

GList *
lw_vocabulary_get_items (LwVocabulary *vocabulary)
{
    return vocabulary->items;
}

void
lw_vocabulary_set_changed (LwVocabulary *vocabulary, gboolean changed)
{
    vocabulary->changed = changed;
}

gboolean
lw_vocabulary_changed (LwVocabulary *vocabulary)
{
  return vocabulary->changed;
}
