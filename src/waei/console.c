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


#include "../private.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <glib.h>

#include <waei/waei.h>


//!
//! @brief Uninstalls the named dictionary, deleting it.
//!
//! @param name A string of the name of the dictionary to uninstall.
//!
int 
w_console_uninstall_dictinfo (WApplication* application, GError **error)
{
    //Sanity check
    if (error != NULL && *error != NULL) return 1;

    //Declarations
    LwDictInfoList *dictinfolist;
    LwDictInfo *di;
    int resolution;
    const gchar *uninstall_switch_data;

    //Initializations
    uninstall_switch_data = w_application_get_uninstall_switch_data (application);
    dictinfolist = w_application_get_dictinfolist (application);
    di = lw_dictinfolist_get_dictinfo_fuzzy (dictinfolist, uninstall_switch_data);
    resolution = 0;

    if (di != NULL)
    {
      printf(gettext("Uninstalling %s...\n"), di->longname);
      lw_dictinfo_uninstall (di, w_console_uninstall_progress_cb, error);
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
int 
w_console_install_dictinst (WApplication *application, GError **error)
{
    //Sanity check
    if (error != NULL && *error != NULL) return 1;

    //Declarations
    LwDictInstList *dictinstlist;
    LwDictInst *di;
    int resolution;
    const gchar *install_switch_data;

    //Initializations
    install_switch_data = w_application_get_install_switch_data (application);
    dictinstlist = w_application_get_dictinstlist (application);
    di = lw_dictinstlist_get_dictinst_fuzzy (dictinstlist, install_switch_data);
    resolution = 0;

    if (di != NULL)
    {
      printf(gettext("Installing %s...\n"), di->longname);
      lw_dictinst_install (di, w_console_install_progress_cb, di, error);
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
    int i;
    int j;
    GList *iter;
    LwDictInstList *dictinstlist;
    LwDictInst* di;

    //Initializations
    i = 0; 
    dictinstlist = w_application_get_dictinstlist (application);
    iter = dictinstlist->list;

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
void 
w_console_print_available_dictionaries (WApplication *application)
{
    //Declarations
    int i;
    int j;
    LwDictInstList *dictinstlist;
    LwDictInfo* di;
    GList *iter;

    //Initializations
    i = 0;
    j = 0;
    dictinstlist = w_application_get_dictinstlist (application);
	  iter = dictinstlist->list;

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


int 
w_console_search (WApplication *application, GError **error)
{
    //Sanity check
    if (error != NULL && *error != NULL) return FALSE;

    //Declarations
    WSearchData *sdata;
    LwSearchItem *item;
    LwDictInfoList *dictinfolist;
    LwPreferences* preferences;

    const gchar* dictionary_switch_data;
    const gchar* query_text_data;
    gboolean quiet_switch;
    gboolean exact_switch;

    char *message_total;
    char *message_relevant;
    LwDictInfo *di;
    int resolution;
    GMainLoop *loop;

    //Initializations
    dictinfolist = w_application_get_dictinfolist (application);
    preferences = w_application_get_preferences (application);

    dictionary_switch_data = w_application_get_dictionary_switch_data (application);
    query_text_data = w_application_get_query_text_data (application);
    quiet_switch = w_application_get_quiet_switch (application);
    exact_switch = w_application_get_exact_switch (application);

    di = lw_dictinfolist_get_dictinfo_fuzzy (dictinfolist, dictionary_switch_data);

    //Sanity checks
    if (di == NULL)
    {
      resolution = 1;
      fprintf (stderr, gettext("Requested dictionary not found!\n"));
      return resolution;
    }

    item = lw_searchitem_new (query_text_data, di, preferences, error);
    resolution = 0;

    if (item == NULL)
    {
      resolution = 1;
      return resolution;
    }

    //Print the search intro
    if (!quiet_switch)
    {
      // TRANSLATORS: 'Searching for "${query}" in ${dictionary long name}'
      printf(gettext("Searching for \"%s\" in %s...\n"), query_text_data, di->longname);
#if WITH_MECAB
      if (item->queryline->morphology) {
          printf(gettext("Also showing results for 「%s」\n"), item->queryline->morphology);
      }
#endif
      printf("\n");
    }

    loop = g_main_loop_new (NULL, FALSE); 
    sdata = w_searchdata_new (loop, application);
    lw_searchitem_set_data (item, sdata, LW_SEARCHITEM_DATA_FREE_FUNC (w_searchdata_free));

    //Print the results
    lw_searchitem_start_search (item, TRUE, exact_switch);

    g_timeout_add_full (
        G_PRIORITY_LOW,
        100,
        (GSourceFunc) w_console_append_result_timeout,
        item,
        NULL
    );

    g_main_loop_run (loop);

    //Print final header
    if (quiet_switch == FALSE)
    {
      message_total = ngettext("Found %d result", "Found %d results", item->total_results);
      message_relevant = ngettext("(%d Relevant)", "(%d Relevant)", item->total_relevant_results);
      printf(message_total, item->total_results);
      if (item->total_relevant_results != item->total_results)
        printf(message_relevant, item->total_relevant_results);
      printf("\n");
    }

    lw_searchitem_cancel_search (item);

    //Cleanup
    lw_searchitem_free (item);
    g_main_loop_unref (loop);

    return 0;
}
