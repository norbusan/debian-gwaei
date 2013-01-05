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
//! @file dictionaryinstallwindow-callbacks.c
//!
//! @brief To be written
//!

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include <gwaei/gettext.h>
#include <gwaei/gwaei.h>
#include <gwaei/dictionaryinstallwindow-private.h>


G_MODULE_EXPORT void 
gw_dictionaryinstallwindow_filename_entry_changed_cb (GtkWidget *widget, gpointer data)
{
    //Sanity check
    g_assert (data != NULL);

    //Declarations
    GwDictionaryInstallWindow *window;
    GwDictionaryInstallWindowPrivate *priv;
    const gchar *value;

    //Initializations
    window = GW_DICTIONARYINSTALLWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_DICTIONARYINSTALLWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    value = gtk_entry_get_text (GTK_ENTRY (widget));

    lw_dictionary_installer_set_files (priv->dictionary, value);

    gw_dictionaryinstallwindow_sync_interface (window);
}


G_MODULE_EXPORT void 
gw_dictionaryinstallwindow_engine_combobox_changed_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwDictionaryInstallWindow *window;
    GwDictionaryInstallWindowPrivate *priv;
    GwApplication *application;
    GwDictionaryList *dictionarylist;
    GtkComboBox *combobox;
    LwDictionary *dictionary;
    gint index;
    GtkTreeModel *treemodel;
    GtkTreeIter treeiter;
    gboolean valid;
    LwDictionaryInstall *install;
    GType type;
    const gchar *FILENAME;
    GList *link;

    //Initializations
    window = GW_DICTIONARYINSTALLWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_DICTIONARYINSTALLWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));
    dictionarylist = gw_application_get_installable_dictionarylist (application);
    combobox = GTK_COMBO_BOX (widget);
    index = gtk_combo_box_get_active (combobox);
    dictionary = priv->dictionary;

    //Get the GType needed
    treemodel = gtk_combo_box_get_model (combobox);
    valid = gtk_tree_model_iter_nth_child (treemodel, &treeiter, NULL, index);
    if (!valid) return;
    gtk_tree_model_get (treemodel, &treeiter, GW_DICTINSTWINDOW_ENGINESTOREFIELD_ID, &type, -1);

    //Get the current dictionary install information
    link = lw_dictionarylist_get_list (LW_DICTIONARYLIST (dictionarylist));
    while (link != NULL && dictionary != LW_DICTIONARY (link->data)) link = link->next;
    if (link == NULL) return;
    install = lw_dictionary_steal_installer (link->data);
    FILENAME = lw_dictionary_get_filename (link->data);

    //Create the new dictionary
    dictionary = LW_DICTIONARY (g_object_new (LW_TYPE_EDICTIONARY, "filename", FILENAME, NULL));
    lw_dictionary_set_installer (dictionary, install);
  
    //Replace the old one
    g_object_unref (link->data);
    link->data = dictionary;

    //Update the interface
    gw_dictionaryinstallwindow_sync_interface (window);
}


G_MODULE_EXPORT void 
gw_dictionaryinstallwindow_source_entry_changed_cb (GtkWidget *widget, gpointer data)
{
    //Sanity check
    g_assert (data != NULL);

    //Declarations
    GwDictionaryInstallWindow *window;
    GwDictionaryInstallWindowPrivate *priv;
    const gchar *value = gtk_entry_get_text (GTK_ENTRY (widget));

    //Initializations
    window = GW_DICTIONARYINSTALLWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_DICTIONARYINSTALLWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    value = gtk_entry_get_text (GTK_ENTRY (widget));

    //Set the LwDictionary value
    lw_dictionary_installer_set_downloads (priv->dictionary, value);

    gw_dictionaryinstallwindow_sync_interface (window);
}


G_MODULE_EXPORT void 
gw_dictionaryinstallwindow_reset_default_uri_cb (GtkWidget *widget, gpointer data)
{
    //Sanity check
    g_assert (data != NULL);

    //Declarations
    GwDictionaryInstallWindow *window;
    GwDictionaryInstallWindowPrivate *priv;

    //Initializations
    window = GW_DICTIONARYINSTALLWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_DICTIONARYINSTALLWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;

    lw_dictionary_installer_reset_downloads (priv->dictionary);
}


G_MODULE_EXPORT void 
gw_dictionaryinstallwindow_select_file_cb (GtkWidget *widget, gpointer data)
{
    //Sanity check
    g_assert (data != NULL);

    //Declarations
    GwDictionaryInstallWindow *window;
    GwDictionaryInstallWindowPrivate *priv;
    GtkWidget *dialog;
    gchar *filename;

    //Initializations
    window = GW_DICTIONARYINSTALLWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_DICTIONARYINSTALLWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    dialog = gtk_file_chooser_dialog_new (
      "Select File",
      GTK_WINDOW (window),
      GTK_FILE_CHOOSER_ACTION_OPEN,
      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
      NULL
    );
    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
      filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
      if (filename != NULL)
      {
        gtk_entry_set_text (priv->source_entry, filename);
        g_free (filename);
      }
    }

    gtk_widget_destroy (dialog);
}


G_MODULE_EXPORT void 
gw_dictionaryinstallwindow_encoding_combobox_changed_cb (GtkWidget *widget, gpointer data)
{
    //Sanity check
    g_assert (data != NULL);

    //Declaraitons
    GwDictionaryInstallWindow *window;
    GwDictionaryInstallWindowPrivate *priv;
    int value;

    //Initializations
    window = GW_DICTIONARYINSTALLWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_DICTIONARYINSTALLWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    value = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));

    lw_dictionary_installer_set_encoding (priv->dictionary, value);
}


G_MODULE_EXPORT void 
gw_dictionaryinstallwindow_postprocess_checkbox_toggled_cb (GtkWidget *widget, gpointer data)
{
    //Sanity check
    g_assert (data != NULL);

    //Declarations
    GwDictionaryInstallWindow *window;
    GwDictionaryInstallWindowPrivate *priv;
    gboolean value;

    //Initializations
    window = GW_DICTIONARYINSTALLWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_DICTIONARYINSTALLWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    value = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));

    lw_dictionary_installer_set_postprocessing (priv->dictionary, value);
}


G_MODULE_EXPORT void 
gw_dictionaryinstallwindow_cursor_changed_cb (GtkTreeView *view, gpointer data)
{
    //Sanity check
    g_return_if_fail (data != NULL);

    //Declarations
    GwDictionaryInstallWindow *window;
    GwDictionaryInstallWindowPrivate *priv;
    GwApplication *application;
    GwDictionaryList *dictionarylist;
    GtkListStore *liststore;
    GtkTreeModel *treemodel;
    GtkTreeSelection *selection;
    GtkWidget *hbox;
    LwDictionary *dictionary;
    GtkTreeIter iter;
    gboolean show_details;
    gboolean has_selection;
    gboolean editable;
    gint height;
    gint width;

    //Initializations
    window = GW_DICTIONARYINSTALLWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_DICTIONARYINSTALLWINDOW));
    if (window == NULL) return;
    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));
    dictionarylist = gw_application_get_installable_dictionarylist (application);
    liststore = gw_dictionarylist_get_liststore (dictionarylist);
    treemodel = GTK_TREE_MODEL (liststore);
    
    hbox = GTK_WIDGET (priv->details_hbox);

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (view));
    has_selection = gtk_tree_selection_get_selected (selection, &treemodel, &iter);
    show_details = gtk_toggle_button_get_active (priv->details_togglebutton);
    editable = FALSE;

    //Set the approprate contents of the detail pane
    if (has_selection)
    {
      gw_dictionaryinstallwindow_clear_details_box (window);
      gtk_tree_model_get (treemodel, &iter, GW_DICTIONARYLIST_COLUMN_DICT_POINTER, &dictionary, -1);
      gw_dictionaryinstallwindow_fill_details_box (window, dictionary);
      editable = !(lw_dictionary_installer_is_builtin (dictionary));
    }

    //Set the approprate show/hide state of the detail pane
    if (has_selection && (show_details || editable))
      gtk_widget_show (hbox);
    else
      gtk_widget_hide (hbox);

    //Make the window shrink if the detail pane disappeared
    gtk_window_get_size (GTK_WINDOW (window), &width, &height);
    gtk_window_resize (GTK_WINDOW (window), 1, height);

    gw_dictionaryinstallwindow_sync_interface (window);
}


G_MODULE_EXPORT void 
gw_dictionaryinstallwindow_listitem_toggled_cb (GtkListStore          *liststore,
                                                gchar                 *path,
                                                GtkCellRendererToggle *renderer)
{
    //Sanity check
    g_return_if_fail (GTK_IS_LIST_STORE (liststore) != FALSE);

    //Declarations
    GtkTreeModel *treemodel;
    GtkTreeIter iter;
    gboolean state;
    LwDictionary *dictionary;

    //Initializations
    treemodel = GTK_TREE_MODEL (liststore);

    gtk_tree_model_get_iter_from_string (treemodel, &iter, path);

    gtk_tree_model_get (treemodel, &iter, 
        GW_DICTIONARYLIST_COLUMN_SELECTED, &state, 
        GW_DICTIONARYLIST_COLUMN_DICT_POINTER, &dictionary, 
        -1
    );
    gtk_list_store_set (liststore, &iter, 
        GW_DICTIONARYLIST_COLUMN_SELECTED, !state, 
        -1
    );

    lw_dictionary_set_selected (dictionary, !state);
}


G_MODULE_EXPORT void 
gw_dictionaryinstallwindow_detail_checkbox_toggled_cb (GtkWidget *widget, gpointer data)
{
    //Sanity check
    g_assert (data != NULL);

    //Declarations
    GwDictionaryInstallWindow *window;
    GwDictionaryInstallWindowPrivate *priv;
    GtkTreePath *path;
    GtkTreeViewColumn *column;

    //Declarations
    window = GW_DICTIONARYINSTALLWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_DICTIONARYINSTALLWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;

    //Trigger the list item selection callback
    gtk_tree_view_get_cursor (priv->view, &path, &column);
    if (path == NULL) return;
    gtk_tree_view_set_cursor (priv->view, path, column, FALSE);

    //Cleanup
    gtk_tree_path_free (path);
}


//!
//! @brief Closes the window passed throught the widget pointer
//! @param widget GtkWidget pointer to the window to close
//! @param data Currently unused gpointer
//!
G_MODULE_EXPORT void 
gw_dictionaryinstallwindow_close_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwDictionaryInstallWindow *window;
    
    //Initializations
    window = GW_DICTIONARYINSTALLWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_DICTIONARYINSTALLWINDOW));
    g_return_if_fail (window != NULL);

    gtk_widget_destroy (GTK_WIDGET (window));
}


G_MODULE_EXPORT void 
gw_dictionaryinstallwindow_add_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwDictionaryInstallWindow *window;
    GwSettingsWindow *settingswindow;
    GwInstallProgressWindow *installprogresswindow;
    GwApplication *application;
    
    //Initializations
    window = GW_DICTIONARYINSTALLWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_DICTIONARYINSTALLWINDOW));
    g_return_if_fail (window != NULL);
    settingswindow = GW_SETTINGSWINDOW (gtk_window_get_transient_for (GTK_WINDOW (window)));
    application = gw_window_get_application (GW_WINDOW (window));
    installprogresswindow = GW_INSTALLPROGRESSWINDOW (gw_installprogresswindow_new (GTK_APPLICATION (application)));

    gtk_widget_destroy (GTK_WIDGET (window));

    gtk_window_set_transient_for (GTK_WINDOW (installprogresswindow), GTK_WINDOW (settingswindow));
    gtk_widget_show (GTK_WIDGET (installprogresswindow));
    gw_installprogresswindow_start (installprogresswindow);
}

