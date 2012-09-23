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

#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gio/gio.h>

#include <gtk/gtk.h>

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include <gwaei/gettext.h>
#include <gwaei/gwaei.h>
#include <gwaei/application-private.h>

static void gw_application_attach_signals (GwApplication*);
static void gw_application_remove_signals (GwApplication*);
static void gw_application_activate (GApplication*);
static gboolean gw_application_local_command_line (GApplication*, gchar***, gint*);
static int gw_application_command_line (GApplication*, GApplicationCommandLine*);
static void gw_application_startup (GApplication*);
static void gw_application_load_app_menu (GwApplication*);
static void gw_application_load_menubar (GwApplication*);


G_DEFINE_TYPE (GwApplication, gw_application, GTK_TYPE_APPLICATION)

//!
//! @brief creates a new instance of the gwaei applicaiton
//!
GApplication* 
gw_application_new ()
{
    //Declarations
    GwApplication *application;
    const gchar *id;
    GApplicationFlags flags;

    //Initializations
    id = "gtk.org.gWaei";
    flags = G_APPLICATION_FLAGS_NONE;
    application = g_object_new (GW_TYPE_APPLICATION, 
                                "application-id", id, 
                                "flags", flags, NULL);

    return G_APPLICATION (application);
}


static void 
gw_application_init (GwApplication *application)
{
    application->priv = GW_APPLICATION_GET_PRIVATE (application);
    memset(application->priv, 0, sizeof(GwApplicationPrivate));
}

static void 
gw_application_constructed (GObject *object)
{
    //Chain the parent class
    {
      G_OBJECT_CLASS (gw_application_parent_class)->constructed (object);
    }

    lw_regex_initialize ();
    gw_application_initialize_accelerators (GW_APPLICATION (object));

/*
#ifdef OS_MINGW
    GtkSettings *settings;
    settings = gtk_settings_get_default ();
    if (settings != NULL)
    {
      g_object_set (settings, "gtk-theme-name", "Raleigh", NULL);
      g_object_set (settings, "gtk-menu-images", FALSE, NULL);
      g_object_set (settings, "gtk-button-images", FALSE, NULL);
      g_object_set (settings, "gtk-cursor-blink", FALSE, NULL);
      g_object_set (settings, "gtk-alternative-button-order", TRUE, NULL);
      g_object_unref (settings);
      settings = NULL;
    }
#endif
*/
}


static void 
gw_application_finalize (GObject *object)
{
    //Declarations
    GwApplication *application;
    GwApplicationPrivate *priv;

    application = GW_APPLICATION (object);
    priv = application->priv;

    gw_application_remove_signals (application);

    if (priv->error != NULL) g_error_free (priv->error); priv->error = NULL;

    if (priv->installable_dictionarylist != NULL) g_object_unref (priv->installable_dictionarylist); priv->installable_dictionarylist = NULL;

    if (priv->installed_dictionarylist != NULL) g_object_unref (priv->installed_dictionarylist); priv->installed_dictionarylist = NULL;

    if (priv->vocabularyliststore != NULL) g_object_unref (priv->vocabularyliststore); 

    if (priv->context != NULL) g_option_context_free (priv->context); priv->context = NULL;
    if (priv->arg_query != NULL) g_free(priv->arg_query); priv->arg_query = NULL;
    if (priv->preferences != NULL) lw_preferences_free (priv->preferences); priv->preferences = NULL;
#if WITH_MECAB
    if (lw_morphologyengine_has_default ()) 
    {
      lw_morphologyengine_free (lw_morphologyengine_get_default ()); 
    }
#endif

    lw_regex_free ();

    G_OBJECT_CLASS (gw_application_parent_class)->finalize (object);
}


static void
gw_application_class_init (GwApplicationClass *klass)
{
  GObjectClass *object_class;
  GApplicationClass *application_class;

  object_class = G_OBJECT_CLASS (klass);
  application_class = G_APPLICATION_CLASS (klass);

  object_class->constructed = gw_application_constructed;
  object_class->finalize = gw_application_finalize;
#ifndef OS_MINGW
  application_class->local_command_line = gw_application_local_command_line;
#endif
  application_class->command_line = gw_application_command_line;
  application_class->activate = gw_application_activate;
  application_class->startup = gw_application_startup;

  g_type_class_add_private (object_class, sizeof (GwApplicationPrivate));
}


static void 
gw_application_attach_signals (GwApplication *application)
{
#ifdef WITH_HUNSPELL
    //Sanity checks
    g_return_if_fail (application != NULL);

    //Declarations
    LwPreferences *preferences;

    //Initializations
    preferences = gw_application_get_preferences (application);

    lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_BASE,
        LW_KEY_SPELLCHECK,
        gw_application_sync_spellcheck_cb,
        application 
    );
#endif
}


static void 
gw_application_remove_signals (GwApplication *application)
{
}


//!
//! @brief Loads the arguments from the command line into the app instance
//!

void 
gw_application_parse_args (GwApplication *application, int *argc, char** argv[])
{
    GwApplicationPrivate *priv;

    priv = application->priv;

    //Reset the switches to their default state
    if (priv->arg_dictionary != NULL) g_free (priv->arg_dictionary);
    priv->arg_dictionary = NULL;
    if (priv->arg_query != NULL) g_free (priv->arg_query);
    priv->arg_query = NULL;
    priv->arg_version_switch = FALSE;
    priv->arg_new_vocabulary_window_switch = FALSE;

    GOptionEntry entries[] =
    {
      { "dictionary", 'd', 0, G_OPTION_ARG_STRING, &(priv->arg_dictionary), gettext("Choose the dictionary to use"), "English" },
      { "word", 'o', 0, G_OPTION_ARG_NONE, &(priv->arg_new_vocabulary_window_switch), gettext("Open the vocabulary manager window"), NULL },
      { "version", 'v', 0, G_OPTION_ARG_NONE, &(priv->arg_version_switch), gettext("Check the gWaei version information"), NULL },
      { NULL }
    };

    //Program flags setup
    GError *error = NULL;
    if (priv->context != NULL) g_option_context_free (priv->context);
    priv->context = g_option_context_new (gettext("- A dictionary program for Japanese-English translation."));
    g_option_context_add_main_entries (priv->context, entries, PACKAGE);
    g_option_context_set_ignore_unknown_options (priv->context, TRUE);
    g_option_context_parse (priv->context, argc, argv, &error);

    //g_log_set_always_fatal (G_LOG_LEVEL_WARNING);

    if (error != NULL)
    {
      gw_application_handle_error (application, NULL, FALSE, &error);
      exit(EXIT_SUCCESS);
    }

    //Get the query after the flags have been parsed out
    priv->arg_query = lw_util_get_query_from_args (*argc, *argv);
}


//!
//! @brief Prints to the terminal the about message for the program.
//!
void 
gw_application_print_about (GwApplication *application)
{
    const gchar *name;
    name = gw_application_get_program_name (GW_APPLICATION (application));

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


void 
gw_application_quit (GwApplication *application)
{
    gw_application_block_searches (application);

    GList *link;
    GtkListStore *liststore;
    gboolean has_changes;
    gboolean should_close;

    liststore = gw_application_get_vocabularyliststore (application);
    has_changes = gw_vocabularyliststore_has_changes (GW_VOCABULARYLISTSTORE (liststore));
    should_close = TRUE;

    if (has_changes)
    {
       link = gtk_application_get_windows (GTK_APPLICATION (application));
       while (link != NULL && GW_IS_VOCABULARYWINDOW (link->data) == FALSE) link = link->next;

       if (link != NULL)
       {
         should_close = gw_vocabularywindow_show_save_dialog (GW_VOCABULARYWINDOW (link->data));
       }
    }

    if (should_close)
    {
      link = gtk_application_get_windows (GTK_APPLICATION (application));
      while (link != NULL)
      {
        gtk_widget_destroy (GTK_WIDGET (link->data));
        link = gtk_application_get_windows (GTK_APPLICATION (application));
      }
    }

    gw_application_unblock_searches (application);
}


//!
//! @brief Returns the program name.  It should not be freed or modified
//! @returns A constanst string representing the program name
//!
const char* 
gw_application_get_program_name (GwApplication *application) 
{
  return gettext("gWaei Japanese-English Dictionary");
}


void 
gw_application_cancel_all_searches (GwApplication *application)
{
    GList *list;
    GList *iter;
    GtkWindow *window;

    list = gtk_application_get_windows (GTK_APPLICATION (application));

    for (iter = list; iter != NULL; iter = iter->next)
    {
      window = GTK_WINDOW (iter->data);
      if (window != NULL && G_OBJECT_TYPE (window) == GW_TYPE_SEARCHWINDOW)
        gw_searchwindow_cancel_all_searches (GW_SEARCHWINDOW (window));
    }
}


//!
//!  @brief Will attempt to get the window of the specified type which is most at the front
//!
GtkWindow* 
gw_application_get_window_by_type (GwApplication *application, const GType TYPE)
{
    //Declarations
    GList *iter;
    GList *list;
    GtkWindow *window;
    GtkWindow *active;
    GtkWindow *fuzzy;

    //Initializations
    list = gtk_application_get_windows (GTK_APPLICATION (application));
    window = NULL;
    fuzzy = NULL;
    active = NULL;

    for (iter = list; iter != NULL; iter = iter->next)
    {
      fuzzy = GTK_WINDOW (iter->data);
      active = GTK_WINDOW (iter->data);

      if (fuzzy == NULL)
      {
        continue;
      }
      if (G_OBJECT_TYPE (active) == TYPE && gtk_window_is_active (active))
      {
        window = active;
        break;
      }
      if (G_OBJECT_TYPE (fuzzy) == TYPE)
      {
        window = fuzzy;
      }
    }

    return window;
}


void 
gw_application_block_searches (GwApplication *application)
{
    GwApplicationPrivate *priv;

    priv = application->priv;

    priv->block_new_searches++;
    gw_application_cancel_all_searches (application);
}


void 
gw_application_unblock_searches (GwApplication *application)
{
    GwApplicationPrivate *priv;

    priv = application->priv;

    if (priv->block_new_searches > 0)
      priv->block_new_searches--;
}

gboolean 
gw_application_can_start_search (GwApplication *application)
{
    GwApplicationPrivate *priv;

    priv = application->priv;

    return (priv->block_new_searches == 0);
}


void 
gw_application_set_error (GwApplication *application, GError *error)
{
  GwApplicationPrivate *priv;
  priv = application->priv;
  if (priv->error != NULL) g_error_free (priv->error);
  priv->error = error;
}


gboolean 
gw_application_has_error (GwApplication *application)
{
  GwApplicationPrivate *priv;
  priv = application->priv;
  return (priv->error != NULL);
}


void 
gw_application_handle_error (GwApplication *application, GtkWindow *transient_for, gboolean show_dialog, GError **error)
{
    GwApplicationPrivate *priv;
    priv = application->priv;

    //If error is null, we check if the GwApplication had an internal error set
    if (error == NULL) error = &(priv->error);

    //Sanity checks
    if (error == NULL || *error == NULL) return;

    //Declarations
    GtkWidget *dialog;

    //Handle the error
    if (show_dialog)
    {
      dialog = gtk_message_dialog_new_with_markup (transient_for,
                                                   GTK_DIALOG_MODAL,
                                                   GTK_MESSAGE_ERROR,
                                                   GTK_BUTTONS_CLOSE,
                                                   "<b>%s</b>\n\n%s",
                                                   "An Error Occured",
                                                   (*error)->message
                                                  );
      g_signal_connect_swapped (dialog, "response", G_CALLBACK (gtk_widget_destroy), dialog);
      gtk_widget_show_all (GTK_WIDGET (dialog));
      gtk_dialog_run (GTK_DIALOG (dialog));
    }
    else
    {
      fprintf(stderr, "ERROR: %s\n", (*error)->message);
    }

    //Cleanup
    g_error_free (*error);
    *error = NULL;
}


void 
gw_application_set_last_focused_searchwindow (GwApplication *application, GwSearchWindow *window)
{
   GwApplicationPrivate *priv;

   priv = application->priv;

   priv->last_focused = window; 
}


GwSearchWindow* 
gw_application_get_last_focused_searchwindow (GwApplication *application)
{
   GwApplicationPrivate *priv;
   GwSearchWindow *window;

   priv = application->priv;
   window = GW_SEARCHWINDOW (gw_application_get_window_by_type (application, GW_TYPE_SEARCHWINDOW));

   if (window != NULL && priv->last_focused != NULL)
     window = priv->last_focused;

   return window;
}


LwPreferences* 
gw_application_get_preferences (GwApplication *application)
{
    GwApplicationPrivate *priv;

    priv = application->priv;

    if (priv->preferences == NULL)
    {
      priv->preferences = lw_preferences_new (NULL);
    }

    return priv->preferences;
}


GwDictionaryList* 
gw_application_get_installed_dictionarylist (GwApplication *application)
{
    //Sanity checks
    g_return_val_if_fail (application != NULL, NULL);

    //Declarations
    GwApplicationPrivate *priv;
    LwPreferences *preferences;
    GwDictionaryList *dictionarylist;
    gpointer *pointer;

    //Initializations;
    priv = application->priv;

    if (priv->installed_dictionarylist == NULL)
    {
      dictionarylist = gw_dictionarylist_new ();
      preferences = gw_application_get_preferences (application);
      lw_dictionarylist_load_installed (LW_DICTIONARYLIST (dictionarylist));
      lw_dictionarylist_load_order (LW_DICTIONARYLIST (dictionarylist), preferences);
      
      priv->installed_dictionarylist = dictionarylist;
      pointer = (gpointer*) &(priv->installed_dictionarylist);
      g_object_add_weak_pointer (G_OBJECT (priv->installed_dictionarylist), pointer);
    }

    return priv->installed_dictionarylist;
}


GwDictionaryList* 
gw_application_get_installable_dictionarylist (GwApplication *application)
{
    //Sanity checks
    g_return_val_if_fail (application != NULL, NULL);

    //Declarations
    GwApplicationPrivate *priv;
    LwPreferences *preferences;
    GwDictionaryList *dictionarylist;
    gpointer *pointer;

    //Initializations
    priv = application->priv;

    if (priv->installable_dictionarylist == NULL)
    {
      dictionarylist = gw_dictionarylist_new ();
      preferences = gw_application_get_preferences (application);
      lw_dictionarylist_load_installable (LW_DICTIONARYLIST (dictionarylist), preferences);

      priv->installable_dictionarylist = dictionarylist;
      pointer = (gpointer*) &(priv->installable_dictionarylist);
      g_object_add_weak_pointer (G_OBJECT (priv->installable_dictionarylist), pointer);
    }

    return priv->installable_dictionarylist;
}


GtkListStore*
gw_application_get_vocabularyliststore (GwApplication *application)
{
  GwApplicationPrivate *priv;
  LwPreferences *preferences;
  gpointer* pointer;

  priv = application->priv;

  if (priv->vocabularyliststore == NULL)
  {
    preferences = gw_application_get_preferences (application);
    priv->vocabularyliststore = gw_vocabularyliststore_new ();
    pointer = (gpointer*) &(priv->vocabularyliststore);
    g_object_add_weak_pointer (G_OBJECT (priv->vocabularyliststore), pointer);
    gw_vocabularyliststore_load_list_order (GW_VOCABULARYLISTSTORE (priv->vocabularyliststore), preferences);
  }

  return priv->vocabularyliststore;
}


static void 
gw_application_activate (GApplication *application)
{
    GwApplicationPrivate *priv;
    GwSearchWindow *searchwindow;
    GwVocabularyWindow *vocabularywindow;
    GwSettingsWindow *settingswindow;
    GwDictionaryList *dictionarylist;

    priv = GW_APPLICATION (application)->priv;
    dictionarylist = gw_application_get_installed_dictionarylist (GW_APPLICATION (application));

    if (priv->arg_new_vocabulary_window_switch)
    {
      vocabularywindow = GW_VOCABULARYWINDOW (gw_vocabularywindow_new (GTK_APPLICATION (application)));
      gtk_widget_show (GTK_WIDGET (vocabularywindow));
      return;
    }

    else
    {
      searchwindow = GW_SEARCHWINDOW (gw_searchwindow_new (GTK_APPLICATION (application)));
      gtk_widget_show (GTK_WIDGET (searchwindow));

      if (lw_dictionarylist_get_total (LW_DICTIONARYLIST (dictionarylist)) == 0)
      {
        settingswindow = GW_SETTINGSWINDOW (gw_settingswindow_new (GTK_APPLICATION (application)));
        gtk_window_set_transient_for (GTK_WINDOW (settingswindow), GTK_WINDOW (searchwindow));
        gtk_widget_show (GTK_WIDGET (settingswindow));
      }
      return;
    }
}


static int 
gw_application_command_line (GApplication *application, GApplicationCommandLine *command_line)
{
    //Declarations
    LwDictionary *dictionary;
    GwSearchWindow *window;
    GwDictionaryList *dictionarylist;
    GwApplicationPrivate *priv;
    gint argc;
    gchar **argv;
    gint position;

    //Initializations
    priv = GW_APPLICATION (application)->priv;
    dictionarylist = gw_application_get_installed_dictionarylist (GW_APPLICATION (application));
    argv = NULL;

    if (command_line != NULL)
    {
      argv = g_application_command_line_get_arguments (command_line, &argc);

      gw_application_parse_args (GW_APPLICATION (application), &argc, &argv);
    }
    g_application_activate (G_APPLICATION (application));
    window = gw_application_get_last_focused_searchwindow (GW_APPLICATION (application));
    if (window == NULL) 
      return 0;
    dictionary = lw_dictionarylist_get_dictionary_fuzzy (LW_DICTIONARYLIST (dictionarylist), priv->arg_dictionary);
    position = lw_dictionarylist_get_position (LW_DICTIONARYLIST (dictionarylist), dictionary);

    //Set the initial dictionary
    if (dictionary != NULL)
    {
      gw_searchwindow_set_dictionary (window, position);
    }

    //Set the initial query text if it was passed as an argument to the program
    if (priv->arg_query != NULL)
    {
      gw_searchwindow_entry_set_text (window, priv->arg_query);
      gw_searchwindow_search_cb (GTK_WIDGET (window), window);
    }

    //Cleanup
    if (argv != NULL) g_strfreev (argv); argv = NULL;

    return 0;
}


static gboolean 
gw_application_local_command_line (GApplication *application, 
                                   gchar ***argv, gint *exit_status)
{
    //Declarations
    int argc;
    gboolean handled;
    int i;

    //Initializations
    argc = g_strv_length (*argv);
    handled = FALSE;

    for (i = 0; (*argv)[i] != NULL; i++)
    {
      if (strcmp((*argv)[i], "-v") == 0 || strcmp((*argv)[i], "--version") == 0)
      {
        gw_application_print_about (GW_APPLICATION (application));
        handled = TRUE;
        break;
      }
      else if (strcmp((*argv)[i], "-h") == 0 || strcmp((*argv)[i], "--help") == 0)
      {
        handled = TRUE;
        gw_application_parse_args (GW_APPLICATION (application), &argc, argv);
        break;
      }
    }

    return handled;
} 


static void
gw_application_startup (GApplication *application)
{
    G_APPLICATION_CLASS (gw_application_parent_class)->startup (application);

    gw_application_load_app_menu (GW_APPLICATION (application));
    gw_application_load_menubar (GW_APPLICATION (application));

    gw_application_attach_signals (GW_APPLICATION (application));
}


gboolean
gw_application_should_quit (GwApplication *application)
{
    GList *windowlist;
    GList *link;
    gboolean should_quit;

    windowlist = gtk_application_get_windows (GTK_APPLICATION (application));
    should_quit = TRUE;

    for (link = windowlist; should_quit && link != NULL; link = link->next)
      if (gw_window_is_important (GW_WINDOW (link->data))) should_quit = FALSE;

    return should_quit;
}


static void
gw_application_initialize_menumodel_links (GwApplication *application)
{
    //Sanity checks
    g_return_if_fail (application != NULL);
  
    //Declarations
    GwVocabularyListStore *store;
    GMenuModel *menumodel;
    GMenuModel *link;

    //Initializations
    menumodel = gtk_application_get_app_menu (GTK_APPLICATION (application));
    g_return_if_fail (menumodel != NULL);

    store = GW_VOCABULARYLISTSTORE (gw_application_get_vocabularyliststore (application));
    link = gw_vocabularyliststore_get_menumodel (store);
    gw_menumodel_set_links (menumodel, "vocabulary-list-link", gettext ("Vocabulary"), G_MENU_LINK_SECTION, link);
}


static void
gw_application_load_app_menu (GwApplication *application)
{
    //Sanity checks
    g_return_if_fail (application != NULL);

    //Declarations
    GtkBuilder *builder;
    GMenuModel *model;
    GtkSettings *settings;
    gboolean loaded;
    gboolean os_shows_app_menu;
    gboolean os_shows_win_menu;
    const gchar *filename;

    //Initializations
    builder = NULL;
    model = NULL;
    loaded = FALSE;
    settings = gtk_settings_get_default ();
    g_object_get (settings, "gtk-shell-shows-app-menu", &os_shows_app_menu, NULL);
    g_object_get (settings, "gtk-shell-shows-menubar", &os_shows_win_menu, NULL);

    gw_application_map_actions (G_ACTION_MAP (application), application);

    if (os_shows_app_menu && os_shows_win_menu) //Mac OS X style
      filename = "application-menumodel-macosx.ui";
    else if (os_shows_app_menu != os_shows_win_menu) //Gnome 3 style
      filename = "application-menumodel-gnome.ui";
    else //Windows style
      filename = NULL;


    if (filename == NULL) goto errored;

    builder = gtk_builder_new (); if (builder == NULL) goto errored;
    loaded = gw_application_load_xml (builder, filename); if (loaded == FALSE) goto errored;
    model = G_MENU_MODEL (gtk_builder_get_object (builder, "menu")); if (model == NULL) goto errored;

    gtk_application_set_app_menu (GTK_APPLICATION (application), model);
    gw_application_initialize_menumodel_links (application);

errored:
    if (builder != NULL) g_object_unref (builder);
}


static void
gw_application_load_menubar (GwApplication *application)
{
    GMenuModel *menumodel;
    menumodel = G_MENU_MODEL (g_menu_new ());
    gtk_application_set_menubar (GTK_APPLICATION (application), menumodel);
}


gboolean
gw_application_load_xml (GtkBuilder *builder, const gchar *FILENAME)
{
    //Declarations
    gint i;
    gchar *path;
    const gint TOTAL_PATHS = 4;
    gchar *paths[TOTAL_PATHS];
    gboolean file_exists;
    gboolean is_valid_xml;
    GError *error;

    //Initializations
    is_valid_xml = FALSE;
    file_exists = FALSE;
    paths[0] = g_build_filename (FILENAME, NULL);
    paths[1] = g_build_filename ("..", "share", PACKAGE, FILENAME, NULL);
    paths[2] = g_build_filename (DATADIR2, PACKAGE, FILENAME, NULL);
    paths[3] = NULL;
    error = NULL;

    //Search for the files
    for (i = 0; i < TOTAL_PATHS; i++)
    {
      path = paths[i];

      file_exists = g_file_test (path, G_FILE_TEST_IS_REGULAR);
      if (file_exists == FALSE)
        continue;

      is_valid_xml = gtk_builder_add_from_file (builder, path,  &error);
      if (error != NULL) 
      {
        g_warning ("Problems loading xml from %s. %s\n", path, error->message);
        g_error_free (error); error = NULL;
        continue;
      }
      if (is_valid_xml == FALSE) 
        continue;

      gtk_builder_connect_signals (builder, NULL);
      break;
    }

    //Cleanup
    for (i = 0; i < TOTAL_PATHS; i++)
    {
      g_free (paths[i]); paths[i] = NULL;
    }

    return (file_exists && is_valid_xml);
}


void
gw_application_map_actions (GActionMap *map, GwApplication *application)
{
    //Sanity checks
    g_return_if_fail (map != NULL);
    g_return_if_fail (application != NULL);

    static GActionEntry entries[] = {
      { "new-window", gw_application_open_searchwindow_cb, NULL, NULL, NULL },
      { "show-about", gw_application_open_aboutdialog_cb, NULL, NULL, NULL },
      { "show-preferences", gw_application_open_settingswindow_cb, NULL, NULL, NULL },
      { "show-vocabulary", gw_application_open_vocabularywindow_cb, NULL, NULL, NULL },
      { "show-vocabulary-index", gw_application_open_vocabularywindow_index_cb, "s", NULL, NULL },
      { "show-help", gw_application_open_help_cb, NULL, NULL, NULL },
      { "show-glossary", gw_application_open_glossary_cb, NULL, NULL, NULL },
#ifdef WITH_HUNSPELL
      { "toggle-spellcheck", gw_application_spellcheck_toggled_cb, NULL, "false", NULL },
#endif
      { "quit", gw_application_quit_cb, NULL, NULL, NULL }
    };

    g_action_map_add_action_entries (map, entries, G_N_ELEMENTS (entries), application);
}


void
gw_menumodel_set_links (GMenuModel *menumodel, const gchar *LABEL, const gchar *NEW_LABEL, const gchar *LINK_TYPE, GMenuModel *link)
{
    //Sanity checks
    g_return_if_fail (menumodel != NULL);
    g_return_if_fail (LABEL != NULL);
    g_return_if_fail (LINK_TYPE != NULL);

    //Declarations
    gint total_items;
    gint index;
    gchar *label;
    gboolean valid;
    GMenuItem *menuitem;
    GMenuModel *sublink;

    //Initializations
    total_items = g_menu_model_get_n_items (menumodel);

    for (index = 0; index < total_items; index++)
    {
      valid = g_menu_model_get_item_attribute (menumodel, index, G_MENU_ATTRIBUTE_LABEL, "s", &label, NULL);
      if (valid == TRUE && label != NULL)
      {
        if (label != NULL && strcmp (label, LABEL) == 0)
        {
          menuitem = g_menu_item_new (NEW_LABEL, NULL);
          g_menu_item_set_link (menuitem, LINK_TYPE, link);
          g_menu_remove (G_MENU (menumodel), index);
          g_menu_insert_item (G_MENU (menumodel), index, menuitem);
          g_object_unref (menuitem); menuitem = NULL;
        }
        g_free (label); label = NULL;
      }

      //Recursive work
      sublink = g_menu_model_get_item_link (menumodel, index, G_MENU_LINK_SUBMENU);
      if (sublink != NULL) gw_menumodel_set_links (sublink, LABEL, NEW_LABEL, LINK_TYPE, link);
      sublink = g_menu_model_get_item_link (menumodel, index, G_MENU_LINK_SECTION);
      if (sublink != NULL) gw_menumodel_set_links (sublink, LABEL, NEW_LABEL, LINK_TYPE, link);
    }
}


void
gw_application_add_accelerators (GwApplication *application, GMenuModel *menumodel)
{
    //Sanity checks
    g_return_if_fail (application != NULL);
    g_return_if_fail (menumodel != NULL);

    //Declarations
    gint total_items;
    gint index;
    gchar *accel = NULL;
    gchar *action = NULL;
    gchar *detail = NULL;
    GMenuModel *sublink = NULL;

    //Initializations
    total_items = g_menu_model_get_n_items (menumodel);

    for (index = 0; index < total_items; index++)
    {
      g_menu_model_get_item_attribute (menumodel, index, "accel", "s", &accel, NULL);
      g_menu_model_get_item_attribute (menumodel, index, G_MENU_ATTRIBUTE_ACTION, "s", &action, NULL);
      g_menu_model_get_item_attribute (menumodel, index, G_MENU_ATTRIBUTE_TARGET, "s", &detail, NULL);
      if (accel != NULL && action != NULL)
      {
        if (detail != NULL)
          gtk_application_add_accelerator (GTK_APPLICATION (application), accel, action, g_variant_new_string (detail));
        else
          gtk_application_add_accelerator (GTK_APPLICATION (application), accel, action, NULL);
      }

      if (accel != NULL) g_free (accel); accel = NULL;
      if (action != NULL) g_free (action); action = NULL;
      if (detail != NULL) g_free (detail); detail = NULL;

      //Recursive work
      sublink = g_menu_model_get_item_link (menumodel, index, G_MENU_LINK_SUBMENU);
      if (sublink != NULL) gw_application_add_accelerators (application, sublink);
      sublink = g_menu_model_get_item_link (menumodel, index, G_MENU_LINK_SECTION);
      if (sublink != NULL) gw_application_add_accelerators (application, sublink);
    }
}


void
gw_application_remove_accelerators (GwApplication *application, GMenuModel *menumodel)
{
    //Sanity checks
    g_return_if_fail (application != NULL);
    g_return_if_fail (menumodel != NULL);

    //Declarations
    gint total_items;
    gint index;
    gchar *accel = NULL;
    gchar *action = NULL;
    gchar *detail = NULL;
    GMenuModel *sublink = NULL;

    //Initializations
    total_items = g_menu_model_get_n_items (menumodel);

    for (index = 0; index < total_items; index++)
    {
      g_menu_model_get_item_attribute (menumodel, index, "accel", "s", &accel, NULL);
      g_menu_model_get_item_attribute (menumodel, index, G_MENU_ATTRIBUTE_ACTION, "s", &action, NULL);
      g_menu_model_get_item_attribute (menumodel, index, G_MENU_ATTRIBUTE_TARGET, "s", &detail, NULL);
      if (accel != NULL && action != NULL)
      {
        if (detail != NULL)
          gtk_application_remove_accelerator (GTK_APPLICATION (application), action, g_variant_new_string (detail));
        else
          gtk_application_remove_accelerator (GTK_APPLICATION (application), action, NULL);
      }

      if (accel != NULL) g_free (accel); accel = NULL;
      if (action != NULL) g_free (action); action = NULL;
      if (detail != NULL) g_free (detail); detail = NULL;

      //Recursive work
      sublink = g_menu_model_get_item_link (menumodel, index, G_MENU_LINK_SUBMENU);
      if (sublink != NULL) gw_application_remove_accelerators (application, sublink);
      sublink = g_menu_model_get_item_link (menumodel, index, G_MENU_LINK_SECTION);
      if (sublink != NULL) gw_application_remove_accelerators (application, sublink);
    }
}


void
gw_application_show_vocabularywindow (GwApplication *application, gint index)
{
    //Declarations
    GtkWindow *window;
    GwVocabularyWindow *vocabularywindow;
    GtkWidget *widget;

    //Initializations
    window = gw_vocabularywindow_new (GTK_APPLICATION (application));
    vocabularywindow = GW_VOCABULARYWINDOW (window);
    widget = GTK_WIDGET (window);

    if (index >= 0)
    {
      gw_vocabularywindow_set_selected_list_by_index (vocabularywindow, index);
      gw_vocabularywindow_show_vocabulary_list (vocabularywindow, FALSE);
    }

    gtk_widget_show (widget);
}


void
gw_application_set_win_menubar (GwApplication *application, GMenuModel *menumodel)
{
    //Sanity checks
    g_return_if_fail (application != NULL);
    g_return_if_fail (menumodel != NULL);
    if (g_menu_model_get_n_items (menumodel) == 0) return;

    //Declarations
    GMenuModel *menubar;
    gint length;

    //Initializations
    menubar = gtk_application_get_menubar (GTK_APPLICATION (application));
    g_return_if_fail (menubar != NULL);
    length = g_menu_model_get_n_items (menubar);

    //Clear the menubar
    while (length-- > 0) g_menu_remove (G_MENU (menubar), 0);

    //Add the menuitem linking the menus 
    {
      GMenuItem *menuitem = g_menu_item_new_section (NULL, menumodel);
      if (menuitem != NULL)
      {
        g_menu_append_item (G_MENU (menubar), menuitem);
        g_object_unref (menuitem); menuitem = NULL;
      }
      gw_application_add_accelerators (application, menubar);
    }
}


void
gw_application_initialize_accelerators (GwApplication *application)
{
    //Sanity checks
    g_return_if_fail (application != NULL);

    //Declarations
    gchar *accel;
    gchar *action;
    gchar *detail;
    gint index;

    //Initializations
    index = 1;

    while (index + 1 < 10)
    {
      accel = g_strdup_printf ("<Alt>%d", index);
      action = g_strdup_printf ("win.set-dictionary");
      detail = g_strdup_printf ("%d", index);
      if (accel != NULL && action != NULL && detail != NULL)
      {
        gtk_application_add_accelerator (GTK_APPLICATION (application), accel, action, g_variant_new_string (detail));
        index++;
      }
      if (accel != NULL) g_free (accel); accel = NULL;
      if (action != NULL) g_free (action); action = NULL;
      if (detail != NULL) g_free (detail); detail = NULL;
    }
}

