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
//! @file src/console/console.c
//!
//! @brief Abstraction layer for the console
//!
//! Used as a go between for functions interacting with the console.
//!


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libintl.h>

#include <glib.h>

#include <waei/waei.h>


//!
//! @brief Print the "less relevant" header where necessary.
//!
void w_console_append_less_relevant_header_to_output(LwSearchItem *item)
{
    if (w_get_color_switch ())
      printf("\n[0;31m***[0m[1m%s[0;31m***************************[0m\n\n\n", gettext("Other Results"));
    else
      printf("\n***%s***************************\n\n\n", gettext("Other Results"));
}


//!
//! @brief Print the "no result" message where necessary.
//!
void w_console_no_result (LwSearchItem *item)
{
    printf("%s\n\n", gettext("No results found!"));
}


//!
//! @brief Uninstalls the named dictionary, deleting it.
//!
//! @param name A string of the name of the dictionary to uninstall.
//!
gboolean w_console_uninstall_dictinfo (const char *FUZZY, GError **error)
{
    //Declarations
    LwDictInfo *di;

    //Initializations
    di = lw_dictinfolist_get_dictinfo_fuzzy (FUZZY);

    if (di != NULL)
    {
      printf(gettext("Uninstalling %s...\n"), di->longname);
      lw_dictinfo_uninstall (di, w_console_uninstall_progress_cb, error);
    }
    else
    {
      printf("\n%s was not found!\n\n", FUZZY);
      w_console_print_available_dictionaries ();
    }

    return (*error == NULL);
}


//!
//! @brief Installs the named dictionary, deleting it.
//!
//! @param name A string of the name of the dictionary to install.
//!
gboolean w_console_install_dictinst (const char *FUZZY, GError **error)
{
    //Declarations
    LwDictInst *di;

    //Initializations
    di = lw_dictinstlist_get_dictinst_fuzzy (FUZZY);

    if (di != NULL)
    {
      printf(gettext("Installing %s...\n"), di->longname);
      lw_dictinst_install (di, w_console_install_progress_cb, error);
    }
    else
    {
      printf("\n%s was not found!\n\n", FUZZY);
      w_console_print_installable_dictionaries ();
    }

    return (*error == NULL);
}


//!
//! @brief Prints to the terminal the about message for the program.
//!
void w_console_about ()
{
    printf ("waei version %s", VERSION);

    printf ("\n\n");

    printf ("Check for the latest updates at <http://gwaei.sourceforge.net/>\n");
    printf ("Code Copyright (C) 2009-2011 Zachary Dovel\n\n");

    printf ("License:\n");
    printf ("Copyright (C) 2008 Free Software Foundation, Inc.\nLicense GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\nThis is free software: you are free to change and redistribute it.\nThere is NO WARRANTY, to the extent permitted by law.\n\n");
}


//!
//! @brief This function prints the start banner
//!
//! This function prints the start banner in both
//! simple and ncurses interface.
//!
//! @param query The query string we are searching
//! @param dictionary The name (string) of the dictionary used
//!
void w_console_start_banner (char *query, char *dictionary)
{
    // TRANSLATORS: 'Searching for "${query}" in ${dictionary long name}'
    printf(gettext("Searching for \"%s\" in %s...\n"), query, dictionary);
    printf("\n");
}


//!
//! @brief Prints out the yet uninstalled available dictionaries.
//!
void w_console_print_installable_dictionaries ()
{
    printf(gettext("Installable dictionaries are:\n"));

    //Declarations
    int i;
    int j;
    GList *iter;
    LwDictInst* di;

    //Initializations
    i = 0; 
    iter = lw_dictinstlist_get_list ();

    while (iter != NULL)
    {
      di = (LwDictInst*) iter->data;
      if (lw_dictinst_data_is_valid (di))
      {
        printf("  %s", di->filename);
        for (j = strlen(di->filename); j < 20; j++) printf(" ");
        printf("(AKA: %s)\n", di->longname);
        i++;
      }
      iter = iter->next;
    }

    if (i == 0)
    {
      printf("  %s\n", gettext("none"));
    }
}


//!
//! @brief Not yet written
//!
void w_console_print_available_dictionaries ()
{
    //Declarations
    int i;
    int j;
    LwDictInfo* di;
    GList *iter;

    //Initializations
    i = 0;
    j = 0;
	  iter = lw_dictinfolist_get_list();

    printf(gettext("Available dictionaries are:\n"));

    while (iter != NULL) {
      di = iter->data;
      printf("  %s", di->filename);
      for (j = strlen(di->filename); j < 20; j++) printf(" ");
      printf("(AKA: %s)\n", di->longname);
      i++;
      iter = iter->next;
    }

    if (i == 0)
    {
      printf("  %s\n", gettext("none"));
    }
}


//!
//! @brief Not yet written
//!
void w_console_append_edict_results_to_buffer (LwSearchItem *item)
{
    //Definitions
    int cont = 0;
    LwResultLine *resultline = item->resultline;

    //Kanji
    if (w_get_color_switch ())
      printf("[32m%s", resultline->kanji_start);
    else
      printf("%s", resultline->kanji_start);
    //Furigana
    if (resultline->furigana_start)
      printf(" [%s]", resultline->furigana_start);
    //Other info
    if (resultline->classification_start)
      if (w_get_color_switch ())
        printf("[0m %s", resultline->classification_start);
      else
        printf("%s", resultline->classification_start);
    //Important Flag
    if (resultline->important)
      if (w_get_color_switch ())
        printf("[0m %s", "P");
      else
        printf("%s", "P");

    printf("\n");
    while (cont < resultline->def_total)
    {
      if (w_get_color_switch ())
        printf("[0m      [35m%s [0m%s\n", resultline->number[cont], resultline->def_start[cont]);
      else
        printf("      %s %s\n", resultline->number[cont], resultline->def_start[cont]);
      cont++;
    }
    printf("\n");
}


//!
//! @brief Not yet written
//!
void w_console_append_kanjidict_results_to_buffer (LwSearchItem *item)
{
    if (item == NULL) return;

    char line_started = FALSE;
      LwResultLine *resultline = item->resultline;

    //Kanji
    if (w_get_color_switch ())
      printf("[32;1m%s[0m\n", resultline->kanji);
    else
      printf("%s\n", resultline->kanji);

    if (resultline->radicals)
      printf("%s%s\n", gettext("Radicals:"), resultline->radicals);

    if (resultline->strokes)
    {
      line_started = TRUE;
      printf("%s%s", gettext("Stroke:"), resultline->strokes);
    }

    if (resultline->frequency)
    {
      if (line_started)
        printf(" ");
      line_started = TRUE;
      printf("%s%s", gettext("Freq:"), resultline->frequency);
    }

    if (resultline->grade)
    {
      if (line_started)
        printf(" ");
      line_started = TRUE;
      printf("%s%s", gettext("Grade:"), resultline->grade);
    }

    if (resultline->jlpt)
    {
      if (line_started)
        printf(" ");
      line_started = TRUE;
      printf("%s%s", gettext("JLPT:"), resultline->jlpt);
    }

    if (line_started)
      printf("\n");

    if (resultline->readings[0])
      printf("%s%s\n", gettext("Readings:"), resultline->readings[0]);
    if (resultline->readings[1])
      printf("%s%s\n", gettext("Name:"), resultline->readings[1]);
    if (resultline->readings[2])
      printf("%s%s\n", gettext("Radical Name:"), resultline->readings[2]);

    if (resultline->meanings)
      printf("%s%s\n", gettext("Meanings:"), resultline->meanings);
    printf("\n");
}


//!
//! @brief Not yet written
//!
void w_console_append_examplesdict_results_to_buffer (LwSearchItem *item)
{
    if (item == NULL) return;

    LwResultLine *resultline = item->resultline;


    if (resultline->def_start[0] != NULL)
    {
      if (w_get_color_switch ())
        printf ("[32;1m%s[0m", gettext("E:\t"));
      else
        printf ("%s", gettext("E:\t"));
      printf ("%s", resultline->def_start[0]);
    }

    if (resultline->kanji_start != NULL)
    {
      if (w_get_color_switch ())
        printf ("[32;1m%s[0m", gettext("\nJ:\t"));
      else
        printf ("%s", gettext("\nJ:\t"));
      printf ("%s", resultline->kanji_start);
    }

    if (resultline->furigana_start != NULL)
    {
      if (w_get_color_switch ())
        printf("[32;1m%s[0m", gettext("\nD:\t"));
      else
        printf("%s", gettext("\nD:\t"));
      printf("%s", resultline->furigana_start);
    }

    printf("\n\n");
}


//!
//! @brief Not yet written
//!
void w_console_append_unknowndict_results_to_buffer (LwSearchItem *item)
{
    if (item == NULL) return;

    printf("%s\n", item->resultline->string);
}


//!
//! @brief Not yet written
//!
void w_console_append_more_relevant_header_to_output (LwSearchItem *item)
{
}


//!
//! @brief Not yet written
//!
void w_console_pre_search_prep (LwSearchItem *item)
{
}

//!
//! @brief Not yet written
//!
void w_console_after_search_cleanup (LwSearchItem *item)
{
    //Finish up
    if (item->total_results == 0 &&
        item->target != GW_TARGET_KANJI &&
        item->status == GW_SEARCH_SEARCHING)
    {
      w_console_no_result(item);
    }
}


int w_console_uninstall_progress_cb (double fraction, gpointer data)
{
  //Declarations
  LwDictInfo *di;
  char *uri;

  //Initializations
  di = data;
  uri = g_build_filename (lw_util_get_directory_for_engine (di->engine), di->filename, NULL);

  printf("Removing %s...\n", uri);

  g_free (uri);

  return FALSE;
}




static gboolean _group_index_changed = FALSE;
static int _previous_percent = -1;
int w_console_install_progress_cb (double fraction, gpointer data)
{
  //Declarations
  LwDictInst *di;
  char *status;
  double current_fraction;
  int current_percent;

  //Initializations
  di = data;
  current_fraction = lw_dictinst_get_process_progress (di, fraction);
  current_percent = (int) (100.0 * current_fraction); 

  //Update the dictinst progress state only when the delta is large enough
  if (current_percent < 100 && _group_index_changed)
  {
    _group_index_changed = FALSE;
    printf("\n");
  }
  else if (current_percent == 100)
  {
    _group_index_changed = TRUE;
  }

  status = lw_dictinst_get_status_string (di, TRUE);
  printf("\r [%d%] %s", current_percent, status);
  _previous_percent = current_percent;
  g_free (status);

  return FALSE;
}


//!
//! @brief Lists the available and installable dictionaries
//!
void w_console_list ()
{
    w_console_print_available_dictionaries ();
    w_console_print_installable_dictionaries ();
}


//!
//! @brief If the GError is set, it prints it and frees the memory
//! @param error A pointer to a gerror pointer
//!
void w_console_handle_error (GError **error)
{
    if (*error != NULL)
    {
      fprintf(stderr, "Error: %s\n", (*error)->message);
      g_error_free (*error);
      *error = NULL;
    }
}


gboolean w_console_search (char* query, char* fuzzy, gboolean quiet, gboolean exact, GError **error)
{
    //Sanity check
    if (*error != NULL) return FALSE;

    //Declarations
    LwSearchItem *item;
    char *message_total;
    char *message_relevant;
    LwDictInfo *di;

    //Initializations
    di = lw_dictinfolist_get_dictinfo_fuzzy (fuzzy);
    if (di == NULL)
    {
      printf (gettext("Requested dictionary not found!\n"));
      return FALSE;
    }

    item = lw_searchitem_new (query, di, GW_TARGET_CONSOLE, error);
    if (item == NULL)
    {
      printf("%s\n", (*error)->message);
      return FALSE;
    }

    //Print the search intro
    if (!quiet)
    {
      w_console_start_banner (query, di->longname);
    }

    //Print the results
    lw_engine_get_results (item, FALSE, exact);

    //Print final header
    if (quiet == FALSE)
    {
      message_total = ngettext("Found %d result", "Found %d results", item->total_results);
      message_relevant = ngettext("(%d Relevant)", "(%d Relevant)", item->total_relevant_results);
      printf(message_total, item->total_results);
      if (item->total_relevant_results != item->total_results)
        printf(message_relevant, item->total_relevant_results);
      printf("\n");
    }

    //Cleanup
    lw_searchitem_free (item);

    return TRUE;
}
