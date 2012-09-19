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
//! @file installprogresswindow-callbacks.c
//!
//! @brief To be written
//!

#include <string.h>
#include <stdlib.h>

#include <gtk/gtk.h>
 
#include <gwaei/gwaei.h>
#include <gwaei/gettext.h>
#include <gwaei/installprogresswindow-private.h>


G_MODULE_EXPORT void 
gw_installprogresswindow_cancel_cb (GtkWidget *widget, gpointer data)
{
    //Sanity checks
    g_return_if_fail (widget != NULL);

    //Declarations
    GwInstallProgressWindow *window;
    GwInstallProgressWindowPrivate *priv;
    GCancellable *cancellable;

    //Initializations
    window = GW_INSTALLPROGRESSWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_INSTALLPROGRESSWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    cancellable = priv->cancellable;

    g_cancellable_cancel (cancellable);
}


void 
gw_installprogresswindow_update_dictionary_cb (LwDictionary *dictionary, gpointer data)
{
    //Declarations
    GwInstallProgressWindow *window;
    GwInstallProgressWindowPrivate *priv;
    gdouble fraction;

    //Initializations
    window = GW_INSTALLPROGRESSWINDOW (data);
    priv = window->priv;
    fraction = lw_dictionary_installer_get_stage_progress (LW_DICTIONARY (dictionary));

    g_mutex_lock (&priv->mutex); 
    priv->install_fraction = fraction;
    g_mutex_unlock (&priv->mutex);
}


static void
gw_installprogresswindow_finish (GwInstallProgressWindow *window)
{
    //Sanity checks
    g_return_if_fail (window != NULL);

    //Declarations
    GwInstallProgressWindowPrivate *priv;
    GwApplication *application;
    LwDictionaryList *dictionarylist;
    LwPreferences *preferences;

    //Initializations
    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));
    dictionarylist = LW_DICTIONARYLIST (gw_application_get_installed_dictionarylist (application));
    preferences = gw_application_get_preferences (application);

    g_mutex_lock (&priv->mutex);

    lw_dictionarylist_clear (dictionarylist);
    lw_dictionarylist_load_installed (dictionarylist);
    lw_dictionarylist_load_order (dictionarylist, preferences);

    g_mutex_unlock (&priv->mutex);

    gtk_widget_destroy (GTK_WIDGET (window));
}


static void
gw_installprogresswindow_sync_progress (GwInstallProgressWindow *window)
{
    GwInstallProgressWindowPrivate *priv;
    GwApplication *application;
    LwDictionaryList *dictionarylist;
    LwDictionary *dictionary;
    GList *link;
    gint current_to_install;
    gint total_to_install;
    gchar *text_installing;
    gchar *text_installing_markup;
    gchar *text_left;
    gchar *text_left_markup;
    gchar *text_progressbar;

    //Initializations
    application = gw_window_get_application (GW_WINDOW (window));
    dictionarylist = LW_DICTIONARYLIST (gw_application_get_installable_dictionarylist (application));
    priv = window->priv;
    current_to_install = 0;
    total_to_install = 0;

    g_mutex_lock (&priv->mutex);

    //Calculate the number of dictionaries left to install
    for (link = lw_dictionarylist_get_list (dictionarylist); link != NULL; link = link->next)
    {
      dictionary = LW_DICTIONARY (link->data);
      if (dictionary != NULL && lw_dictionary_is_selected (dictionary))
      {
        current_to_install++;
      }
      if (link->data == priv->dictionary) break;
    }

    //Calculate the number of dictionaries left to install
    for (link = lw_dictionarylist_get_list (dictionarylist); link != NULL; link = link->next)
    {
      dictionary = LW_DICTIONARY (link->data);
      if (lw_dictionary_is_selected (dictionary))
      {
        total_to_install++;
      }
    }
    
    dictionary = priv->dictionary;

    text_progressbar =  g_markup_printf_escaped (gettext("Installing %s..."), lw_dictionary_get_name (dictionary));
    if (text_progressbar == NULL) goto errored;
    text_left = g_strdup_printf (gettext("Installing dictionary %d of %d..."), current_to_install, total_to_install);
    if (text_left == NULL) goto errored;
    text_left_markup = g_markup_printf_escaped ("<big><b>%s</b></big>", text_left);
    if (text_left_markup == NULL) goto errored;
    text_installing = lw_dictionary_installer_get_status_message (dictionary, TRUE);
    if (text_installing == NULL) goto errored;
    text_installing_markup = g_markup_printf_escaped ("<small>%s</small>", text_installing);
    if (text_installing_markup == NULL) goto errored;

    gtk_label_set_markup (priv->label, text_left_markup);
    gtk_label_set_markup (priv->sublabel, text_installing_markup);
    gtk_progress_bar_set_fraction (priv->progressbar, priv->install_fraction);
    gtk_progress_bar_set_text (priv->progressbar, text_progressbar);

errored:
    //Cleanup
    if (text_progressbar != NULL) g_free (text_progressbar); text_progressbar = NULL;
    if (text_left != NULL) g_free (text_left); text_left = NULL;
    if (text_left_markup != NULL) g_free (text_left_markup); text_left_markup = NULL;
    if (text_installing != NULL) g_free (text_installing); text_installing = NULL;
    if (text_installing_markup != NULL) g_free (text_installing_markup); text_installing_markup = NULL;

    g_mutex_unlock (&priv->mutex);
}


//!
//! @brief Callback to update the install dialog progress.  The data passed to it should be
//!        in the form of a LwDictionary.  If it is NULL, the progress window will be closed.
//!
G_MODULE_EXPORT gboolean 
gw_installprogresswindow_update_ui_timeout (gpointer data)
{
    //Sanity check
    g_return_val_if_fail (data != NULL, FALSE);

    //Declarations
    GwInstallProgressWindow *window;
    GwInstallProgressWindowPrivate *priv;
    GwApplication *application;
    gint status;
    
    //Initializations
    window = GW_INSTALLPROGRESSWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_INSTALLPROGRESSWINDOW));
    g_return_val_if_fail (window != NULL, FALSE);
    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));
    status = FALSE;

    //The install is complete close the window
    if (priv->dictionary == NULL)
    {
      gw_installprogresswindow_finish (window);
      gw_application_handle_error (application, NULL, TRUE, NULL);
      status = FALSE;
    }
    else
    {
      gw_installprogresswindow_sync_progress (window);
      status = TRUE;
    }

    return status;
}


