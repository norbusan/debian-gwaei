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
//! @file application-callbacks.c
//!
//! @brief To be written
//!

#include <string.h>
#include <stdlib.h>

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include <gtk/gtk.h>

#include <gwaei/gettext.h>
#include <gwaei/gwaei.h>
#include <gwaei/application-private.h>


G_MODULE_EXPORT void 
gw_application_open_searchwindow_cb (GSimpleAction *action, 
                                     GVariant      *parameter, 
                                     gpointer       data)
{
    //Declarations
    GwApplication *application;
    GtkWindow *window;

    //Initializations
    application = GW_APPLICATION (data);
    g_return_if_fail (application != NULL);
    window = gw_searchwindow_new (GTK_APPLICATION (application));

    gtk_widget_show (GTK_WIDGET (window));
}


void 
gw_application_quit_cb (GSimpleAction *action,
                        GVariant      *parameter,
                        gpointer       data)
{
    gw_application_quit (GW_APPLICATION (data));
}


void 
gw_application_open_settingswindow_cb (GSimpleAction *action,
                                       GVariant      *parameter,
                                       gpointer       data)
{
    //Declarations
    GwApplication *application;
    GwSearchWindow *searchwindow;
    GtkWindow *settingswindow;
    GList *link;

    //Initializations
    searchwindow = GW_SEARCHWINDOW (gw_application_get_last_focused_searchwindow (GW_APPLICATION (data)));
    application = gw_window_get_application (GW_WINDOW (searchwindow));
    link = gtk_application_get_windows (GTK_APPLICATION (application));

    while (link != NULL && !GW_IS_SETTINGSWINDOW (link->data)) link = link->next;

    if (link != NULL)
    {
      settingswindow = GTK_WINDOW (link->data);
      gtk_window_set_transient_for (GTK_WINDOW (settingswindow), GTK_WINDOW (searchwindow));
      gtk_window_present (GTK_WINDOW (settingswindow));
    }
    else
    {
      settingswindow = gw_settingswindow_new (GTK_APPLICATION (application));
      gtk_window_set_transient_for (GTK_WINDOW (settingswindow), GTK_WINDOW (searchwindow));
      gtk_widget_show (GTK_WIDGET (settingswindow));
    }
}


G_MODULE_EXPORT void 
gw_application_open_vocabularywindow_cb (GSimpleAction *action, 
                                         GVariant      *parameter,
                                         gpointer       data)
{
    GwApplication *application;

    application = GW_APPLICATION (data);

    gw_application_show_vocabularywindow (application, -1);
}


G_MODULE_EXPORT void 
gw_application_open_vocabularywindow_index_cb (GSimpleAction *action, 
                                               GVariant      *parameter,
                                               gpointer       data)
{
    GwApplication *application;
    const gchar *value;
    gint index;

    application = GW_APPLICATION (data);
    value = g_variant_get_string (parameter, NULL);
    index = (gint) g_ascii_strtoll (value, NULL, 10);

    gw_application_show_vocabularywindow (application, index);
}


//!
//! @brief Opens the gWaei about dialog
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_application_open_aboutdialog_cb (GSimpleAction *action, 
                                    GVariant      *parameter,
                                    gpointer       data)
{
    gchar *global_path = DATADIR2 G_DIR_SEPARATOR_S PACKAGE G_DIR_SEPARATOR_S "logo.png";
    gchar *local_path = ".." G_DIR_SEPARATOR_S "share" G_DIR_SEPARATOR_S PACKAGE G_DIR_SEPARATOR_S "logo.png";

    gchar *programmer_credits[] = 
    {
      "Zachary Dovel <pizzach@gmail.com>",
      "Fabrizio Sabatini",
      NULL
    };

    GdkPixbuf *logo;
    if ( (logo = gdk_pixbuf_new_from_file (global_path,    NULL)) == NULL &&
         (logo = gdk_pixbuf_new_from_file (local_path, NULL)) == NULL    )
    {
      printf ("Was unable to load the gwaei logo.\n");
    }

    GtkWidget *about = g_object_new (GTK_TYPE_ABOUT_DIALOG,
               "program-name", "gWaei", 
               "version", VERSION,
               "copyright", "gWaei (C) 2008-2012 Zachary Dovel\n" 
                            "Kanjipad backend (C) 2002 Owen Taylor\n"
                            "JStroke backend (C) 1997 Robert Wells\n"
                            "Dedicated to Chuus (gl :-O)",
               "comments", gettext("Program for Japanese translation and reference. The\n"
                                    "dictionaries are supplied by Jim Breen's WWWJDIC.\n"
                                    "Special thanks to the maker of GJITEN who served as an inspiration."),
               "license", "This software is GPL Licensed.\n\n"
                          "gWaei is free software: you can redistribute it and/or modify\n"
                          "it under the terms of the GNU General Public License as published by\n "
                          "the Free Software Foundation, either version 3 of the License, or\n"
                          "(at your option) any later version.\n\n"
                          "gWaei is distributed in the hope that it will be useful,\n"
                          "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
                          "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
                          "GNU General Public License for more details.\n\n"
                          "You should have received a copy of the GNU General Public License\n"
                          "along with gWaei.  If not, see <http://www.gnu.org/licenses/>.",
               "logo", logo,
               // TRANSLATORS: You can add your own name to the translation of this field, it will be displayed in the "about" box when gwaei is run in your language
               "translator-credits", gettext("translator-credits"),
               "authors", programmer_credits,
               "website", "http://gwaei.sourceforge.net/",
               NULL);
    gtk_dialog_run (GTK_DIALOG (about));
    g_object_unref (logo);
    gtk_widget_destroy (about);
}


//!
//! @brief Sends the user to the gWaei irc channel for help
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_application_open_irc_channel_cb (GSimpleAction *action, 
                                    GVariant      *parameter,
                                    gpointer       data)
{
    //Initializations
    GError *error;
    GwSearchWindow *window;
    GwApplication *application;

    //Declarations
    error = NULL;
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    application = gw_window_get_application (GW_WINDOW (window));

    gtk_show_uri (NULL, "irc://irc.freenode.net/gWaei", gtk_get_current_event_time (), &error);

    //Cleanup
    gw_application_handle_error (application, GTK_WINDOW (window), TRUE, &error);
}


//!
//! @brief Sends the user to the gWaei homepage for whatever they need
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_application_open_homepage_cb (GSimpleAction *action, 
                                 GVariant      *parameter,
                                 gpointer       data)
{
    //Declarations
    GError *error;
    GwSearchWindow *window;
    GwApplication *application;

    //Initializations
    error = NULL;
    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    g_return_if_fail (window != NULL);
    application = gw_window_get_application (GW_WINDOW (window));

    gtk_show_uri (NULL, "http://gwaei.sourceforge.net/", gtk_get_current_event_time (), &error);

    //Cleanup
    gw_application_handle_error (application, GTK_WINDOW (window), TRUE, &error);
}


//!
//! @brief Opens the gWaei help documentation
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_application_open_help_cb (GSimpleAction *action,
                             GVariant      *parameter,
                             gpointer       data)
{
    gtk_show_uri (NULL, "ghelp:gwaei", gtk_get_current_event_time (), NULL);
}


//!
//! @brief Opens the gWaei dictionary glossary help documentation
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_application_open_glossary_cb (GSimpleAction *action,
                                 GVariant      *parameter,
                                 gpointer       data)
{
    //Declarations
    gchar *uri;
    GError *error;
    GwApplication *application;

    //Initializations
    application = GW_APPLICATION (data);
    uri = g_build_filename ("ghelp://", DATADIR2, "gnome", "help", "gwaei", "C", "glossary.xml", NULL);
    error = NULL;
    
    if (uri != NULL)
    {
      gtk_show_uri (NULL, uri, gtk_get_current_event_time (), &error);
      g_free (uri); uri = NULL;
    }

    gw_application_handle_error (application, NULL, FALSE, &error);
}


#ifdef WITH_HUNSPELL
//!
//! @brief Callback to toggle spellcheck in the search entry
//! @param widget Unused pointer to a GtkWidget
//! @param data Unused gpointer
//!
void 
gw_application_spellcheck_toggled_cb (GSimpleAction *action, 
                                      GVariant      *parameter, 
                                      gpointer       data)
{
    //Declarations
    GwApplication *application;
    LwPreferences *preferences;
    gboolean state;

    //Initializations
    application = GW_APPLICATION (data);
    preferences = gw_application_get_preferences (application);
    state = lw_preferences_get_boolean_by_schema (preferences, LW_SCHEMA_BASE, LW_KEY_SPELLCHECK);

    lw_preferences_set_boolean_by_schema (preferences, LW_SCHEMA_BASE, LW_KEY_SPELLCHECK, !state);
}

void
gw_application_sync_spellcheck_cb (GSettings *settings,
                                   gchar     *key,
                                   gpointer   data)
{
    //Declarations
    GwApplication *application;
    gboolean state;
    GAction *action;

    //Initializations
    application = GW_APPLICATION (data);
    state = lw_preferences_get_boolean (settings, key);
    action = g_action_map_lookup_action (G_ACTION_MAP (application), "toggle-spellcheck");

    if (action != NULL) g_simple_action_set_state (G_SIMPLE_ACTION (action), g_variant_new_boolean (state));
}
#endif


