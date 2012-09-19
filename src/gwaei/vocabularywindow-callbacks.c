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
//! @file vocabularywindow-callbacks.c
//!
//! @brief To be written
//!


#include "../private.h"

#include <stdlib.h>
#include <string.h>

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <gwaei/gwaei.h>
#include <gwaei/vocabularyliststore.h>
#include <gwaei/vocabularywindow-private.h>


G_MODULE_EXPORT void
gw_vocabularywindow_new_list_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GwApplication *application;
    GtkListStore *liststore, *wordstore;
    GtkTreeSelection *selection;
    GtkTreeIter iter;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));
    liststore = gw_application_get_vocabularyliststore (application);
    selection = gtk_tree_view_get_selection (priv->list_treeview);

    gw_vocabularyliststore_new_list (GW_VOCABULARYLISTSTORE (liststore), &iter);
    gtk_tree_selection_select_iter (selection, &iter);
    wordstore = gw_vocabularyliststore_get_wordstore_by_iter (GW_VOCABULARYLISTSTORE (liststore), &iter);
    gtk_tree_view_set_model (priv->word_treeview, GTK_TREE_MODEL (wordstore));
    gtk_tree_view_set_search_column (priv->word_treeview, GW_VOCABULARYWORDSTORE_COLUMN_DEFINITIONS);
    gw_vocabularywindow_update_flashcard_menu_sensitivities (window);
    gw_vocabularywindow_show_vocabulary_list (window, TRUE);
}


G_MODULE_EXPORT void
gw_vocabularywindow_remove_list_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GList *rowlist;
    GtkListStore *store;
    GtkTreeIter iter;
    GtkTreePath *path;
    gboolean valid;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    selection = gtk_tree_view_get_selection (priv->list_treeview);
    model = gtk_tree_view_get_model (priv->list_treeview);
    rowlist = gtk_tree_selection_get_selected_rows (selection, &model);

    gw_vocabularyliststore_remove_path_list (GW_VOCABULARYLISTSTORE (model), rowlist);

    path = (GtkTreePath*) rowlist->data;
    gtk_tree_path_prev (path);
    valid = gtk_tree_model_get_iter (model, &iter, path);

    if (valid)
    {
      gtk_tree_selection_select_iter (selection, &iter);
      store = gw_vocabularyliststore_get_wordstore_by_iter (GW_VOCABULARYLISTSTORE (model), &iter);
      gtk_tree_view_set_model (priv->word_treeview, GTK_TREE_MODEL (store));
      gtk_tree_view_set_search_column (priv->word_treeview, GW_VOCABULARYWORDSTORE_COLUMN_DEFINITIONS);
    }
    else
    {
      gtk_tree_view_set_model (priv->word_treeview, NULL);
    }

    g_list_foreach (rowlist, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (rowlist); rowlist = NULL;

    gw_vocabularywindow_update_flashcard_menu_sensitivities (window);

    gw_vocabularywindow_show_vocabulary_list (window, TRUE);
}


static void
gw_vocabularywindow_select_new_word_from_dialog_cb (GtkWidget *widget, gpointer data)
{
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GwAddVocabularyWindow *avw;
    GtkTreeIter iter;
    GtkTreeSelection *selection;
    gboolean valid;

    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    avw = GW_ADDVOCABULARYWINDOW (widget);
    selection = gtk_tree_view_get_selection (priv->word_treeview);
    valid = gw_addvocabularywindow_get_iter (GW_ADDVOCABULARYWINDOW (avw), &iter);

    if (valid)
    {
      gtk_tree_selection_unselect_all (selection);
      gtk_tree_selection_select_iter (selection, &iter);
    }
}


G_MODULE_EXPORT void
gw_vocabularywindow_new_word_cb (GtkWidget *widget, gpointer data)
{
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GwApplication *application;
    GtkWindow *avw;
    GtkTreeModel *model;
    GtkTreeSelection *selection;
    GtkTreeIter iter;
    gboolean valid;
    gchar *list;

    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));
    model = gtk_tree_view_get_model (priv->list_treeview);
    if (model == NULL) return;
    selection = gtk_tree_view_get_selection (priv->list_treeview);
    valid = gtk_tree_selection_get_selected (selection, &model, &iter);

    if (valid)
    {
      avw = gw_addvocabularywindow_new (GTK_APPLICATION (application));
      list = gw_vocabularyliststore_get_name_by_iter (GW_VOCABULARYLISTSTORE (model), &iter);
      if (list != NULL)
      {
        gw_addvocabularywindow_set_list (GW_ADDVOCABULARYWINDOW (avw), list);
        gw_addvocabularywindow_set_focus (GW_ADDVOCABULARYWINDOW (avw), GW_ADDVOCABULARYWINDOW_FOCUS_KANJI);
        g_free (list);
      }
      gtk_window_set_transient_for (avw, GTK_WINDOW (window));
      g_signal_connect (G_OBJECT (avw), "word-added", G_CALLBACK (gw_vocabularywindow_select_new_word_from_dialog_cb), window);
      gtk_widget_show (GTK_WIDGET (avw));
    }
}


G_MODULE_EXPORT void
gw_vocabularywindow_remove_word_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GList *rowlist;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    selection = gtk_tree_view_get_selection (priv->word_treeview);
    model = gtk_tree_view_get_model (priv->word_treeview);
    rowlist = gtk_tree_selection_get_selected_rows (selection, &model);

    gw_vocabularywordstore_remove_path_list (GW_VOCABULARYWORDSTORE (model), rowlist);

    g_list_foreach (rowlist, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (rowlist); rowlist = NULL;
}


G_MODULE_EXPORT void
gw_vocabularywindow_list_selection_changed_cb (GtkTreeView *view, gpointer data)
{
    //Declarations
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GwApplication *application;
    GtkTreeModel *model;
    GtkListStore *liststore, *wordstore;
    GtkTreeSelection *selection;
    GtkTreeIter iter;
    gboolean has_changes;
    gchar *title;
    const gchar *name;
    gboolean valid;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    if (window == NULL) return;
    g_return_if_fail (window != NULL);
    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));
    liststore = gw_application_get_vocabularyliststore (application);
    model = GTK_TREE_MODEL (liststore);
    selection = gtk_tree_view_get_selection (priv->list_treeview);

    valid = gtk_tree_selection_get_selected (selection, &model, &iter);

    if (valid)
    {
      wordstore = gw_vocabularyliststore_get_wordstore_by_iter (GW_VOCABULARYLISTSTORE (liststore), &iter);
      gtk_tree_view_set_model (priv->word_treeview, GTK_TREE_MODEL (wordstore));
      gtk_tree_view_set_search_column (priv->word_treeview, GW_VOCABULARYWORDSTORE_COLUMN_DEFINITIONS);

      has_changes = gw_vocabularywindow_current_wordstore_has_changes (window);
      gtk_action_set_sensitive (priv->revert_action, has_changes);

      gw_vocabularywindow_update_flashcard_menu_sensitivities (window);

      name = gw_vocabularywordstore_get_name (GW_VOCABULARYWORDSTORE (wordstore));
      title = g_strdup_printf ("%s - %s", name, gettext("gWaei Vocabulary Manager"));
    }
    else
    {
      title = g_strdup_printf (gettext("gWaei Vocabulary Manager"));
    }

    if (title != NULL)
    {
      gtk_window_set_title (GTK_WINDOW (window), title);
      g_free (title); title = NULL;
    }
}


G_MODULE_EXPORT void
gw_vocabularywindow_cell_edited_cb (GtkCellRendererText *renderer,
                                    gchar               *path_string,
                                    gchar               *new_text,
                                    gpointer             data       )
{
    //Declarations
    GwVocabularyWindow *window;
    GtkTreeView *view;
    GtkTreeModel *model;
    GtkTreeIter iter;
    gint column;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_if_fail (window != NULL);
    view = GTK_TREE_VIEW (data);
    model = gtk_tree_view_get_model (view);
    gtk_tree_model_get_iter_from_string (model, &iter, path_string);
    column = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (renderer), "column"));

    gw_vocabularywordstore_set_string (GW_VOCABULARYWORDSTORE (model), &iter, column, new_text);
}


G_MODULE_EXPORT void
gw_vocabularywindow_list_cell_edited_cb (GtkCellRendererText *renderer,
                                         gchar               *path_string,
                                         gchar               *new_text,
                                         gpointer             data       )
{
    //Declarations
    GwVocabularyWindow *window;
    GtkTreeView *view;
    GtkTreeModel *model;
    GtkTreeIter iter;
    gint column;
    gchar *text;
    gboolean exists;
    gboolean valid;
    GtkListStore *wordstore;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_if_fail (window != NULL);
    view = GTK_TREE_VIEW (data);
    model = GTK_TREE_MODEL (gtk_tree_view_get_model (view));
    column = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (renderer), "column"));
    exists = FALSE;
    valid = gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter);

    while (valid && !exists)
    {
      gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, column, &text, -1);
      if (text != NULL)
      {
        if (strcmp(text, new_text) == 0) exists = TRUE;
        g_free (text);
      }
      valid = gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter);
    } 

    if (!exists)
    {
      gtk_tree_model_get_iter_from_string (model, &iter, path_string);
      gtk_list_store_set (GTK_LIST_STORE (model), &iter, column, new_text, -1);
      wordstore = gw_vocabularyliststore_get_wordstore_by_iter (GW_VOCABULARYLISTSTORE (model), &iter);
      gw_vocabularywordstore_set_name (GW_VOCABULARYWORDSTORE (wordstore), new_text);
    }
}


G_MODULE_EXPORT void
gw_vocabularywindow_save_cb (GtkAction *action, gpointer data)
{
    //Declarations
    GwVocabularyWindow *window;
    GwApplication *application;
    LwPreferences *preferences;
    GtkListStore *liststore;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_if_fail (window != NULL);
    application = gw_window_get_application (GW_WINDOW (window));
    preferences = gw_application_get_preferences (application);
    liststore = gw_application_get_vocabularyliststore (application);

    gw_vocabularyliststore_save_all (GW_VOCABULARYLISTSTORE (liststore));
    gw_vocabularyliststore_save_list_order (GW_VOCABULARYLISTSTORE (liststore), preferences);
}


G_MODULE_EXPORT void
gw_vocabularywindow_reset_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GwApplication *application;
    LwPreferences *preferences;
    GtkListStore *liststore, *wordstore;
    GtkTreeIter iter;
    GtkTreeModel *model;
    GtkTreeSelection *selection;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));
    preferences = gw_application_get_preferences (application);
    liststore = gw_application_get_vocabularyliststore (application);
    model = GTK_TREE_MODEL (liststore);
    selection = gtk_tree_view_get_selection (priv->list_treeview);

    gw_vocabularyliststore_revert_all (GW_VOCABULARYLISTSTORE (liststore));
    gw_vocabularyliststore_load_list_order (GW_VOCABULARYLISTSTORE (liststore), preferences);

    if (!gtk_tree_selection_get_selected (selection, &model, NULL))
    {
      gtk_tree_model_get_iter_first (model, &iter);
      gtk_tree_selection_select_iter (selection, &iter);
      wordstore = gw_vocabularyliststore_get_wordstore_by_iter (GW_VOCABULARYLISTSTORE (liststore), &iter);
      gtk_tree_view_set_model (priv->word_treeview, GTK_TREE_MODEL (wordstore));
      gtk_tree_view_set_search_column (priv->word_treeview, GW_VOCABULARYWORDSTORE_COLUMN_DEFINITIONS);
    }
}


G_MODULE_EXPORT gboolean
gw_vocabularywindow_delete_event_cb (GtkWidget *widget, GdkEvent *event, gpointer data)
{
    gw_vocabularywindow_close_cb (widget, data);

    return TRUE;
}


G_MODULE_EXPORT void
gw_vocabularywindow_close_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwVocabularyWindow *window;
    GwApplication *application;
    gboolean close_window;
    GList *link;
    gint count;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_if_fail (window != NULL);
    application = gw_window_get_application (GW_WINDOW (window));
    link = gtk_application_get_windows (GTK_APPLICATION (application));
    count = 0;

    while (link != NULL)
    {
      if (GW_IS_VOCABULARYWINDOW (link->data) == TRUE) count++;
      link = link->next;
    }

    if (count == 1 && gw_vocabularywindow_has_changes (window))
    {
      close_window = gw_vocabularywindow_show_save_dialog (window);
    }
    else
    {
      close_window = TRUE;
    }
    
    if (close_window == TRUE) gtk_widget_destroy (GTK_WIDGET (window));

    if (gw_application_should_quit (application))
    {
      gw_application_quit (application);
    }
}


G_MODULE_EXPORT void
gw_vocabularywindow_export_cb (GtkAction *action, gpointer data)
{
    //Declarations
    GwVocabularyWindow *window;
    GtkWidget *dialog;
    GtkListStore *store;
    gint response;
    gchar *filename;
    gchar *final_path;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_if_fail (window != NULL);
    store = gw_vocabularywindow_get_selected_wordstore (window);
    if (store == NULL) return;

    dialog = gtk_file_chooser_dialog_new (
        gettext ("Export Vocabulary List..."), NULL, GTK_FILE_CHOOSER_ACTION_SAVE, 
        GTK_STOCK_CANCEL, GTK_RESPONSE_NO,
        "Export",         GTK_RESPONSE_YES,
        NULL);
    //gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), g_get_home_dir ());
    filename = g_strjoin (".", gw_vocabularywordstore_get_name (GW_VOCABULARYWORDSTORE (store)), "txt", NULL);
    if (filename != NULL)
    {
      gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), filename);
      g_free (filename);
    }

    response = gtk_dialog_run (GTK_DIALOG (dialog));

    switch (response)
    {
      case GTK_RESPONSE_YES:
        final_path = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
        if (final_path != NULL)
        {
          gw_vocabularywordstore_save (GW_VOCABULARYWORDSTORE (store), final_path);
          g_free (final_path);
        }
        break;
      case GTK_RESPONSE_NO:
      default:
        break;
    }

    gtk_widget_destroy (GTK_WIDGET (dialog));
}


G_MODULE_EXPORT void
gw_vocabularywindow_import_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwVocabularyWindow *window;
    GtkWidget *dialog;
    gint response;
    gchar *path;
    GtkListStore *store;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_if_fail (window != NULL);
    dialog = gtk_file_chooser_dialog_new (
        "Export as...", NULL, GTK_FILE_CHOOSER_ACTION_OPEN, 
        GTK_STOCK_CANCEL, GTK_RESPONSE_NO,
        "Import", GTK_RESPONSE_YES,
        NULL);
    response = gtk_dialog_run (GTK_DIALOG (dialog));

    switch (response)
    {
      case GTK_RESPONSE_YES:
        path = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
        if (path != NULL && g_file_test (path, G_FILE_TEST_IS_REGULAR))
        {
          gw_vocabularywindow_new_list_cb (GTK_WIDGET (window), window);
          store = gw_vocabularywindow_get_selected_wordstore (window);
          gw_vocabularywordstore_load (GW_VOCABULARYWORDSTORE (store), path);
          g_free (path);
        }
        break;
      case GTK_RESPONSE_NO:
      default:
        break;
    }

    gtk_widget_destroy (GTK_WIDGET (dialog));
}


G_MODULE_EXPORT void
gw_vocabularywindow_delete_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GList *rowlist;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    model = gtk_tree_view_get_model (priv->word_treeview);

    if (model != NULL)
    {
      selection = gtk_tree_view_get_selection (priv->word_treeview);
      rowlist = gtk_tree_selection_get_selected_rows (selection, &model);

      gw_vocabularywordstore_remove_path_list (GW_VOCABULARYWORDSTORE (model), rowlist);

      g_list_foreach (rowlist, (GFunc) gtk_tree_path_free, NULL);
      g_list_free (rowlist); rowlist = NULL;
    }
}


G_MODULE_EXPORT void
gw_vocabularywindow_cut_cb (GtkWidget *widget, gpointer data)
{
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GtkClipboard *clipboard;
    gchar *text;
    GtkTreeSelection *selection;
    GList *rowlist;
    GtkTreeModel *model;
    
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    model = gtk_tree_view_get_model (priv->word_treeview);

    if (model != NULL)
    {
      clipboard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
      selection = gtk_tree_view_get_selection (priv->word_treeview);
      rowlist = gtk_tree_selection_get_selected_rows (selection, &model);
      text = gw_vocabularywordstore_path_list_to_string (GW_VOCABULARYWORDSTORE (model), rowlist);

      gw_vocabularywordstore_remove_path_list (GW_VOCABULARYWORDSTORE (model), rowlist);

      gtk_clipboard_set_text (clipboard, text, -1);

      g_list_foreach (rowlist, (GFunc) gtk_tree_path_free, NULL);
      g_list_free (rowlist); rowlist = NULL;
      g_free (text); text = NULL;
    }
}


G_MODULE_EXPORT void
gw_vocabularywindow_copy_cb (GtkWidget *widget, gpointer data)
{
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GtkClipboard *clipboard;
    gchar *text;
    GtkTreeSelection *selection;
    GList *rowlist;
    GtkTreeModel *model;
    
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    model = gtk_tree_view_get_model (priv->word_treeview);

    if (model != NULL)
    {
      clipboard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
      selection = gtk_tree_view_get_selection (priv->word_treeview);
      rowlist = gtk_tree_selection_get_selected_rows (selection, &model);
      text = gw_vocabularywordstore_path_list_to_string (GW_VOCABULARYWORDSTORE (model), rowlist);

      gtk_clipboard_set_text (clipboard, text, -1);

      g_list_foreach (rowlist, (GFunc) gtk_tree_path_free, NULL);
      g_list_free (rowlist); rowlist = NULL;
      g_free (text); text = NULL;
    }
}


G_MODULE_EXPORT void
gw_vocabularywindow_paste_cb (GtkWidget *widget, gpointer data)
{
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GtkClipboard *clipboard;
    GtkListStore *store;
    GtkTreeSelection *selection;
    gchar *text;

    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    store = GTK_LIST_STORE (gtk_tree_view_get_model (priv->word_treeview));

    if (store != NULL)
    {
      clipboard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
      selection = gtk_tree_view_get_selection (priv->word_treeview);
      text = gtk_clipboard_wait_for_text (clipboard);

      gw_vocabularywordstore_append_text (GW_VOCABULARYWORDSTORE (store), NULL, TRUE, text);
      gtk_tree_selection_unselect_all (selection);

      g_free (text);
    }
}


G_MODULE_EXPORT gboolean
gw_vocabularywindow_event_after_cb (GtkWidget *widget, GdkEvent *event, gpointer data)
{
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GtkWidget *focus;
    gboolean sensitive;
    int i = 0;

    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_val_if_fail (window != NULL, FALSE);
    priv = window->priv;
    GtkMenuItem *menuitem[] = { 
      priv->copy_menuitem, 
      priv->paste_menuitem, 
      priv->cut_menuitem, 
      priv->delete_menuitem, 
      NULL };
    focus = gtk_window_get_focus (GTK_WINDOW (window));
    sensitive = (GTK_WIDGET (priv->word_treeview) == focus);

    for (i = 0; menuitem[i] != NULL; i++)
    {
      gtk_widget_set_sensitive (GTK_WIDGET (menuitem[i]), sensitive);
    }

    return FALSE;
}


G_MODULE_EXPORT void
gw_vocabularywindow_liststore_changed_cb (GwVocabularyListStore *store, gpointer data)
{
    GwVocabularyWindow *window;
    gboolean has_changes;

    window = GW_VOCABULARYWINDOW (data);
    has_changes = gw_vocabularyliststore_has_changes (store);

    gw_vocabularywindow_set_has_changes (window, has_changes);
    gw_vocabularywindow_update_flashcard_menu_sensitivities (window);
}


G_MODULE_EXPORT void
gw_vocabularywindow_revert_wordstore_cb (GtkWidget *widget, gpointer data)
{
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GwVocabularyListStore *liststore;
    GtkListStore *wordstore;
    GtkTreeSelection *selection;
    GtkTreeIter iter;
    GtkTreeModel *model;
    gboolean valid;

    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    liststore = GW_VOCABULARYLISTSTORE (gtk_tree_view_get_model (priv->list_treeview));
    selection = gtk_tree_view_get_selection (priv->list_treeview);
    model = GTK_TREE_MODEL (liststore);
    valid = gtk_tree_selection_get_selected (selection, &model, &iter);

    if (valid)
    {
      wordstore = gw_vocabularyliststore_get_wordstore_by_iter (liststore, &iter);
      gw_vocabularywordstore_reset (GW_VOCABULARYWORDSTORE (wordstore));
      gw_vocabularywordstore_load (GW_VOCABULARYWORDSTORE (wordstore), NULL);
    }
}


G_MODULE_EXPORT void
gw_vocabularywindow_toggle_editing_cb (GtkWidget *widget, gpointer data)
{
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    gboolean state;

    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    state = gtk_toggle_tool_button_get_active (priv->edit_toolbutton);

    g_object_set (G_OBJECT (priv->renderer[GW_VOCABULARYWORDSTORE_COLUMN_KANJI]), "editable", state, NULL);
    g_object_set (G_OBJECT (priv->renderer[GW_VOCABULARYWORDSTORE_COLUMN_FURIGANA]), "editable", state, NULL);
    g_object_set (G_OBJECT (priv->renderer[GW_VOCABULARYWORDSTORE_COLUMN_DEFINITIONS]), "editable", state, NULL);
}


G_MODULE_EXPORT gboolean
gw_vocabularywindow_set_word_tooltip_text_cb (GtkWidget  *widget,
                                              gint        x,
                                              gint        y,
                                              gboolean    keyboard_mode,
                                              GtkTooltip *tooltip,
                                              gpointer    data)
{
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GwVocabularyWordStore *store;
    LwVocabularyItem *item;
    GtkTreeIter iter;
    gchar *text;
    GtkTreePath *path;
    gboolean valid;
    gchar *markup;

    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_val_if_fail (window != NULL, FALSE);
    priv = window->priv;
    store = GW_VOCABULARYWORDSTORE (gtk_tree_view_get_model (priv->word_treeview));
    valid = gtk_tree_view_get_tooltip_context (priv->word_treeview, &x, &y, keyboard_mode, NULL, &path, &iter);
    if (valid) 
    {
      text = gw_vocabularywordstore_iter_to_string (store, &iter);
      item = lw_vocabularyitem_new_from_string (text);
      markup = g_markup_printf_escaped ("<b>%s [%s]</b>\n%s", 
        lw_vocabularyitem_get_kanji (item), 
        lw_vocabularyitem_get_furigana (item), 
        lw_vocabularyitem_get_definitions (item));
      gtk_tree_view_set_tooltip_row (priv->word_treeview, tooltip, path);
      gtk_tooltip_set_markup (tooltip, markup);
      g_free (text);
      g_free (markup);
      gtk_tree_path_free (path);
      lw_vocabularyitem_free (item);
    }

    return valid;
}


G_MODULE_EXPORT void
gw_vocabularywindow_kanji_definition_flashcards_cb (GtkWidget *widget, gpointer data)
{
    GwVocabularyWindow *window;
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_if_fail (window != NULL);

    gw_vocabularywindow_start_flashcards (
      window,
      gettext("Kanji→Definition"),
      gettext("What is the definition of this word?"),
      GW_VOCABULARYWORDSTORE_COLUMN_KANJI,
      GW_VOCABULARYWORDSTORE_COLUMN_DEFINITIONS
    );
}


G_MODULE_EXPORT void
gw_vocabularywindow_definition_kanji_flashcards_cb (GtkWidget *widget, gpointer data)
{
    GwVocabularyWindow *window;
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_if_fail (window != NULL);

    gw_vocabularywindow_start_flashcards (
      window,
      gettext("Definition→Kanji"),
      gettext("What is the Japanese word for this definition?"),
      GW_VOCABULARYWORDSTORE_COLUMN_DEFINITIONS,
      GW_VOCABULARYWORDSTORE_COLUMN_KANJI
    );
}


G_MODULE_EXPORT void
gw_vocabularywindow_furigana_definition_flashcards_cb (GtkWidget *widget, gpointer data)
{
    GwVocabularyWindow *window;
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_if_fail (window != NULL);

    gw_vocabularywindow_start_flashcards (
      window,
      gettext("Furigana→Definition"),
      gettext("What is the definition of this word?"),
      GW_VOCABULARYWORDSTORE_COLUMN_FURIGANA,
      GW_VOCABULARYWORDSTORE_COLUMN_DEFINITIONS
    );
}


G_MODULE_EXPORT void
gw_vocabularywindow_definition_furigana_flashcards_cb (GtkWidget *widget, gpointer data)
{
    GwVocabularyWindow *window;
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_if_fail (window != NULL);

    gw_vocabularywindow_start_flashcards (
      window,
      gettext("Definition→Furigana"),
      gettext("What is the Furigana for this definition?"),
      GW_VOCABULARYWORDSTORE_COLUMN_DEFINITIONS,
      GW_VOCABULARYWORDSTORE_COLUMN_FURIGANA
    );
}


G_MODULE_EXPORT void
gw_vocabularywindow_kanji_furigana_flashcards_cb (GtkWidget *widget, gpointer data)
{
    GwVocabularyWindow *window;
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_if_fail (window != NULL);

    gw_vocabularywindow_start_flashcards (
      window,
      gettext("Kanji→Furigana"),
      gettext("What is the Furigana for this Kanji?"),
      GW_VOCABULARYWORDSTORE_COLUMN_KANJI,
      GW_VOCABULARYWORDSTORE_COLUMN_FURIGANA
    );
}


G_MODULE_EXPORT void
gw_vocabularywindow_furigana_kanji_flashcards_cb (GtkWidget *widget, gpointer data)
{
    GwVocabularyWindow *window;
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_if_fail (window != NULL);

    gw_vocabularywindow_start_flashcards (
      window,
      gettext("Furigana→Kanji"),
      gettext("What is the Kanji for this Furigana?"),
      GW_VOCABULARYWORDSTORE_COLUMN_FURIGANA,
      GW_VOCABULARYWORDSTORE_COLUMN_KANJI
    );
}


G_MODULE_EXPORT void
gw_vocabularywindow_shuffle_toggled_cb (GtkAction *action, gpointer data)
{
    //Declarations
    GwApplication *application;
    GwVocabularyWindow *window;
    LwPreferences *preferences;
    gboolean request;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_if_fail (window != NULL);
    application = gw_window_get_application (GW_WINDOW (window));
    preferences = gw_application_get_preferences (application);
    request = lw_preferences_get_boolean_by_schema (preferences, LW_SCHEMA_VOCABULARY, LW_KEY_SHUFFLE_FLASHCARDS);

    lw_preferences_set_boolean_by_schema (preferences, LW_SCHEMA_VOCABULARY, LW_KEY_SHUFFLE_FLASHCARDS, !request);
}


G_MODULE_EXPORT void
gw_vocabularywindow_sync_shuffle_flashcards_cb (GSettings *settings, gchar *key, gpointer data)
{
    //Declarations
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GtkWidget *toplevel;
    GtkToggleAction *action;
    gboolean request;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    toplevel = gw_window_get_toplevel (GW_WINDOW (window));
    action = priv->shuffle_toggleaction;
    request = lw_preferences_get_boolean (settings, key);
    priv->shuffle = request;

    G_GNUC_EXTENSION g_signal_handlers_block_by_func (action, gw_vocabularywindow_shuffle_toggled_cb, toplevel);
    gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action), request);
    G_GNUC_EXTENSION g_signal_handlers_unblock_by_func (action, gw_vocabularywindow_shuffle_toggled_cb, toplevel);
}


G_MODULE_EXPORT void
gw_vocabularywindow_trim_toggled_cb (GtkAction *action, gpointer data)
{
    //Declarations
    GwApplication *application;
    GwVocabularyWindow *window;
    LwPreferences *preferences;
    gboolean request;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_if_fail (window != NULL);
    application = gw_window_get_application (GW_WINDOW (window));
    preferences = gw_application_get_preferences (application);
    request = lw_preferences_get_boolean_by_schema (preferences, LW_SCHEMA_VOCABULARY, LW_KEY_TRIM_FLASHCARDS);

    lw_preferences_set_boolean_by_schema (preferences, LW_SCHEMA_VOCABULARY, LW_KEY_TRIM_FLASHCARDS, !request);
}


G_MODULE_EXPORT void
gw_vocabularywindow_sync_trim_flashcards_cb (GSettings *settings, gchar *key, gpointer data)
{
    //Declarations
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GtkWidget *toplevel;
    GtkToggleAction *action;
    gboolean request;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    toplevel = gw_window_get_toplevel (GW_WINDOW (window));
    action = priv->trim_toggleaction;
    request = lw_preferences_get_boolean (settings, key);
    priv->trim = request;

    G_GNUC_EXTENSION g_signal_handlers_block_by_func (action, gw_vocabularywindow_trim_toggled_cb, toplevel);
    gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action), request);
    G_GNUC_EXTENSION g_signal_handlers_unblock_by_func (action, gw_vocabularywindow_trim_toggled_cb, toplevel);
}



G_MODULE_EXPORT void
gw_vocabularywindow_track_results_toggled_cb (GtkAction *action, gpointer data)
{
    //Declarations
    GwApplication *application;
    GwVocabularyWindow *window;
    LwPreferences *preferences;
    gboolean request;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_if_fail (window != NULL);
    application = gw_window_get_application (GW_WINDOW (window));
    preferences = gw_application_get_preferences (application);
    request = lw_preferences_get_boolean_by_schema (preferences, LW_SCHEMA_VOCABULARY, LW_KEY_TRACK_RESULTS);

    lw_preferences_set_boolean_by_schema (preferences, LW_SCHEMA_VOCABULARY, LW_KEY_TRACK_RESULTS, !request);
}


G_MODULE_EXPORT void
gw_vocabularywindow_sync_track_results_cb (GSettings *settings, gchar *key, gpointer data)
{
    //Declarations
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GtkWidget *toplevel;
    GtkToggleAction *action;
    gboolean request;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    toplevel = gw_window_get_toplevel (GW_WINDOW (window));
    action = priv->track_results_toggleaction;
    request = lw_preferences_get_boolean (settings, key);
    priv->track = request;

    G_GNUC_EXTENSION g_signal_handlers_block_by_func (action, gw_vocabularywindow_track_results_toggled_cb, toplevel);
    gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action), request);
    G_GNUC_EXTENSION g_signal_handlers_unblock_by_func (action, gw_vocabularywindow_track_results_toggled_cb, toplevel);
}


G_MODULE_EXPORT void
gw_vocabularywindow_sync_list_order_cb (GSettings *settings, gchar *key, gpointer data)
{
    GwVocabularyWindow *window;
    GwApplication *application;
    LwPreferences *preferences;
    GtkListStore *store;

    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_if_fail (window != NULL);
    application = gw_window_get_application (GW_WINDOW (window));
    preferences = gw_application_get_preferences (application);
    store = gw_application_get_vocabularyliststore (application);

    gw_vocabularyliststore_load_list_order (GW_VOCABULARYLISTSTORE (store), preferences);
}


G_MODULE_EXPORT void
gw_vocabularywindow_drag_begin_cb (
  GtkWidget      *widget,
  GdkDragContext *context,
  gpointer        data)
{
  cairo_surface_t *surface;
  GtkTreeView *view;
  GtkTreeModel *model;
  GtkTreeSelection *selection;
  GtkTreePath *path;
  GList *selectedlist;

  view = GTK_TREE_VIEW (widget);
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (widget));
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (widget));
  selectedlist = gtk_tree_selection_get_selected_rows (selection, &model);
  path = (GtkTreePath*) selectedlist->data;
  surface = gtk_tree_view_create_row_drag_icon (view, path);

  gtk_drag_set_icon_surface (context, surface);

  cairo_surface_destroy (surface);
  g_list_foreach (selectedlist, (GFunc) gtk_tree_path_free, NULL);
  g_list_free (selectedlist);
}


G_MODULE_EXPORT void
gw_vocabularywindow_list_drag_motion_cb (
  GtkWidget *widget,
  GdkDragContext *context,
  gint x,
  gint y,
  guint time,
  gpointer data)
{
    //Declarations
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GtkTreeView *view;
    GtkTreeModel *model;
    GtkTreeViewDropPosition drop_position;
    GtkTreeSelection *selection;
    GtkTreePath *path;
    GtkTreeIter iter, previous_iter;
    GtkTreeView *source;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (widget), GW_TYPE_VOCABULARYWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    view = GTK_TREE_VIEW (widget);
    selection = gtk_tree_view_get_selection (view);
    model = gtk_tree_view_get_model (view);
    source = GTK_TREE_VIEW (gtk_drag_get_source_widget (context));

    if (source == priv->list_treeview)
    {
      gtk_tree_view_get_dest_row_at_pos (view, x, y, &path, &drop_position);

      if (path != NULL)
      {
        if (drop_position == GTK_TREE_VIEW_DROP_INTO_OR_BEFORE) 
          drop_position = GTK_TREE_VIEW_DROP_BEFORE;
        else if (drop_position == GTK_TREE_VIEW_DROP_INTO_OR_AFTER) 
          drop_position = GTK_TREE_VIEW_DROP_AFTER;

        gtk_tree_view_set_drag_dest_row (view, path, drop_position);
      }
    }
    else if (source == priv->word_treeview)
    {
      gtk_tree_view_get_path_at_pos (view, x, y, &path, NULL, NULL, NULL);
      if (path == NULL)
      {
        if (gtk_tree_model_get_iter_first (model, &iter)) {
          previous_iter = iter;
          while (gtk_tree_model_iter_next (model, &iter)) previous_iter = iter;
          iter = previous_iter;
        }
      }
      else
      {
        gtk_tree_path_prev (path);
        gtk_tree_model_get_iter (model, &iter, path);
      }
        gtk_tree_selection_select_iter (selection, &iter);
    }

    if (path != NULL) gtk_tree_path_free (path); path = NULL;
}


static gboolean
gw_vocabularywindow_list_drag_reorder (
  GtkWidget *widget,
  GdkDragContext *context,
  gint x,
  gint y,
  guint time,
  gpointer user_data)
{
    //Declarations
    GwVocabularyWindow *window;
    GwApplication *application;
    LwPreferences *preferences;
    GtkTreeViewDropPosition drop_position;
    GtkTreePath *path;
    GtkTreeView *view;
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GtkTreeIter iter, position;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (widget), GW_TYPE_VOCABULARYWINDOW));
    g_return_val_if_fail (window != NULL, FALSE);
    application = gw_window_get_application (GW_WINDOW (window));
    preferences = gw_application_get_preferences (application);
    view = GTK_TREE_VIEW (widget);
    selection = gtk_tree_view_get_selection (view);
    model = gtk_tree_view_get_model (view);

    gtk_tree_view_get_dest_row_at_pos (view, x, y, &path, &drop_position);
    if (path == NULL) return FALSE;
    gtk_tree_model_get_iter (model, &position, path);
    gtk_tree_path_free (path); path = NULL;

    if (drop_position == GTK_TREE_VIEW_DROP_INTO_OR_BEFORE) 
      drop_position = GTK_TREE_VIEW_DROP_BEFORE;
    else if (drop_position == GTK_TREE_VIEW_DROP_INTO_OR_AFTER) 
      drop_position = GTK_TREE_VIEW_DROP_AFTER;

    gtk_tree_selection_get_selected (selection, &model, &iter);

    if (drop_position == GTK_TREE_VIEW_DROP_BEFORE) 
      gtk_list_store_move_before (GTK_LIST_STORE (model), &iter, &position);
    else if (drop_position == GTK_TREE_VIEW_DROP_AFTER) 
      gtk_list_store_move_after (GTK_LIST_STORE (model), &iter, &position);

    gw_vocabularyliststore_save_list_order (GW_VOCABULARYLISTSTORE (model), preferences);

    return TRUE;
}


static gboolean
gw_vocabularywindow_list_drag_drop (
  GtkWidget *widget,
  GdkDragContext *context,
  gint x,
  gint y,
  guint time,
  gpointer user_data)
{
    //Declarations
    GtkTreePath *path;
    GtkTreeIter iter;
    GtkTreeView *list_treeview, *word_treeview;
    GtkListStore *liststore, *source_wordstore, *target_wordstore, *temp_wordstore;
    gchar *text;
    GtkTreeSelection *selection;
    GList *rowlist;
    GtkTreeModel *model;
    gboolean valid;

    //Initializations
    list_treeview = GTK_TREE_VIEW (widget);
    gtk_tree_view_get_path_at_pos (list_treeview, x, y, &path, NULL, NULL, NULL);
    if (path != NULL) gtk_tree_path_prev (path);
    word_treeview = GTK_TREE_VIEW (gtk_drag_get_source_widget (context));
    liststore = GTK_LIST_STORE (gtk_tree_view_get_model (list_treeview));
    source_wordstore = GTK_LIST_STORE (gtk_tree_view_get_model (word_treeview));

    //Get the data from the source wordstore
    selection = gtk_tree_view_get_selection (word_treeview);
    model = GTK_TREE_MODEL (source_wordstore);
    rowlist = gtk_tree_selection_get_selected_rows (selection, &model);
    text = gw_vocabularywordstore_path_list_to_string (GW_VOCABULARYWORDSTORE (source_wordstore), rowlist);
    gw_vocabularywordstore_remove_path_list (GW_VOCABULARYWORDSTORE (source_wordstore), rowlist);

    //Get the target wordstore
    selection = gtk_tree_view_get_selection (list_treeview);
    model = GTK_TREE_MODEL (liststore);
    gtk_tree_selection_get_selected (selection, &model, &iter);
    target_wordstore = gw_vocabularyliststore_get_wordstore_by_iter (GW_VOCABULARYLISTSTORE (liststore), &iter);
    
    //Append the text to the target wordstore
    gw_vocabularywordstore_append_text (GW_VOCABULARYWORDSTORE (target_wordstore), NULL, FALSE, text);
    selection = gtk_tree_view_get_selection (list_treeview);
    model = GTK_TREE_MODEL (liststore);

    valid = gtk_tree_model_get_iter_first (model, &iter);
    temp_wordstore = NULL;
    while (valid)
    {
      gtk_tree_model_get (model, &iter, GW_VOCABULARYLISTSTORE_COLUMN_OBJECT, &temp_wordstore, -1);
      if (temp_wordstore == source_wordstore)
        gtk_tree_selection_select_iter (selection, &iter);
      g_object_unref (temp_wordstore); temp_wordstore = NULL;

      valid = gtk_tree_model_iter_next (model, &iter);
    }

    g_free (text); text = NULL;
    g_list_foreach (rowlist, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (rowlist); rowlist = NULL;

    return TRUE;
}


G_MODULE_EXPORT gboolean
gw_vocabularywindow_list_drag_drop_cb (
  GtkWidget *widget,
  GdkDragContext *context,
  gint x,
  gint y,
  guint time,
  gpointer user_data)
{
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GtkTreeView *source;
    gboolean success;

    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (widget), GW_TYPE_VOCABULARYWINDOW));
    g_return_val_if_fail (window != NULL, FALSE);
    priv = window->priv;
    source = GTK_TREE_VIEW (gtk_drag_get_source_widget (context));
    success = FALSE;

    if (source == priv->list_treeview)
      gw_vocabularywindow_list_drag_reorder (widget, context, x, y, time, user_data);
    else if (source == priv->word_treeview)
      success = gw_vocabularywindow_list_drag_drop (widget, context, y, y, time, user_data);

    if (success) gw_vocabularywindow_set_has_changes (window, TRUE);
    gtk_drag_finish (context, success, FALSE, time);

    return success;
}



static gboolean
gw_vocabularywindow_word_drag_reorder (
  GtkWidget *widget,
  GdkDragContext *context,
  gint x,
  gint y,
  guint time,
  gpointer user_data)
{
    //Declarations
    GtkTreeViewDropPosition drop_position;
    GtkTreePath *path;
    GtkTreeView *view;
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GtkTreeIter position;
    GList *rowlist;
    gchar *text;
    gboolean before;

    //Initializations
    view = GTK_TREE_VIEW (widget);
    selection = gtk_tree_view_get_selection (view);
    model = gtk_tree_view_get_model (view);
    before = FALSE;

    gtk_tree_view_get_dest_row_at_pos (view, x, y, &path, &drop_position);
    if (path == NULL) return FALSE;
    gtk_tree_model_get_iter (model, &position, path);
    gtk_tree_path_free (path); path = NULL;

    if (drop_position == GTK_TREE_VIEW_DROP_INTO_OR_BEFORE || drop_position == GTK_TREE_VIEW_DROP_BEFORE)
      before = TRUE;
    else if (drop_position == GTK_TREE_VIEW_DROP_INTO_OR_AFTER || drop_position == GTK_TREE_VIEW_DROP_AFTER)
      before = FALSE;

    rowlist = gtk_tree_selection_get_selected_rows (selection, &model);
    text = gw_vocabularywordstore_path_list_to_string (GW_VOCABULARYWORDSTORE (model), rowlist);
    gw_vocabularywordstore_append_text (GW_VOCABULARYWORDSTORE (model), &position, before, text);
    g_free (text); text = NULL;
    g_list_foreach (rowlist, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (rowlist); rowlist = NULL;

    rowlist = gtk_tree_selection_get_selected_rows (selection, &model);
    gw_vocabularywordstore_remove_path_list (GW_VOCABULARYWORDSTORE (model), rowlist);

    g_list_foreach (rowlist, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (rowlist); rowlist = NULL;

    return TRUE;
}


G_MODULE_EXPORT gboolean
gw_vocabularywindow_word_drag_drop_cb (
  GtkWidget *widget,
  GdkDragContext *context,
  gint x,
  gint y,
  guint time,
  gpointer user_data)
{
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GtkTreeView *source;
    gboolean success;

    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (widget), GW_TYPE_VOCABULARYWINDOW));
    g_return_val_if_fail (window != NULL, FALSE);
    priv = window->priv;
    source = GTK_TREE_VIEW (gtk_drag_get_source_widget (context));
    success = FALSE;

    if (source == priv->word_treeview)
      success = gw_vocabularywindow_word_drag_reorder (widget, context, x, y, time, user_data);

    if (success) gw_vocabularywindow_set_has_changes (window, TRUE);
    gtk_drag_finish (context, success, FALSE, time);

    return success;
}


G_MODULE_EXPORT void
gw_vocabularywindow_word_drag_motion_cb (
  GtkWidget *widget,
  GdkDragContext *context,
  gint x,
  gint y,
  guint time,
  gpointer data)
{
    //Declarations
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GtkTreeView *view;
    GtkTreeViewDropPosition drop_position;
    GtkTreePath *path;
    GtkTreeView *source;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (widget), GW_TYPE_VOCABULARYWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    view = GTK_TREE_VIEW (widget);
    source = GTK_TREE_VIEW (gtk_drag_get_source_widget (context));

    if (source == priv->word_treeview)
    {
      gtk_tree_view_get_dest_row_at_pos (view, x, y, &path, &drop_position);

      if (path != NULL)
      {
        if (drop_position == GTK_TREE_VIEW_DROP_INTO_OR_BEFORE) 
          drop_position = GTK_TREE_VIEW_DROP_BEFORE;
        else if (drop_position == GTK_TREE_VIEW_DROP_INTO_OR_AFTER) 
          drop_position = GTK_TREE_VIEW_DROP_AFTER;

        gtk_tree_view_set_drag_dest_row (view, path, drop_position);
      }
    }

    if (path != NULL) gtk_tree_path_free (path); path = NULL;
}


G_MODULE_EXPORT void
gw_vocabularywindow_sync_toolbar_show_cb (GSettings *settings, gchar *key, gpointer data)
{
    //Declarations
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GtkWidget *toplevel;
    GtkToggleAction *action;
    gboolean request;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    toplevel = gw_window_get_toplevel (GW_WINDOW (window));
    request = lw_preferences_get_boolean (settings, key);
    if (request == TRUE)
      gtk_widget_show (GTK_WIDGET (priv->study_toolbar));
    else
      gtk_widget_hide (GTK_WIDGET (priv->study_toolbar));
    action = priv->show_toolbar_toggleaction;

    G_GNUC_EXTENSION g_signal_handlers_block_by_func (action, gw_vocabularywindow_toolbar_toggled_cb, toplevel);
    gtk_toggle_action_set_active (action, request);
    G_GNUC_EXTENSION g_signal_handlers_unblock_by_func (action, gw_vocabularywindow_toolbar_toggled_cb, toplevel);

}


G_MODULE_EXPORT void
gw_vocabularywindow_sync_position_column_show_cb (GSettings *settings, gchar *key, gpointer data)
{
    //Declarations
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GtkWidget *toplevel;
    GtkToggleAction *action;
    gboolean request;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    toplevel = gw_window_get_toplevel (GW_WINDOW (window));
    request = lw_preferences_get_boolean (settings, key);
    action = priv->show_position_column_toggleaction;

    gtk_tree_view_column_set_visible (priv->position_column, request);

    G_GNUC_EXTENSION g_signal_handlers_block_by_func (action, gw_vocabularywindow_position_column_toggled_cb, toplevel);
    gtk_toggle_action_set_active (action, request);
    G_GNUC_EXTENSION g_signal_handlers_unblock_by_func (action, gw_vocabularywindow_position_column_toggled_cb, toplevel);
}


G_MODULE_EXPORT void
gw_vocabularywindow_sync_score_column_show_cb (GSettings *settings, gchar *key, gpointer data)
{
    //Declarations
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GtkWidget *toplevel;
    GtkToggleAction *action;
    gboolean request;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    toplevel = gw_window_get_toplevel (GW_WINDOW (window));
    request = lw_preferences_get_boolean (settings, key);
    action = priv->show_score_column_toggleaction;

    gtk_tree_view_column_set_visible (priv->score_column, request);

    G_GNUC_EXTENSION g_signal_handlers_block_by_func (action, gw_vocabularywindow_score_column_toggled_cb, toplevel);
    gtk_toggle_action_set_active (action, request);
    G_GNUC_EXTENSION g_signal_handlers_unblock_by_func (action, gw_vocabularywindow_score_column_toggled_cb, toplevel);
}


G_MODULE_EXPORT void
gw_vocabularywindow_sync_timestamp_column_show_cb (GSettings *settings, gchar *key, gpointer data)
{
    //Declarations
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GtkWidget *toplevel;
    GtkToggleAction *action;
    gboolean request;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    toplevel = gw_window_get_toplevel (GW_WINDOW (window));
    request = lw_preferences_get_boolean (settings, key);
    action = priv->show_timestamp_column_toggleaction;

    gtk_tree_view_column_set_visible (priv->timestamp_column, request);

    G_GNUC_EXTENSION g_signal_handlers_block_by_func (action, gw_vocabularywindow_timestamp_column_toggled_cb, toplevel);
    gtk_toggle_action_set_active (action, request);
    G_GNUC_EXTENSION g_signal_handlers_unblock_by_func (action, gw_vocabularywindow_timestamp_column_toggled_cb, toplevel);
}


G_MODULE_EXPORT void
gw_vocabularywindow_toolbar_toggled_cb (GtkAction *action, gpointer data)
{
    //Declarations
    GwVocabularyWindow *window;
    GwApplication *application;
    LwPreferences *preferences;
    gboolean state;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_if_fail (window != NULL);
    application = gw_window_get_application (GW_WINDOW (window));
    preferences = gw_application_get_preferences (application);

    state = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));
    lw_preferences_set_boolean_by_schema (preferences, LW_SCHEMA_VOCABULARY, LW_KEY_TOOLBAR_SHOW, state);
}


G_MODULE_EXPORT void
gw_vocabularywindow_position_column_toggled_cb (GtkAction *action, gpointer data)
{
    //Declarations
    GwVocabularyWindow *window;
    GwApplication *application;
    LwPreferences *preferences;
    gboolean state;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_if_fail (window != NULL);
    application = gw_window_get_application (GW_WINDOW (window));
    preferences = gw_application_get_preferences (application);

    state = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));
    lw_preferences_set_boolean_by_schema (preferences, LW_SCHEMA_VOCABULARY, LW_KEY_POSITION_COLUMN_SHOW, state);
}


G_MODULE_EXPORT void
gw_vocabularywindow_score_column_toggled_cb (GtkAction *action, gpointer data)
{
    //Declarations
    GwVocabularyWindow *window;
    GwApplication *application;
    LwPreferences *preferences;
    gboolean state;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_if_fail (window != NULL);
    application = gw_window_get_application (GW_WINDOW (window));
    preferences = gw_application_get_preferences (application);

    state = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));
    lw_preferences_set_boolean_by_schema (preferences, LW_SCHEMA_VOCABULARY, LW_KEY_SCORE_COLUMN_SHOW, state);
}


G_MODULE_EXPORT void
gw_vocabularywindow_timestamp_column_toggled_cb (GtkAction *action, gpointer data)
{
    //Declarations
    GwVocabularyWindow *window;
    GwApplication *application;
    LwPreferences *preferences;
    gboolean state;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_if_fail (window != NULL);
    application = gw_window_get_application (GW_WINDOW (window));
    preferences = gw_application_get_preferences (application);

    state = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));
    lw_preferences_set_boolean_by_schema (preferences, LW_SCHEMA_VOCABULARY, LW_KEY_TIMESTAMP_COLUMN_SHOW, state);
}
