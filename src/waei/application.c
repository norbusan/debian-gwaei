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
//! @file application.c
//!
//! @brief To be written
//!


#include "../private.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include <waei/waei.h>
#include <waei/application-private.h>


G_DEFINE_TYPE (WApplication, w_application, G_TYPE_OBJECT)

static void w_application_parse_args (WApplication*, int*, char***);

//!
//! @brief creates a new instance of the gwaei applicaiton
//!
GObject* 
w_application_new ()
{
    //Declarations
    WApplication *application;

    //Initializations
    application = g_object_new (W_TYPE_APPLICATION, NULL);

    return G_OBJECT (application);
}


static void 
w_application_init (WApplication *application)
{
    application->priv = W_APPLICATION_GET_PRIVATE (application);
    memset(application->priv, 0, sizeof(WApplicationPrivate));
}


static void 
w_application_constructed (GObject *object)
{
    //Chain the parent class
    {
      G_OBJECT_CLASS (w_application_parent_class)->constructed (object);
    }

    lw_regex_initialize ();
}


static void 
w_application_finalize (GObject *object)
{
    //Declarations
    WApplication *application;
    WApplicationPrivate *priv;

    application = W_APPLICATION (object);
    priv = application->priv;

    if (priv->dictinstlist != NULL) lw_dictinstlist_free (priv->dictinstlist); priv->dictinstlist = NULL;
    if (priv->dictinfolist != NULL) lw_dictinfolist_free (priv->dictinfolist); priv->dictinfolist = NULL;
    if (priv->context != NULL) g_option_context_free (priv->context); priv->context = NULL;
    if (priv->arg_query_text_data != NULL) g_free(priv->arg_query_text_data); priv->arg_query_text_data = NULL;
    if (priv->preferences != NULL) lw_preferences_free (priv->preferences); priv->preferences = NULL;
#if WITH_MECAB
    if (lw_morphologyengine_has_default ()) 
    {
      lw_morphologyengine_free (lw_morphologyengine_get_default ()); 
    }
#endif

    lw_regex_free ();

    G_OBJECT_CLASS (w_application_parent_class)->finalize (object);
}


static void
w_application_class_init (WApplicationClass *klass)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = w_application_constructed;
  object_class->finalize = w_application_finalize;

  g_type_class_add_private (object_class, sizeof (WApplicationPrivate));
}


//!
//! @brief Loads the arguments from the command line into the app instance
//!
static void 
w_application_parse_args (WApplication *application, int *argc, char** argv[])
{
    WApplicationPrivate *priv;
    const gchar *summary_text;
    const gchar *description_text;
    GError *error;

    priv = application->priv;

    //Reset the switches to their default state
    if (priv->arg_dictionary_switch_data != NULL) g_free (priv->arg_dictionary_switch_data); priv->arg_dictionary_switch_data = NULL;
    if (priv->arg_query_text_data != NULL) g_free (priv->arg_query_text_data); priv->arg_query_text_data = NULL;
    priv->arg_version_switch = FALSE;
    error = NULL;
    if (priv->context != NULL) g_option_context_free (priv->context); priv->context = NULL;

    priv->context = g_option_context_new (gettext("- A dictionary program for Japanese-English translation."));
    summary_text = gettext("waei generally outputs directly to the console.");
    description_text = g_strdup_printf(
        gettext(
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
           "  waei -d Places %s       Look up a place in the places dictionary"
         )
         , "にほん", "にほん", "日本", "日本", "日.語", "魚", "Miyabe", "Tokyo"
    );
    GOptionEntry entries[] = {
      { "exact", 'e', 0, G_OPTION_ARG_NONE, &(priv->arg_exact_switch), gettext("Do not display less relevant results"), NULL },
      { "quiet", 'q', 0, G_OPTION_ARG_NONE, &(priv->arg_quiet_switch), gettext("Display less information"), NULL },
      { "color", 'c', 0, G_OPTION_ARG_NONE, &(priv->arg_color_switch), gettext("Display results with color"), NULL },
      { "dictionary", 'd', 0, G_OPTION_ARG_STRING, &(priv->arg_dictionary_switch_data), gettext("Search using a chosen dictionary"), NULL },
      { "list", 'l', 0, G_OPTION_ARG_NONE, &(priv->arg_list_switch), gettext("Show available dictionaries for searches"), NULL },
      { "install", 'i', 0, G_OPTION_ARG_STRING, &(priv->arg_install_switch_data), gettext("Install dictionary"), NULL },
      { "uninstall", 'u', 0, G_OPTION_ARG_STRING, &(priv->arg_uninstall_switch_data), gettext("Uninstall dictionary"), NULL },
      { "version", 'v', 0, G_OPTION_ARG_NONE, &(priv->arg_version_switch), gettext("Check the waei version information"), NULL },
      { NULL }
    };

    g_option_context_set_description (priv->context, description_text);
    g_option_context_set_summary (priv->context, summary_text);
    g_option_context_add_main_entries (priv->context, entries, PACKAGE);
    g_option_context_set_ignore_unknown_options (priv->context, TRUE);
    g_option_context_parse (priv->context, argc, argv, &error);

    if (error != NULL)
    {
      w_application_handle_error (application, &error);
      exit(1);
    }

    //Get the query after the flags have been parsed out
    priv->arg_query_text_data = lw_util_get_query_from_args (*argc, *argv);
}


//!
//! @brief Prints to the terminal the about message for the program.
//!
void 
w_application_print_about (WApplication *application)
{
    const gchar *name;
    name = w_application_get_program_name (W_APPLICATION (application));

    printf (gettext ("%s version %s"), name, VERSION);

    printf ("\n\n");

    printf ("Check for the latest updates at <http://gwaei.sourceforge.net/>\n");
    printf ("Code Copyright (C) 2009-2012 Zachary Dovel\n\n");

    printf ("License:\n");
    printf ("Copyright (C) 2008 Free Software Foundation, Inc.\nLicense GPLv3+: "
            "GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\nThis"
            " is free software: you are free to change and redistribute it.\nThe"
            "re is NO WARRANTY, to the extent permitted by law.\n\n"             );
}


//!
//! @brief Returns the program name.  It should not be freed or modified
//! @returns A constanst string representing the program name
//!
const char*
w_application_get_program_name (WApplication *application) 
{
  return gettext("Waei Japanese-English Dictionary");
}


void 
w_application_handle_error (WApplication *application, GError **error)
{
    //Sanity checks
    if (error == NULL || *error == NULL) return;

    fprintf(stderr, "ERROR: %s\n", (*error)->message);

    //Cleanup
    g_error_free (*error);
    *error = NULL;
}


LwPreferences* 
w_application_get_preferences (WApplication *application)
{
    WApplicationPrivate *priv;

    priv = application->priv;


    if (priv->preferences == NULL)
    {
      g_io_extension_point_register ("gsettings-backend");
      priv->preferences = lw_preferences_new (g_memory_settings_backend_new ());
    }

    return priv->preferences;
}


LwDictInfoList* 
w_application_get_dictinfolist (WApplication *application)
{
    WApplicationPrivate *priv;
    LwPreferences *preferences;

    priv = application->priv;
    preferences = w_application_get_preferences (application);

    if (priv->dictinfolist == NULL)
    {
      priv->dictinfolist = lw_dictinfolist_new (20);
      lw_dictinfolist_load_order (priv->dictinfolist, preferences);
    }

    return priv->dictinfolist;
}


LwDictInstList* 
w_application_get_dictinstlist (WApplication *application)
{
  WApplicationPrivate *priv;
  LwPreferences *preferences;

  priv = application->priv;

  if (priv->dictinstlist == NULL)
  {
    preferences = w_application_get_preferences (application);
    priv->dictinstlist = lw_dictinstlist_new (preferences);
  }

  return priv->dictinstlist;
}


//!
//! @brief Equivalent to the main function for many programs.  This is what starts the program
//! @param argc Your argc from your main function
//! @param argv Your array of strings from main
//!
gint 
w_application_run (WApplication *application, int *argc, char **argv[])
{
    w_application_parse_args (application, argc, argv);

    //Declarations
    WApplicationPrivate *priv;
    GError *error;
    int resolution;

    //Initializations
    resolution = 0;
    priv = application->priv;
    error = NULL;

    //User wants to see what dictionaries are available
    if (priv->arg_list_switch)
      w_console_list (application);

    //User wants to see the version of waei
    else if (priv->arg_version_switch)
      w_console_about (application);

    //User wants to install a dictionary
    else if (priv->arg_install_switch_data != NULL)
      resolution = w_console_install_dictinst (application, &error);

    //User wants to uninstall a dictionary
    else if (priv->arg_uninstall_switch_data != NULL)
      resolution = w_console_uninstall_dictinfo (application, &error);

    //User wants to do a search
    else if (priv->arg_query_text_data != NULL)
      resolution = w_console_search (application, &error);

    //User didn't specify enough information for an action
    else 
    {
      gchar *text = g_option_context_get_help (priv->context, FALSE, NULL);
      if (text != NULL)
      {
        printf("%s\n", text);
        g_free (text); text = NULL;
      }
    }

    //Cleanup
    w_application_handle_error (application, &error);

    return resolution;
}


gboolean
w_application_get_quiet_switch (WApplication *application)
{
  WApplicationPrivate *priv;
  priv = application->priv;
  return priv->arg_quiet_switch;
}


gboolean
w_application_get_exact_switch (WApplication *application)
{
  WApplicationPrivate *priv;
  priv = application->priv;
  return priv->arg_exact_switch;
}


gboolean
w_application_get_list_switch (WApplication *application)
{
  WApplicationPrivate *priv;
  priv = application->priv;
  return priv->arg_list_switch;
}


gboolean
w_application_get_version_switch (WApplication *application)
{
  WApplicationPrivate *priv;
  priv = application->priv;
  return priv->arg_version_switch;
}


gboolean
w_application_get_color_switch (WApplication *application)
{
  WApplicationPrivate *priv;
  priv = application->priv;
  return priv->arg_color_switch;
}


const gchar*
w_application_get_dictionary_switch_data (WApplication *application)
{
  WApplicationPrivate *priv;
  priv = application->priv;
  return priv->arg_dictionary_switch_data;
}


const gchar*
w_application_get_install_switch_data (WApplication *application)
{
  WApplicationPrivate *priv;
  priv = application->priv;
  return priv->arg_install_switch_data;
}


const gchar*
w_application_get_uninstall_switch_data (WApplication *application)
{
  WApplicationPrivate *priv;
  priv = application->priv;
  return priv->arg_uninstall_switch_data;
}


const gchar*
w_application_get_query_text_data (WApplication *application)
{
  WApplicationPrivate *priv;
  priv = application->priv;
  return priv->arg_query_text_data;
}

