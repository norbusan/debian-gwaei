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


#include "../private.h"

#include <string.h>
#include <stdlib.h>

#include <gtk/gtk.h>
 
#include <gwaei/gwaei.h>
#include <gwaei/installprogresswindow-private.h>


G_MODULE_EXPORT void 
gw_installprogresswindow_cancel_cb (GtkWidget *widget, gpointer data)
{
    GwInstallProgressWindow *window;
    GwApplication *application;
    LwDictInstList *dictinstlist;

    window = GW_INSTALLPROGRESSWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_INSTALLPROGRESSWINDOW));
    g_return_if_fail (window != NULL);
    application = gw_window_get_application (GW_WINDOW (window));
    dictinstlist = gw_application_get_dictinstlist (application);

    lw_dictinstlist_set_cancel_operations (dictinstlist, TRUE);
}


G_MODULE_EXPORT int 
gw_installprogresswindow_update_dictinst_cb (double fraction, gpointer data)
{
    //Declarations
    GwInstallProgressWindow *window;
    GwInstallProgressWindowPrivate *priv;

    //Initializations
    window = GW_INSTALLPROGRESSWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_INSTALLPROGRESSWINDOW));
    if (window == NULL) return 0;
    priv = window->priv;

    g_mutex_lock (&priv->mutex); 
    priv->install_fraction = lw_dictinst_get_total_progress (priv->di, fraction);
    g_mutex_unlock (&priv->mutex);

    return 0;
}


//!
//! @brief Callback to update the install dialog progress.  The data passed to it should be
//!        in the form of a LwDictInst.  If it is NULL, the progress window will be closed.
//!
G_MODULE_EXPORT gboolean 
gw_installprogresswindow_update_ui_timeout (gpointer data)
{
    //Sanity check
    g_assert (data != NULL);

    //Declarations
    GwInstallProgressWindow *window;
    GwInstallProgressWindowPrivate *priv;
    GtkWindow *settingswindow;
    GwApplication *application;
    GtkListStore *dictionarystore;
    LwDictInstList *dictinstlist;
    LwDictInst *di;
    LwPreferences *preferences;
    GList *link;
    gint current_to_install;
    gint total_to_install;
    gchar *text_installing;
    gchar *text_installing_markup;
    gchar *text_left;
    gchar *text_left_markup;
    gchar *text_progressbar;

    //Initializations
    window = GW_INSTALLPROGRESSWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_INSTALLPROGRESSWINDOW));
    g_return_val_if_fail (window != NULL, FALSE);
    application = gw_window_get_application (GW_WINDOW (window));
    dictionarystore = gw_application_get_dictionarystore (application);
    dictinstlist = gw_application_get_dictinstlist (application);
    preferences = gw_application_get_preferences (application);
    priv = window->priv;
    current_to_install = 0;
    total_to_install = 0;

    //The install is complete close the window
    if (priv->di == NULL)
    {
      settingswindow = gtk_window_get_transient_for (GTK_WINDOW (window));

      gw_dictionarystore_reload (GW_DICTIONARYSTORE (dictionarystore), preferences);

      gtk_widget_destroy (GTK_WIDGET (window));

      gw_application_handle_error (application, GTK_WINDOW (settingswindow), TRUE, NULL);

      lw_dictinstlist_set_cancel_operations (dictinstlist, FALSE);
      gw_settingswindow_check_for_dictionaries (GW_SETTINGSWINDOW (settingswindow));

      return FALSE;
    }

    g_mutex_lock (&priv->mutex);

    //Calculate the number of dictionaries left to install
    for (link = dictinstlist->list; link != NULL; link = link->next)
    {
      di = LW_DICTINST (link->data);
      if (di != NULL && di->selected)
      {
        current_to_install++;
      }
      if (link->data == priv->di) break;
    }

    //Calculate the number of dictionaries left to install
    for (link = dictinstlist->list; link != NULL; link = link->next)
    {
      di = LW_DICTINST (link->data);
      if (di->selected)
      {
        total_to_install++;
      }
    }
    
    di = priv->di;

    text_progressbar =  g_markup_printf_escaped (gettext("Installing %s..."), di->filename);
    text_left = g_strdup_printf (gettext("Installing dictionary %d of %d..."), current_to_install, total_to_install);
    text_left_markup = g_markup_printf_escaped ("<big><b>%s</b></big>", text_left);
    text_installing = lw_dictinst_get_status_string (di, TRUE);
    text_installing_markup = g_markup_printf_escaped ("<small>%s</small>", text_installing);

    gtk_label_set_markup (priv->label, text_left_markup);
    gtk_label_set_markup (priv->sublabel, text_installing_markup);
    gtk_progress_bar_set_fraction (priv->progressbar, priv->install_fraction);
    gtk_progress_bar_set_text (priv->progressbar, text_progressbar);

    g_mutex_unlock (&priv->mutex);

    //Cleanup
    g_free (text_progressbar);
    g_free (text_left);
    g_free (text_left_markup);
    g_free (text_installing);
    g_free (text_installing_markup);

    return TRUE;
}


