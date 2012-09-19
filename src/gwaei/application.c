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

#include <gio/gio.h>

#include <gtk/gtk.h>

#include <gwaei/gwaei.h>
#include <gwaei/application-private.h>

static void gw_application_attach_signals (GwApplication*);
static void gw_application_remove_signals (GwApplication*);
static void gw_application_activate (GApplication*);
static gboolean gw_application_local_command_line (GApplication*, gchar***, gint*);
static int gw_application_command_line (GApplication*, GApplicationCommandLine*);

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
    //Declarations
    GwApplication *application;

    //Chain the parent class
    {
      G_OBJECT_CLASS (gw_application_parent_class)->constructed (object);
    }

    //Initialization
    application = GW_APPLICATION (object);

    lw_regex_initialize ();

#ifdef OS_MINGW
/*
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
*/
#endif

    gw_application_attach_signals (application);
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

    if (priv->dictinstlist != NULL) lw_dictinstlist_free (priv->dictinstlist); priv->dictinstlist = NULL;

    if (priv->dictionarystore != NULL) g_object_unref (priv->dictionarystore); 

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

  g_type_class_add_private (object_class, sizeof (GwApplicationPrivate));
}


static void 
gw_application_attach_signals (GwApplication *application)
{
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
    priv->arg_new_window_switch = FALSE;
    if (priv->arg_dictionary != NULL) g_free (priv->arg_dictionary);
    priv->arg_dictionary = NULL;
    if (priv->arg_query != NULL) g_free (priv->arg_query);
    priv->arg_query = NULL;
    priv->arg_version_switch = FALSE;
    priv->arg_new_vocabulary_window_switch = FALSE;

    GOptionEntry entries[] =
    {
      { "new", 'n', 0, G_OPTION_ARG_NONE, &(priv->arg_new_window_switch), gettext("Force a new instance window"), NULL },
      { "dictionary", 'd', 0, G_OPTION_ARG_STRING, &(priv->arg_dictionary), gettext("Choose the dictionary to use"), "English" },
      { "vocabulary", 'o', 0, G_OPTION_ARG_NONE, &(priv->arg_new_vocabulary_window_switch), gettext("Open the vocabulary manager window"), NULL },
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


//!
//! @brief Gets a GwWindow from the application's windowlist
//! @param app A GwApplication instance to work on
//! @param TYPE The window type to get
//! @param widget A widget from the window so you can get a specific instance.  If NULL, you cet the first window to match the GwWindowType
//!
GtkWindow* 
gw_application_get_window_by_widget (GwApplication *application, GtkWidget *widget)
{
    //Declarations
    GList *iter;
    GList *list;
    GtkWindow *window;
    GtkWindow *fuzzy;
    GtkWindow *toplevel;

    //Initializations
    window = NULL;
    fuzzy = NULL;
    toplevel = GTK_WINDOW (gtk_widget_get_toplevel (widget));
    list = gtk_application_get_windows (GTK_APPLICATION (application));

    for (iter = list; iter != NULL; iter = iter->next)
    {
      fuzzy = GTK_WINDOW (iter->data);

      if (fuzzy == NULL)
      {
        continue;
      }
      else if (fuzzy == toplevel)
      {
        window = fuzzy;
        break;
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


GtkListStore* 
gw_application_get_dictionarystore (GwApplication *application)
{
    GwApplicationPrivate *priv;
    LwPreferences *preferences;
    gpointer *pointer;

    priv = application->priv;

    if (priv->dictionarystore == NULL)
    {
      priv->dictionarystore = gw_dictionarystore_new ();
      pointer = (gpointer*) &(priv->dictionarystore);
      preferences = gw_application_get_preferences (application);
      gw_dictionarystore_load_order (GW_DICTIONARYSTORE (priv->dictionarystore), preferences);
      g_object_add_weak_pointer (G_OBJECT (priv->dictionarystore), pointer);
    }

    return priv->dictionarystore;
}


LwDictInstList* 
gw_application_get_dictinstlist (GwApplication *application)
{
    GwApplicationPrivate *priv;
    priv = application->priv;

    if (priv->dictinstlist == NULL)
      priv->dictinstlist = lw_dictinstlist_new (priv->preferences);

    return priv->dictinstlist;
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
    GwDictionaryStore *dictionarystore;
    LwDictInfoList *dictinfolist;

    priv = GW_APPLICATION (application)->priv;
    searchwindow = gw_application_get_last_focused_searchwindow (GW_APPLICATION (application));
    dictionarystore = GW_DICTIONARYSTORE (gw_application_get_dictionarystore (GW_APPLICATION (application)));
    dictinfolist = gw_dictionarystore_get_dictinfolist (dictionarystore);

    if (priv->arg_new_vocabulary_window_switch)
    {
      vocabularywindow = GW_VOCABULARYWINDOW (gw_vocabularywindow_new (GTK_APPLICATION (application)));
      gtk_widget_show (GTK_WIDGET (vocabularywindow));
      return;
    }

    else if (searchwindow == NULL || priv->arg_new_window_switch)
    {
      searchwindow = GW_SEARCHWINDOW (gw_searchwindow_new (GTK_APPLICATION (application)));
      gtk_widget_show (GTK_WIDGET (searchwindow));

      if (lw_dictinfolist_get_total (dictinfolist) == 0)
      {
        settingswindow = GW_SETTINGSWINDOW (gw_settingswindow_new (GTK_APPLICATION (application)));
        gtk_window_set_transient_for (GTK_WINDOW (settingswindow), GTK_WINDOW (searchwindow));
        gtk_widget_show (GTK_WIDGET (settingswindow));
      }
      return;
    }
    else
    {
      gtk_window_present (GTK_WINDOW (searchwindow));
      return;
    }
}


static int 
gw_application_command_line (GApplication *application, GApplicationCommandLine *command_line)
{
    //Declarations
    LwDictInfo *di;
    GwSearchWindow *window;
    GwDictionaryStore *dictionarystore;
    LwDictInfoList *dictinfolist;
    GwApplicationPrivate *priv;
    int argc;
    char **argv;

    //Initializations
    priv = GW_APPLICATION (application)->priv;
    dictionarystore = GW_DICTIONARYSTORE (gw_application_get_dictionarystore (GW_APPLICATION (application)));
    dictinfolist = gw_dictionarystore_get_dictinfolist (dictionarystore);
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
    di = lw_dictinfolist_get_dictinfo_fuzzy (dictinfolist, priv->arg_dictionary);

    //Set the initial dictionary
    if (di != NULL)
    {
      gw_searchwindow_set_dictionary (window, di->load_position);
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
