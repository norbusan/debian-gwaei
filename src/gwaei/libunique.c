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
//! @file src/gtk-main-interface-unique.c
//!
//! @brief File to abstract out usage of the libunique library
//!
//! Libunique allows us to only have one instance of the application open if we want to.
//!


#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <libintl.h>

#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include <unique/unique.h>

#include <gwaei/gwaei.h>


static UniqueApp *_app;
static UniqueResponse response;
static gboolean _initialized = FALSE;

static void _add_window_watcher (GtkWidget*);
static gboolean _is_unique (gboolean);



void gw_libunique_initialize (gboolean new_instance)
{

    //Declarations
    GtkBuilder *builder;
    GtkWidget *window;

    //Initializations
    builder = gw_common_get_builder ();
    window = GTK_WIDGET (gtk_builder_get_object (builder, "main_window"));

    if (_initialized == TRUE) gw_libunique_free();

    _app = unique_app_new ("org.dictionary.gWaei", NULL);
    _initialized = TRUE;

    //Sanity check
    if (!_is_unique (new_instance)) exit(0);

    _add_window_watcher (window);
}


void gw_libunique_free ()
{
    //Sanity check;
    if (!_initialized) return; 

    g_object_unref (G_OBJECT (_app));
    _app = NULL;
}




//!
//! @brief To be written
//!
static UniqueResponse _message_received_cb (UniqueApp         *app,
                                            UniqueCommand      command,
                                            UniqueMessageData *message,
                                            guint              time_,
                                            gpointer           user_data)
{
    UniqueResponse res;
    GtkBuilder *builder = gw_common_get_builder ();
    GtkWidget *main_window, *settings_window, *radicals_window, *kanjipad_window;
    main_window = GTK_WIDGET (gtk_builder_get_object (builder, "main_window"));
    kanjipad_window = GTK_WIDGET (gtk_builder_get_object (builder, "kanjipad_window"));
    radicals_window = GTK_WIDGET (gtk_builder_get_object (builder, "radicals_window"));
    settings_window = GTK_WIDGET (gtk_builder_get_object (builder, "settings_window"));
    switch (command)
    {
        case UNIQUE_ACTIVATE:
          if (gtk_widget_get_visible (main_window))
          {
          gdk_x11_window_move_to_current_desktop (gtk_widget_get_window (main_window));
          gtk_window_set_screen (GTK_WINDOW (main_window), unique_message_data_get_screen (message));
          gtk_window_present_with_time (GTK_WINDOW (main_window), time_);
          }
          if (gtk_widget_get_visible (kanjipad_window))
          {
            gdk_x11_window_move_to_current_desktop (gtk_widget_get_window (main_window));
            gtk_window_set_screen (GTK_WINDOW (kanjipad_window), unique_message_data_get_screen (message));
            gtk_window_present_with_time (GTK_WINDOW (kanjipad_window), time_);
          }
          if (gtk_widget_get_visible (radicals_window))
          {
            gdk_x11_window_move_to_current_desktop (gtk_widget_get_window (radicals_window));
            gtk_window_set_screen (GTK_WINDOW (radicals_window), unique_message_data_get_screen (message));
            gtk_window_present_with_time (GTK_WINDOW (radicals_window), time_);
          }
          if (gtk_widget_get_visible (settings_window))
          {
            gdk_x11_window_move_to_current_desktop (gtk_widget_get_window (settings_window));
            gtk_window_set_screen (GTK_WINDOW (settings_window), unique_message_data_get_screen (message));
            gtk_window_present_with_time (GTK_WINDOW (settings_window), time_);
          }
          res = UNIQUE_RESPONSE_OK;
          break;
        default:
          res = UNIQUE_RESPONSE_OK;
          break;
    }
    return res;
}


static gboolean _is_unique (gboolean arg_create_new_instance)
{
    //Initializaitons
    gboolean status;

    //Declarations
    status = TRUE;

    if (arg_create_new_instance == FALSE && unique_app_is_running (_app))
    {
      response = unique_app_send_message (_app, UNIQUE_ACTIVATE, NULL);
      status = FALSE;
    }

    return status;
}

static void _add_window_watcher (GtkWidget *main_window)
{
      unique_app_watch_window (_app, GTK_WINDOW (main_window));
      g_signal_connect (_app, "message-received", G_CALLBACK (_message_received_cb), NULL);
}
