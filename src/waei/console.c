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
//! @file console.c
//!
//! @brief Abstraction layer for the console
//!
//! Used as a go between for functions interacting with the console.
//!


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <glib.h>

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include <waei/gettext.h>
#include <waei/waei.h>


//!
//! @brief Uninstalls the named dictionary, deleting it.
//!
//! @param name A string of the name of the dictionary to uninstall.
//!
gint 
w_console_uninstall_dictionary (WApplication* application, GError **error)
{
    //Sanity check
    if (error != NULL && *error != NULL) return 1;

    //Declarations
    LwDictionaryList *dictionarylist;
    LwDictionary *dictionary;
    gint resolution;
    const gchar *uninstall_switch_data;

    //Initializations
    uninstall_switch_data = w_application_get_uninstall_switch_data (application);
    dictionarylist = w_application_get_installed_dictionarylist (application);
    dictionary = lw_dictionarylist_get_dictionary_fuzzy (dictionarylist, uninstall_switch_data);
    resolution = 0;

    if (dictionary != NULL)
    {
      printf(gettext("Uninstalling %s Dictionary...\n"), lw_dictionary_get_name (dictionary));
      lw_dictionary_uninstall (dictionary, w_console_uninstall_progress_cb, error);
    }
    else
    {
      printf("\n%s was not found!\n\n", uninstall_switch_data);
      w_console_print_available_dictionaries (application);
    }

    if (*error != NULL)
    {
      resolution = 1;
    }

    return resolution;
}


//!
//! @brief Installs the named dictionary, deleting it.
//!
//! @param name A string of the name of the dictionary to install.
//!
gint 
w_console_install_dictionary (WApplication *application, GError **error)
{
    //Sanity check
    if (error != NULL && *error != NULL) return 1;

    //Declarations
    LwDictionaryList *dictionarylist;
    LwDictionary *dictionary;
    gint resolution;
    gulong signalid;
    const gchar *install_switch_data;

    //Initializations
    install_switch_data = w_application_get_install_switch_data (application);
    dictionarylist = w_application_get_installable_dictionarylist (application);
    dictionary = lw_dictionarylist_get_dictionary_fuzzy (dictionarylist, install_switch_data);
    resolution = 0;

    if (dictionary != NULL)
    {
      printf(gettext("Installing %s Dictionary...\n"), lw_dictionary_get_name (dictionary));
      signalid = g_signal_connect (G_OBJECT (dictionary), "progress-changed", G_CALLBACK (w_console_update_progress_cb), application);
      lw_dictionary_install (dictionary, NULL, error);
      if (g_signal_handler_is_connected (G_OBJECT (dictionary), signalid))
        g_signal_handler_disconnect (G_OBJECT (dictionary), signalid);
      if (*error == NULL) 
      {
        printf("\n%s\n", gettext("Installation complete."));
      }
      else
      {
        printf ("\n%s\n", gettext("Installation failed!"));
      }
    }
    else
    {
      printf("\n%s was not found!\n\n", install_switch_data);
      w_console_print_installable_dictionaries (application);
    }

    if (*error != NULL)
    {
      resolution = 1;
    }

    return resolution;
}


//!
//! @brief Prints to the terminal the about message for the program.
//!
void 
w_console_about (WApplication* app)
{
    printf ("waei version %s", VERSION);

    printf ("\n\n");

    printf ("Check for the latest updates at <http://gwaei.sourceforge.net/>\n");
    printf ("Code Copyright (C) 2009-2011 Zachary Dovel\n\n");

    printf ("License:\n");
    printf ("Copyright (C) 2008 Free Software Foundation, Inc.\nLicense GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\nThis is free software: you are free to change and redistribute it.\nThere is NO WARRANTY, to the extent permitted by law.\n\n");
}


//!
//! @brief Prints out the yet uninstalled available dictionaries.
//!
void 
w_console_print_installable_dictionaries (WApplication *application)
{
    printf(gettext("Installable dictionaries are:\n"));

    //Declarations
    gint i;
    gint j;
    GList *iter;
    LwDictionaryList *dictionarylist;
    LwDictionary* dictionary;
    const gchar* filename;

    //Initializations
    i = 0; 
    dictionarylist = w_application_get_installable_dictionarylist (application);
    iter = lw_dictionarylist_get_list (dictionarylist);

    while (iter != NULL)
    {
      dictionary = LW_DICTIONARY (iter->data);
      if (lw_dictionary_installer_is_valid (dictionary))
      {
        filename = lw_dictionary_get_filename (dictionary);
        printf("  %s", filename);
        for (j = strlen(filename); j < 20; j++) printf(" ");
        printf("(AKA: %s Dictionary)\n", lw_dictionary_get_name (dictionary));
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
void 
w_console_print_available_dictionaries (WApplication *application)
{
    //Declarations
    gint i;
    gint j;
    LwDictionaryList *dictionarylist;
    LwDictionary* dictionary;
    GList *link;
    const gchar *filename;

    //Initializations
    i = 0;
    j = 0;
    dictionarylist = w_application_get_installed_dictionarylist (application);
	  link = lw_dictionarylist_get_list (dictionarylist);

    printf(gettext("Available dictionaries are:\n"));

    while (link != NULL) {
      dictionary = LW_DICTIONARY (link->data);
      filename = lw_dictionary_get_filename (dictionary);

      printf("  %s", filename);
      for (j = strlen(filename); j < 20; j++) printf(" ");
      printf("(AKA: %s Dictionary)\n", lw_dictionary_get_name (dictionary));

      i++;
      link = link->next;
    }

    if (i == 0)
    {
      printf("  %s\n", gettext("none"));
    }
}


//!
//! @brief Lists the available and installable dictionaries
//!
void 
w_console_list (WApplication *app)
{
    w_console_print_available_dictionaries (app);
    w_console_print_installable_dictionaries (app);
}


//!
//! @brief If the GError is set, it prints it and frees the memory
//! @param error A pointer to a gerror pointer
//!
void 
w_console_handle_error (WApplication* app, GError **error)
{
    if (error != NULL && *error != NULL)
    {
      fprintf(stderr, "Error: %s\n", (*error)->message);
      g_error_free (*error);
      *error = NULL;
    }
}


gint 
w_console_search (WApplication *application, GError **error)
{
    //Sanity check
    if (error != NULL && *error != NULL) return FALSE;

    //Declarations
    WSearchData *sdata;
    LwSearch *search;
    LwDictionaryList *dictionarylist;
//    LwPreferences* preferences;

    const gchar* dictionary_switch_data;
    const gchar* query_text_data;
    gboolean quiet_switch;
    gboolean exact_switch;
    gint total_results;
    gint total_relevant_results;

    char *message_total;
    char *message_relevant;
    LwDictionary *dictionary;
    gint resolution;
    GMainLoop *loop;
    LwSearchFlags flags;

    //Initializations
    dictionarylist = w_application_get_installed_dictionarylist (application);

    dictionary_switch_data = w_application_get_dictionary_switch_data (application);
    query_text_data = w_application_get_query_text_data (application);
    quiet_switch = w_application_get_quiet_switch (application);
    exact_switch = w_application_get_exact_switch (application);
    flags = 0;

    dictionary = lw_dictionarylist_get_dictionary_fuzzy (dictionarylist, dictionary_switch_data);
    if (exact_switch) flags = flags | LW_SEARCH_FLAG_EXACT;
    if (dictionary == NULL) printf("dictionary equals zero! %s\n", dictionary_switch_data);
    search = lw_search_new (dictionary, query_text_data, flags, error);
    resolution = 0;

    //Sanity checks
    if (dictionary == NULL)
    {
      resolution = 1;
      fprintf (stderr, gettext("Requested dictionary not found!\n"));
      return resolution;
    }

    if (search == NULL)
    {
      resolution = 1;
      return resolution;
    }

    //Print the search intro
    if (!quiet_switch)
    {
      // TRANSLATORS: 'Searching for "${query}" in ${dictionary long name}'
      printf(gettext("Searching for \"%s\" in %s Dictionary...\n"), query_text_data, lw_dictionary_get_name (dictionary));
      printf("\n");
    }

    loop = g_main_loop_new (NULL, FALSE); 
    sdata = w_searchdata_new (loop, application);
    lw_search_set_data (search, sdata, LW_SEARCH_DATA_FREE_FUNC (w_searchdata_free));

    //Print the results
    lw_search_start (search, TRUE);

    g_timeout_add_full (
        G_PRIORITY_LOW,
        100,
        (GSourceFunc) w_console_append_result_timeout,
        search,
        NULL
    );

    g_main_loop_run (loop);

    //Print final header
    if (quiet_switch == FALSE)
    {
      total_results = lw_search_get_total_results (search);
      message_total = ngettext("Found %d result", "Found %d results", total_results);
      printf(message_total, total_results);

      total_relevant_results = lw_search_get_total_relevant_results (search);
      message_relevant = ngettext("(%d Relevant)", "(%d Relevant)", total_relevant_results);
      if (total_relevant_results != total_results)
        printf(message_relevant, total_relevant_results);

      printf("\n");
    }

    lw_search_cancel (search);

    //Cleanup
    lw_search_free (search);
    g_main_loop_unref (loop);

    return 0;
}
