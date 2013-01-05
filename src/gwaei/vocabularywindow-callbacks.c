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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <gwaei/gettext.h>
#include <gwaei/gwaei.h>
#include <gwaei/vocabularyliststore.h>
#include <gwaei/vocabularywindow-private.h>


void
gw_vocabularywindow_new_list_cb (GSimpleAction *action, 
                                 GVariant      *parameter, 
                                 gpointer       data)
{
    //Declarations
    GwVocabularyWindow *window;

    //Initializations
    window = GW_VOCABULARYWINDOW (data);

    gw_vocabularywindow_new_list (window);
}


static GtkWidget*
gw_vocabularywindow_get_remove_list_confirm_dialog (GwVocabularyWindow *window, GList *list)
{
    GwVocabularyWindowPrivate *priv;
    GtkWidget *dialog;
    const gchar *TITLE;

    GtkTreePath *treepath;
    GtkTreeIter treeiter;
    GList *link;
    gchar *listname;
    GtkTreeModel *treemodel;
    


    priv = window->priv;
    treemodel = gtk_tree_view_get_model (priv->list_treeview);
    TITLE = ngettext ("Perminently remove this list?", "Perminently remove these lists?", g_list_length (list));
    dialog = gtk_dialog_new ();
    
    gtk_dialog_add_buttons (GTK_DIALOG (dialog), 
                            GTK_STOCK_CANCEL,
                            GTK_RESPONSE_REJECT,
                            GTK_STOCK_REMOVE,
                            GTK_RESPONSE_ACCEPT,
                            NULL);

    {
      GtkWidget *content_area, *box, *image, *label;
      GString *text;

      content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
      box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 16);
      image = gtk_image_new_from_stock (GTK_STOCK_DIALOG_WARNING, GTK_ICON_SIZE_DIALOG);
      label = gtk_label_new (NULL);
      text = g_string_new ("");

      g_string_append_printf (text, "<big><b>%s</b></big>\n\n", TITLE);
      for (link = list; link != NULL; link = link->next)
      {
        treepath = (GtkTreePath*) link->data;
        gtk_tree_model_get_iter (treemodel, &treeiter, treepath);
        gtk_tree_model_get (treemodel, &treeiter, GW_VOCABULARYLISTSTORE_COLUMN_NAME, &listname, -1);
        if (listname != NULL)
        {
          g_string_append_printf (text, "%s\n", listname);
          g_free (listname); listname = NULL;
        }
      }
      gtk_label_set_markup (GTK_LABEL (label), text->str);

      gtk_misc_set_padding (GTK_MISC (label), 0, 8);
      gtk_box_pack_start (GTK_BOX (box), image, FALSE, FALSE, 0);
      gtk_box_pack_start (GTK_BOX (box), label, TRUE, TRUE, 0);
      gtk_widget_show_all (box);

      gtk_container_add (GTK_CONTAINER (content_area), box);
      gtk_container_set_border_width (GTK_CONTAINER (box), 8);
      gtk_container_set_border_width (GTK_CONTAINER (dialog), 6);

      g_string_free (text, TRUE); text = NULL;
    }

    return dialog;
}


G_MODULE_EXPORT void
gw_vocabularywindow_remove_list_cb (GSimpleAction *action, 
                                    GVariant      *parameter,
                                    gpointer       data)
{
    //Declarations
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GList *rowlist;
    GtkTreePath *path;
    gint response;

    //Initializations
    window = GW_VOCABULARYWINDOW (data);
    g_return_if_fail (window != NULL);

    priv = window->priv;
    selection = gtk_tree_view_get_selection (priv->list_treeview);
    model = gtk_tree_view_get_model (priv->list_treeview);
    rowlist = gtk_tree_selection_get_selected_rows (selection, &model);
    if (rowlist == NULL) return;

    {
      GtkWidget *dialog;
      dialog = gw_vocabularywindow_get_remove_list_confirm_dialog (window, rowlist);
      gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (window));
      response = gtk_dialog_run (GTK_DIALOG (dialog));
      gtk_widget_destroy (GTK_WIDGET (dialog));
      
    }

    if (response == GTK_RESPONSE_ACCEPT) 
    {
      gw_vocabularyliststore_remove_path_list (GW_VOCABULARYLISTSTORE (model), rowlist);
    }

    path = (GtkTreePath*) rowlist->data;
    gtk_tree_path_prev (path);

    gtk_tree_view_set_cursor (priv->list_treeview, path, NULL, FALSE);

    g_list_foreach (rowlist, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (rowlist); rowlist = NULL;

    gw_vocabularywindow_update_flashcard_menu_sensitivities (window);

    gw_vocabularywindow_show_vocabulary_list (window, TRUE);
}


G_MODULE_EXPORT void
gw_vocabularywindow_select_new_word_from_dialog_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GwAddVocabularyWindow *avw;
    GtkTreeIter iter;
    GtkTreeSelection *selection;
    gboolean valid;

    //Initializations
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


void
gw_vocabularywindow_new_word_cb (GSimpleAction *action,
                                 GVariant      *parameter,
                                 gpointer       data)
{
    GwVocabularyWindow *window;

    window = GW_VOCABULARYWINDOW (data);

    gw_vocabularywindow_new_word (window);
    gw_vocabularywindow_sync_has_changes (window);
}


static GtkWidget*
gw_vocabularywindow_get_remove_word_confirm_dialog (GwVocabularyWindow *window, GList *list)
{
    //Declarations
    GtkWidget *dialog;
    const gchar *TITLE;

    //Initializations
    TITLE = ngettext ("Remove this word?", "Remove all selected words?", g_list_length (list));
    dialog = gtk_dialog_new ();
    
    gtk_dialog_add_buttons (GTK_DIALOG (dialog), 
                            GTK_STOCK_CANCEL,
                            GTK_RESPONSE_REJECT,
                            GTK_STOCK_REMOVE,
                            GTK_RESPONSE_ACCEPT,
                            NULL);

    {
      GtkWidget *content_area, *box, *image, *label;
      GString *text;

      content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
      box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 16);
      image = gtk_image_new_from_stock (GTK_STOCK_DIALOG_WARNING, GTK_ICON_SIZE_DIALOG);
      label = gtk_label_new (NULL);
      text = g_string_new ("");

      g_string_append_printf (text, "<big><b>%s</b></big>\n\n", TITLE);
      gtk_label_set_markup (GTK_LABEL (label), text->str);

      gtk_misc_set_padding (GTK_MISC (label), 0, 8);
      gtk_box_pack_start (GTK_BOX (box), image, FALSE, FALSE, 0);
      gtk_box_pack_start (GTK_BOX (box), label, TRUE, TRUE, 0);
      gtk_widget_show_all (box);

      gtk_container_add (GTK_CONTAINER (content_area), box);
      gtk_container_set_border_width (GTK_CONTAINER (box), 8);
      gtk_container_set_border_width (GTK_CONTAINER (dialog), 6);

      g_string_free (text, TRUE); text = NULL;
    }

    return dialog;
}


G_MODULE_EXPORT void
gw_vocabularywindow_remove_word_cb (GSimpleAction *action, 
                                    GVariant      *parameter,
                                    gpointer       data)
{
    //Declarations
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GList *rowlist;
    gint response;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    selection = gtk_tree_view_get_selection (priv->word_treeview);
    model = gtk_tree_view_get_model (priv->word_treeview);
    rowlist = gtk_tree_selection_get_selected_rows (selection, &model);

    {
      GtkWidget *dialog;
      dialog = gw_vocabularywindow_get_remove_word_confirm_dialog (window, rowlist);
      gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (window));
      response = gtk_dialog_run (GTK_DIALOG (dialog));
      gtk_widget_destroy (GTK_WIDGET (dialog));
      
    }

    if (response == GTK_RESPONSE_ACCEPT) 
    {
      gw_vocabularywordstore_remove_path_list (GW_VOCABULARYWORDSTORE (model), rowlist);
    }

    g_list_foreach (rowlist, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (rowlist); rowlist = NULL;
}


G_MODULE_EXPORT void
gw_vocabularywindow_list_selection_changed_cb (GtkTreeView *view, gpointer data)
{
    //Sanity checks
    g_return_if_fail (view != NULL);

    //Declarations
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GwApplication *application;
    GtkTreeModel *model;
    GtkListStore *liststore, *wordstore;
    GtkTreeSelection *selection;
    GtkTreeIter iter;
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

      name = gw_vocabularywordstore_get_name (GW_VOCABULARYWORDSTORE (wordstore));
      title = g_strdup_printf ("%s - %s", name, gettext("gWaei Vocabulary"));
    }
    else
    {
      title = g_strdup_printf (gettext("gWaei Vocabulary"));
    }

    if (title != NULL)
    {
      gtk_window_set_title (GTK_WINDOW (window), title);
      g_free (title); title = NULL;
    }

    gw_vocabularywindow_update_flashcard_menu_sensitivities (window);
    gw_vocabularywindow_sync_has_changes (window);
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


void
gw_vocabularywindow_save_cb (GSimpleAction *action, 
                             GVariant      *parameter,
                             gpointer       data)
{
    //Declarations
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GwApplication *application;
    LwPreferences *preferences;
    GtkTreeView *treeview;
    GtkTreeSelection *treeselection;
    GtkListStore *liststore;
    GtkTreePath *treepath;
    GtkTreeIter treeiter;
    GtkTreeModel *treemodel;
    gboolean valid;
    GList *rowlink, *rowlist;

    //Initializations
    window = GW_VOCABULARYWINDOW (data);
    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));
    preferences = gw_application_get_preferences (application);
    treeview = priv->list_treeview;
    liststore = gw_application_get_vocabularyliststore (application);
    treemodel = GTK_TREE_MODEL (liststore);

    treeselection = gtk_tree_view_get_selection (treeview);
    rowlink = rowlist = gtk_tree_selection_get_selected_rows (treeselection, &treemodel);

    while (rowlink != NULL)
    {
      treepath = (GtkTreePath*) rowlink->data;
      if (treepath != NULL)
      {
        valid = gtk_tree_model_get_iter (treemodel, &treeiter, treepath);
        if (valid)
        {
          gw_vocabularyliststore_save (GW_VOCABULARYLISTSTORE (liststore), &treeiter);
        }
      }
      rowlink = rowlink->next;
    }
    gw_vocabularyliststore_save_list_order (GW_VOCABULARYLISTSTORE (liststore), preferences);
}


void
gw_vocabularywindow_revert_cb (GSimpleAction *action,
                               GVariant      *variant,
                               gpointer       data)
{
    //Declarations
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GwApplication *application;
    GtkTreeView *treeview;
    GtkTreeSelection *treeselection;
    GtkListStore *liststore;
    GtkTreePath *treepath;
    GtkTreeIter treeiter;
    GtkTreeModel *treemodel;
    gboolean valid;
    GList *rowlink, *rowlist;

    //Initializations
    window = GW_VOCABULARYWINDOW (data);
    priv = window->priv;
    treeview = priv->list_treeview;
    application = gw_window_get_application (GW_WINDOW (window));
    liststore = gw_application_get_vocabularyliststore (application);
    treemodel = GTK_TREE_MODEL (liststore);

    treeselection = gtk_tree_view_get_selection (treeview);
    rowlink = rowlist = gtk_tree_selection_get_selected_rows (treeselection, &treemodel);

    while (rowlink != NULL)
    {
      treepath = (GtkTreePath*) rowlink->data;
      if (treepath != NULL)
      {
        valid = gtk_tree_model_get_iter (treemodel, &treeiter, treepath);
        if (valid)
        {
          gw_vocabularyliststore_revert (GW_VOCABULARYLISTSTORE (liststore), &treeiter);
        }
      }
      rowlink = rowlink->next;
    }
}


void
gw_vocabularywindow_close_cb (GSimpleAction *action, 
                              GVariant      *parameter, 
                              gpointer       data)
{
    //Declarations
    GwVocabularyWindow *window;
    GwApplication *application;
    gboolean close_window;
    GList *link;
    gint count;

    //Initializations
    window = GW_VOCABULARYWINDOW (data);
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


void
gw_vocabularywindow_export_cb (GSimpleAction *action, 
                               GVariant      *parameter,
                               gpointer       data)
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


void
gw_vocabularywindow_import_cb (GSimpleAction *action, 
                               GVariant      *parameter,
                               gpointer       data)
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
          gw_vocabularywindow_new_list_cb (NULL, NULL, window);
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


void
gw_vocabularywindow_delete_cb (GSimpleAction *action, 
                               GVariant      *parameter,
                               gpointer       data)
{
    //Declarations
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GtkWidget *focus;

    //Initializations
    window = GW_VOCABULARYWINDOW (data);
    priv = window->priv;
    focus = gtk_window_get_focus (GTK_WINDOW (window));

    if (focus == GTK_WIDGET (priv->word_treeview))
    {
      gw_vocabularywindow_remove_word_cb (action, parameter, data);
    }
    else if (focus == GTK_WIDGET (priv->list_treeview))
    {
      gw_vocabularywindow_remove_list_cb (action, parameter, data);
    }
}


void
gw_vocabularywindow_cut_cb (GSimpleAction *action, 
                            GVariant      *parameter,
                            gpointer       data)
{
    //Declarations
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GtkClipboard *clipboard;
    gchar *text;
    GtkTreeSelection *selection;
    GList *rowlist;
    GtkTreeModel *model;
    
    //Initializations
    window = GW_VOCABULARYWINDOW (data);
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


void
gw_vocabularywindow_copy_cb (GSimpleAction *action, 
                             GVariant      *variant,
                             gpointer       data)
{
    //Declarations
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GtkClipboard *clipboard;
    gchar *text;
    GtkTreeSelection *selection;
    GList *rowlist;
    GtkTreeModel *model;
    
    //Initializations
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


void
gw_vocabularywindow_paste_cb (GSimpleAction *action, 
                              GVariant      *parameter,
                              gpointer       data)
{
    //Declarations
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GtkClipboard *clipboard;
    GtkListStore *store;
    GtkTreeSelection *selection;
    gchar *text;

    //Initializations
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
    GActionMap *map;
    GSimpleAction *action;

    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_val_if_fail (window != NULL, FALSE);
    priv = window->priv;
    map = G_ACTION_MAP (window);

    focus = gtk_window_get_focus (GTK_WINDOW (window));
    sensitive = (GTK_WIDGET (priv->word_treeview) == focus);

    action = G_SIMPLE_ACTION (g_action_map_lookup_action (map, "copy"));
    g_simple_action_set_enabled (action, sensitive);

    action = G_SIMPLE_ACTION (g_action_map_lookup_action (map, "paste"));
    g_simple_action_set_enabled (action, sensitive);

    action = G_SIMPLE_ACTION (g_action_map_lookup_action (map, "cut"));
    g_simple_action_set_enabled (action, sensitive);

    action = G_SIMPLE_ACTION (g_action_map_lookup_action (map, "delete"));
    g_simple_action_set_enabled (action, sensitive);
    
    return FALSE;
}


G_MODULE_EXPORT void
gw_vocabularywindow_liststore_changed_cb (GwVocabularyListStore *store, gpointer data)
{
    //Declarations
    GwVocabularyWindow *window;

    //Initializations
    window = GW_VOCABULARYWINDOW (data);

    gw_vocabularywindow_sync_has_changes (window);
    gw_vocabularywindow_update_flashcard_menu_sensitivities (window);
}


G_MODULE_EXPORT void
gw_vocabularywindow_revert_wordstore_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GwVocabularyListStore *liststore;
    GtkListStore *wordstore;
    GtkTreeSelection *selection;
    GtkTreeIter iter;
    GtkTreeModel *model;
    gboolean valid;
  
    //Initialiations
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
gw_vocabularywindow_editable_toggled_cb (GSimpleAction *action, 
                                         GVariant      *parameter,
                                         gpointer       data)
{
    //Declarations
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GVariant *state;
    gboolean editable;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    state = g_action_get_state (G_ACTION (action));
    editable = g_variant_get_boolean (state);
    g_simple_action_set_state (G_SIMPLE_ACTION (action), g_variant_new_boolean (!editable));

    g_object_set (G_OBJECT (priv->renderer[GW_VOCABULARYWORDSTORE_COLUMN_KANJI]), "editable", !editable, NULL);
    g_object_set (G_OBJECT (priv->renderer[GW_VOCABULARYWORDSTORE_COLUMN_FURIGANA]), "editable", !editable, NULL);
    g_object_set (G_OBJECT (priv->renderer[GW_VOCABULARYWORDSTORE_COLUMN_DEFINITIONS]), "editable", !editable, NULL);
}


G_MODULE_EXPORT gboolean
gw_vocabularywindow_set_word_tooltip_text_cb (GtkWidget  *widget,
                                              gint        x,
                                              gint        y,
                                              gboolean    keyboard_mode,
                                              GtkTooltip *tooltip,
                                              gpointer    data)
{
    //Declarations
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GwVocabularyWordStore *store;
    LwWord *word;
    GtkTreeIter iter;
    gchar *text;
    GtkTreePath *path;
    gboolean valid;
    gchar *markup;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_val_if_fail (window != NULL, FALSE);
    priv = window->priv;
    store = GW_VOCABULARYWORDSTORE (gtk_tree_view_get_model (priv->word_treeview));
    valid = gtk_tree_view_get_tooltip_context (priv->word_treeview, &x, &y, keyboard_mode, NULL, &path, &iter);

    if (valid) 
    {
      text = gw_vocabularywordstore_iter_to_string (store, &iter);
      word = lw_word_new_from_string (text);
      markup = g_markup_printf_escaped ("<b>%s [%s]</b>\n%s", 
        lw_word_get_kanji (word), 
        lw_word_get_furigana (word), 
        lw_word_get_definitions (word));
      gtk_tree_view_set_tooltip_row (priv->word_treeview, tooltip, path);
      gtk_tooltip_set_markup (tooltip, markup);
      g_free (text);
      g_free (markup);
      gtk_tree_path_free (path);
      lw_word_free (word);
    }

    return valid;
}


void
gw_vocabularywindow_kanji_definition_flashcards_cb (GSimpleAction *action, 
                                                    GVariant      *parameter,
                                                    gpointer       data)
{
    GwVocabularyWindow *window;
    window = GW_VOCABULARYWINDOW (data);

    gw_vocabularywindow_start_flashcards (
      window,
      gettext("Kanji→Definition"),
      gettext("What is the definition of this word?"),
      GW_VOCABULARYWORDSTORE_COLUMN_KANJI,
      GW_VOCABULARYWORDSTORE_COLUMN_DEFINITIONS
    );
}


void
gw_vocabularywindow_definition_kanji_flashcards_cb (GSimpleAction *action, 
                                                    GVariant      *parameter,
                                                    gpointer       data)
{
    GwVocabularyWindow *window;
    window = GW_VOCABULARYWINDOW (data);

    gw_vocabularywindow_start_flashcards (
      window,
      gettext("Definition→Kanji"),
      gettext("What is the Japanese word for this definition?"),
      GW_VOCABULARYWORDSTORE_COLUMN_DEFINITIONS,
      GW_VOCABULARYWORDSTORE_COLUMN_KANJI
    );
}


void
gw_vocabularywindow_furigana_definition_flashcards_cb (GSimpleAction *action, 
                                                       GVariant      *parameter,
                                                       gpointer       data)
{
    GwVocabularyWindow *window;

    window = GW_VOCABULARYWINDOW (data);

    gw_vocabularywindow_start_flashcards (
      window,
      gettext("Furigana→Definition"),
      gettext("What is the definition of this word?"),
      GW_VOCABULARYWORDSTORE_COLUMN_FURIGANA,
      GW_VOCABULARYWORDSTORE_COLUMN_DEFINITIONS
    );
}


void
gw_vocabularywindow_definition_furigana_flashcards_cb (GSimpleAction *action,
                                                       GVariant      *parameter,
                                                       gpointer data)
{
    GwVocabularyWindow *window;

    window = GW_VOCABULARYWINDOW (data);

    gw_vocabularywindow_start_flashcards (
      window,
      gettext("Definition→Furigana"),
      gettext("What is the Furigana for this definition?"),
      GW_VOCABULARYWORDSTORE_COLUMN_DEFINITIONS,
      GW_VOCABULARYWORDSTORE_COLUMN_FURIGANA
    );
}


void
gw_vocabularywindow_kanji_furigana_flashcards_cb (GSimpleAction *action, 
                                                  GVariant      *parameter,
                                                  gpointer       data)
{
    GwVocabularyWindow *window;

    window = GW_VOCABULARYWINDOW (data);

    g_return_if_fail (window != NULL);

    gw_vocabularywindow_start_flashcards (
      window,
      gettext("Kanji→Furigana"),
      gettext("What is the Furigana for this Kanji?"),
      GW_VOCABULARYWORDSTORE_COLUMN_KANJI,
      GW_VOCABULARYWORDSTORE_COLUMN_FURIGANA
    );
}


void
gw_vocabularywindow_furigana_kanji_flashcards_cb (GSimpleAction *action, 
                                                  GVariant      *parameter,
                                                  gpointer       data)
{
    GwVocabularyWindow *window;

    window = GW_VOCABULARYWINDOW (data);

    gw_vocabularywindow_start_flashcards (
      window,
      gettext("Furigana→Kanji"),
      gettext("What is the Kanji for this Furigana?"),
      GW_VOCABULARYWORDSTORE_COLUMN_FURIGANA,
      GW_VOCABULARYWORDSTORE_COLUMN_KANJI
    );
}


void
gw_vocabularywindow_shuffle_toggled_cb (GSimpleAction *action, 
                                        GVariant      *parameter,
                                        gpointer       data)
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


void
gw_vocabularywindow_sync_shuffle_flashcards_cb (GSettings *settings, 
                                                gchar     *key, 
                                                gpointer   data)
{
    //Declarations
    GwVocabularyWindow *window;
    gboolean show;
    GAction *action;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_if_fail (window != NULL);
    show = (lw_preferences_get_boolean (settings, key));
    action = g_action_map_lookup_action (G_ACTION_MAP (window), "toggle-shuffle");

    g_simple_action_set_state (G_SIMPLE_ACTION (action), g_variant_new_boolean (show));
}


void
gw_vocabularywindow_trim_toggled_cb (GSimpleAction *action, 
                                     GVariant      *parameter,
                                     gpointer       data)
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


void
gw_vocabularywindow_sync_trim_flashcards_cb (GSettings *settings, 
                                             gchar     *key, 
                                             gpointer   data)
{
    //Declarations
    GwVocabularyWindow *window;
    gboolean show;
    GAction *action;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_if_fail (window != NULL);
    show = (lw_preferences_get_boolean (settings, key));
    action = g_action_map_lookup_action (G_ACTION_MAP (window), "toggle-trim");

    g_simple_action_set_state (G_SIMPLE_ACTION (action), g_variant_new_boolean (show));
}



void
gw_vocabularywindow_track_results_toggled_cb (GSimpleAction *action, 
                                              GVariant      *parameter,
                                              gpointer       data)
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


void
gw_vocabularywindow_sync_track_results_cb (GSettings *settings, 
                                           gchar     *key, 
                                           gpointer   data)
{
    //Declarations
    GwVocabularyWindow *window;
    gboolean show;
    GAction *action;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_if_fail (window != NULL);
    show = (lw_preferences_get_boolean (settings, key));
    action = g_action_map_lookup_action (G_ACTION_MAP (window), "toggle-track-results");

    g_simple_action_set_state (G_SIMPLE_ACTION (action), g_variant_new_boolean (show));
}


void
gw_vocabularywindow_sync_list_order_cb (GSettings *settings, 
                                        gchar     *key, 
                                        gpointer   data)
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
gw_vocabularywindow_drag_begin_cb (GtkWidget      *widget,
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
gw_vocabularywindow_list_drag_motion_cb (GtkWidget      *widget,
                                         GdkDragContext *context,
                                         gint            x,
                                         gint            y,
                                         guint           time,
                                         gpointer        data)
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

    if (success) gw_vocabularywindow_sync_has_changes (window);
    gtk_drag_finish (context, success, FALSE, time);

    return success;
}



static gboolean
gw_vocabularywindow_word_drag_reorder (GtkWidget      *widget,
                                       GdkDragContext *context,
                                       gint            x,
                                       gint            y,
                                       guint           time,
                                       gpointer        user_data)
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

    if (success) gw_vocabularywindow_sync_has_changes (window);
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


void
gw_vocabularywindow_sync_toolbar_show_cb (GSettings *settings, 
                                          gchar     *key, 
                                          gpointer   data)
{
    //Declarations
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    gboolean show;
    GtkToolbar *toolbar;
    GAction *action;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    show = lw_preferences_get_boolean (settings, key);
    toolbar = GTK_TOOLBAR (priv->primary_toolbar);
    action = g_action_map_lookup_action (G_ACTION_MAP (window), "toggle-toolbar-show");

    if (show == TRUE)
      gtk_widget_show (GTK_WIDGET (toolbar));
    else
      gtk_widget_hide (GTK_WIDGET (toolbar));

    g_simple_action_set_state (G_SIMPLE_ACTION (action), g_variant_new_boolean (show));
}


void
gw_vocabularywindow_sync_position_column_show_cb (GSettings *settings, 
                                                  gchar     *key, 
                                                  gpointer   data)
{
    //Declarations
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GAction *action;
    gboolean show;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    show = lw_preferences_get_boolean (settings, key);
    action = g_action_map_lookup_action (G_ACTION_MAP (window), "toggle-position-column-show");

    gtk_tree_view_column_set_visible (priv->position_column, show);
    g_simple_action_set_state (G_SIMPLE_ACTION (action), g_variant_new_boolean (show));
}


void
gw_vocabularywindow_sync_score_column_show_cb (GSettings *settings, 
                                               gchar     *key, 
                                               gpointer   data)
{
    //Declarations
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GAction *action;
    gboolean show;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    show = lw_preferences_get_boolean (settings, key);
    action = g_action_map_lookup_action (G_ACTION_MAP (window), "toggle-score-column-show");

    gtk_tree_view_column_set_visible (priv->score_column, show);
    g_simple_action_set_state (G_SIMPLE_ACTION (action), g_variant_new_boolean (show));
}


void
gw_vocabularywindow_sync_timestamp_column_show_cb (GSettings *settings, 
                                                   gchar     *key, 
                                                   gpointer   data)
{
    //Declarations
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GAction *action;
    gboolean show;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_if_fail (window != NULL);
    priv = window->priv;
    show = lw_preferences_get_boolean (settings, key);
    action = g_action_map_lookup_action (G_ACTION_MAP (window), "toggle-timestamp-column-show");

    gtk_tree_view_column_set_visible (priv->timestamp_column, show);
    g_simple_action_set_state (G_SIMPLE_ACTION (action), g_variant_new_boolean (show));
}


void
gw_vocabularywindow_toolbar_show_toggled_cb (GSimpleAction *action,
                                             GVariant      *parameter,
                                             gpointer       data)
{
    //Declarations
    GwApplication *application;
    GwVocabularyWindow *window;
    LwPreferences *preferences;
    gboolean show;

    //Initializations
    window = GW_VOCABULARYWINDOW (data);
    application = gw_window_get_application (GW_WINDOW (window));
    preferences = gw_application_get_preferences (application);
    show = lw_preferences_get_boolean_by_schema (preferences, LW_SCHEMA_VOCABULARY, LW_KEY_TOOLBAR_SHOW);

    lw_preferences_set_boolean_by_schema (preferences, LW_SCHEMA_VOCABULARY, LW_KEY_TOOLBAR_SHOW, !show);
}


void
gw_vocabularywindow_position_column_toggled_cb (GSimpleAction *action, 
                                                GVariant      *parameter,
                                                gpointer       data)
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
    state = lw_preferences_get_boolean_by_schema (preferences, LW_SCHEMA_VOCABULARY, LW_KEY_POSITION_COLUMN_SHOW);

    lw_preferences_set_boolean_by_schema (preferences, LW_SCHEMA_VOCABULARY, LW_KEY_POSITION_COLUMN_SHOW, !state);
}


void
gw_vocabularywindow_score_column_toggled_cb (GSimpleAction *action, 
                                             GVariant      *parameter,
                                             gpointer       data)
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
    state = lw_preferences_get_boolean_by_schema (preferences, LW_SCHEMA_VOCABULARY, LW_KEY_SCORE_COLUMN_SHOW);

    lw_preferences_set_boolean_by_schema (preferences, LW_SCHEMA_VOCABULARY, LW_KEY_SCORE_COLUMN_SHOW, !state);
}


void
gw_vocabularywindow_timestamp_column_toggled_cb (GSimpleAction *action, 
                                                 GVariant      *parameter,
                                                 gpointer       data)
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
    state = lw_preferences_get_boolean_by_schema (preferences, LW_SCHEMA_VOCABULARY, LW_KEY_TIMESTAMP_COLUMN_SHOW);

    lw_preferences_set_boolean_by_schema (preferences, LW_SCHEMA_VOCABULARY, LW_KEY_TIMESTAMP_COLUMN_SHOW, !state);
}


//!
//! @brief Syncs the gui to the preference settinging.  It should be attached to the gsettings object
//!
void 
gw_vocabularywindow_sync_menubar_show_cb (GSettings *settings, 
                                          gchar     *key, 
                                          gpointer   data)
{
    //Declarations
    GwVocabularyWindow *window;
    gboolean show;
    GAction *action;
    GtkSettings *gtk_settings;
    gboolean shell_shows_menubar;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    g_return_if_fail (window != NULL);
    gtk_settings = gtk_settings_get_default ();
    g_object_get (G_OBJECT (gtk_settings), "gtk-shell-shows-menubar", &shell_shows_menubar, NULL);
    show = (lw_preferences_get_boolean (settings, key));
    action = g_action_map_lookup_action (G_ACTION_MAP (window), "toggle-menubar-show");

    gw_window_show_menubar (GW_WINDOW (window), show && !shell_shows_menubar);
    gtk_window_set_hide_titlebar_when_maximized (GTK_WINDOW (window), !show || shell_shows_menubar);
    g_simple_action_set_state (G_SIMPLE_ACTION (action), g_variant_new_boolean (show));
}


//!
//! @brief Sets the show menu boolean to match the widget
//! @see gw_searchwindow_set_menu_show ()
//! @param widget Unused GtkWidget pointer.
//! @param data Unused gpointer
//!
void 
gw_vocabularywindow_menubar_show_toggled_cb (GSimpleAction *action, 
                                             GVariant      *parameter, 
                                             gpointer       data)
{
    //Declarations
    GwApplication *application;
    GwVocabularyWindow *window;
    LwPreferences *preferences;
    gboolean show;

    //Initializations
    window = GW_VOCABULARYWINDOW (data);
    application = gw_window_get_application (GW_WINDOW (window));
    preferences = gw_application_get_preferences (application);
    show = lw_preferences_get_boolean_by_schema (preferences, LW_SCHEMA_VOCABULARY, LW_KEY_MENUBAR_SHOW);

    lw_preferences_set_boolean_by_schema (preferences, LW_SCHEMA_VOCABULARY, LW_KEY_MENUBAR_SHOW, !show);
}

