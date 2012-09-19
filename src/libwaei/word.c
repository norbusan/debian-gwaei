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
//!  @file word.c
//!

#include <locale.h>

#include <libwaei/gettext.h>
#include <libwaei/libwaei.h>

const gchar* lw_word_get_kanji (LwWord *word)
{
  return word->fields[LW_WORD_FIELD_KANJI];
}

void lw_word_set_kanji (LwWord *word, const gchar *text)
{
  if (word->fields[LW_WORD_FIELD_KANJI] != NULL)
    g_free (word->fields[LW_WORD_FIELD_KANJI]);
  word->fields[LW_WORD_FIELD_KANJI] = g_strdup (text);
}

const gchar* lw_word_get_furigana (LwWord *word)
{
  return word->fields[LW_WORD_FIELD_FURIGANA];
}

void lw_word_set_furigana (LwWord *word, const gchar *text)
{
  if (word->fields[LW_WORD_FIELD_FURIGANA] != NULL)
    g_free (word->fields[LW_WORD_FIELD_FURIGANA]);
  word->fields[LW_WORD_FIELD_FURIGANA] = g_strdup (text);
}

const gchar* lw_word_get_definitions (LwWord *word)
{
  return word->fields[LW_WORD_FIELD_DEFINITIONS];
}

void lw_word_set_definitions (LwWord *word, const gchar *text)
{
  if (word->fields[LW_WORD_FIELD_DEFINITIONS] != NULL)
    g_free (word->fields[LW_WORD_FIELD_DEFINITIONS]);
  word->fields[LW_WORD_FIELD_DEFINITIONS] = g_strdup (text);
}

gint lw_word_get_correct_guesses (LwWord *word)
{
  return word->correct_guesses;
}

void lw_word_set_correct_guesses (LwWord *word, gint number)
{
  if (word->fields[LW_WORD_FIELD_CORRECT_GUESSES] != NULL)
    g_free (word->fields[LW_WORD_FIELD_CORRECT_GUESSES]);
  word->fields[LW_WORD_FIELD_CORRECT_GUESSES] = g_strdup_printf ("%d", number);
  word->correct_guesses = number;
  if (word->score != NULL) g_free (word->score); word->score = NULL;
}

gint lw_word_get_incorrect_guesses (LwWord *word)
{
  return word->incorrect_guesses;
}

void lw_word_set_incorrect_guesses (LwWord *word, gint number)
{
  if (word->fields[LW_WORD_FIELD_INCORRECT_GUESSES] != NULL)
    g_free (word->fields[LW_WORD_FIELD_INCORRECT_GUESSES]);
  word->fields[LW_WORD_FIELD_INCORRECT_GUESSES] = g_strdup_printf ("%d", number);
  word->incorrect_guesses = number;
  if (word->score != NULL) g_free (word->score); word->score = NULL;
}


gint 
lw_word_get_score (LwWord *word)
{
    gint total = word->correct_guesses + word->incorrect_guesses;
    if (total == 0) return 0.0;
    else return (word->correct_guesses * 100 / total);
}


const gchar* 
lw_word_get_score_as_string (LwWord *word)
{
    gint total;
    
    if (word->score == NULL)
    {
      total = word->correct_guesses + word->incorrect_guesses;
      if (total == 0)
        word->score = g_strdup (gettext("Untested"));
      else
        word->score = g_strdup_printf ("%3d%%", lw_word_get_score (word));
    }

    return word->score;
}

guint32
lw_word_timestamp_to_hours (gint64 timestamp)
{
    const gint MICROSECONDS = 1000000;
    const gint SECONDS = 60;
    const gint MINUTES = 60;
    return (guint32) (timestamp / MICROSECONDS / SECONDS / MINUTES);
}


void
lw_word_set_timestamp (LwWord *word, gint64 timestamp)
{
    guint32 hours = lw_word_timestamp_to_hours (timestamp);
    lw_word_set_hours (word, hours);
}


void
lw_word_update_timestamp (LwWord *word)
{
    lw_word_set_timestamp (word, g_get_real_time ());
}


void
lw_word_set_hours (LwWord *word, guint32 hours)
{
    word->timestamp = hours;

    if (word->days != NULL) g_free (word->days); word->days = NULL;
    if (word->fields[LW_WORD_FIELD_TIMESTAMP] != NULL)
      g_free (word->fields[LW_WORD_FIELD_TIMESTAMP]);

    word->fields[LW_WORD_FIELD_TIMESTAMP] = g_strdup_printf ("%" G_GUINT32_FORMAT, word->timestamp);
}


guint32
lw_word_get_hours (LwWord *word)
{
    return word->timestamp;
}


const gchar*
lw_word_get_timestamp_as_string (LwWord *word)
{
    if (word->days == NULL)
    {
      guint32 days = lw_word_get_hours (word) / 24;
      guint32 today = lw_word_timestamp_to_hours ( g_get_real_time ()) / 24;
      guint32 difference = today - days;
      if (difference < 0) difference = 0;

      if (days == 0) word->days = g_strdup (pgettext("noun", "Never"));
      else if (difference == 0) word->days = g_strdup (gettext("Today"));
      else if (difference == 1) word->days = g_strdup (gettext("Yesterday"));
      else word->days = g_strdup_printf (ngettext("%d Day Ago", "%d Days Ago", difference), difference);
    }

    return word->days;
}


LwWord*
lw_word_new ()
{
    LwWord *word;

    word = g_new0 (LwWord, 1);

    return word;
}


LwWord*
lw_word_new_from_string (const gchar *text)
{
    //Declarations
    LwWord *word;
    gchar *ptr;
    gchar *endptr;
    gchar **atoms;
    gint i;

    word = g_new0 (LwWord, 1);
    if (word != NULL)
    {

      atoms = g_strsplit (text, ";", TOTAL_LW_WORD_FIELDS);
      if (atoms != NULL)
      {
        //Set up the strings
        for (i = 0; atoms[i] != NULL && i < TOTAL_LW_WORD_FIELDS; i++)
        {
          word->fields[i] = g_strdup (g_strstrip(atoms[i]));
        }
        for (i = 0; i < TOTAL_LW_WORD_FIELDS; i++)
        {
          if (word->fields[i] == NULL) word->fields[i] = g_strdup ("");
        }

        //Set up the integers
        ptr = word->fields[LW_WORD_FIELD_CORRECT_GUESSES];
        word->correct_guesses = (gint) g_ascii_strtoll (ptr, &endptr, 10);
        ptr = word->fields[LW_WORD_FIELD_INCORRECT_GUESSES];
        word->incorrect_guesses =  (gint) g_ascii_strtoll (ptr, &endptr, 10);
        ptr = word->fields[LW_WORD_FIELD_TIMESTAMP];
        word->timestamp =  (guint32) g_ascii_strtoll (ptr, &endptr, 10);
      }
      g_strfreev (atoms); atoms = NULL;
    }

    return word;
}


void
lw_word_free (LwWord *word)
{
  gint i;
  for (i = 0; i < TOTAL_LW_WORD_FIELDS; i++)
  {
    if (word->fields[i] != NULL)
    {
      g_free (word->fields[i]);
      word->fields[i] = NULL;
    }
  }

  g_free (word->score); word->score = NULL;
  g_free (word->days); word->days = NULL;

  g_free (word);
}


gchar* 
lw_word_to_string (LwWord *word)
{
    gchar* text;
    text = lw_strjoinv (';', word->fields, TOTAL_LW_WORD_FIELDS);
    return text;
}
