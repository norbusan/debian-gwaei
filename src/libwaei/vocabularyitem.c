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
//!  @file vocabularyitem.c
//!


#include "../private.h"

#include <locale.h>

#include <libwaei/libwaei.h>

const gchar* lw_vocabularyitem_get_kanji (LwVocabularyItem *item)
{
  return item->fields[LW_VOCABULARYITEM_FIELD_KANJI];
}

void lw_vocabularyitem_set_kanji (LwVocabularyItem *item, const gchar *text)
{
  if (item->fields[LW_VOCABULARYITEM_FIELD_KANJI] != NULL)
    g_free (item->fields[LW_VOCABULARYITEM_FIELD_KANJI]);
  item->fields[LW_VOCABULARYITEM_FIELD_KANJI] = g_strdup (text);
}

const gchar* lw_vocabularyitem_get_furigana (LwVocabularyItem *item)
{
  return item->fields[LW_VOCABULARYITEM_FIELD_FURIGANA];
}

void lw_vocabularyitem_set_furigana (LwVocabularyItem *item, const gchar *text)
{
  if (item->fields[LW_VOCABULARYITEM_FIELD_FURIGANA] != NULL)
    g_free (item->fields[LW_VOCABULARYITEM_FIELD_FURIGANA]);
  item->fields[LW_VOCABULARYITEM_FIELD_FURIGANA] = g_strdup (text);
}

const gchar* lw_vocabularyitem_get_definitions (LwVocabularyItem *item)
{
  return item->fields[LW_VOCABULARYITEM_FIELD_DEFINITIONS];
}

void lw_vocabularyitem_set_definitions (LwVocabularyItem *item, const gchar *text)
{
  if (item->fields[LW_VOCABULARYITEM_FIELD_DEFINITIONS] != NULL)
    g_free (item->fields[LW_VOCABULARYITEM_FIELD_DEFINITIONS]);
  item->fields[LW_VOCABULARYITEM_FIELD_DEFINITIONS] = g_strdup (text);
}

gint lw_vocabularyitem_get_correct_guesses (LwVocabularyItem *item)
{
  return item->correct_guesses;
}

void lw_vocabularyitem_set_correct_guesses (LwVocabularyItem *item, gint number)
{
  if (item->fields[LW_VOCABULARYITEM_FIELD_CORRECT_GUESSES] != NULL)
    g_free (item->fields[LW_VOCABULARYITEM_FIELD_CORRECT_GUESSES]);
  item->fields[LW_VOCABULARYITEM_FIELD_CORRECT_GUESSES] = g_strdup_printf ("%d", number);
  item->correct_guesses = number;
  if (item->score != NULL) g_free (item->score); item->score = NULL;
}

gint lw_vocabularyitem_get_incorrect_guesses (LwVocabularyItem *item)
{
  return item->incorrect_guesses;
}

void lw_vocabularyitem_set_incorrect_guesses (LwVocabularyItem *item, gint number)
{
  if (item->fields[LW_VOCABULARYITEM_FIELD_INCORRECT_GUESSES] != NULL)
    g_free (item->fields[LW_VOCABULARYITEM_FIELD_INCORRECT_GUESSES]);
  item->fields[LW_VOCABULARYITEM_FIELD_INCORRECT_GUESSES] = g_strdup_printf ("%d", number);
  item->incorrect_guesses = number;
  if (item->score != NULL) g_free (item->score); item->score = NULL;
}


gint 
lw_vocabularyitem_get_score (LwVocabularyItem *item)
{
    gint total = item->correct_guesses + item->incorrect_guesses;
    if (total == 0) return 0.0;
    else return (item->correct_guesses * 100 / total);
}


const gchar* 
lw_vocabularyitem_get_score_as_string (LwVocabularyItem *item)
{
    gint total;
    
    if (item->score == NULL)
    {
      total = item->correct_guesses + item->incorrect_guesses;
      if (total == 0)
        item->score = g_strdup (gettext("Untested"));
      else
        item->score = g_strdup_printf ("%3d%%", lw_vocabularyitem_get_score (item));
    }

    return item->score;
}

guint32
lw_vocabularyitem_timestamp_to_hours (gint64 timestamp)
{
    const gint MICROSECONDS = 1000000;
    const gint SECONDS = 60;
    const gint MINUTES = 60;
    return (guint32) (timestamp / MICROSECONDS / SECONDS / MINUTES);
}


void
lw_vocabularyitem_set_timestamp (LwVocabularyItem *item, gint64 timestamp)
{
    guint32 hours = lw_vocabularyitem_timestamp_to_hours (timestamp);
    lw_vocabularyitem_set_hours (item, hours);
}


void
lw_vocabularyitem_update_timestamp (LwVocabularyItem *item)
{
    lw_vocabularyitem_set_timestamp (item, g_get_real_time ());
}


void
lw_vocabularyitem_set_hours (LwVocabularyItem *item, guint32 hours)
{
    item->timestamp = hours;

    if (item->days != NULL) g_free (item->days); item->days = NULL;
    if (item->fields[LW_VOCABULARYITEM_FIELD_TIMESTAMP] != NULL)
      g_free (item->fields[LW_VOCABULARYITEM_FIELD_TIMESTAMP]);

    item->fields[LW_VOCABULARYITEM_FIELD_TIMESTAMP] = g_strdup_printf ("%" G_GUINT32_FORMAT, item->timestamp);
}


guint32
lw_vocabularyitem_get_hours (LwVocabularyItem *item)
{
    return item->timestamp;
}


const gchar*
lw_vocabularyitem_get_timestamp_as_string (LwVocabularyItem *item)
{
    if (item->days == NULL)
    {
      guint32 days = lw_vocabularyitem_get_hours (item) / 24;
      guint32 today = lw_vocabularyitem_timestamp_to_hours ( g_get_real_time ()) / 24;
      guint32 difference = today - days;
      if (difference < 0) difference = 0;

      if (days == 0) item->days = g_strdup (pgettext("noun", "Never"));
      else if (difference == 0) item->days = g_strdup (gettext("Today"));
      else if (difference == 1) item->days = g_strdup (gettext("Yesterday"));
      else item->days = g_strdup_printf (ngettext("%d Day Ago", "%d Days Ago", difference), difference);
    }

    return item->days;
}


LwVocabularyItem*
lw_vocabularyitem_new ()
{
    LwVocabularyItem *item;

    item = g_new0 (LwVocabularyItem, 1);

    return item;
}


LwVocabularyItem*
lw_vocabularyitem_new_from_string (const gchar *text)
{
    //Declarations
    LwVocabularyItem *item;
    gchar *ptr;
    gchar *endptr;
    gchar **atoms;
    gint i;

    item = g_new0 (LwVocabularyItem, 1);
    if (item != NULL)
    {

      atoms = g_strsplit (text, ";", TOTAL_LW_VOCABULARYITEM_FIELDS);
      if (atoms != NULL)
      {
        //Set up the strings
        for (i = 0; atoms[i] != NULL && i < TOTAL_LW_VOCABULARYITEM_FIELDS; i++)
        {
          item->fields[i] = g_strdup (g_strstrip(atoms[i]));
        }
        for (i = 0; i < TOTAL_LW_VOCABULARYITEM_FIELDS; i++)
        {
          if (item->fields[i] == NULL) item->fields[i] = g_strdup ("");
        }

        //Set up the integers
        ptr = item->fields[LW_VOCABULARYITEM_FIELD_CORRECT_GUESSES];
        item->correct_guesses = (gint) g_ascii_strtoll (ptr, &endptr, 10);
        ptr = item->fields[LW_VOCABULARYITEM_FIELD_INCORRECT_GUESSES];
        item->incorrect_guesses =  (gint) g_ascii_strtoll (ptr, &endptr, 10);
        ptr = item->fields[LW_VOCABULARYITEM_FIELD_TIMESTAMP];
        item->timestamp =  (guint32) g_ascii_strtoll (ptr, &endptr, 10);
      }
      g_strfreev (atoms); atoms = NULL;
    }

    return item;
}


void
lw_vocabularyitem_free (LwVocabularyItem *item)
{
  gint i;
  for (i = 0; i < TOTAL_LW_VOCABULARYITEM_FIELDS; i++)
  {
    if (item->fields[i] != NULL)
    {
      g_free (item->fields[i]);
      item->fields[i] = NULL;
    }
  }

  g_free (item->score); item->score = NULL;
  g_free (item->days); item->days = NULL;

  g_free (item);
}


gchar* 
lw_vocabularyitem_to_string (LwVocabularyItem *item)
{
    gchar* text;
    text = lw_strjoinv (';', item->fields, TOTAL_LW_VOCABULARYITEM_FIELDS);
    return text;
}
