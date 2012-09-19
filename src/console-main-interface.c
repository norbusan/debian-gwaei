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
//! @file src/console-main-interface.c
//!
//! @brief Abstraction layer for the console
//!
//! Used as a go between for functions interacting with the console.
//!


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libintl.h>
#include <regex.h>

#include <glib.h>

#include <gwaei/definitions.h>
#include <gwaei/regex.h>
#include <gwaei/dictionary-objects.h>
#include <gwaei/search-objects.h>
#include <gwaei/engine.h>
#include <gwaei/preferences.h>
#include <gwaei/utilities.h>
#include <gwaei/io.h>

#include <gwaei/console-main-interface.h>
#include <gwaei/main.h>

static gboolean ncurses_switch = FALSE;
static gboolean exact_switch = FALSE;
static gboolean quiet_switch = FALSE;
static gboolean list_switch = FALSE;
static gboolean version_switch = FALSE;

static char* dictionary_switch_data = NULL;
static char* install_switch_data = NULL;
static char* uninstall_switch_data = NULL;
static char* query_text_data = NULL;


//!
//! @brief Print the "less relevant" header where necessary.
//!
void gw_console_append_less_relevant_header_to_output(GwSearchItem *item)
{
    printf("\n[0;31m***[0m[1m%s[0;31m***************************[0m\n\n\n", gettext("Other Results"));
}


//!
//! @brief Print the "no result" message where necessary.
//!
void gw_console_no_result(GwSearchItem *item)
{
    printf("%s\n\n", gettext("No results found!"));
}


//!
//! @brief Callback function for printing a status message to the console
//!
//! @param message String of message to be shown.
//! @param percent An integer of how much a progress bar should be filled.  1-100.
//! @param data Currently unused gpointer.
//!
static int print_message_to_console (char *message, int percent, gpointer data)
{
    if (message != NULL) printf("%s", message);
    if (percent > -1 && percent < 101) printf("%d%s", percent, "%");
    if (message != NULL || (percent > -1 && percent < 101)) printf("\n");

    return FALSE;
}


//!
//! @brief Uninstalls the named dictionary, deleting it.
//!
//! @param name A string of the name of the dictionary to uninstall.
//!
void gw_console_uninstall_dictinfo (GwDictInfo *di)
{
    gw_io_uninstall_dictinfo (di, &print_message_to_console, NULL, TRUE);
    printf(gettext("Finished\n"));
}


//!
//! @brief Installs the named dictionary, deleting it.
//!
//! @param name A string of the name of the dictionary to install.
//!
gboolean gw_console_install_dictinfo (GwDictInfo *di)
{
    printf(gettext("Trying to install the %s...\n"), di->long_name);

    GQuark quark;
    quark = g_quark_from_string (GW_GENERIC_ERROR);
    GError *error = NULL;

    if (di->status != GW_DICT_STATUS_NOT_INSTALLED) return FALSE;

    gw_io_install_dictinfo (di, &print_message_to_console, NULL, TRUE, &error);

    //Install auxillery dictionaries when appropriate
    if (error == NULL && di->id == GW_DICT_ID_KANJI)
    {
      di = gw_dictlist_get_dictinfo_by_id (GW_DICT_ID_RADICALS);
      gw_io_install_dictinfo (di, &print_message_to_console, NULL, TRUE, &error);
    }
    else if (error == NULL && di->id == GW_DICT_ID_RADICALS)
    {
      di = gw_dictlist_get_dictinfo_by_id (GW_DICT_ID_KANJI);
      gw_io_install_dictinfo (di, &print_message_to_console, NULL, TRUE, &error);
    }

    //Print final messages
    if (error != NULL)
    {
      printf("%s\n", error->message);
		  g_error_free (error);
      error = NULL;
      return FALSE;
    }
    else
    {
      printf(gettext("Finished\n"));
    }

    return TRUE;
}


//!
//! @brief Prints to the terminal the about message for the program.
//!
static void print_about_program ()
{
#ifdef WITH_GTK
    printf ("gWaei version %s with the Gnome font end compiled in.", VERSION);
#elif WITH_QT
    printf ("gWaei version %s with the QT font end compiled in.", VERSION);
#else
    printf ("gWaei version %s with no end compiled in.", VERSION);
#endif

    printf ("\n\n");

    printf ("Check for the latest updates at <http://gwaei.sourceforge.net/>\n");
    printf ("Code Copyright (C) 2009-2010 Zachary Dovel\n\n");

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
static void print_search_start_banner (char *query, char *dictionary)
{
    // TRANSLATORS: 'Searching for "${query}" in ${dictionary long name}'
    printf(gettext("Searching for \"%s\" in %s...\n"), query, dictionary);
    printf("\n");
}


//!
//! @brief Prints out the yet uninstalled available dictionaries.
//!
static void print_installable_dictionaries ()
{
    if (quiet_switch == FALSE)
    {
      printf(gettext("Installable dictionaries are:\n"));
    }

    int i = 0; 

    GwDictInfo* di;
    GList *list = gw_dictlist_get_list ();
    while (list != NULL)
    {
      di = (GwDictInfo*) list->data;
      if (di->gckey != NULL && di->status == GW_DICT_STATUS_NOT_INSTALLED &&
          di->id != GW_DICT_ID_RADICALS && di->id != GW_DICT_ID_MIX            )
      {
        printf("  %s (AKA: %s)\n", di->name, di->long_name);
        i++;
      }
      list = list->next;
    }

    if (i == 0)
      printf("  %s\n", gettext("none"));

    printf ("\n");
}


//!
//! @brief Not yet written
//!
static void print_available_dictionaries()
{
    int i = 0;
    GwDictInfo* di;
    GList *list;

	  if (quiet_switch == FALSE)
    {
    	printf(gettext("Available dictionaries are:\n"));
    }

	  list = gw_dictlist_get_list();
    while (list != NULL) {
      di = list->data;
      if (di->status == GW_DICT_STATUS_INSTALLED && di->id != GW_DICT_ID_RADICALS && di->id != GW_DICT_ID_MIX) {
        printf("  %s (AKA: %s)\n", di->name, di->long_name);
        i++;
      }
      list = list->next;
    }

    if (i == 0)
      printf("  %s\n", gettext("none"));

    printf ("\n");
}


//!
//! @brief CONSOLE Main function
//!
//! This function will NOT use GError because it have
//! nobody to report to.
//!
//! @param argc Standard argc from main
//! @param argv Standard argv from main
//!
void initialize_console_interface (int argc, char **argv)
{
    GwDictInfo *di = NULL;
    GError *error = NULL;
    GOptionContext *context;
    context = g_option_context_new (gettext("- Japanese-English dictionary program that allows regex searches"));
    char *summary_text = gettext("waei generally outputs directly to the console.  If you want to do multiple\nsearches, please start gWaei with the -n switch for the multisearch mode.");
    g_option_context_set_summary (context, summary_text);
    char *description_text = NULL;
    description_text = g_strdup_printf(gettext(
                   "Examples:\n"
                   "  waei English               Search for the english word English\n"
                   "  waei \"cats&dogs\"           Search for results containing cats and dogs\n"
                   "  waei \"cats|dogs\"           Search for results containing cats or dogs\n"
                   "  waei cats dogs             Search for results containing \"cats dogs\"\n"
                   "  waei %s                Search for the Japanese word %s\n"
                   "  waei -e %s               Search for %s and ignore similar results\n"
                   "  waei %s                 When you don't know a kanji character\n"
                   "  waei -d Kanji %s           Find a kanji character in the kanji dictionary\n"
                   "  waei -d Names %s       Look up a name in the names dictionary\n"
                   "  waei -d Places %s       Look up a place in the places dictionary")
             , "にほん", "にほん", "日本", "日本", "日.語", "魚", "Miyabe", "Tokyo"
             );
    if (description_text != NULL)
    {
      g_option_context_set_description (context, description_text);
      g_free (description_text);
      description_text = NULL;
    }
    GOptionEntry entries[] = 
    {
#ifdef WITH_NCURSES
      { "ncurses", 'n', 0, G_OPTION_ARG_NONE, &ncurses_switch, gettext("Open up the multisearch window (beta)"), NULL },
#endif
      { "exact", 'e', 0, G_OPTION_ARG_NONE, &exact_switch, gettext("Do not display less relevant results"), NULL },
      { "quiet", 'q', 0, G_OPTION_ARG_NONE, &quiet_switch, gettext("Display less information"), NULL },
      { "dictionary", 'd', 0, G_OPTION_ARG_STRING, &dictionary_switch_data, gettext("Search using a chosen dictionary"), NULL },
      { "list", 'l', 0, G_OPTION_ARG_NONE, &list_switch, gettext("Show available dictionaries for searches"), NULL },
      { "install", 'i', 0, G_OPTION_ARG_STRING, &install_switch_data, gettext("Install dictionary"), NULL },
      { "uninstall", 'u', 0, G_OPTION_ARG_STRING, &uninstall_switch_data, gettext("Uninstall dictionary"), NULL },
      { "version", 'v', 0, G_OPTION_ARG_NONE, &version_switch, gettext("Check the waei version information"), NULL },
      { NULL }
    };
    g_option_context_add_main_entries (context, entries, PACKAGE);
    if (!g_option_context_parse (context, &argc, &argv, &error))
    {
      // TRANSLATORS: The "%s" stands for the error message
      g_print (gettext("Option parsing failed: %s\n"), error->message);
      exit (EXIT_FAILURE);
    }
    if (error != NULL)
    {
      printf("%s\n", error->message);
      g_error_free (error);
      error = NULL;
      exit (EXIT_FAILURE);
    }

    if (list_switch)
    {
      print_available_dictionaries ();
      print_installable_dictionaries();
      exit (EXIT_SUCCESS);
    }

    //User wants to see the version of waei
    if (version_switch)
    {
      print_about_program ();
      exit (EXIT_SUCCESS);
    }

    //User wants to sync dictionaries
/*
    else if (rsync_switch)
    {
      printf("");
      printf(gettext("Syncing possible installed dictionaries..."));
      printf("[0m\n");

      GwDictInfo* di;
      GList *list = gw_dictlist_get_list();

      while (list != NULL && error == NULL) {
        di = list->data;
        gw_dictlist_sync_dictionary (di, &error);
        list = list->next;
      }

      if (error == NULL)
      {
        printf("");
        printf(gettext("Finished"));
        printf("[0m\n");
      }
      else
      {
        printf("");
        printf("%s", error->message);
        printf("[0m\n");
        g_error_free (error);
        error = NULL;
      }

      return;
    }
  */

    //User wants to install dictionary
    if (install_switch_data != NULL)
    {

      di = gw_dictlist_get_dictinfo_by_name (install_switch_data);
      if (di != NULL && di->status != GW_DICT_STATUS_NOT_INSTALLED && di->gckey[0] != '\0')
      {
        printf(gettext("%s is already Installed. "), di->long_name);
        print_installable_dictionaries();
      }
      else if (di == NULL || di->gckey[0] == '\0' || di->id == GW_DICT_ID_RADICALS || di->id == GW_DICT_ID_MIX)
      {
        printf(gettext("%s is not installable with this mechanism. "), install_switch_data);
        print_installable_dictionaries();
      }
      else if (di != NULL)
        gw_console_install_dictinfo (di);
      exit (EXIT_SUCCESS);
    }

    //User wants to uninstall dictionary
    if (uninstall_switch_data != NULL)
    {

      di = gw_dictlist_get_dictinfo_by_name (uninstall_switch_data);
      if (di != NULL)
      {
        if (di->status = GW_DICT_STATUS_INSTALLED && di->id != GW_DICT_ID_RADICALS && di->id != GW_DICT_ID_MIX)
          gw_console_uninstall_dictinfo (di);
        else
        {
          printf(gettext("The %s is not installed. "), di->long_name);
          print_available_dictionaries();
        }
      }
      else
      {
        // TRANSLATORS: The "%s" stands for the value provided by the user to the "waei uninstall"
        printf(gettext("%s is not installed. "), uninstall_switch_data);
        print_available_dictionaries();
      }
      exit (EXIT_SUCCESS);
    }

    //We weren't interupted by any switches! Now to the search....

    //Set dictionary
    if (dictionary_switch_data == NULL)
      di = gw_dictlist_get_dictinfo_by_alias ("English");
    else
      di = gw_dictlist_get_dictinfo_by_alias (dictionary_switch_data);
    if (di == NULL || di->status != GW_DICT_STATUS_INSTALLED)
    {
      printf (gettext("Requested dictionary not found!\n"));
      exit (EXIT_FAILURE);
    }

    //Set query text
    if (argc > 1 && query_text_data == NULL)
    {
      query_text_data = gw_util_strdup_args_to_query (argc, argv);
      if (query_text_data == NULL)
      {
        printf ("Memory error creating initial query string!\n");
        exit (EXIT_FAILURE);
      }
    }
    else
    {
      //No query means to print the help screen
      char *help_text = g_option_context_get_help (context, TRUE, NULL);
      if (help_text != NULL)
      {
        printf("%s\n", help_text);
        g_free (help_text);
        help_text = NULL;
      }
      exit (EXIT_SUCCESS);
    }

    //Set the output generic functions
    gw_console_initialize_interface_output_generics ();

    //Print the search intro
    if (!quiet_switch)
    {
      print_search_start_banner (query_text_data, di->long_name);
    }

    //Start the search
    GwSearchItem *item;
    item = gw_searchitem_new (query_text_data, di, GW_TARGET_CONSOLE);
    if (item == NULL)
    {
      printf(gettext("Query parse error\n"));
      exit (EXIT_FAILURE);
    }

    //Print the number of results
    item->show_less_relevant_results = !exact_switch;
    gw_search_get_results (item);

    //Final header
    if (quiet_switch == FALSE)
    {
      char *message_total = ngettext("Found %d result", "Found %d results", item->total_results);
      char *message_relevant = ngettext("(%d Relevant)", "(%d Relevant)", item->total_relevant_results);
      printf(message_total, item->total_results);
      if (item->total_relevant_results != item->total_results)
        printf(message_relevant, item->total_relevant_results);
      printf("\n");
    }

    //Cleanup
    gw_searchitem_free (item);
    if (query_text_data != NULL)
    {
      g_free (query_text_data);
      query_text_data = NULL;
    }

    //Finish
    exit (EXIT_SUCCESS);
}


//!
//! @brief Not yet written
//!
void gw_console_append_edict_results (GwSearchItem *item)
{
    //Definitions
    int cont = 0;
    GwResultLine *resultline = item->resultline;

    //Kanji
    printf("[32m%s", resultline->kanji_start);
    //Furigana
    if (resultline->furigana_start)
      printf(" [%s]", resultline->furigana_start);
    //Other info
    if (resultline->classification_start)
      printf("[0m %s", resultline->classification_start);
    //Important Flag
    if (resultline->important)
      printf("[0m %s", "P");

    printf("\n");
    while (cont < resultline->def_total)
    {
      printf("[0m      [35m%s [0m%s\n", resultline->number[cont], resultline->def_start[cont]);
      cont++;
    }
    printf("\n");
}


//!
//! @brief Not yet written
//!
void gw_console_append_kanjidict_results (GwSearchItem *item)
{
    if (item == NULL) return;

    char line_started = FALSE;
      GwResultLine *resultline = item->resultline;

    //Kanji
    printf("[32;1m%s[0m\n", resultline->kanji);

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
void gw_console_append_examplesdict_results (GwSearchItem *item)
{
    if (item == NULL) return;

    GwResultLine *resultline = item->resultline;


    if (resultline->def_start[0] != NULL)
    {
      printf ("[32;1m%s[0m", gettext("E:\t"));
      printf ("%s", resultline->def_start[0]);
    }

    if (resultline->kanji_start != NULL)
    {
      printf ("[32;1m%s[0m", gettext("\nJ:\t"));
      printf ("%s", resultline->kanji_start);
    }

    if (resultline->furigana_start != NULL)
    {
      printf("[32;1m%s[0m", gettext("\nD:\t"));
      printf("%s", resultline->furigana_start);
    }

    printf("\n\n");
}


//!
//! @brief Not yet written
//!
void gw_console_append_unknowndict_results (GwSearchItem *item)
{
    if (item == NULL) return;

    printf("%s\n", item->resultline->string);
}


//!
//! @brief Not yet written
//!
void gw_console_update_progress_feedback (GwSearchItem *item)
{
}


//!
//! @brief Not yet written
//!
void gw_console_append_more_relevant_header_to_output (GwSearchItem *item)
{
}


//!
//! @brief Not yet written
//!
void gw_console_pre_search_prep (GwSearchItem *item)
{
}

//!
//! @brief Not yet written
//!
void gw_console_after_search_cleanup (GwSearchItem *item)
{
    //Finish up
    if (item->total_results == 0 &&
        item->target != GW_TARGET_KANJI &&
        item->status == GW_SEARCH_SEARCHING)
    {
      gw_console_no_result(item);
    }
}


//!
//! @brief Set the output generics functions when a search is being done
//!
void gw_console_initialize_interface_output_generics ()
{
    gw_output_generic_append_edict_results = &gw_console_append_edict_results;
    gw_output_generic_append_kanjidict_results = &gw_console_append_kanjidict_results;
    gw_output_generic_append_examplesdict_results = &gw_console_append_examplesdict_results;
    gw_output_generic_append_unknowndict_results = &gw_console_append_unknowndict_results;

    gw_output_generic_update_progress_feedback = &gw_console_update_progress_feedback;
    gw_output_generic_append_less_relevant_header_to_output = &gw_console_append_less_relevant_header_to_output;
    gw_output_generic_append_more_relevant_header_to_output = &gw_console_append_more_relevant_header_to_output;
    gw_output_generic_pre_search_prep = &gw_console_pre_search_prep;
    gw_output_generic_after_search_cleanup = &gw_console_after_search_cleanup;
}

