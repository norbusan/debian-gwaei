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
//! @file src/main.c
//!
//! @brief Main entrance into the program.
//!
//! Main entrance into the program.
//!


#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <regex.h>
#include <stdio.h>
#include <locale.h>
#include <libintl.h>

#include <curl/curl.h>
#include <glib.h>
#include <glib-object.h>

#include <gwaei/definitions.h>
#include <gwaei/regex.h>
#include <gwaei/utilities.h>
#include <gwaei/io.h>
#include <gwaei/dictionary-objects.h>
#include <gwaei/search-objects.h>

#include <gwaei/main.h>
#ifdef WITH_GTK
#include <gwaei/gtk-main-interface.h>
#endif
#ifdef WITH_NCURSES
#include <gwaei/ncurses-main-interface.h>
#endif
#include <gwaei/console-main-interface.h>



int main (int argc, char *argv[])
{    
    //Setup for localized messages
    setlocale(LC_MESSAGES, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    bind_textdomain_codeset (PACKAGE, "UTF-8");
    textdomain(PACKAGE);

    //Set the default CTYPE local
    setlocale(LC_CTYPE, "");
    setlocale(LC_COLLATE, "");
    setlocale(LC_MESSAGES, "");
    //Check if it's the correct locale
    if (gw_util_is_japanese_ctype() == FALSE)
      gw_util_force_japanese_locale();

    if (!g_thread_supported ())
      g_thread_init (NULL);

    g_type_init ();
    curl_global_init (CURL_GLOBAL_ALL);

    gw_util_initialize_runmode (argc, argv);
    gw_regex_initialize_constant_regular_expressions ();
    gw_dictionaries_initialize_dictionary_list ();
    gw_history_initialize_history ();
    gw_main_initialize_generic_output_functions_to_null ();
    gw_io_check_for_rsync ();

    //Start the runmode chosen by the user
    if  (gw_util_get_runmode() == GW_CONSOLE_RUNMODE)
      initialize_console_interface (argc, argv);
#ifdef WITH_NCURSES
    else if  (gw_util_get_runmode () == GW_NCURSES_RUNMODE)
      initialize_ncurses_interface (argc, argv);
#endif
#ifdef WITH_GTK
    else if  (gw_util_get_runmode () == GW_GTK_RUNMODE)
      initialize_gui_interface (argc, argv);
#endif
    else
      initialize_console_interface (argc, argv);

    //Cleanup and exit
    gw_dictlist_free ();
    gw_regex_free_constant_regular_expressions ();

    exit(EXIT_SUCCESS);
}


//!
//! @brief Sets all output function pointers to NULL
//!
void gw_main_initialize_generic_output_functions_to_null ()
{
    gw_output_generic_append_edict_results = NULL;
    gw_output_generic_append_kanjidict_results = NULL;
    gw_output_generic_append_examplesdict_results = NULL;
    gw_output_generic_append_unknowndict_results = NULL;
    gw_output_generic_update_progress_feedback = NULL;
    gw_output_generic_append_less_relevant_header_to_output = NULL;
    gw_output_generic_append_more_relevant_header_to_output = NULL;
    gw_output_generic_pre_search_prep = NULL;
    gw_output_generic_after_search_cleanup = NULL;
}


//!
//! @brief Makes sure that all the output functions have been set properly by the interface
//!
gboolean gw_main_verify_output_generic_functions ()
{
    if (gw_output_generic_append_edict_results == NULL)
      return FALSE;
    if (gw_output_generic_append_kanjidict_results == NULL)
      return FALSE;
    if (gw_output_generic_append_examplesdict_results == NULL)
      return FALSE;
    if (gw_output_generic_append_unknowndict_results == NULL)
      return FALSE;
    if (gw_output_generic_update_progress_feedback == NULL)
      return FALSE;
    if (gw_output_generic_append_less_relevant_header_to_output == NULL)
      return FALSE;
    if (gw_output_generic_append_more_relevant_header_to_output == NULL)
      return FALSE;
    if (gw_output_generic_pre_search_prep == NULL)
      return FALSE;
    if (gw_output_generic_after_search_cleanup == NULL)
      return FALSE;

    return TRUE;
}



