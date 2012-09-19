/******************************************************************************
    AUTHOR:
    KanjiPad - Japanese handwriting recognition front end
    Copyright (C) 1997 Owen Taylor
    File heavily modified and updated by Zachary Dovel.

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
//! @file src/gtk-kanjipad-interface.c
//!
//! @brief Abstraction layer for gtk GUI element interaction
//!
//! Callbacks for activities initiated by the user. Most of the gtk code here
//! should still be abstracted to the interface C file when possible.
//!


#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <gtk/gtk.h>

#include <gwaei/definitions.h>
#include <gwaei/preferences.h>

#include <gwaei/gtk-kanjipad-interface.h>
#include <gwaei/gtk-kanjipad-interface-drawingarea.h>
#include <gwaei/gtk-kanjipad-interface-candidatearea.h>
#include <gwaei/gtk-kanjipad-callbacks.h>


#define BUFLEN 256

static GtkWidget *target_text_widget = NULL;
static gboolean gw_kanjipad_is_initialized = FALSE;


//!
//! @brief To be written
//!
static gboolean engine_input_handler (GIOChannel *source, GIOCondition condition, gpointer data)
{
    static gchar *p;
    static gchar *line;
    GError *err = NULL;
    GIOStatus status;
    int i;

    status = g_io_channel_read_line (pa->from_engine, &line, NULL, NULL, &err);
    switch (status)
    {
      case G_IO_STATUS_ERROR:
        g_printerr ("Error reading from engine: %s\n", err->message);
        exit(1);
        break;
      case G_IO_STATUS_NORMAL:
        break;
      case G_IO_STATUS_EOF:
        g_printerr ("Engine no longer exists");
        exit (1);
        break;
      case G_IO_STATUS_AGAIN:
        g_assert_not_reached ();
        break;
    }

    if (line[0] == 'K')
    {
      unsigned int t1, t2;
      p = line+1;
      for (i=0; i<MAX_GUESSES; i++)
      {
        while (*p && isspace(*p)) p++;
        if (!*p || sscanf(p, "%2x%2x", &t1, &t2) != 2)
        {
            i--;
            break;
        }
        pa->kanji_candidates[i][0] = t1;
        pa->kanji_candidates[i][1] = t2;
        while (*p && !isspace(*p)) p++;
      }
      pa->total_candidates = i+1;

      candidatearea_draw (pa->candidate_widget);
    }

    g_free (line);

    return TRUE;
}


//!
//! @brief Open the connection to the engine
//!
void kanjipad_init_engine (GwKanjipad *pa)
{
    gchar *argv[] = { LIBDIR G_DIR_SEPARATOR_S PACKAGE G_DIR_SEPARATOR_S "kpengine", "--data-file", NULL, NULL };
    GError *err = NULL;
    gchar *uninstalled;
    int stdin_fd, stdout_fd;

    uninstalled = g_build_filename (".", "kpengine", NULL);
    if (g_file_test (uninstalled, G_FILE_TEST_EXISTS))
      argv[0] = uninstalled;

    if (pa->data_file)
      argv[2] = pa->data_file;
    else
      argv[1] = NULL;

    if (!g_spawn_async_with_pipes (NULL, /* working directory */
           argv, NULL,  /* argv, envp */
           0,
           NULL, NULL,  /* child_setup */
           &pa->engine_pid,   /* child pid */
           &stdin_fd, &stdout_fd, NULL,
           &err))
    {
      GtkWidget *dialog;

      dialog = gtk_message_dialog_new (NULL, 0,
               GTK_MESSAGE_ERROR,
               GTK_BUTTONS_OK,
               "Could not start engine '%s': %s",
               argv[0], err->message);
      gtk_dialog_run (GTK_DIALOG (dialog));
      g_error_free (err);
      exit (EXIT_FAILURE);
    }

    g_free (uninstalled);
    
    if (!(pa->to_engine = g_io_channel_unix_new (stdin_fd)))
      g_error ("Couldn't create pipe to child process: %s", g_strerror(errno));
    if (!(pa->from_engine = g_io_channel_unix_new (stdout_fd)))
      g_error ("Couldn't create pipe from child process: %s", g_strerror(errno));

    g_io_add_watch (pa->from_engine, G_IO_IN, engine_input_handler, NULL);
}


//!
//! @brief To be written
//!
void gw_kanjipad_set_target_text_widget (GtkWidget* widget)
{
    target_text_widget = GTK_WIDGET (widget);
}


//!
//! @brief To be written
//!
GtkWidget* gw_kanjipad_get_target_text_widget ()
{
    return target_text_widget;
}


//!
//! @brief To be written
//!
GwKanjipad *kanjipad_create (GtkWidget *drawing_widget, GtkWidget *candidate_widget)
{
    GwKanjipad *temp = g_new0 (GwKanjipad, 1);
    temp->strokes = NULL;
    temp->curstroke = NULL;
    temp->instroke = FALSE;
    temp->annotate = FALSE;
    temp->pixmap = NULL;
    temp->kpixmap = NULL;
    temp->total_candidates = 0;
    temp->data_file = NULL;
    temp->drawing_widget = GTK_WIDGET (drawing_widget); 
    temp->candidate_widget = GTK_WIDGET (candidate_widget); 

    gw_kanjipad_drawingarea_initialize (temp);
    gw_kanjipad_candidatearea_initialize (temp);

    return temp;
}


//!
//! @brief Sets up kanjipad, aquiring any needed resources
//!
void gw_kanjipad_initialize (GtkBuilder *builder)
{
    if (gw_kanjipad_is_initialized == TRUE) return;

    GtkWidget *drawingarea;
    drawingarea = GTK_WIDGET (gtk_builder_get_object (builder, "kdrawing_area"));
    GtkWidget *candidatearea;
    candidatearea = GTK_WIDGET (gtk_builder_get_object (builder, "kguesses"));

    pa = kanjipad_create (drawingarea, candidatearea);
    kanjipad_init_engine (pa);

    gw_kanjipad_is_initialized = TRUE;
}


//!
//! @brief Frees any resources taken by the initialization of kanjipad
//!
void gw_kanjipad_free_resources ()
{
    if (gw_kanjipad_is_initialized == FALSE) return;

    GError *error = NULL;

    g_io_channel_shutdown (pa->from_engine, FALSE, &error);
    if (error != NULL)
    {
      printf("Errored: %s\n", error->message);
      exit(EXIT_FAILURE);
    }
    g_io_channel_unref (pa->from_engine);
    pa->from_engine = NULL;

    g_io_channel_shutdown (pa->to_engine, FALSE, &error);
    if (error != NULL)
    {
      printf("Errored: %s\n", error->message);
      exit(EXIT_FAILURE);
    }
    g_io_channel_unref (pa->to_engine);
    pa->to_engine = NULL;

    g_free (pa);
    pa = NULL;

    gw_kanjipad_is_initialized = FALSE;
}


