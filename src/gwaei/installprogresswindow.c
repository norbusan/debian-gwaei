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
//! @file installprogresswindow.c
//!
//! @brief To be written
//!

#include <string.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include <gwaei/gwaei.h>
#include <gwaei/gettext.h>
#include <gwaei/installprogresswindow-private.h>

static gpointer gw_installprogresswindow_install_thread (gpointer);

G_DEFINE_TYPE (GwInstallProgressWindow, gw_installprogresswindow, GW_TYPE_WINDOW)

//!
//! @brief Sets up the variables in main-interface.c and main-callbacks.c for use
//!
GtkWindow* 
gw_installprogresswindow_new (GtkApplication *application)
{
    g_assert (application != NULL);

    //Declarations
    GwInstallProgressWindow *window;

    //Initializations
    window = GW_INSTALLPROGRESSWINDOW (g_object_new (GW_TYPE_INSTALLPROGRESSWINDOW,
                                            "type",        GTK_WINDOW_TOPLEVEL,
                                            "application", GW_APPLICATION (application),
                                            "ui-xml",      "installprogresswindow.ui",
                                            NULL));

    return GTK_WINDOW (window);
}


static void 
gw_installprogresswindow_init (GwInstallProgressWindow *window)
{
    window->priv = GW_INSTALLPROGRESSWINDOW_GET_PRIVATE (window);
    memset(window->priv, 0, sizeof(GwInstallProgressWindowPrivate));
}


static void 
gw_installprogresswindow_finalize (GObject *object)
{
    GwInstallProgressWindow *window;
    GwInstallProgressWindowPrivate *priv;

    window = GW_INSTALLPROGRESSWINDOW (object);
    priv = window->priv;
 
    g_mutex_clear (&priv->mutex); 
    priv->label = NULL;
    priv->sublabel = NULL;
    priv->progressbar = NULL;
    if (priv->cancellable != NULL) g_object_unref (priv->cancellable); priv->cancellable = NULL;

    G_OBJECT_CLASS (gw_installprogresswindow_parent_class)->finalize (object);
}


static void 
gw_installprogresswindow_constructed (GObject *object)
{
    //Declarations
    GwInstallProgressWindow *window;
    GwInstallProgressWindowPrivate *priv;
    GtkAccelGroup *accelgroup;

    //Chain the parent class
    {
      G_OBJECT_CLASS (gw_installprogresswindow_parent_class)->constructed (object);
    }

    window = GW_INSTALLPROGRESSWINDOW (object);
    priv = window->priv;
    accelgroup = gw_window_get_accel_group (GW_WINDOW (window));

    g_mutex_init (&priv->mutex);
    priv->label = GTK_LABEL (gw_window_get_object (GW_WINDOW (window), "progress_label"));
    priv->sublabel = GTK_LABEL (gw_window_get_object (GW_WINDOW (window), "sub_progress_label"));
    priv->progressbar = GTK_PROGRESS_BAR (gw_window_get_object (GW_WINDOW (window), "progress_progressbar"));
    priv->cancel_button = GTK_BUTTON (gw_window_get_object (GW_WINDOW (window), "cancel_button"));
    priv->cancellable = g_cancellable_new ();

    gtk_window_set_title (GTK_WINDOW (window), gettext("Installing Dictionaries..."));
    gtk_window_set_resizable (GTK_WINDOW (window), TRUE);
    gtk_window_set_type_hint (GTK_WINDOW (window), GDK_WINDOW_TYPE_HINT_DIALOG);
    gtk_window_set_skip_taskbar_hint (GTK_WINDOW (window), TRUE);
    gtk_window_set_skip_pager_hint (GTK_WINDOW (window), TRUE);
    gtk_window_set_destroy_with_parent (GTK_WINDOW (window), TRUE);
    gtk_window_set_icon_name (GTK_WINDOW (window), "gwaei");
    gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_window_set_modal (GTK_WINDOW (window), TRUE);
    gtk_window_set_default_size (GTK_WINDOW (window), 500, -1);
    gtk_window_set_has_resize_grip (GTK_WINDOW (window), FALSE);
    gtk_container_set_border_width (GTK_CONTAINER (window), 4);


    gtk_widget_add_accelerator (GTK_WIDGET (priv->cancel_button), "activate", 
      accelgroup, (GDK_KEY_W), GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator (GTK_WIDGET (priv->cancel_button), "activate", 
      accelgroup, (GDK_KEY_Escape), 0, GTK_ACCEL_VISIBLE);

    gw_window_unload_xml (GW_WINDOW (window));
}


static void
gw_installprogresswindow_class_init (GwInstallProgressWindowClass *klass)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = gw_installprogresswindow_constructed;
  object_class->finalize = gw_installprogresswindow_finalize;

  g_type_class_add_private (object_class, sizeof (GwInstallProgressWindowPrivate));
}


//!
//! @brief Starts the install when the add button on the dictionary chooser is selected
//!
void 
gw_installprogresswindow_start (GwInstallProgressWindow *window)
{
    //Sanity check
    g_assert (window != NULL);

    //Declarations
    GwApplication *application;
    GError *error;

    //Initializations
    application = gw_window_get_application (GW_WINDOW (window));
    error = NULL;

    //Set the new window
    g_thread_try_new (
      "gwaei-install-thread", 
      gw_installprogresswindow_install_thread, 
      window, 
      &error
    );

    gw_application_handle_error (application, gtk_window_get_transient_for (GTK_WINDOW (window)), TRUE, &error);
}


static gpointer 
gw_installprogresswindow_install_thread (gpointer data)
{
    //Declarations
    GwInstallProgressWindow *window;
    GwInstallProgressWindowPrivate *priv;
    GwApplication *application;
    GwDictionaryList *dictionarylist;
    GList *link;
    LwDictionary *dictionary;
    GError *error;
    gulong signalid;
    GCancellable *cancellable;

    //Initializations
    window = GW_INSTALLPROGRESSWINDOW (data);
    if (window == NULL) return NULL;
    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));
    dictionarylist = gw_application_get_installable_dictionarylist (application);
    cancellable = priv->cancellable;
    error = NULL;
    link = lw_dictionarylist_get_list (LW_DICTIONARYLIST (dictionarylist));

    //Do the installation
    g_timeout_add (100, gw_installprogresswindow_update_ui_timeout, window);
    while (link != NULL && error == NULL)
    {
      dictionary = LW_DICTIONARY (link->data);
      if (dictionary != NULL && lw_dictionary_is_selected (dictionary))
      {
        g_mutex_lock (&priv->mutex);
        priv->dictionary = dictionary;
        g_mutex_unlock (&priv->mutex);
        signalid = g_signal_connect (dictionary, "progress-changed", G_CALLBACK (gw_installprogresswindow_update_dictionary_cb), window);
        lw_dictionary_install (dictionary, cancellable, &error);
        if (g_signal_handler_is_connected (dictionary, signalid))
          g_signal_handler_disconnect (dictionary, signalid);
      }

      link = link->next;
    }

    gw_application_set_error (application, error);
    error = NULL;

    g_mutex_lock (&priv->mutex);
    //This will clue the progress window to close itself
    priv->dictionary = NULL;
    g_mutex_unlock (&priv->mutex);

    return NULL;
}


