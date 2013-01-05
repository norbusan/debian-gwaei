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
//! @file console-output.c
//!
//! @brief Abstraction layer for the console
//!
//! Used as a go between for functions interacting with the console.
//!

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <glib.h>

#include <waei/gettext.h>
#include <waei/waei.h>


static void w_console_append_edict_result (WApplication*, LwSearch*);
static void w_console_append_kanjidict_result (WApplication*, LwSearch*);
static void w_console_append_examplesdict_result (WApplication*, LwSearch*);
static void w_console_append_unknowndict_result (WApplication*, LwSearch*);

void 
w_console_append_result (WApplication *application, LwSearch *search)
{
    //Sanity checks
    g_return_if_fail (application != NULL);
    g_return_if_fail (search != NULL);

    //Declarations
    GType type;

    //Initializations
    type = G_OBJECT_TYPE (search->dictionary);

    if (g_type_is_a (type, LW_TYPE_EDICTIONARY))
      w_console_append_edict_result (application, search);
    else if (g_type_is_a (type, LW_TYPE_KANJIDICTIONARY))
      w_console_append_kanjidict_result (application, search);
    else if (g_type_is_a (type, LW_TYPE_EXAMPLEDICTIONARY))
      w_console_append_examplesdict_result (application, search);
    else if (g_type_is_a (type, LW_TYPE_UNKNOWNDICTIONARY))
      w_console_append_unknowndict_result (application, search);
    else
      g_warning ("%s\n", gettext("This is an unknown dictionary type!"));
}


//!
//! @brief Not yet written
//!
static void 
w_console_append_edict_result (WApplication *application, LwSearch *search)
{
    //Definitions
    LwResult *result;
    gboolean color_switch;
    gint cont;

    //Initializations
    result = lw_search_get_result (search);
    if (result == NULL) return;
    color_switch = w_application_get_color_switch (application);
    cont = 0;

    //Kanji
    if (result->kanji_start)
    {
      if (color_switch)
        printf("[32m%s", result->kanji_start);
      else
        printf("%s", result->kanji_start);
    }
    //Furigana
    if (result->furigana_start)
      printf(" [%s]", result->furigana_start);
    //Other info
    if (result->classification_start)
    {
      if (color_switch)
        printf(" [0m %s", result->classification_start);
      else
        printf(" %s", result->classification_start);
    }
    //Important Flag
    if (result->important)
    {
      if (color_switch)
        printf(" [0m %s", "P");
      else
        printf(" %s", "P");
    }

    printf("\n");
    while (cont < result->def_total)
    {
      if (color_switch)
        printf("[0m      [35m%s [0m%s\n", result->number[cont], result->def_start[cont]);
      else
        printf("      %s %s\n", result->number[cont], result->def_start[cont]);
      cont++;
    }
    printf("\n");

    //Cleanup
    lw_result_free (result);
}


//!
//! @brief Not yet written
//!
static void 
w_console_append_kanjidict_result (WApplication *application, LwSearch *search)
{
    if (search == NULL) return;

    //Definitions
    LwResult *result;
    gboolean color_switch;
    gboolean line_started;

    //Initializations
    result = lw_search_get_result (search);
    if (result == NULL) return;
    color_switch = w_application_get_color_switch (application);
    line_started = FALSE;

    //Kanji
    if (color_switch)
      printf("[32;1m%s[0m\n", result->kanji);
    else
      printf("%s\n", result->kanji);

    if (result->radicals)
      printf("%s%s\n", gettext("Radicals:"), result->radicals);

    if (result->strokes)
    {
      line_started = TRUE;
      printf("%s%s", gettext("Stroke:"), result->strokes);
    }

    if (result->frequency)
    {
      if (line_started)
        printf(" ");
      line_started = TRUE;
      printf("%s%s", gettext("Freq:"), result->frequency);
    }

    if (result->grade)
    {
      if (line_started)
        printf(" ");
      line_started = TRUE;
      printf("%s%s", gettext("Grade:"), result->grade);
    }

    if (result->jlpt)
    {
      if (line_started)
        printf(" ");
      line_started = TRUE;
      printf("%s%s", gettext("JLPT:"), result->jlpt);
    }

    if (line_started)
      printf("\n");

    if (result->readings[0])
      printf("%s%s\n", gettext("Readings:"), result->readings[0]);
    if (result->readings[1])
      printf("%s%s\n", gettext("Name:"), result->readings[1]);
    if (result->readings[2])
      printf("%s%s\n", gettext("Radical Name:"), result->readings[2]);

    if (result->meanings)
      printf("%s%s\n", gettext("Meanings:"), result->meanings);
    printf("\n");

    //Cleanup
    lw_result_free (result);
}


//!
//! @brief Not yet written
//!
static void 
w_console_append_examplesdict_result (WApplication *application, LwSearch *search)
{
    if (search == NULL) return;

    //Definitions
    LwResult *result;
    gboolean color_switch;

    //Initializations
    result = lw_search_get_result (search);
    if (result == NULL) return;
    color_switch = w_application_get_color_switch (application);

    if (result->def_start[0] != NULL)
    {
      if (color_switch)
        printf ("[32;1m%s[0m", gettext("E:\t"));
      else
        printf ("%s", gettext("E:\t"));
      printf ("%s", result->def_start[0]);
    }

    if (result->kanji_start != NULL)
    {
      if (color_switch)
        printf ("[32;1m%s[0m", gettext("\nJ:\t"));
      else
        printf ("%s", gettext("\nJ:\t"));
      printf ("%s", result->kanji_start);
    }

    if (result->furigana_start != NULL)
    {
      if (color_switch)
        printf("[32;1m%s[0m", gettext("\nD:\t"));
      else
        printf("%s", gettext("\nD:\t"));
      printf("%s", result->furigana_start);
    }

    printf("\n\n");

    //Cleanup
    lw_result_free (result);
}


//!
//! @brief Not yet written
//!
static void 
w_console_append_unknowndict_result (WApplication *application, LwSearch *search)
{
    if (search == NULL) return;

    //Definitions
    LwResult *result;

    //Initializations
    result = lw_search_get_result (search);
    if (result == NULL) return;

    printf("%s\n", result->text);

    //Cleanup
    lw_result_free (result);
}


//!
//! @brief Print the "no result" message where necessary.
//!
void 
w_console_no_result (WApplication *application, LwSearch *search)
{
    //Sanity check
    if (application == NULL || search == NULL || search->dictionary == NULL) return;
    if (search->status != LW_SEARCHSTATUS_IDLE || search->current == 0L || search->total_results > 0) return; 

    //Declarations
    gboolean color_switch;
    gboolean quiet_switch;

    //Initializations
    color_switch = w_application_get_color_switch (application);
    quiet_switch = w_application_get_quiet_switch (application);

    if (quiet_switch) return;

    if (color_switch)
      printf("%s\n\n", gettext("No results found!"));
    else
      printf("%s\n\n", gettext("No results found!"));
}
